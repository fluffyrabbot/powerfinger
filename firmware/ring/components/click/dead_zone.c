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

static void dead_zone_reset_state(dead_zone_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->state = (uint8_t)DZ_IDLE;
    ctx->click_start_ms = 0;
    ctx->accumulated_distance = 0;
}

void dead_zone_init(dead_zone_ctx_t *ctx)
{
    dead_zone_reset_state(ctx);
}

bool dead_zone_update(dead_zone_ctx_t *ctx, bool click_pressed, int16_t *dx, int16_t *dy,
                      uint32_t now_ms)
{
    return dead_zone_update_with_config(ctx,
                                        click_pressed,
                                        dx,
                                        dy,
                                        now_ms,
                                        DEAD_ZONE_TIME_MS,
                                        DEAD_ZONE_DISTANCE);
}

bool dead_zone_update_with_config(dead_zone_ctx_t *ctx, bool click_pressed,
                                  int16_t *dx, int16_t *dy,
                                  uint32_t now_ms, uint16_t dead_zone_time_ms,
                                  uint8_t dead_zone_distance)
{
    if (!ctx || !dx || !dy) return false;

    dz_state_t state = (dz_state_t)ctx->state;

    switch (state) {
    case DZ_IDLE:
        if (click_pressed) {
            ctx->state = (uint8_t)DZ_ACTIVE;
            ctx->click_start_ms = now_ms;
            ctx->accumulated_distance = 0;
            // Suppress this tick's deltas
            *dx = 0;
            *dy = 0;
            return true;
        }
        return false;

    case DZ_ACTIVE: {
        if (!click_pressed) {
            // Click released while still in dead zone — normal click complete
            ctx->state = (uint8_t)DZ_IDLE;
            *dx = 0;
            *dy = 0;
            return false;
        }

        // Accumulate distance
        ctx->accumulated_distance += (uint32_t)(abs(*dx) + abs(*dy));

        uint32_t elapsed = now_ms - ctx->click_start_ms;
        bool time_met = (elapsed >= dead_zone_time_ms);
        bool distance_met = (ctx->accumulated_distance >= dead_zone_distance);

        if (time_met && distance_met) {
            // Both exit conditions met — transition to drag mode
            ctx->state = (uint8_t)DZ_DRAG;
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
            ctx->state = (uint8_t)DZ_IDLE;
        }
        // Deltas flow through in drag mode
        return false;
    }

    return false;
}

void dead_zone_reset(dead_zone_ctx_t *ctx)
{
    dead_zone_reset_state(ctx);
}
