// SPDX-License-Identifier: MIT
// PowerFinger — BLE HID mouse interface

#pragma once

#include "hal_types.h"
#include "hal_ble.h"

// Initialize BLE HID mouse profile. Registers GATT services, starts advertising.
// cb: application callback for BLE events (connection, disconnection, etc.)
hal_status_t ble_hid_mouse_init(hal_ble_event_cb_t cb, void *arg);

// Send a mouse report to the connected host.
// buttons: bit 0 = left, bit 1 = right, bit 2 = middle
// dx, dy: relative movement (-127 to 127)
// wheel: vertical scroll (-127 to 127)
hal_status_t ble_hid_mouse_send(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel);
