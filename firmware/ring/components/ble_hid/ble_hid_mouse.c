// SPDX-License-Identifier: MIT
// PowerFinger — BLE HID mouse component

#include "ble_hid_mouse.h"
#include "ble_config.h"
#include "hal_ble.h"
#include "esp_log.h"

static const char *TAG = "ble_hid";

hal_status_t ble_hid_mouse_init(hal_ble_event_cb_t cb, void *arg)
{
    ESP_LOGI(TAG, "initializing BLE HID mouse");
    return hal_ble_init(BLE_DEVICE_NAME, cb, arg);
}

hal_status_t ble_hid_mouse_send(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel)
{
    hal_hid_mouse_report_t report = {
        .buttons = buttons,
        .dx = dx,
        .dy = dy,
        .wheel = wheel,
    };
    return hal_ble_send_mouse_report(&report);
}
