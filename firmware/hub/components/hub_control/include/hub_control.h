// SPDX-License-Identifier: MIT
// PowerFinger Hub — Control helpers
//
// Companion commands should mutate hub state through this module so persistent
// role changes and live event-composer cache updates stay aligned.

#pragma once

#include "role_engine.h"

// Reassign a ring's role by MAC and, if the ring is currently connected, update
// the event-composer cache immediately so subsequent reports use the new role.
hal_status_t hub_control_set_role(const uint8_t mac[6], ring_role_t role);

// Swap the roles for two known rings. If either ring is currently connected,
// the event-composer cache is updated to the swapped roles as part of the same
// control operation.
hal_status_t hub_control_swap_roles(const uint8_t mac_a[6], const uint8_t mac_b[6]);

// Forget a ring by MAC, dropping live input immediately if the ring is
// connected, requesting disconnect, deleting the current bond entry, and
// removing the persisted role assignment.
hal_status_t hub_control_forget_ring(const uint8_t mac[6]);
