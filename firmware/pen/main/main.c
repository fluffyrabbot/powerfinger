// SPDX-License-Identifier: MIT
// PowerPen Firmware - Entry Point

#include "app_runtime.h"

static const powerfinger_app_config_t s_pen_app_config = {
    .log_tag = "powerpen",
    .firmware_name = "PowerPen",
};

void app_main(void)
{
    powerfinger_app_main(&s_pen_app_config);
}
