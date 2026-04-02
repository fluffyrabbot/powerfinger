// SPDX-License-Identifier: MIT
// PowerFinger Hub — Event composer interface
//
// Combines HID reports from multiple rings into a single USB HID report.
// Critical invariant: on disconnect, immediately release all buttons from
// that ring. A stuck button is an accessibility hazard.

#pragma once

#include "hal_types.h"
#include "role_engine.h"
#include <stdint.h>

// Composed USB HID report (output to host)
typedef struct {
    uint8_t buttons;    // OR of all connected rings' buttons per role
    int16_t cursor_dx;  // SUM of cursor-role ring deltas
    int16_t cursor_dy;
    int8_t  scroll_v;   // SUM of scroll-role ring Y deltas
    int8_t  scroll_h;   // SUM of scroll-role ring X deltas
} composed_report_t;

// Initialize event composer
hal_status_t event_composer_init(void);

// Mark a ring as connected (called from connection callback only).
// The connected flag is managed exclusively by mark_connected / ring_disconnected.
// feed() does NOT set connected — prevents stale BLE notifications from
// resurrecting a disconnected ring.
void event_composer_mark_connected(uint8_t ring_index, ring_role_t role);

// Update a connected ring's cached role.
// On role change, buttons and accumulated deltas are cleared so a held click
// cannot change meaning across roles (e.g. left -> right) without a release.
void event_composer_set_role(uint8_t ring_index, ring_role_t role);

// Atomically update the cached roles for two connected rings.
// Both rings have buttons and buffered deltas cleared under one lock so a
// companion-driven swap does not leave an intermediate mixed-role window.
void event_composer_swap_roles(uint8_t ring_index_a,
                               ring_role_t role_a,
                               uint8_t ring_index_b,
                               ring_role_t role_b);

// Feed a ring's HID report into the composer.
// ring_index: which ring (0-3)
// buttons, dx, dy: from the ring's BLE HID notification
void event_composer_feed(uint8_t ring_index, uint8_t buttons, int8_t dx, int8_t dy);

// Handle ring disconnection. Immediately releases all buttons and
// zeroes deltas contributed by this ring.
void event_composer_ring_disconnected(uint8_t ring_index);

// Compose the final report (called at USB poll rate, ~1ms).
// Clears accumulated deltas after composing.
void event_composer_compose(composed_report_t *out);
