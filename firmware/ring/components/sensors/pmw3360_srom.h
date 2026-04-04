// SPDX-License-Identifier: MIT
// PowerFinger — PMW3360 SROM firmware blob
//
// The PMW3360 requires a firmware upload on every power-on. The blob is
// published by PixArt and used in open-source projects (QMK, Ploopy, etc.).
//
// SROM version 0x04 (4), 4094 bytes.
//
// Source: PixArt PMW3360DM-T2QU reference design package.
// This is sensor microcode, not application software. It is freely
// redistributable and does not create a proprietary dependency.

#pragma once

#include <stdint.h>

#define PMW3360_SROM_LENGTH  4094

// The actual SROM blob. In a production build, this array contains the
// 4094-byte firmware from the PixArt reference package. For initial
// development and host-side testing, a zero-filled placeholder is used.
// Replace with the real blob before flashing to hardware.
//
// To obtain the real blob:
//   1. Download from PixArt via NDA (free for production use), or
//   2. Extract from QMK: keyboards/ploopyco/trackball/pmw3360_srom.h
//   3. Extract from Ploopy: firmware/pmw3360_srom_0x04.h
//
// The blob is NOT included in this repository to avoid any ambiguity
// about redistribution rights. Copy it into this array before building
// with CONFIG_SENSOR_PMW3360=y for hardware targets.

static const uint8_t pmw3360_srom_data[PMW3360_SROM_LENGTH] = {
    // PLACEHOLDER: Replace with actual SROM blob before hardware testing.
    // The sensor will report SROM_ID = 0x00 with this placeholder,
    // and sensor_init() will return HAL_ERR_IO on SROM verification failure.
    0
};
