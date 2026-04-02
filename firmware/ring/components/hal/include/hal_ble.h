// SPDX-License-Identifier: MIT
// PowerFinger HAL — BLE HID peripheral interface
//
// Abstraction at the HID application level, not at the GATT level.
// NimBLE / Zephyr BLE specifics are confined to the implementation file.
//
// This interface covers the ring's BLE peripheral role. The hub's BLE
// central role has its own component (ble_central) that uses platform
// APIs directly, since the central role is ESP32-S3-specific for now.

#pragma once

#include "hal_types.h"

// HID mouse report — the ring's output to the host
typedef struct {
    uint8_t buttons;    // bit 0 = left, bit 1 = right, bit 2 = middle
    int8_t  dx;
    int8_t  dy;
    int8_t  wheel;      // vertical scroll
} hal_hid_mouse_report_t;

// BLE event types delivered to the application callback
typedef enum {
    HAL_BLE_EVT_CONNECTED,          // BLE link established
    HAL_BLE_EVT_DISCONNECTED,       // BLE link lost
    HAL_BLE_EVT_BOND_RESTORED,      // Reconnected with existing bond
    HAL_BLE_EVT_BOND_FAILED,        // Bond asymmetry or pairing failure
    HAL_BLE_EVT_CONN_PARAMS_UPDATED,// Connection interval changed
    HAL_BLE_EVT_CONN_PARAMS_REJECTED,// Central rejected interval request
} hal_ble_event_t;

// Event data passed with the callback
typedef struct {
    hal_ble_event_t type;
    union {
        struct {
            uint16_t conn_interval_1_25ms;  // in units of 1.25ms
        } conn_params;
    } data;
} hal_ble_event_data_t;

// Application callback for BLE events (called from task context)
typedef void (*hal_ble_event_cb_t)(const hal_ble_event_data_t *evt, void *arg);

// Initialize BLE stack and register HID service.
// device_name: advertised name (e.g. "PowerFinger")
// cb: application callback for connection/disconnection events
hal_status_t hal_ble_init(const char *device_name, hal_ble_event_cb_t cb, void *arg);

// Start BLE advertising. Stops automatically after timeout_ms (0 = no timeout).
hal_status_t hal_ble_start_advertising(uint32_t timeout_ms);

// Stop BLE advertising
hal_status_t hal_ble_stop_advertising(void);

// Send a HID mouse report to the connected host.
// Returns HAL_ERR_BUSY if not connected.
hal_status_t hal_ble_send_mouse_report(const hal_hid_mouse_report_t *report);

// Request a connection interval change.
// min/max in units of 1.25ms (e.g. 6 = 7.5ms, 12 = 15ms).
// The central may accept, reject, or propose a different value.
// Result is delivered via HAL_BLE_EVT_CONN_PARAMS_UPDATED or _REJECTED.
hal_status_t hal_ble_request_conn_params(uint16_t min_1_25ms, uint16_t max_1_25ms);

// Delete all stored bonds. Used for factory reset or bond recovery.
hal_status_t hal_ble_delete_all_bonds(void);

// Update the Battery Service value exposed over GATT.
// level_percent is clamped to 0..100 and may be an approximate estimate.
void hal_ble_set_battery_level(uint8_t level_percent);

// Check if currently connected to a host
bool hal_ble_is_connected(void);
