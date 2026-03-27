// SPDX-License-Identifier: MIT
// PowerFinger Hub — Role engine interface
//
// Maps each connected ring's BLE MAC address to a role.
// Roles determine how ring events map to USB HID report fields.

#pragma once

#include "hal_types.h"
#include <stdint.h>

typedef enum {
    ROLE_CURSOR,    // X/Y → cursor, click → left click
    ROLE_SCROLL,    // X/Y → scroll, click → right click
    ROLE_MODIFIER,  // X/Y ignored, click → middle click
    ROLE_COUNT,
} ring_role_t;

// Initialize role engine. Loads saved roles from NVS.
// Default: first ring = CURSOR, second = SCROLL.
hal_status_t role_engine_init(void);

// Get the role for a ring by its MAC address.
// Returns ROLE_CURSOR as default if MAC is not assigned.
ring_role_t role_engine_get_role(const uint8_t mac[6]);

// Reassign a ring's role. Atomic — no intermediate state.
// Persists to NVS during idle.
hal_status_t role_engine_set_role(const uint8_t mac[6], ring_role_t role);

// Remove a ring's role assignment (frees a slot for a new ring).
// Call when a ring is physically replaced or factory-reset.
hal_status_t role_engine_forget(const uint8_t mac[6]);

// Get role name as string (for logging)
const char *role_engine_role_name(ring_role_t role);
