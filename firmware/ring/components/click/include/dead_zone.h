// SPDX-License-Identifier: MIT
// PowerFinger — Dead zone state machine
//
// Suppresses cursor movement during click actuation to prevent
// click-induced micro-movements from dragging the cursor.
//
// Dead zone exit requires BOTH conditions:
//   1. At least DEAD_ZONE_TIME_MS since click actuation
//   2. Accumulated distance >= DEAD_ZONE_DISTANCE since click actuation
//
// When both conditions are met and the button is still held, deltas
// are re-enabled for intentional click-and-drag.

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t state;
    uint32_t click_start_ms;
    uint32_t accumulated_distance;
} dead_zone_ctx_t;

// Initialize the dead zone state machine
void dead_zone_init(dead_zone_ctx_t *ctx);

// Called every tick with the current click state and raw sensor deltas.
// Modifies dx/dy in place (zeros them if suppressed).
// Returns true if dead zone is currently active (deltas suppressed).
bool dead_zone_update(dead_zone_ctx_t *ctx, bool click_pressed, int16_t *dx, int16_t *dy,
                      uint32_t now_ms);

// Same state machine, but with caller-supplied thresholds so runtime BLE
// settings can override the compiled defaults.
bool dead_zone_update_with_config(dead_zone_ctx_t *ctx, bool click_pressed,
                                  int16_t *dx, int16_t *dy,
                                  uint32_t now_ms, uint16_t dead_zone_time_ms,
                                  uint8_t dead_zone_distance);

// Reset dead zone state (e.g. on disconnect)
void dead_zone_reset(dead_zone_ctx_t *ctx);
