// SPDX-License-Identifier: MIT
// PowerFinger Ring Firmware — Entry Point
//
// Architecture: BLE events are posted to a FreeRTOS queue from the NimBLE
// callback task. The main loop drains the queue and dispatches all events
// through the state machine on a single task, eliminating cross-task races
// on the state machine and ensuring action flags are always consumed.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hal_timer.h"
#include "hal_ble.h"
#include "ring_settings.h"
#include "ble_hid_mouse.h"
#include "ble_config.h"
#include "ring_state.h"
#include "ring_config.h"
#include "sensor_interface.h"
#include "click_interface.h"
#include "dead_zone.h"
#include "calibration.h"
#include "ring_diagnostics.h"
#include "power_manager.h"
#include "ring_runtime_health.h"

static const char *TAG = "powerfinger";

// --- Event queue: BLE callback -> main loop ---

#define EVT_QUEUE_LEN 8

typedef enum {
    APP_EVT_RING_STATE = 0,
    APP_EVT_BOND_RESTORED,
    APP_EVT_BOND_FAILED,
    APP_EVT_CONN_PARAMS_UPDATED,
    APP_EVT_CONN_PARAMS_REJECTED,
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    ring_event_t ring_evt;
    uint16_t conn_interval_1_25ms;
} app_event_t;

static QueueHandle_t s_evt_queue = NULL;

// --- Clamp int16_t to int8_t range (prevents direction reversal on overflow) ---
static inline int8_t clamp_i8(int16_t v)
{
    if (v > 127) return 127;
    if (v < -127) return -127;
    return (int8_t)v;
}

static void sync_sensor_diagnostics(ring_diagnostics_t *diagnostics,
                                    bool sensor_ok,
                                    bool sensor_calibrated)
{
    ring_diagnostics_note_sensor_path(diagnostics, sensor_ok, sensor_calibrated);
}

static void sync_battery_diagnostics(ring_diagnostics_t *diagnostics)
{
    ring_diagnostics_note_battery(diagnostics,
                                  power_manager_get_last_battery_mv(),
                                  power_manager_get_battery_level());
}

static void log_diagnostics_snapshot(const char *reason,
                                     const ring_diagnostics_t *diagnostics)
{
    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(diagnostics);
    uint32_t interval_centims = (uint32_t)snapshot.conn_interval_1_25ms * 125U;
    uint32_t interval_ms_whole = interval_centims / 100U;
    uint32_t interval_ms_frac = interval_centims % 100U;

    if (snapshot.conn_interval_1_25ms == 0) {
        ESP_LOGI(TAG,
                 "diag[%s]: state=%s sensor=%s cal=%s bond=%s conn=unknown rejected=%s batt=%u%% (%lumV)",
                 reason,
                 ring_state_name(snapshot.ring_state),
                 ring_diag_sensor_state_name(snapshot.sensor_state),
                 snapshot.calibration_valid ? "valid" : "pending",
                 ring_diag_bond_state_name(snapshot.bond_state),
                 snapshot.conn_param_rejected ? "yes" : "no",
                 snapshot.battery_pct,
                 (unsigned long)snapshot.battery_mv);
        return;
    }

    ESP_LOGI(TAG,
             "diag[%s]: state=%s sensor=%s cal=%s bond=%s conn=%lu.%02lums rejected=%s batt=%u%% (%lumV)",
             reason,
             ring_state_name(snapshot.ring_state),
             ring_diag_sensor_state_name(snapshot.sensor_state),
             snapshot.calibration_valid ? "valid" : "pending",
             ring_diag_bond_state_name(snapshot.bond_state),
             (unsigned long)interval_ms_whole,
             (unsigned long)interval_ms_frac,
             snapshot.conn_param_rejected ? "yes" : "no",
             snapshot.battery_pct,
             (unsigned long)snapshot.battery_mv);
}

static hal_status_t attempt_sensor_recovery(bool had_working_sensor)
{
    hal_status_t rc = had_working_sensor ? sensor_wake() : sensor_init();
    if (rc == HAL_OK || had_working_sensor == false) {
        return rc;
    }

    // If the sensor stopped responding after a previously healthy session,
    // fall back to a full re-init before giving up.
    return sensor_init();
}

static bool try_late_calibration(bool sensor_ok,
                                 bool *sensor_calibrated,
                                 uint32_t *next_calibration_attempt_ms,
                                 uint32_t now_ms)
{
    if (!sensor_ok || !sensor_calibrated || !next_calibration_attempt_ms) {
        return false;
    }
    if (*sensor_calibrated || hal_ble_is_connected() ||
        now_ms < *next_calibration_attempt_ms) {
        return false;
    }

    hal_status_t cal_rc = calibration_attempt_once();
    if (cal_rc == HAL_OK) {
        *sensor_calibrated = calibration_is_valid();
        *next_calibration_attempt_ms = 0;
        ESP_LOGI(TAG, "late calibration succeeded — motion input ready");
        return *sensor_calibrated;
    }

    calibration_reset();
    *next_calibration_attempt_ms = now_ms + CALIBRATION_RETRY_DELAY_MS;
    ESP_LOGW(TAG, "late calibration failed — retrying in %d ms while disconnected",
             CALIBRATION_RETRY_DELAY_MS);
    return false;
}

static void handle_hid_send_result(ring_runtime_health_t *runtime_health,
                                   hal_status_t send_rc,
                                   uint32_t now_ms)
{
    ring_hid_health_update_t update =
        ring_runtime_health_note_hid_send_result(runtime_health, send_rc, now_ms);

    if (update.event == RING_HID_HEALTH_FAULT_STARTED) {
        ESP_LOGW(TAG, "BLE HID send error: %d — monitoring for recovery", update.status);
    } else if (update.event == RING_HID_HEALTH_RECOVERED) {
        ESP_LOGI(TAG, "BLE HID send path recovered");
    } else if (update.event == RING_HID_HEALTH_RESTART_REQUIRED) {
        ESP_LOGE(TAG, "BLE HID send failed for %lu ms (last=%d) — restarting",
                 (unsigned long)update.fault_elapsed_ms, update.status);
        esp_restart();
    }
}

// --- BLE event callback (posts to queue, never dispatches directly) ---

static void ble_event_callback(const hal_ble_event_data_t *evt, void *arg)
{
    (void)arg;
    app_event_t app_evt = {0};

    switch (evt->type) {
    case HAL_BLE_EVT_CONNECTED:
        app_evt.type = APP_EVT_RING_STATE;
        app_evt.ring_evt = RING_EVT_BLE_CONNECTED;
        app_evt.conn_interval_1_25ms = evt->data.conn_params.conn_interval_1_25ms;
        break;
    case HAL_BLE_EVT_DISCONNECTED:
        app_evt.type = APP_EVT_RING_STATE;
        app_evt.ring_evt = RING_EVT_BLE_DISCONNECTED;
        break;
    case HAL_BLE_EVT_BOND_RESTORED:
        app_evt.type = APP_EVT_BOND_RESTORED;
        break;
    case HAL_BLE_EVT_BOND_FAILED:
        app_evt.type = APP_EVT_BOND_FAILED;
        break;
    case HAL_BLE_EVT_CONN_PARAMS_UPDATED:
        app_evt.type = APP_EVT_CONN_PARAMS_UPDATED;
        app_evt.conn_interval_1_25ms = evt->data.conn_params.conn_interval_1_25ms;
        break;
    case HAL_BLE_EVT_CONN_PARAMS_REJECTED:
        app_evt.type = APP_EVT_CONN_PARAMS_REJECTED;
        break;
    default:
        return;
    }

    // Post to queue (non-blocking; NimBLE callbacks are task context, not ISR).
    // If the queue is full the event is dropped. Under normal operation the
    // main loop drains faster than events arrive, so this should not fire.
    if (xQueueSend(s_evt_queue, &app_evt, 0) != pdTRUE) {
        ESP_LOGW(TAG, "BLE event queue full — event type %d dropped", (int)app_evt.type);
    }
}

// --- Execute state machine action flags ---

static void execute_actions(const ring_actions_t *a)
{
    if (a->stop_advertising) {
        hal_ble_stop_advertising();
    }
    if (a->start_advertising) {
        if (hal_ble_start_advertising(BLE_ADVERTISE_TIMEOUT_MS) != HAL_OK) {
            ESP_LOGE(TAG, "advertising start failed — entering deep sleep");
            power_manager_enter_sleep(true);
            return;
        }
    }
    if (a->enter_deep_sleep) {
        power_manager_enter_sleep(true);
        // Does not return for deep sleep
    }
}

// --- Phase 0: fake circular motion for BLE HID validation ---

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)

#define FAKE_MOTION_PERIOD_MS   15
#define FAKE_MOTION_RADIUS      3

static void phase0_fake_motion_loop(void)
{
    ESP_LOGI(TAG, "Phase 0: fake motion mode (no sensor, no click)");

    uint32_t tick = 0;
    uint32_t adv_start_ms = hal_timer_get_ms();
    ring_runtime_health_t runtime_health;
    ring_runtime_health_init(&runtime_health);
    ring_diagnostics_t diagnostics;
    ring_diagnostics_init(&diagnostics);
    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
    sync_battery_diagnostics(&diagnostics);

    while (1) {
        hal_timer_delay_ms(FAKE_MOTION_PERIOD_MS);

        // Drain BLE event queue (connection/disconnection)
        app_event_t queued_evt;
        ring_actions_t actions;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            if (queued_evt.type == APP_EVT_RING_STATE) {
                ring_state_dispatch(queued_evt.ring_evt, &actions);
                ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
                execute_actions(&actions);
                if (queued_evt.ring_evt == RING_EVT_BLE_CONNECTED) {
                    adv_start_ms = 0;  // connected, stop tracking adv timeout
                    power_manager_on_connect();
                    ring_runtime_health_reset_hid_send(&runtime_health);
                    ring_diagnostics_note_connected(&diagnostics,
                                                    queued_evt.conn_interval_1_25ms);
                    log_diagnostics_snapshot("connect", &diagnostics);
                }
                if (queued_evt.ring_evt == RING_EVT_BLE_DISCONNECTED) {
                    adv_start_ms = hal_timer_get_ms();
                    power_manager_on_disconnect();
                    ring_runtime_health_reset_hid_send(&runtime_health);
                    ring_diagnostics_note_disconnected(&diagnostics);
                    log_diagnostics_snapshot("disconnect", &diagnostics);
                }
            } else if (queued_evt.type == APP_EVT_BOND_RESTORED) {
                ring_diagnostics_note_bond_restored(&diagnostics);
                log_diagnostics_snapshot("bond-restored", &diagnostics);
            } else if (queued_evt.type == APP_EVT_BOND_FAILED) {
                ring_diagnostics_note_bond_failed(&diagnostics);
                log_diagnostics_snapshot("bond-failed", &diagnostics);
            } else if (queued_evt.type == APP_EVT_CONN_PARAMS_UPDATED) {
                ring_diagnostics_note_conn_params_updated(&diagnostics,
                                                          queued_evt.conn_interval_1_25ms);
                log_diagnostics_snapshot("conn-update", &diagnostics);
            } else if (queued_evt.type == APP_EVT_CONN_PARAMS_REJECTED) {
                ring_diagnostics_note_conn_param_rejected(&diagnostics);
                log_diagnostics_snapshot("conn-rejected", &diagnostics);
            }
        }

        // Check advertising timeout
        uint32_t now = hal_timer_get_ms();
        if (adv_start_ms != 0 &&
            ring_state_get() == RING_STATE_ADVERTISING &&
            (now - adv_start_ms) >= RECONNECT_TIMEOUT_MS) {
            ring_state_dispatch(RING_EVT_BLE_ADV_TIMEOUT, &actions);
            ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
            execute_actions(&actions);
            log_diagnostics_snapshot("adv-timeout", &diagnostics);
        }

        // Feed watchdog even in Phase 0
        power_manager_feed_watchdog();

        if (!hal_ble_is_connected()) {
            tick = 0;
            continue;
        }

        float angle = fmodf((float)tick * 0.05f, 2.0f * 3.14159265f);
        int8_t dx = (int8_t)(FAKE_MOTION_RADIUS * cosf(angle));
        int8_t dy = (int8_t)(FAKE_MOTION_RADIUS * sinf(angle));

        uint8_t buttons = ((tick % 200) < 10) ? 0x01 : 0x00;

        handle_hid_send_result(&runtime_health,
                               ble_hid_mouse_send(buttons, dx, dy, 0),
                               now);
        tick++;
    }
}

#endif // CONFIG_SENSOR_NONE && CONFIG_CLICK_NONE

// --- Main application entry point ---

void app_main(void)
{
    ESP_LOGI(TAG, "PowerFinger ring firmware starting");

    // Initialize NVS (required for BLE bonding and settings persistence)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition erased, re-initializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create BLE event queue (must exist before BLE init)
    s_evt_queue = xQueueCreate(EVT_QUEUE_LEN, sizeof(app_event_t));
    if (!s_evt_queue) {
        ESP_LOGE(TAG, "BLE event queue alloc failed — restarting");
        esp_restart();
    }

    // Initialize state machine
    ring_state_init();

    ring_diagnostics_t diagnostics;
    ring_diagnostics_init(&diagnostics);
    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());

    ring_runtime_health_t runtime_health;
    ring_runtime_health_init(&runtime_health);
    calibration_reset();

    hal_status_t settings_rc = ring_settings_init();
    if (settings_rc != HAL_OK && settings_rc != HAL_ERR_INVALID_ARG) {
        ESP_LOGW(TAG, "ring settings init failed (%d) — using runtime defaults only",
                 settings_rc);
    } else if (settings_rc == HAL_ERR_INVALID_ARG) {
        ESP_LOGW(TAG, "ring settings blob invalid — reverted to defaults and will repair on next idle flush");
    }

    // Initialize sensor (check for failure)
    bool sensor_ok = (sensor_init() == HAL_OK);
    bool sensor_calibrated = false;
    uint32_t next_calibration_attempt_ms = 0;
    if (!sensor_ok) {
        ESP_LOGE(TAG, "sensor init failed — motion input disabled, click path still available");
        ring_runtime_health_mark_sensor_unavailable(&runtime_health, hal_timer_get_ms());
    }
    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);

    // Initialize click (check for failure)
    bool click_ok = (click_init() == HAL_OK);
    if (!click_ok) {
        ESP_LOGW(TAG, "click init failed — buttons disabled");
    }
    dead_zone_init();

    // Run calibration (blocks until complete)
    ring_actions_t actions;
    hal_status_t cal_ret = HAL_ERR_TIMEOUT;
    if (sensor_ok) {
        cal_ret = calibration_run();
        sensor_calibrated = calibration_is_valid();
        if (!sensor_calibrated) {
            next_calibration_attempt_ms = hal_timer_get_ms() + CALIBRATION_RETRY_DELAY_MS;
        }
    } else {
        ESP_LOGW(TAG, "skipping calibration until sensor recovers");
    }
    if (cal_ret == HAL_OK) {
        ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &actions);
    } else {
        ring_state_dispatch(RING_EVT_CALIBRATION_FAILED, &actions);
    }
    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
    execute_actions(&actions);

    // Initialize BLE HID (state machine already set ADVERTISING + action flag)
    if (ble_hid_mouse_init(ble_event_callback, NULL) != HAL_OK) {
        ESP_LOGE(TAG, "BLE init failed — entering deep sleep");
        power_manager_enter_sleep(true);
        return;  // unreachable after deep sleep
    }

    // Initialize power management — battery monitoring is safety-critical
    // for LiPo protection. Without it, the cell can be damaged by deep discharge.
    if (power_manager_init() != HAL_OK) {
        ESP_LOGE(TAG, "power manager init failed — entering deep sleep (LiPo safety)");
        power_manager_enter_sleep(true);
        return;
    }
    hal_ble_set_battery_level(power_manager_get_battery_level());
    sync_battery_diagnostics(&diagnostics);

    // Confirm this firmware is valid — cancels automatic rollback.
    // Placed after all critical init (BLE + power manager) and before the
    // main loop so a build that crashes during init still gets rolled back.
    esp_ota_mark_app_valid_cancel_rollback();

    ESP_LOGI(TAG, "PowerFinger ring firmware ready");
    log_diagnostics_snapshot("boot", &diagnostics);

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)
    phase0_fake_motion_loop();
#else
    // --- Main loop: state machine driven ---
    uint32_t adv_start_ms = hal_timer_get_ms();  // track advertising timeout
    uint8_t prev_buttons = 0;
    uint32_t next_settings_flush_ms = 0;

    while (1) {
        uint32_t now = hal_timer_get_ms();
        ring_settings_snapshot_t settings = ring_settings_snapshot();
        memset(&actions, 0, sizeof(actions));

        // --- Drain BLE event queue (serializes all state machine access) ---
        app_event_t queued_evt;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            if (queued_evt.type == APP_EVT_RING_STATE) {
                ring_state_dispatch(queued_evt.ring_evt, &actions);
                ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
                execute_actions(&actions);

                if (queued_evt.ring_evt == RING_EVT_BLE_CONNECTED) {
                    adv_start_ms = 0;
                    power_manager_on_connect();
                    ring_runtime_health_reset_hid_send(&runtime_health);
                    ring_diagnostics_note_connected(&diagnostics,
                                                    queued_evt.conn_interval_1_25ms);
                    prev_buttons = 0;
                    log_diagnostics_snapshot("connect", &diagnostics);
                }
                if (queued_evt.ring_evt == RING_EVT_BLE_DISCONNECTED) {
                    dead_zone_reset();
                    adv_start_ms = now;
                    power_manager_on_disconnect();
                    ring_runtime_health_reset_hid_send(&runtime_health);
                    ring_diagnostics_note_disconnected(&diagnostics);
                    prev_buttons = 0;
                    log_diagnostics_snapshot("disconnect", &diagnostics);
                }
            } else if (queued_evt.type == APP_EVT_BOND_RESTORED) {
                ring_diagnostics_note_bond_restored(&diagnostics);
                log_diagnostics_snapshot("bond-restored", &diagnostics);
            } else if (queued_evt.type == APP_EVT_BOND_FAILED) {
                ring_diagnostics_note_bond_failed(&diagnostics);
                log_diagnostics_snapshot("bond-failed", &diagnostics);
            } else if (queued_evt.type == APP_EVT_CONN_PARAMS_UPDATED) {
                ring_diagnostics_note_conn_params_updated(&diagnostics,
                                                          queued_evt.conn_interval_1_25ms);
                log_diagnostics_snapshot("conn-update", &diagnostics);
            } else if (queued_evt.type == APP_EVT_CONN_PARAMS_REJECTED) {
                ring_diagnostics_note_conn_param_rejected(&diagnostics);
                log_diagnostics_snapshot("conn-rejected", &diagnostics);
            }
        }

        // --- Advertising timeout ---
        if (adv_start_ms != 0 &&
            ring_state_get() == RING_STATE_ADVERTISING &&
            (now - adv_start_ms) >= RECONNECT_TIMEOUT_MS) {
            memset(&actions, 0, sizeof(actions));
            ring_state_dispatch(RING_EVT_BLE_ADV_TIMEOUT, &actions);
            ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
            execute_actions(&actions);
            log_diagnostics_snapshot("adv-timeout", &diagnostics);
        }

        // --- Read sensor ---
        sensor_reading_t reading = {0};
        bool sensor_valid = false;
        if (ring_runtime_health_sensor_recovery_due(&runtime_health, now)) {
            hal_status_t recovery_rc = attempt_sensor_recovery(sensor_ok);
            ring_sensor_health_update_t recovery_update =
                ring_runtime_health_note_sensor_recovery_attempt(
                    &runtime_health, recovery_rc, now);

            if (recovery_rc == HAL_OK) {
                sensor_ok = true;
                if (!sensor_calibrated) {
                    next_calibration_attempt_ms = now;
                    ESP_LOGI(TAG, "sensor recovered — waiting for late calibration before motion resumes");
                } else if (recovery_update.event == RING_SENSOR_HEALTH_RECOVERED) {
                    ESP_LOGI(TAG, "sensor recovery succeeded — motion input restored");
                }
                sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
                log_diagnostics_snapshot("sensor-recovered", &diagnostics);
            } else {
                sensor_ok = false;
                ESP_LOGW(TAG, "sensor recovery attempt failed: %d", recovery_rc);
                sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
            }
        }

        if (try_late_calibration(sensor_ok,
                                 &sensor_calibrated,
                                 &next_calibration_attempt_ms,
                                 now)) {
            dead_zone_reset();
            sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
            log_diagnostics_snapshot("late-calibration", &diagnostics);
        }

        if (sensor_ok && sensor_calibrated) {
            hal_status_t sensor_rc = sensor_read(&reading);
            ring_sensor_health_update_t sensor_update =
                ring_runtime_health_note_sensor_result(&runtime_health, sensor_rc, now);

            if (sensor_rc == HAL_OK) {
                if (sensor_update.event == RING_SENSOR_HEALTH_RECOVERED) {
                    ESP_LOGI(TAG, "sensor recovered — motion input restored");
                }

                calibration_apply(&reading.dx, &reading.dy);
                reading.dx = ring_settings_scale_delta(reading.dx);
                reading.dy = ring_settings_scale_delta(reading.dy);
                sensor_valid = true;

                if (reading.motion_detected) {
                    memset(&actions, 0, sizeof(actions));
                    ring_state_dispatch(RING_EVT_MOTION_DETECTED, &actions);
                    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
                    execute_actions(&actions);
                    power_manager_on_motion();
                }
            } else {
                if (sensor_update.consecutive_failures == 1) {
                    ESP_LOGW(TAG, "sensor read error: %d", sensor_rc);
                }
                if (sensor_update.event == RING_SENSOR_HEALTH_DEGRADED) {
                    ESP_LOGE(TAG,
                             "sensor read failed %u consecutive times — degrading to click-only until recovery",
                             sensor_update.consecutive_failures);
                    dead_zone_reset();
                    sensor_ok = false;
                    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
                    log_diagnostics_snapshot("sensor-degraded", &diagnostics);
                }
            }
        }

        // --- Click + dead zone ---
        bool clicked = click_ok ? click_is_pressed() : false;
        uint8_t buttons = clicked ? 0x01 : 0x00;

        // Clicks are real user activity even if the sensor is perfectly still.
        // Promote a stationary click from IDLE to ACTIVE so press/release
        // reports are not dropped waiting for motion.
        if (clicked) {
            power_manager_on_click();
            if (ring_state_get() == RING_STATE_CONNECTED_IDLE) {
                memset(&actions, 0, sizeof(actions));
                ring_state_dispatch(RING_EVT_CLICK_ACTIVITY, &actions);
                ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
                execute_actions(&actions);
            }
        }

        if (sensor_valid) {
            dead_zone_update_with_config(clicked,
                                         &reading.dx,
                                         &reading.dy,
                                         now,
                                         settings.dead_zone_time_ms,
                                         settings.dead_zone_distance);
        }

        // --- Send HID report if in active state ---
        bool button_changed = (buttons != prev_buttons);
        if (ring_state_get() == RING_STATE_CONNECTED_ACTIVE &&
            (sensor_valid || button_changed)) {
            handle_hid_send_result(&runtime_health,
                                   ble_hid_mouse_send(buttons,
                                                      sensor_valid ? clamp_i8(reading.dx) : 0,
                                                      sensor_valid ? clamp_i8(reading.dy) : 0,
                                                      0),
                                   now);
        }
        prev_buttons = buttons;

        // --- Power management tick ---
        power_event_t pwr_evt = power_manager_tick(now);
        if (pwr_evt != POWER_EVT_NONE) {
            // Map power events to ring state machine events
            ring_event_t ring_evt = RING_EVT_NONE;
            if (pwr_evt == POWER_EVT_IDLE_TIMEOUT)   ring_evt = RING_EVT_IDLE_TIMEOUT;
            if (pwr_evt == POWER_EVT_LOW_BATTERY)    ring_evt = RING_EVT_LOW_BATTERY;
            if (pwr_evt == POWER_EVT_SLEEP_TIMEOUT)  ring_evt = RING_EVT_SLEEP_TIMEOUT;

            if (ring_evt != RING_EVT_NONE) {
                memset(&actions, 0, sizeof(actions));
                ring_state_dispatch(ring_evt, &actions);
                ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
                execute_actions(&actions);
                log_diagnostics_snapshot("power-event", &diagnostics);
            }
        }
        hal_ble_set_battery_level(power_manager_get_battery_level());
        sync_battery_diagnostics(&diagnostics);

        if (ring_settings_needs_flush() &&
            ring_state_get() != RING_STATE_CONNECTED_ACTIVE &&
            now >= next_settings_flush_ms) {
            hal_status_t flush_rc = ring_settings_flush();
            if (flush_rc == HAL_OK) {
                next_settings_flush_ms = 0;
            } else {
                next_settings_flush_ms = now + SETTINGS_FLUSH_RETRY_MS;
                ESP_LOGW(TAG, "ring settings flush failed (%d) — retrying in %d ms",
                         flush_rc, SETTINGS_FLUSH_RETRY_MS);
            }
        }

        power_manager_feed_watchdog();

        hal_timer_delay_ms(SENSOR_POLL_INTERVAL_MS);
    }
#endif
}
