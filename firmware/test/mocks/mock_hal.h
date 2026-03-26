// SPDX-License-Identifier: MIT
// PowerFinger — Mock HAL for host-side unit testing
//
// Provides stub implementations of HAL functions that record calls
// and return configurable values.

#pragma once

#include "hal_types.h"

// Reset all mock state between tests
void mock_hal_reset(void);

// --- Timer mock ---
// Set the "current time" returned by hal_timer_get_ms()
void mock_hal_set_time_ms(uint32_t ms);
void mock_hal_advance_time_ms(uint32_t ms);
