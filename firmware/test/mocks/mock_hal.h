// SPDX-License-Identifier: MIT
// PowerFinger — Mock HAL for host-side unit testing
//
// Provides stub implementations of HAL functions that record calls
// and return configurable values for failure injection.

#pragma once

#include "hal_types.h"

// Reset all mock state between tests
void mock_hal_reset(void);

// --- Timer mock ---
void mock_hal_set_time_ms(uint32_t ms);
void mock_hal_advance_time_ms(uint32_t ms);

// --- ADC mock (for battery monitoring tests) ---
void mock_hal_set_adc_mv(uint32_t mv);

// --- Failure injection ---
// Set the return value for a specific HAL module's next N calls.
// After N calls, reverts to HAL_OK.
void mock_hal_inject_ble_send_failure(hal_status_t status, int count);
