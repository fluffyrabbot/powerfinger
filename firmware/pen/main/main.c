// SPDX-License-Identifier: MIT
// PowerPen Firmware — Entry Point
//
// PP2 reuses the shared single-input ring application flow. PP3 will replace
// this wrapper with pen-specific dual-click handling on top of the shared
// interfaces added in PP1.

#define POWERFINGER_APP_LOG_TAG "powerpen"
#define POWERFINGER_APP_FIRMWARE_NAME "PowerPen"

#include "../../ring/main/main.c"
