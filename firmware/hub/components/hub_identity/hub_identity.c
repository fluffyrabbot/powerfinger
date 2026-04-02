// SPDX-License-Identifier: MIT
// PowerFinger Hub — Identity helpers

#include "hub_identity.h"

#define HUB_IDENTITY_STR_HELPER(x) #x
#define HUB_IDENTITY_STR(x) HUB_IDENTITY_STR_HELPER(x)

static const char *s_firmware_revision =
    HUB_IDENTITY_STR(HUB_IDENTITY_FW_VERSION_MAJOR) "."
    HUB_IDENTITY_STR(HUB_IDENTITY_FW_VERSION_MINOR) "."
    HUB_IDENTITY_STR(HUB_IDENTITY_FW_VERSION_PATCH);

const char *hub_identity_firmware_revision(void)
{
    return s_firmware_revision;
}

const char *hub_identity_hardware_revision(void)
{
    return HUB_IDENTITY_HARDWARE_REVISION;
}

void hub_identity_firmware_version(uint8_t out_version[3])
{
    if (!out_version) {
        return;
    }

    out_version[0] = HUB_IDENTITY_FW_VERSION_MAJOR;
    out_version[1] = HUB_IDENTITY_FW_VERSION_MINOR;
    out_version[2] = HUB_IDENTITY_FW_VERSION_PATCH;
}
