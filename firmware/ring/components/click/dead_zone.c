// SPDX-License-Identifier: MIT
// PowerFinger — Dead zone state machine implementation

#include "dead_zone.h"
#include "ring_config.h"
#include <stdlib.h>

typedef enum {
    DZ_IDLE,        // No click, no suppression
    DZ_ACTIVE,      // Click pressed, deltas suppressed
    DZ_DRAG,        // Click held past dead zone exit — deltas re-enabled
} dz_state_t;

static dz_state_t s_state;
static uint32_t s_click_start_ms;
static uint32_t s_accumulated_distance;

void dead_zone_init(void)
{
    s_state = DZ_IDLE;
    s_click_start_ms = 0;
    s_accumulated_distance = 0;
}

bool dead_zone_update(bool click_pressed, int16_t *dx, int16_t *dy,
                      uint32_t now_ms)
{
    return dead_zone_update_with_config(click_pressed,
                                        dx,
                                        dy,
                                        now_ms,
                                        DEAD_ZONE_TIME_MS,
                                        DEAD_ZONE_DISTANCE);
}

bool dead_zone_update_with_config(bool click_pressed, int16_t *dx, int16_t *dy,
                                  uint32_t now_ms, uint16_t dead_zone_time_ms,
                                  uint8_t dead_zone_distance)
{
    if (!dx || !dy) return false;

    switch (s_state) {
    case DZ_IDLE:
        if (click_pressed) {
            s_state = DZ_ACTIVE;
            s_click_start_ms = now_ms;
            s_accumulated_distance = 0;
            // Suppress this tick's deltas
            *dx = 0;
            *dy = 0;
            return true;
        }
        return false;

    case DZ_ACTIVE: {
        if (!click_pressed) {
            // Click released while still in dead zone — normal click complete
            s_state = DZ_IDLE;
            *dx = 0;
            *dy = 0;
            return false;
        }

        // Accumulate distance
        s_accumulated_distance += (uint32_t)(abs(*dx) + abs(*dy));

        uint32_t elapsed = now_ms - s_click_start_ms;
        bool time_met = (elapsed >= dead_zone_time_ms);
        bool distance_met = (s_accumulated_distance >= dead_zone_distance);

        if (time_met && distance_met) {
            // Both exit conditions met — transition to drag mode
            s_state = DZ_DRAG;
            // Re-enable deltas (don't suppress this tick)
            return false;
        }

        // Still in dead zone — suppress
        *dx = 0;
        *dy = 0;
        return true;
    }

    case DZ_DRAG:
        if (!click_pressed) {
            // Drag complete
            s_state = DZ_IDLE;
        }
        // Deltas flow through in drag mode
        return false;
    }

    return false;
}

void dead_zone_reset(void)
{
    dead_zone_init();
}
