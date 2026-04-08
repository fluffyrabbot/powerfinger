// SPDX-License-Identifier: MIT
// PowerFinger Hub — Control helpers
//
// Companion commands should mutate hub state through this module so persistent
// role changes and live event-composer cache updates stay aligned.

#pragma once

#include "gesture_engine.h"
#include "hub_settings.h"
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

// Update one hub-owned gesture mapping and keep the live event-composer cache
// aligned with the persisted gesture table.
hal_status_t hub_control_set_gesture(uint8_t trigger, gesture_action_t action);

// Update one persisted hub setting and apply the live side effects that can be
// honored immediately by the current firmware.
hal_status_t hub_control_set_hub_setting(hub_settings_param_t param, uint8_t value);
