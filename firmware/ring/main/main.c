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
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hal_timer.h"
#include "hal_ble.h"
#include "ble_hid_mouse.h"
#include "ble_config.h"
#include "ring_state.h"
#include "ring_config.h"
#include "sensor_interface.h"
#include "click_interface.h"
#include "dead_zone.h"
#include "calibration.h"
#include "power_manager.h"

static const char *TAG = "powerfinger";

// --- Event queue: BLE callback -> main loop ---

#define EVT_QUEUE_LEN 8

static QueueHandle_t s_evt_queue = NULL;

// --- Clamp int16_t to int8_t range (prevents direction reversal on overflow) ---
static inline int8_t clamp_i8(int16_t v)
{
    if (v > 127) return 127;
    if (v < -127) return -127;
    return (int8_t)v;
}

// --- BLE event callback (posts to queue, never dispatches directly) ---

static void ble_event_callback(const hal_ble_event_data_t *evt, void *arg)
{
    (void)arg;
    ring_event_t ring_evt;

    switch (evt->type) {
    case HAL_BLE_EVT_CONNECTED:
        ring_evt = RING_EVT_BLE_CONNECTED;
        break;
    case HAL_BLE_EVT_DISCONNECTED:
        ring_evt = RING_EVT_BLE_DISCONNECTED;
        break;
    case HAL_BLE_EVT_BOND_FAILED:
        // Bond recovery: HAL already deleted the stale bond and terminated.
        // The disconnect event will follow via GAP handler.
        return;
    default:
        return;
    }

    // Post to queue (non-blocking; NimBLE callbacks are task context, not ISR).
    // If the queue is full the event is dropped. Under normal operation the
    // main loop drains faster than events arrive, so this should not fire.
    if (xQueueSend(s_evt_queue, &ring_evt, 0) != pdTRUE) {
        ESP_LOGW(TAG, "BLE event queue full — event %d dropped", (int)ring_evt);
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

    while (1) {
        hal_timer_delay_ms(FAKE_MOTION_PERIOD_MS);

        // Drain BLE event queue (connection/disconnection)
        ring_event_t queued_evt;
        ring_actions_t actions;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            ring_state_dispatch(queued_evt, &actions);
            execute_actions(&actions);
            if (queued_evt == RING_EVT_BLE_CONNECTED) {
                adv_start_ms = 0;  // connected, stop tracking adv timeout
                power_manager_on_connect();
            }
            if (queued_evt == RING_EVT_BLE_DISCONNECTED) {
                adv_start_ms = hal_timer_get_ms();
                power_manager_on_disconnect();
            }
        }

        // Check advertising timeout
        uint32_t now = hal_timer_get_ms();
        if (adv_start_ms != 0 &&
            ring_state_get() == RING_STATE_ADVERTISING &&
            (now - adv_start_ms) >= RECONNECT_TIMEOUT_MS) {
            ring_state_dispatch(RING_EVT_BLE_ADV_TIMEOUT, &actions);
            execute_actions(&actions);
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

        ble_hid_mouse_send(buttons, dx, dy, 0);
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
    s_evt_queue = xQueueCreate(EVT_QUEUE_LEN, sizeof(ring_event_t));
    if (!s_evt_queue) {
        ESP_LOGE(TAG, "BLE event queue alloc failed — restarting");
        esp_restart();
    }

    // Initialize state machine
    ring_state_init();

    // Initialize sensor (check for failure)
    bool sensor_ok = (sensor_init() == HAL_OK);
    if (!sensor_ok) {
        ESP_LOGE(TAG, "sensor init failed — HID reports will be suppressed");
    }

    // Initialize click (check for failure)
    bool click_ok = (click_init() == HAL_OK);
    if (!click_ok) {
        ESP_LOGW(TAG, "click init failed — buttons disabled");
    }
    dead_zone_init();

    // Run calibration (blocks until complete)
    ring_actions_t actions;
    hal_status_t cal_ret = calibration_run();
    if (cal_ret == HAL_OK) {
        ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &actions);
    } else {
        ring_state_dispatch(RING_EVT_CALIBRATION_FAILED, &actions);
    }
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

    // Confirm this firmware is valid — cancels automatic rollback.
    // Placed after all critical init (BLE + power manager) and before the
    // main loop so a build that crashes during init still gets rolled back.
    esp_ota_mark_app_valid_cancel_rollback();

    ESP_LOGI(TAG, "PowerFinger ring firmware ready");

#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)
    phase0_fake_motion_loop();
#else
    // --- Main loop: state machine driven ---
    uint32_t adv_start_ms = hal_timer_get_ms();  // track advertising timeout
    uint8_t prev_buttons = 0;

    // Consecutive sensor failures — after 50 failures (~750ms at 15ms poll),
    // enter deep sleep and reboot. Eliminates false positives from surface lift.
    #define SENSOR_FAILURE_THRESHOLD 50
    uint8_t consecutive_sensor_failures = 0;

    while (1) {
        uint32_t now = hal_timer_get_ms();
        memset(&actions, 0, sizeof(actions));

        // --- Drain BLE event queue (serializes all state machine access) ---
        ring_event_t queued_evt;
        while (xQueueReceive(s_evt_queue, &queued_evt, 0) == pdTRUE) {
            ring_state_dispatch(queued_evt, &actions);
            execute_actions(&actions);

            if (queued_evt == RING_EVT_BLE_CONNECTED) {
                adv_start_ms = 0;
                power_manager_on_connect();
                prev_buttons = 0;
            }
            if (queued_evt == RING_EVT_BLE_DISCONNECTED) {
                dead_zone_reset();
                adv_start_ms = now;
                power_manager_on_disconnect();
                prev_buttons = 0;
            }
        }

        // --- Advertising timeout ---
        if (adv_start_ms != 0 &&
            ring_state_get() == RING_STATE_ADVERTISING &&
            (now - adv_start_ms) >= RECONNECT_TIMEOUT_MS) {
            memset(&actions, 0, sizeof(actions));
            ring_state_dispatch(RING_EVT_BLE_ADV_TIMEOUT, &actions);
            execute_actions(&actions);
        }

        // --- Read sensor ---
        sensor_reading_t reading = {0};
        bool sensor_valid = false;
        if (sensor_ok && sensor_read(&reading) == HAL_OK) {
            calibration_apply(&reading.dx, &reading.dy);
            sensor_valid = true;
            consecutive_sensor_failures = 0;

            if (reading.motion_detected) {
                memset(&actions, 0, sizeof(actions));
                ring_state_dispatch(RING_EVT_MOTION_DETECTED, &actions);
                execute_actions(&actions);
                power_manager_on_motion();
            }
        } else if (sensor_ok) {
            consecutive_sensor_failures++;
            if (consecutive_sensor_failures >= SENSOR_FAILURE_THRESHOLD) {
                ESP_LOGE(TAG, "sensor failed %d consecutive reads — rebooting",
                         SENSOR_FAILURE_THRESHOLD);
                power_manager_enter_sleep(true);
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
                execute_actions(&actions);
            }
        }

        if (sensor_valid) {
            dead_zone_update(clicked, &reading.dx, &reading.dy, now);
        }

        // --- Send HID report if in active state ---
        bool button_changed = (buttons != prev_buttons);
        if (ring_state_get() == RING_STATE_CONNECTED_ACTIVE &&
            (sensor_valid || button_changed)) {
            ble_hid_mouse_send(buttons,
                               sensor_valid ? clamp_i8(reading.dx) : 0,
                               sensor_valid ? clamp_i8(reading.dy) : 0,
                               0);
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
                execute_actions(&actions);
            }
        }
        hal_ble_set_battery_level(power_manager_get_battery_level());

        power_manager_feed_watchdog();

        hal_timer_delay_ms(SENSOR_POLL_INTERVAL_MS);
    }
#endif
}
