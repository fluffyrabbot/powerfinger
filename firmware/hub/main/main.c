// SPDX-License-Identifier: MIT
// PowerFinger USB Hub Dongle Firmware — Entry Point
//
// The hub is a BLE central + USB HID device. It:
// 1. Scans for PowerFinger ring peripherals over BLE
// 2. Pairs, bonds, and receives HID reports from up to 4 rings
// 3. Maps each ring to a role (cursor, scroll, modifier) via the role engine
// 4. Composes all ring inputs into a single USB HID mouse report
// 5. Presents as one standard USB HID mouse to the host OS

#include <stdio.h>
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "hal_timer.h"
#include "ble_central.h"
#include "role_engine.h"
#include "event_composer.h"
#include "usb_hid_mouse.h"

static const char *TAG = "powerfinger_hub";

// USB HID poll interval (1ms for USB full-speed)
#define USB_POLL_INTERVAL_MS 1

// --- Callbacks from BLE central ---

static void on_ring_report(uint8_t ring_index, const hub_ring_report_t *report, void *arg)
{
    (void)arg;

    // Look up this ring's role
    uint8_t mac[6];
    if (ble_central_get_mac(ring_index, mac) != HAL_OK) return;

    ring_role_t role = role_engine_get_role(mac);

    // Feed into event composer
    event_composer_feed(ring_index, role,
                        report->buttons, report->dx, report->dy);
}

static void on_ring_connection(uint8_t ring_index, bool connected, void *arg)
{
    (void)arg;

    if (connected) {
        event_composer_mark_connected(ring_index);
        uint8_t mac[6];
        if (ble_central_get_mac(ring_index, mac) == HAL_OK) {
            ring_role_t role = role_engine_get_role(mac);
            ESP_LOGI(TAG, "ring %d connected, role=%s",
                     ring_index, role_engine_role_name(role));
        }
    } else {
        ESP_LOGI(TAG, "ring %d disconnected", ring_index);
        // Critical: immediately release buttons to prevent stuck state
        event_composer_ring_disconnected(ring_index);
    }
}

// --- Main ---

void app_main(void)
{
    ESP_LOGI(TAG, "PowerFinger USB hub dongle starting");

    // Initialize NVS (required for BLE bonding and role persistence)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition erased, re-initializing");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize components
    role_engine_init();
    event_composer_init();
    usb_hid_mouse_init();
    ble_central_init(on_ring_report, on_ring_connection, NULL);

    // Register main task with watchdog (hub must not hang silently)
    esp_task_wdt_add(NULL);

    ESP_LOGI(TAG, "hub ready, scanning for rings...");

    // --- Main loop: compose and send USB HID reports at 1ms ---
    // Track whether the previous report was non-zero so we always send
    // a final zero report to release buttons. Suppressing the zero report
    // after disconnect would leave a stuck button on the host — the exact
    // accessibility hazard the event composer is designed to prevent.
    bool prev_report_nonzero = false;

    while (1) {
        composed_report_t report;
        event_composer_compose(&report);

        bool report_nonzero = (report.buttons != 0 ||
                               report.cursor_dx != 0 || report.cursor_dy != 0 ||
                               report.scroll_v != 0 || report.scroll_h != 0);

        if (report_nonzero || prev_report_nonzero) {
            usb_hid_mouse_send(&report);
        }

        prev_report_nonzero = report_nonzero;

        esp_task_wdt_reset();
        hal_timer_delay_ms(USB_POLL_INTERVAL_MS);
    }
}
