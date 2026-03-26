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

// Initialize the dead zone state machine
void dead_zone_init(void);

// Called every tick with the current click state and raw sensor deltas.
// Modifies dx/dy in place (zeros them if suppressed).
// Returns true if dead zone is currently active (deltas suppressed).
bool dead_zone_update(bool click_pressed, int16_t *dx, int16_t *dy,
                      uint32_t now_ms);

// Reset dead zone state (e.g. on disconnect)
void dead_zone_reset(void);
