// SPDX-License-Identifier: MIT
// PowerPuck Firmware - Entry Point

#include "app_runtime.h"

static const powerfinger_app_config_t s_puck_app_config = {
    .log_tag = "powerpuck",
    .firmware_name = "PowerPuck",
};

void app_main(void)
{
    powerfinger_app_main(&s_puck_app_config);
}
