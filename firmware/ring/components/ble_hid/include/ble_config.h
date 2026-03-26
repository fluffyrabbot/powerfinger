// SPDX-License-Identifier: MIT
// PowerFinger — BLE HID configuration constants

#pragma once

// Connection interval units are 1.25ms
#define BLE_CONN_ITVL_7_5MS         6    // 6 * 1.25ms = 7.5ms
#define BLE_CONN_ITVL_15MS          12   // 12 * 1.25ms = 15ms

// Default connection parameters (idle — allows inter-event light sleep)
#define BLE_CONN_ITVL_MIN_DEFAULT   BLE_CONN_ITVL_15MS
#define BLE_CONN_ITVL_MAX_DEFAULT   BLE_CONN_ITVL_15MS

// Active tracking parameters (low latency)
#define BLE_CONN_ITVL_MIN_ACTIVE    BLE_CONN_ITVL_7_5MS
#define BLE_CONN_ITVL_MAX_ACTIVE    BLE_CONN_ITVL_7_5MS

// Supervision timeout: 4 seconds (units of 10ms)
#define BLE_SUPERVISION_TIMEOUT     400

// Advertising timeout before giving up and entering deep sleep
#define BLE_ADVERTISE_TIMEOUT_MS    60000

// Device name
#define BLE_DEVICE_NAME             "PowerFinger"
