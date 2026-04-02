// SPDX-License-Identifier: MIT
// PowerFinger Hub — Identity helpers
//
// Keeps hub firmware and hardware revision strings in one place so logs,
// companion readback, and future transport surfaces do not drift.

#pragma once

#include "hal_types.h"

#define HUB_IDENTITY_FW_VERSION_MAJOR 0
#define HUB_IDENTITY_FW_VERSION_MINOR 1
#define HUB_IDENTITY_FW_VERSION_PATCH 0
#define HUB_IDENTITY_HARDWARE_REVISION "DEVBOARD-S3"

const char *hub_identity_firmware_revision(void);
const char *hub_identity_hardware_revision(void);
void hub_identity_firmware_version(uint8_t out_version[3]);
