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
#include "esp_ota_ops.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "hal_timer.h"
#include "ble_central.h"
#include "role_engine.h"
#include "event_composer.h"
#include "hub_identity.h"
#include "usb_hid_mouse.h"

static const char *TAG = "powerfinger_hub";

// USB HID poll interval (1ms for USB full-speed)
#define USB_POLL_INTERVAL_MS 1

// Poll for rings stuck in GATT discovery at a human timescale, not every
// USB tick. 1s granularity is plenty for a 10s discovery timeout.
#define DISCOVERY_TIMEOUT_CHECK_MS 1000

// H1: USB HID send failure escalation.
// After this many consecutive send errors (excluding HAL_ERR_BUSY which is
// transient), the hub restarts. At 1ms poll rate, 5000 = 5 seconds of
// continuous failure — long enough to survive brief USB bus resets, short
// enough to not leave a user without input indefinitely.
#define USB_SEND_FAIL_THRESHOLD 5000

// --- Callbacks from BLE central ---

static void on_ring_report(uint8_t ring_index, const hub_ring_report_t *report, void *arg)
{
    (void)arg;

    // Feed into event composer. Role is cached on connect so the hot path does
    // not take the role-engine mutex for every BLE notification.
    event_composer_feed(ring_index, report->buttons, report->dx, report->dy);
}

static void on_ring_connection(uint8_t ring_index, bool connected, void *arg)
{
    (void)arg;

    if (connected) {
        ring_role_t role = ROLE_CURSOR;
        uint8_t mac[6];
        if (ble_central_get_mac(ring_index, mac) == HAL_OK) {
            role = role_engine_get_role(mac);
        } else {
            ESP_LOGW(TAG, "ring %d connected but MAC lookup failed, defaulting role to CURSOR",
                     ring_index);
        }
        event_composer_mark_connected(ring_index, role);
        ESP_LOGI(TAG, "ring %d connected, role=%s",
                 ring_index, role_engine_role_name(role));
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

    // Initialize components — BLE is fatal, others are degradable
    if (role_engine_init() != HAL_OK) {
        // Mutex allocation failure means OOM at boot — restart rather than
        // running the role engine without synchronization.
        ESP_LOGE(TAG, "role engine init failed (OOM) — restarting");
        esp_restart();
    }
    event_composer_init();
    if (usb_hid_mouse_init() != HAL_OK) {
        ESP_LOGE(TAG, "USB HID init failed — hub has no output path");
        esp_restart();
    }
    if (ble_central_init(on_ring_report, on_ring_connection, NULL) != HAL_OK) {
        ESP_LOGE(TAG, "BLE central init failed — hub cannot function");
        esp_restart();
    }

    // Register main task with watchdog (hub must not hang silently)
    esp_task_wdt_add(NULL);

    // Confirm this firmware is valid — cancels automatic rollback.
    // Must be called after all critical init succeeds and the watchdog is
    // armed, so a broken build that hangs here still gets rolled back.
    esp_ota_mark_app_valid_cancel_rollback();

    ESP_LOGI(TAG, "hub ready, scanning for rings...");
    ESP_LOGI(TAG, "hub identity: fw=%s hw=%s",
             hub_identity_firmware_revision(),
             hub_identity_hardware_revision());

    // --- Main loop: compose and send USB HID reports at 1ms ---
    // Track whether the previous report was non-zero so we always send
    // a final zero report to release buttons. Suppressing the zero report
    // after disconnect would leave a stuck button on the host — the exact
    // accessibility hazard the event composer is designed to prevent.
    bool prev_report_nonzero = false;

    uint32_t last_discovery_check_ms = hal_timer_get_ms();

    // H1: consecutive USB HID send failure counter
    uint32_t usb_fail_count = 0;

    while (1) {
        uint32_t now = hal_timer_get_ms();
        composed_report_t report;
        event_composer_compose(&report);

        bool report_nonzero = (report.buttons != 0 ||
                               report.cursor_dx != 0 || report.cursor_dy != 0 ||
                               report.scroll_v != 0 || report.scroll_h != 0);

        if (report_nonzero || prev_report_nonzero) {
            hal_status_t send_rc = usb_hid_mouse_send(&report);
            if (send_rc == HAL_OK || send_rc == HAL_ERR_BUSY) {
                usb_fail_count = 0;
            } else {
                usb_fail_count++;
                if (usb_fail_count >= USB_SEND_FAIL_THRESHOLD) {
                    ESP_LOGE(TAG, "USB HID send failed %lu consecutive times — restarting",
                             (unsigned long)USB_SEND_FAIL_THRESHOLD);
                    esp_restart();
                }
                if ((usb_fail_count % 1000) == 1) {
                    ESP_LOGW(TAG, "USB HID send error: %d (%lu consecutive)",
                             send_rc, (unsigned long)usb_fail_count);
                }
            }
        }

        prev_report_nonzero = report_nonzero;

        if ((now - last_discovery_check_ms) >= DISCOVERY_TIMEOUT_CHECK_MS) {
            // H8: disconnect rings stuck in GATT discovery
            ble_central_check_discovery_timeout();
            last_discovery_check_ms = now;
        }

        esp_task_wdt_reset();
        hal_timer_delay_ms(USB_POLL_INTERVAL_MS);
    }
}
