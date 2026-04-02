// SPDX-License-Identifier: MIT
// PowerFinger — Mock HAL for host-side unit testing
//
// Provides stub implementations of HAL functions that record calls
// and return configurable values for failure injection.

#pragma once

#include "hal_types.h"
#include <stdint.h>

// Reset all mock state between tests
void mock_hal_reset(void);

// --- Timer mock ---
void mock_hal_set_time_ms(uint32_t ms);
void mock_hal_advance_time_ms(uint32_t ms);

// --- ADC mock (for battery monitoring tests) ---
void mock_hal_set_adc_mv(uint32_t mv);
void mock_hal_set_adc_status(hal_status_t status);

// --- Storage mock (for role engine persistence tests) ---
void mock_hal_storage_seed(const char *key, const void *data, size_t len);
void mock_hal_inject_storage_set_failure(hal_status_t status, int count);
void mock_hal_inject_storage_commit_failure(hal_status_t status, int count);

// --- Hub BLE central mock (for hub control / companion protocol tests) ---
void mock_ble_central_clear_connected_rings(void);
void mock_ble_central_set_connected_ring(uint8_t ring_index, const uint8_t mac[6]);
void mock_ble_central_clear_bonds(void);
void mock_ble_central_seed_bond(const uint8_t mac[6]);
bool mock_ble_central_has_bond(const uint8_t mac[6]);

// --- Failure injection ---
// Set the return value for a specific HAL module's next N calls.
// After N calls, reverts to HAL_OK.
void mock_hal_inject_ble_send_failure(hal_status_t status, int count);
void mock_hal_set_ble_conn_param_status(hal_status_t status);
int mock_hal_get_ble_conn_param_request_count(void);
void mock_hal_get_last_ble_conn_param_request(uint16_t *min_1_25ms,
                                              uint16_t *max_1_25ms);
