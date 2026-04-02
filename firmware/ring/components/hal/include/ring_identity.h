// SPDX-License-Identifier: MIT
// PowerFinger — Ring identity helpers
//
// Keeps user-visible device identity in one place so Device Information,
// custom config services, and logs all agree.

#pragma once

#include "hal_types.h"

#define RING_IDENTITY_FW_VERSION_MAJOR 0
#define RING_IDENTITY_FW_VERSION_MINOR 1
#define RING_IDENTITY_FW_VERSION_PATCH 0
#define RING_IDENTITY_HARDWARE_REVISION "DEVBOARD-C3"

const char *ring_identity_model_number(void);
const char *ring_identity_firmware_revision(void);
const char *ring_identity_hardware_revision(void);
void ring_identity_firmware_version(uint8_t out_version[3]);

// Formats a stable serial from a 6-byte MAC as 12 uppercase hex digits.
hal_status_t ring_identity_format_serial(const uint8_t mac[6],
                                         char *out_serial,
                                         size_t out_len);
