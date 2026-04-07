// SPDX-License-Identifier: MIT
// PowerFinger Ring Firmware - Entry Point

#include "app_runtime.h"

static const powerfinger_app_config_t s_ring_app_config = {
    .log_tag = "powerfinger",
    .firmware_name = "PowerFinger ring",
};

void app_main(void)
{
    powerfinger_app_main(&s_ring_app_config);
}
