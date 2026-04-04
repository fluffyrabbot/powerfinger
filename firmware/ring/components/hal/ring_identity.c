// SPDX-License-Identifier: MIT
// PowerFinger — Ring identity helpers

#include "ring_identity.h"

#include <stdio.h>

#define RING_IDENTITY_STR_HELPER(x) #x
#define RING_IDENTITY_STR(x) RING_IDENTITY_STR_HELPER(x)

static const char *s_firmware_revision =
    RING_IDENTITY_STR(RING_IDENTITY_FW_VERSION_MAJOR) "."
    RING_IDENTITY_STR(RING_IDENTITY_FW_VERSION_MINOR) "."
    RING_IDENTITY_STR(RING_IDENTITY_FW_VERSION_PATCH);

#if defined(CONFIG_POWERFINGER_FORM_FACTOR_PEN)
#define RING_IDENTITY_MODEL_PREFIX "Pen"
#elif defined(CONFIG_POWERFINGER_FORM_FACTOR_PUCK)
#define RING_IDENTITY_MODEL_PREFIX "Puck"
#else
#define RING_IDENTITY_MODEL_PREFIX "Ring"
#endif

const char *ring_identity_model_number(void)
{
#if defined(CONFIG_SENSOR_NONE) && defined(CONFIG_CLICK_NONE)
    return RING_IDENTITY_MODEL_PREFIX "-Dev";
#elif defined(CONFIG_SENSOR_PMW3360) || defined(CONFIG_CLICK_PIEZO_LRA)
    return RING_IDENTITY_MODEL_PREFIX "-P";
#else
    return RING_IDENTITY_MODEL_PREFIX "-S";
#endif
}

const char *ring_identity_firmware_revision(void)
{
    return s_firmware_revision;
}

const char *ring_identity_hardware_revision(void)
{
    return RING_IDENTITY_HARDWARE_REVISION;
}

void ring_identity_firmware_version(uint8_t out_version[3])
{
    if (!out_version) {
        return;
    }

    out_version[0] = RING_IDENTITY_FW_VERSION_MAJOR;
    out_version[1] = RING_IDENTITY_FW_VERSION_MINOR;
    out_version[2] = RING_IDENTITY_FW_VERSION_PATCH;
}

hal_status_t ring_identity_format_serial(const uint8_t mac[6],
                                         char *out_serial,
                                         size_t out_len)
{
    if (!mac || !out_serial || out_len < 13) {
        return HAL_ERR_INVALID_ARG;
    }

    int written = snprintf(out_serial,
                           out_len,
                           "%02X%02X%02X%02X%02X%02X",
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (written != 12) {
        return HAL_ERR_IO;
    }

    return HAL_OK;
}
