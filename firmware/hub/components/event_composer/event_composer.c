// SPDX-License-Identifier: MIT
// PowerFinger Hub — Event composer implementation
//
// Combines HID reports from up to 4 rings into a single USB HID report.
//
// Thread safety: feed() and ring_disconnected() are called from the NimBLE
// task; compose() is called from the main loop. A spinlock protects all
// access to s_rings[] to prevent torn reads on ESP32-S3 dual-core.
//
// Critical invariant: on disconnect, immediately release all buttons from
// that ring and zero its delta contributions. A stuck button is an
// accessibility hazard — a user with limited mobility may not be able to
// resolve it without assistance.

#include "event_composer.h"
#include "ble_central.h"

#include <string.h>
#include <limits.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "composer";
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
#define LOCK()   portENTER_CRITICAL(&s_lock)
#define UNLOCK() portEXIT_CRITICAL(&s_lock)
#else
#define LOCK()   (void)0
#define UNLOCK() (void)0
#endif

// Saturating int32_t -> int16_t clamp
static inline int16_t sat_add_i16(int16_t a, int8_t b)
{
    int32_t sum = (int32_t)a + (int32_t)b;
    if (sum > INT16_MAX) return INT16_MAX;
    if (sum < INT16_MIN) return INT16_MIN;
    return (int16_t)sum;
}

// Clamp int16_t to int8_t range for scroll output
static inline int8_t clamp_i8(int16_t v)
{
    if (v > 127) return 127;
    if (v < -127) return -127;
    return (int8_t)v;
}

// Per-ring state
typedef struct {
    uint8_t buttons;
    int16_t acc_dx;     // accumulated X delta since last compose
    int16_t acc_dy;     // accumulated Y delta since last compose
    ring_role_t role;
    bool connected;
} ring_state_t;

static ring_state_t s_rings[HUB_MAX_RINGS];

hal_status_t event_composer_init(void)
{
    memset(s_rings, 0, sizeof(s_rings));
    return HAL_OK;
}

static void clear_ring_state(ring_state_t *r)
{
    r->buttons = 0;
    r->acc_dx = 0;
    r->acc_dy = 0;
}

void event_composer_mark_connected(uint8_t ring_index, ring_role_t role)
{
    if (ring_index >= HUB_MAX_RINGS) return;
    LOCK();
    s_rings[ring_index].connected = true;
    s_rings[ring_index].role = role;
    clear_ring_state(&s_rings[ring_index]);
    UNLOCK();
}

void event_composer_set_role(uint8_t ring_index, ring_role_t role)
{
    if (ring_index >= HUB_MAX_RINGS || role >= ROLE_COUNT) return;

    LOCK();
    ring_state_t *r = &s_rings[ring_index];
    if (!r->connected) {
        UNLOCK();
        return;
    }

    if (r->role != role) {
        // If a live role changes while a button is held, the mapping changes
        // (CURSOR bit0=left, SCROLL bit0=right, MODIFIER bit0=middle). Clear
        // buffered state so the host never sees a silent left->right remap.
        clear_ring_state(r);
        r->role = role;
    }
    UNLOCK();
}

void event_composer_swap_roles(uint8_t ring_index_a,
                               ring_role_t role_a,
                               uint8_t ring_index_b,
                               ring_role_t role_b)
{
    if (ring_index_a >= HUB_MAX_RINGS || ring_index_b >= HUB_MAX_RINGS ||
        ring_index_a == ring_index_b ||
        role_a >= ROLE_COUNT || role_b >= ROLE_COUNT) {
        return;
    }

    LOCK();
    ring_state_t *a = &s_rings[ring_index_a];
    ring_state_t *b = &s_rings[ring_index_b];
    if (!a->connected || !b->connected) {
        UNLOCK();
        return;
    }

    clear_ring_state(a);
    clear_ring_state(b);
    a->role = role_a;
    b->role = role_b;
    UNLOCK();
}

void event_composer_feed(uint8_t ring_index, uint8_t buttons, int8_t dx, int8_t dy)
{
    if (ring_index >= HUB_MAX_RINGS) return;

    LOCK();
    ring_state_t *r = &s_rings[ring_index];
    // Only accept data from connected rings — a stale BLE notification
    // arriving after disconnect must not re-enable the ring.
    if (!r->connected) {
        UNLOCK();
        return;
    }
    r->buttons = buttons;
    r->acc_dx = sat_add_i16(r->acc_dx, dx);
    r->acc_dy = sat_add_i16(r->acc_dy, dy);
    UNLOCK();
}

void event_composer_ring_disconnected(uint8_t ring_index)
{
    if (ring_index >= HUB_MAX_RINGS) return;

    LOCK();
    ring_state_t *r = &s_rings[ring_index];

#ifdef ESP_PLATFORM
    if (r->buttons != 0) {
        ESP_LOGW(TAG, "ring %d disconnected with buttons=0x%02X, releasing",
                 ring_index, r->buttons);
    }
#endif

    // Immediately release all buttons and zero deltas
    clear_ring_state(r);
    r->connected = false;
    UNLOCK();
}

void event_composer_compose(composed_report_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));

    // Accumulate in wider types to prevent overflow when composing
    // multiple rings with the same role (e.g. two CURSOR rings both at INT16_MAX)
    int32_t cursor_dx_acc = 0;
    int32_t cursor_dy_acc = 0;
    int32_t scroll_h_acc = 0;
    int32_t scroll_v_acc = 0;

    LOCK();
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        ring_state_t *r = &s_rings[i];
        if (!r->connected) continue;

        switch (r->role) {
        case ROLE_CURSOR:
            out->buttons |= (r->buttons & 0x01);
            cursor_dx_acc += r->acc_dx;
            cursor_dy_acc += r->acc_dy;
            break;

        case ROLE_SCROLL:
            if (r->buttons & 0x01) {
                out->buttons |= 0x02;
            }
            scroll_h_acc += r->acc_dx;
            scroll_v_acc += r->acc_dy;
            break;

        case ROLE_MODIFIER:
            if (r->buttons & 0x01) {
                out->buttons |= 0x04;
            }
            break;

        default:
            break;
        }

        // Clear accumulators after composing
        r->acc_dx = 0;
        r->acc_dy = 0;
    }
    UNLOCK();

    // Clamp cursor to int16_t after accumulating across all rings
    if (cursor_dx_acc > INT16_MAX) cursor_dx_acc = INT16_MAX;
    if (cursor_dx_acc < INT16_MIN) cursor_dx_acc = INT16_MIN;
    if (cursor_dy_acc > INT16_MAX) cursor_dy_acc = INT16_MAX;
    if (cursor_dy_acc < INT16_MIN) cursor_dy_acc = INT16_MIN;
    out->cursor_dx = (int16_t)cursor_dx_acc;
    out->cursor_dy = (int16_t)cursor_dy_acc;

    // Clamp scroll to int8_t after accumulating across all rings
    out->scroll_h = clamp_i8(scroll_h_acc);
    out->scroll_v = clamp_i8(scroll_v_acc);
}
