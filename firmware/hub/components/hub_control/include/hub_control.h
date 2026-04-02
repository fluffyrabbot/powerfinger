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
