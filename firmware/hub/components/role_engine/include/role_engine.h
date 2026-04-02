// SPDX-License-Identifier: MIT
// PowerFinger Hub — Role engine interface
//
// Maps each connected ring's BLE MAC address to a role.
// Roles determine how ring events map to USB HID report fields.

#pragma once

#include "hal_types.h"
#include <stdint.h>
#include <stddef.h>

typedef enum {
    ROLE_CURSOR,    // X/Y → cursor, click → left click
    ROLE_SCROLL,    // X/Y → scroll, click → right click
    ROLE_MODIFIER,  // X/Y ignored, click → middle click
    ROLE_COUNT,
} ring_role_t;

typedef struct {
    uint8_t mac[6];
    ring_role_t role;
} role_engine_entry_t;

// Initialize role engine. Loads saved roles from NVS.
// Default: first ring = CURSOR, second = SCROLL.
hal_status_t role_engine_init(void);

// Get the role for a ring by its MAC address.
// Returns ROLE_CURSOR as default if MAC is not assigned.
ring_role_t role_engine_get_role(const uint8_t mac[6]);

// Bulk-read all persisted role assignments in insertion order.
// Pass entries_out=NULL and max_entries=0 to query the current count only.
hal_status_t role_engine_get_all(role_engine_entry_t *entries_out,
                                 size_t max_entries,
                                 size_t *count_out);

// Reassign a ring's role. Atomic — no intermediate state.
// Persists asynchronously via the role engine's background flush path.
hal_status_t role_engine_set_role(const uint8_t mac[6], ring_role_t role);

// Swap the roles for two known rings under one mutex acquisition.
// This avoids the transient double-role state that two sequential set_role
// calls would create during a companion-driven swap.
hal_status_t role_engine_swap(const uint8_t mac_a[6], const uint8_t mac_b[6]);

// Remove a ring's role assignment (frees a slot for a new ring).
// Call when a ring is physically replaced or factory-reset.
hal_status_t role_engine_forget(const uint8_t mac[6]);

// Get role name as string (for logging)
const char *role_engine_role_name(ring_role_t role);

// Flush any pending role changes to NVS.
// On ESP targets this is normally handled by the role engine's background
// worker task; this entry point remains for host-side tests and non-RTOS paths.
void role_engine_flush_if_dirty(void);
