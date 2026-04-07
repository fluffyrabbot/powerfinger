// SPDX-License-Identifier: MIT
// PowerPuck Firmware — Entry Point
//
// Reuses the shared ring application flow while enabling the puck-specific
// desktop form factor through Kconfig-gated shared code.
// Structurally identical to the ring entry point — only log strings
// and Kconfig defaults (device name, sleep timeout) differ.

#define POWERFINGER_APP_LOG_TAG "powerpuck"
#define POWERFINGER_APP_FIRMWARE_NAME "PowerPuck"

#include "../../ring/main/main.c"
