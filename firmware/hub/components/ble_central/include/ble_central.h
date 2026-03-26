// SPDX-License-Identifier: MIT
// PowerFinger Hub — BLE central interface
//
// Scans for, connects to, and manages multiple PowerFinger ring peripherals.

#pragma once

#include "hal_types.h"
#include <stdint.h>

#define HUB_MAX_RINGS 4

// HID report received from a connected ring
typedef struct {
    uint8_t buttons;
    int8_t  dx;
    int8_t  dy;
    int8_t  wheel;
} hub_ring_report_t;

// Callback when a ring sends a HID report
typedef void (*hub_ring_report_cb_t)(uint8_t ring_index,
                                     const hub_ring_report_t *report,
                                     void *arg);

// Callback when a ring connects or disconnects
typedef void (*hub_ring_conn_cb_t)(uint8_t ring_index, bool connected, void *arg);

// Initialize BLE central. Starts scanning for PowerFinger peripherals.
hal_status_t ble_central_init(hub_ring_report_cb_t report_cb,
                              hub_ring_conn_cb_t conn_cb,
                              void *arg);

// Get the BLE MAC address of a connected ring (for role engine)
hal_status_t ble_central_get_mac(uint8_t ring_index, uint8_t mac_out[6]);

// Get number of currently connected rings
uint8_t ble_central_connected_count(void);
