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

// NimBLE may retain an update procedure for up to 40 seconds. Do not submit
// another request before this deadline after a successful HAL submission: a
// late completion must not be correlated with a newer request.
#define BLE_CONN_PARAM_UPDATE_TIMEOUT_MS 40000U
// Three transient submission retries per connection, with exponential backoff.
#define BLE_CONN_PARAM_RETRY_DELAY_MS    250U
#define BLE_CONN_PARAM_MAX_RETRIES       3U

// Advertising timeout before giving up and entering deep sleep
#define BLE_ADVERTISE_TIMEOUT_MS    60000

// Device name — override via Kconfig POWERFINGER_DEVICE_NAME per form factor
#ifdef CONFIG_POWERFINGER_DEVICE_NAME
#define BLE_DEVICE_NAME             CONFIG_POWERFINGER_DEVICE_NAME
#else
#define BLE_DEVICE_NAME             "PowerFinger"
#endif
