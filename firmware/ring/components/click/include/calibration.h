// SPDX-License-Identifier: MIT
// PowerFinger — Boot-time sensor calibration
//
// Captures zero-offset from sensor at startup so drift from mounting
// angle doesn't bias the cursor.

#pragma once

#include "hal_types.h"
#include <stdbool.h>
#include <stdint.h>

// Run calibration. Blocks until complete or failed.
// On success, stores offsets internally. On failure (device was moving),
// uses zero offset and returns HAL_ERR_TIMEOUT after max retries.
hal_status_t calibration_run(void);

// Run one calibration sampling pass with no built-in retries.
// Used for late calibration attempts after sensor recovery so the main loop
// can bound the amount of blocking work per retry.
hal_status_t calibration_attempt_once(void);

// Clear stored offsets and mark calibration invalid.
void calibration_reset(void);

// Returns true when stored offsets come from a successful calibration pass.
bool calibration_is_valid(void);

// Apply calibration offset to a sensor reading (subtract offset).
void calibration_apply(int16_t *dx, int16_t *dy);

// Get calibration offsets (for diagnostics)
void calibration_get_offsets(int16_t *out_offset_x, int16_t *out_offset_y);
