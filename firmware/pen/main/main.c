// SPDX-License-Identifier: MIT
// PowerPen Firmware — Entry Point
//
// Reuses the shared ring application flow while enabling the pen-specific
// dual-click behavior through form-factor gated shared code.

#define POWERFINGER_APP_LOG_TAG "powerpen"
#define POWERFINGER_APP_FIRMWARE_NAME "PowerPen"

#include "../../ring/main/main.c"
