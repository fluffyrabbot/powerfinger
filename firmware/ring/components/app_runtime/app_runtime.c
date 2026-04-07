// SPDX-License-Identifier: MIT
// PowerFinger - Shared runtime for ring-derived firmware variants.
//
// Architecture: BLE events are posted to a FreeRTOS queue from the NimBLE
// callback task. The main loop drains the queue and dispatches all events
// through the state machine on a single task, eliminating cross-task races
// on the state machine and ensuring action flags are always consumed.

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdatomic.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "app_controller.h"
#include "app_runtime.h"
#include "ble_hid_mouse.h"
#include "calibration.h"
#include "click_interface.h"
#include "dead_zone.h"
#include "hal_ble.h"
#include "hal_timer.h"
#include "power_manager.h"
#include "ring_config.h"
#include "ring_diagnostics.h"
#include "ring_runtime_health.h"
#include "ring_settings.h"
#include "ring_state.h"
#include "sensor_interface.h"
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
#include "pen_wake_debounce.h"
#endif

#define DEFAULT_POWERFINGER_APP_LOG_TAG "powerfinger"
#define DEFAULT_POWERFINGER_APP_FIRMWARE_NAME "PowerFinger ring"

#define EVT_QUEUE_LEN 8

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)
#define FAKE_MOTION_PERIOD_MS 15
#define FAKE_MOTION_RADIUS 3
#endif

#define TAG s_log_tag

static const powerfinger_app_config_t s_default_app_config = {
    .log_tag = DEFAULT_POWERFINGER_APP_LOG_TAG,
    .firmware_name = DEFAULT_POWERFINGER_APP_FIRMWARE_NAME,
};

static const char *s_log_tag = DEFAULT_POWERFINGER_APP_LOG_TAG;
static const char *s_firmware_name = DEFAULT_POWERFINGER_APP_FIRMWARE_NAME;

static QueueHandle_t s_evt_queue = NULL;
static atomic_uint s_evt_queue_dropped = ATOMIC_VAR_INIT(0);

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

static void sync_pen_wake_diagnostics(ring_diagnostics_t *diagnostics)
{
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
    ring_diagnostics_note_pen_wake(diagnostics,
                                   pen_wake_debounce_drv5032_enabled(),
                                   pen_wake_debounce_spurious_wake_count());
#else
    (void)diagnostics;
#endif
}

static void publish_ble_diagnostics(const ring_diagnostics_t *diagnostics)
{
    uint8_t payload[RING_DIAG_BLE_PAYLOAD_LEN] = {0};
    size_t payload_len = ring_diagnostics_encode_ble_payload(diagnostics,
                                                             payload,
                                                             sizeof(payload));

    if (payload_len == 0) {
        hal_ble_set_diagnostic_payload(NULL, 0);
        return;
    }

    hal_ble_set_diagnostic_payload(payload, payload_len);
}

static hal_status_t attempt_sensor_recovery(bool had_working_sensor)
{
    hal_status_t power_rc = power_manager_set_sensor_power(true);
    if (power_rc != HAL_OK) {
        return power_rc;
    }

    hal_status_t rc = had_working_sensor ? sensor_wake() : sensor_init();
    if (rc == HAL_OK) {
        return rc;
    }

    if (had_working_sensor) {
        // If the sensor stopped responding after a previously healthy session,
        // fall back to a full re-init before giving up.
        rc = sensor_init();
    }

    if (rc != HAL_OK) {
        hal_status_t gate_rc = power_manager_set_sensor_power(false);
        if (gate_rc != HAL_OK) {
            ESP_LOGW(TAG, "failed to gate sensor power after recovery error: %d", gate_rc);
        }
    }

    return rc;
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
        ESP_LOGI(TAG, "late calibration succeeded -- motion input ready");
        return *sensor_calibrated;
    }

    calibration_reset();
    *next_calibration_attempt_ms = now_ms + CALIBRATION_RETRY_DELAY_MS;
    hal_status_t gate_rc = power_manager_set_sensor_power(false);
    if (gate_rc != HAL_OK) {
        ESP_LOGW(TAG, "failed to gate sensor power after calibration error: %d", gate_rc);
    }
    ESP_LOGW(TAG, "late calibration failed -- retrying in %d ms while disconnected",
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
        ESP_LOGW(TAG, "BLE HID send error: %d -- monitoring for recovery", update.status);
    } else if (update.event == RING_HID_HEALTH_RECOVERED) {
        ESP_LOGI(TAG, "BLE HID send path recovered");
    } else if (update.event == RING_HID_HEALTH_RESTART_REQUIRED) {
        ESP_LOGE(TAG, "BLE HID send failed for %lu ms (last=%d) -- restarting",
                 (unsigned long)update.fault_elapsed_ms, update.status);
        esp_restart();
    }
}

static void ble_event_callback(const hal_ble_event_data_t *evt, void *arg)
{
    (void)arg;
    app_controller_event_t app_evt = {0};

    switch (evt->type) {
    case HAL_BLE_EVT_CONNECTED:
        app_evt.type = APP_CONTROLLER_EVT_RING_STATE;
        app_evt.ring_evt = RING_EVT_BLE_CONNECTED;
        app_evt.conn_interval_1_25ms = evt->data.conn_params.conn_interval_1_25ms;
        break;
    case HAL_BLE_EVT_DISCONNECTED:
        app_evt.type = APP_CONTROLLER_EVT_RING_STATE;
        app_evt.ring_evt = RING_EVT_BLE_DISCONNECTED;
        break;
    case HAL_BLE_EVT_BOND_RESTORED:
        app_evt.type = APP_CONTROLLER_EVT_BOND_RESTORED;
        break;
    case HAL_BLE_EVT_BOND_FAILED:
        app_evt.type = APP_CONTROLLER_EVT_BOND_FAILED;
        break;
    case HAL_BLE_EVT_CONN_PARAMS_UPDATED:
        app_evt.type = APP_CONTROLLER_EVT_CONN_PARAMS_UPDATED;
        app_evt.conn_interval_1_25ms = evt->data.conn_params.conn_interval_1_25ms;
        break;
    case HAL_BLE_EVT_CONN_PARAMS_REJECTED:
        app_evt.type = APP_CONTROLLER_EVT_CONN_PARAMS_REJECTED;
        break;
    default:
        return;
    }

    // Post to queue (non-blocking; NimBLE callbacks are task context, not ISR).
    // If the queue is full the event is dropped. Under normal operation the
    // main loop drains faster than events arrive, so this should not fire.
    if (xQueueSend(s_evt_queue, &app_evt, 0) != pdTRUE) {
        unsigned int dropped = atomic_fetch_add(&s_evt_queue_dropped, 1U) + 1U;
        ESP_LOGW(TAG,
                 "BLE event queue full -- event type %d dropped (%u pending repair)",
                 (int)app_evt.type,
                 dropped);
    }
}

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)

static void phase0_fake_motion_loop(void)
{
    ESP_LOGI(TAG, "Phase 0: fake motion mode (no sensor, no click)");

    uint32_t tick = 0;
    ring_runtime_health_t runtime_health;
    ring_runtime_health_init(&runtime_health);
    ring_diagnostics_t diagnostics;
    ring_diagnostics_init(&diagnostics);
    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());
    app_controller_t controller;
    app_controller_init(&controller, &diagnostics, &runtime_health, NULL, NULL);
    controller.advertising_started_ms = hal_timer_get_ms();
    sync_battery_diagnostics(&diagnostics);
    publish_ble_diagnostics(&diagnostics);

    while (1) {
        hal_timer_delay_ms(FAKE_MOTION_PERIOD_MS);

        app_controller_event_t queued_evt;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            app_controller_handle_event(&controller, &queued_evt, hal_timer_get_ms());
        }

        app_controller_reconcile_ble_event_drops(&controller,
                                                 atomic_exchange(&s_evt_queue_dropped, 0U),
                                                 hal_ble_is_connected(),
                                                 hal_timer_get_ms());

        uint32_t now = hal_timer_get_ms();
        app_controller_check_advertising_timeout(&controller, now);

        publish_ble_diagnostics(&diagnostics);
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

#endif

void powerfinger_app_main(const powerfinger_app_config_t *config)
{
    if (!config) {
        config = &s_default_app_config;
    }

    s_log_tag = config->log_tag ? config->log_tag : s_default_app_config.log_tag;
    s_firmware_name = config->firmware_name
        ? config->firmware_name
        : s_default_app_config.firmware_name;

    ESP_LOGI(TAG, "%s firmware starting", s_firmware_name);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition erased, re-initializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_evt_queue = xQueueCreate(EVT_QUEUE_LEN, sizeof(app_controller_event_t));
    if (!s_evt_queue) {
        ESP_LOGE(TAG, "BLE event queue alloc failed -- restarting");
        esp_restart();
    }

    ring_state_init();

    ring_diagnostics_t diagnostics;
    ring_diagnostics_init(&diagnostics);
    ring_diagnostics_note_ring_state(&diagnostics, ring_state_get());

    ring_runtime_health_t runtime_health;
    ring_runtime_health_init(&runtime_health);
    calibration_reset();

    hal_status_t settings_rc = ring_settings_init();
    if (settings_rc != HAL_OK && settings_rc != HAL_ERR_INVALID_ARG) {
        ESP_LOGW(TAG, "ring settings init failed (%d) -- using runtime defaults only",
                 settings_rc);
    } else if (settings_rc == HAL_ERR_INVALID_ARG) {
        ESP_LOGW(TAG, "ring settings blob invalid -- reverted to defaults and will repair on next idle flush");
    }

    if (power_manager_init() != HAL_OK) {
        ESP_LOGE(TAG, "power manager init failed -- entering deep sleep (LiPo safety)");
        power_manager_enter_sleep(true);
        return;
    }

#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
    hal_status_t wake_rc = pen_wake_debounce_init();
    if (wake_rc != HAL_OK) {
        ESP_LOGW(TAG, "pen wake debounce init failed (%d) -- keeping default wake mask", wake_rc);
    }
#endif

    bool sensor_ok = (sensor_init() == HAL_OK);
    bool sensor_calibrated = false;
    uint32_t next_calibration_attempt_ms = 0;
    if (!sensor_ok) {
        ESP_LOGE(TAG, "sensor init failed -- motion input disabled, click path still available");
        ring_runtime_health_mark_sensor_unavailable(&runtime_health, hal_timer_get_ms());
    }
    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
    sync_pen_wake_diagnostics(&diagnostics);

    bool click_ok = (click_init() == HAL_OK);
    if (!click_ok) {
        ESP_LOGW(TAG, "click init failed -- buttons disabled");
    }
    dead_zone_ctx_t dead_zone;
    dead_zone_ctx_t secondary_dead_zone;
    dead_zone_init(&dead_zone);
    dead_zone_init(&secondary_dead_zone);
    app_controller_t controller;
    app_controller_init(&controller,
                        &diagnostics,
                        &runtime_health,
                        &dead_zone,
                        &secondary_dead_zone);

    hal_status_t cal_ret = HAL_ERR_TIMEOUT;
    if (sensor_ok) {
        cal_ret = calibration_run();
        sensor_calibrated = calibration_is_valid();
        if (!sensor_calibrated) {
            next_calibration_attempt_ms = hal_timer_get_ms() + CALIBRATION_RETRY_DELAY_MS;
            hal_status_t gate_rc = power_manager_set_sensor_power(false);
            if (gate_rc != HAL_OK) {
                ESP_LOGW(TAG, "failed to gate sensor power after boot calibration error: %d",
                         gate_rc);
            }
        }
    } else {
        ESP_LOGW(TAG, "skipping calibration until sensor recovers");
        hal_status_t gate_rc = power_manager_set_sensor_power(false);
        if (gate_rc != HAL_OK) {
            ESP_LOGW(TAG, "failed to gate sensor power after sensor init error: %d", gate_rc);
        }
    }
    ring_event_t calibration_event = RING_EVT_CALIBRATION_DONE;
    if (cal_ret != HAL_OK) {
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
        pen_wake_debounce_note_validation_failure();
#endif
        calibration_event = RING_EVT_CALIBRATION_FAILED;
    }

    if (ble_hid_mouse_init(ble_event_callback, NULL) != HAL_OK) {
        ESP_LOGE(TAG, "BLE init failed -- entering deep sleep");
        power_manager_enter_sleep(true);
        return;
    }
    app_controller_dispatch_ring_event(&controller,
                                       calibration_event,
                                       0,
                                       hal_timer_get_ms(),
                                       NULL);
    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
    sync_pen_wake_diagnostics(&diagnostics);

    hal_ble_set_battery_level(power_manager_get_battery_level());
    sync_battery_diagnostics(&diagnostics);
    sync_pen_wake_diagnostics(&diagnostics);
    publish_ble_diagnostics(&diagnostics);

    esp_ota_mark_app_valid_cancel_rollback();

    ESP_LOGI(TAG, "%s firmware ready", s_firmware_name);
    app_controller_log_diagnostics_snapshot("boot", &diagnostics);

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)
    phase0_fake_motion_loop();
#else
    uint32_t next_settings_flush_ms = 0;

    while (1) {
        uint32_t now = hal_timer_get_ms();
        ring_settings_snapshot_t settings = ring_settings_snapshot();

        app_controller_event_t queued_evt;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            app_controller_handle_event(&controller, &queued_evt, now);
        }

        app_controller_reconcile_ble_event_drops(&controller,
                                                 atomic_exchange(&s_evt_queue_dropped, 0U),
                                                 hal_ble_is_connected(),
                                                 now);

        app_controller_check_advertising_timeout(&controller, now);

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
                    ESP_LOGI(TAG, "sensor recovered -- waiting for late calibration before motion resumes");
                } else if (recovery_update.event == RING_SENSOR_HEALTH_RECOVERED) {
                    ESP_LOGI(TAG, "sensor recovery succeeded -- motion input restored");
                }
                sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
                app_controller_log_diagnostics_snapshot("sensor-recovered", &diagnostics);
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
            dead_zone_reset(&dead_zone);
            dead_zone_reset(&secondary_dead_zone);
            sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
            app_controller_log_diagnostics_snapshot("late-calibration", &diagnostics);
        }

        if (sensor_ok && sensor_calibrated) {
            hal_status_t sensor_rc = sensor_read(&reading);
            ring_sensor_health_update_t sensor_update =
                ring_runtime_health_note_sensor_result(&runtime_health, sensor_rc, now);

            if (sensor_rc == HAL_OK) {
                if (sensor_update.event == RING_SENSOR_HEALTH_RECOVERED) {
                    ESP_LOGI(TAG, "sensor recovered -- motion input restored");
                }

                calibration_apply(&reading.dx, &reading.dy);
                reading.dx = ring_settings_scale_delta(reading.dx);
                reading.dy = ring_settings_scale_delta(reading.dy);
                sensor_valid = true;

                if (reading.motion_detected) {
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
                    pen_wake_debounce_note_motion();
                    sync_pen_wake_diagnostics(&diagnostics);
#endif
                    app_controller_dispatch_ring_event(&controller,
                                                       RING_EVT_MOTION_DETECTED,
                                                       0,
                                                       now,
                                                       NULL);
                    power_manager_on_motion();
                }
            } else {
                if (sensor_update.consecutive_failures == 1) {
                    ESP_LOGW(TAG, "sensor read error: %d", sensor_rc);
                }
                if (sensor_update.event == RING_SENSOR_HEALTH_DEGRADED) {
                    ESP_LOGE(TAG,
                             "sensor read failed %u consecutive times -- degrading to click-only until recovery",
                             sensor_update.consecutive_failures);
                    dead_zone_reset(&dead_zone);
                    dead_zone_reset(&secondary_dead_zone);
                    sensor_ok = false;
                    sync_sensor_diagnostics(&diagnostics, sensor_ok, sensor_calibrated);
                    app_controller_log_diagnostics_snapshot("sensor-degraded", &diagnostics);
                }
            }
        }

        bool primary_clicked = click_ok ? click_is_pressed(CLICK_SOURCE_PRIMARY) : false;
        bool secondary_clicked = click_ok ? click_is_pressed(CLICK_SOURCE_SECONDARY) : false;
        bool clicked = primary_clicked || secondary_clicked;
        uint8_t buttons = 0;
        if (primary_clicked) {
            buttons |= 0x01;
        }
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
        if (secondary_clicked) {
            buttons |= 0x02;
        }
#endif

        if (clicked) {
            power_manager_on_click();
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
            if (primary_clicked) {
                pen_wake_debounce_note_barrel_press();
                sync_pen_wake_diagnostics(&diagnostics);
            }
#endif
            if (ring_state_get() == RING_STATE_CONNECTED_IDLE) {
                app_controller_dispatch_ring_event(&controller,
                                                   RING_EVT_CLICK_ACTIVITY,
                                                   0,
                                                   now,
                                                   NULL);
            }
        }

        if (sensor_valid) {
#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
            dead_zone_update_with_config(&secondary_dead_zone,
                                         secondary_clicked,
                                         &reading.dx,
                                         &reading.dy,
                                         now,
                                         settings.dead_zone_time_ms,
                                         settings.dead_zone_distance);
#else
            dead_zone_update_with_config(&dead_zone,
                                         primary_clicked,
                                         &reading.dx,
                                         &reading.dy,
                                         now,
                                         settings.dead_zone_time_ms,
                                         settings.dead_zone_distance);
#endif
        }

        bool button_changed = (buttons != app_controller_previous_buttons(&controller));
        if (ring_state_get() == RING_STATE_CONNECTED_ACTIVE &&
            (sensor_valid || button_changed)) {
            handle_hid_send_result(&runtime_health,
                                   ble_hid_mouse_send(buttons,
                                                      sensor_valid ? clamp_i8(reading.dx) : 0,
                                                      sensor_valid ? clamp_i8(reading.dy) : 0,
                                                      0),
                                   now);
        }
        app_controller_set_previous_buttons(&controller, buttons);

#ifdef CONFIG_POWERFINGER_FORM_FACTOR_PEN
        pen_wake_debounce_tick(now);
        sync_pen_wake_diagnostics(&diagnostics);
#endif

        power_event_t pwr_evt = power_manager_tick(now);
        if (pwr_evt != POWER_EVT_NONE) {
            app_controller_handle_power_event(&controller, pwr_evt, now);
        }
        hal_ble_set_battery_level(power_manager_get_battery_level());
        sync_battery_diagnostics(&diagnostics);
        sync_pen_wake_diagnostics(&diagnostics);
        publish_ble_diagnostics(&diagnostics);

        if (ring_settings_needs_flush() &&
            ring_state_get() != RING_STATE_CONNECTED_ACTIVE &&
            now >= next_settings_flush_ms) {
            hal_status_t flush_rc = ring_settings_flush();
            if (flush_rc == HAL_OK) {
                next_settings_flush_ms = 0;
            } else {
                next_settings_flush_ms = now + SETTINGS_FLUSH_RETRY_MS;
                ESP_LOGW(TAG, "ring settings flush failed (%d) -- retrying in %d ms",
                         flush_rc, SETTINGS_FLUSH_RETRY_MS);
            }
        }

        power_manager_feed_watchdog();
        hal_timer_delay_ms(SENSOR_POLL_INTERVAL_MS);
    }
#endif
}
