// SPDX-License-Identifier: MIT
// PowerFinger — PMW3360 optical-on-ball sensor driver (internal header)
//
// The PMW3360 (PixArt) uses standard SPI Mode 3 (CPOL=1, CPHA=1).
// It requires a firmware (SROM) blob upload on every power-on.
//
// Reference: PMW3360DM-T2QU datasheet, PixArt application notes.
// SROM blob from PixArt reference firmware (same blob used by QMK, Ploopy).

#pragma once

#include <stdint.h>

// --- Register addresses ---

#define PMW3360_REG_PRODUCT_ID       0x00  // Expected: 0x42
#define PMW3360_REG_REVISION_ID      0x01
#define PMW3360_REG_MOTION           0x02  // Bit 7 = motion flag
#define PMW3360_REG_DELTA_X_L        0x03  // Low byte, signed 16-bit
#define PMW3360_REG_DELTA_X_H        0x04  // High byte
#define PMW3360_REG_DELTA_Y_L        0x05  // Low byte, signed 16-bit
#define PMW3360_REG_DELTA_Y_H        0x06  // High byte
#define PMW3360_REG_SQUAL            0x07  // Surface quality (0-255)
#define PMW3360_REG_RAW_DATA_SUM     0x08
#define PMW3360_REG_MAX_RAW_DATA     0x09
#define PMW3360_REG_MIN_RAW_DATA     0x0A
#define PMW3360_REG_SHUTTER_LOWER    0x0B
#define PMW3360_REG_SHUTTER_UPPER    0x0C
#define PMW3360_REG_CONFIG1          0x0F  // CPI low byte: (CPI/100) - 1
#define PMW3360_REG_CONFIG2          0x10  // CPI high byte + REST mode
#define PMW3360_REG_ANGLE_TUNE       0x11
#define PMW3360_REG_FRAME_CAPTURE    0x12
#define PMW3360_REG_SROM_ENABLE      0x13
#define PMW3360_REG_RUN_DOWNSHIFT    0x14
#define PMW3360_REG_REST1_RATE       0x15
#define PMW3360_REG_REST1_DOWNSHIFT  0x16
#define PMW3360_REG_REST2_RATE       0x17
#define PMW3360_REG_REST2_DOWNSHIFT  0x18
#define PMW3360_REG_REST3_RATE       0x19
#define PMW3360_REG_OBSERVATION      0x24
#define PMW3360_REG_DATA_OUT_LOWER   0x25
#define PMW3360_REG_DATA_OUT_UPPER   0x26
#define PMW3360_REG_RAW_DATA_DUMP    0x29
#define PMW3360_REG_SROM_ID          0x2A
#define PMW3360_REG_MIN_SQ_RUN       0x2B
#define PMW3360_REG_RAW_DATA_THRESH  0x2C
#define PMW3360_REG_CONTROL          0x2D
#define PMW3360_REG_CONFIG5          0x2F
#define PMW3360_REG_POWER_UP_RESET   0x3A
#define PMW3360_REG_SHUTDOWN         0x3B
#define PMW3360_REG_INVERSE_PROD_ID  0x3F
#define PMW3360_REG_LIFTCUTOFF_CAL3  0x41
#define PMW3360_REG_ANGLE_SNAP       0x42
#define PMW3360_REG_LIFTCUTOFF_CAL1  0x4A
#define PMW3360_REG_MOTION_BURST     0x50

// --- Expected values ---

#define PMW3360_PRODUCT_ID           0x42
#define PMW3360_SROM_ID_EXPECTED     0x04  // PixArt reference SROM v4
#define PMW3360_INVERSE_PROD_ID      0xBD  // ~0x42

// --- SPI timing (microseconds) ---

#define PMW3360_T_SWW_US             180   // Write-to-write delay
#define PMW3360_T_SWR_US             180   // Write-to-read delay
#define PMW3360_T_SRW_US             20    // Read-to-write delay
#define PMW3360_T_SRR_US             20    // Read-to-read delay
#define PMW3360_T_NCS_SCLK_US        1     // NCS low to first SCLK
#define PMW3360_T_SCLK_NCS_US        1     // Last SCLK to NCS high (read)
#define PMW3360_T_SRAD_US            160   // Address byte to data byte (read)
#define PMW3360_T_BEXIT_US           1     // Burst exit: NCS high time

// --- Power timing (milliseconds) ---

#define PMW3360_T_POWERUP_MS         50    // Power-up to first register access
#define PMW3360_T_SROM_LOAD_MS       10    // Delay before SROM download
#define PMW3360_T_WAKEUP_MS          50    // Shutdown to first access

// --- SPI configuration ---

#define PMW3360_SPI_CLOCK_HZ         2000000  // 2 MHz max
#define PMW3360_SPI_MODE             3        // CPOL=1, CPHA=1

// --- CPI configuration ---

// PMW3360 supports 100-12000 CPI in steps of 100.
// Register value = (CPI / 100) - 1
// Stored in CONFIG1 (low byte) and CONFIG2[0] (high bit, bit 0).
#define PMW3360_CPI_MIN              100
#define PMW3360_CPI_MAX              12000
#define PMW3360_CPI_STEP             100

#ifdef CONFIG_POWERFINGER_PMW3360_CPI_DEFAULT
#define PMW3360_CPI_DEFAULT          CONFIG_POWERFINGER_PMW3360_CPI_DEFAULT
#else
#define PMW3360_CPI_DEFAULT          1600   // Good default for Pro tier
#endif

// --- Burst motion read ---

// Burst read returns 12 bytes: MOTION, Observation, Delta_X_L, Delta_X_H,
// Delta_Y_L, Delta_Y_H, SQUAL, Raw_Data_Sum, Max_Raw_Data, Min_Raw_Data,
// Shutter_Upper, Shutter_Lower
#define PMW3360_BURST_LEN            12

// Offsets within burst data
#define PMW3360_BURST_MOTION         0
#define PMW3360_BURST_OBSERVATION    1
#define PMW3360_BURST_DELTA_X_L      2
#define PMW3360_BURST_DELTA_X_H      3
#define PMW3360_BURST_DELTA_Y_L      4
#define PMW3360_BURST_DELTA_Y_H      5
#define PMW3360_BURST_SQUAL          6

// --- Shutdown / wake ---

#define PMW3360_SHUTDOWN_VALUE       0xB6
#define PMW3360_RESET_VALUE          0x5A
