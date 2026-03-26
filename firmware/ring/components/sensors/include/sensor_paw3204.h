// SPDX-License-Identifier: MIT
// PowerFinger — PAW3204 optical sensor driver (internal header)
//
// The PAW3204 (PixArt) uses a proprietary 2-wire serial protocol:
//   SCLK: clock, driven by MCU
//   SDIO: bidirectional data (MCU drives for writes, sensor drives for reads)
//
// This is NOT standard SPI. The driver bit-bangs via hal_gpio.
//
// Reference: PAW3204DB-TJ3L datasheet

#pragma once

#include <stdint.h>

// PAW3204 register addresses
#define PAW3204_REG_PRODUCT_ID      0x00    // Expected: 0x30
#define PAW3204_REG_MOTION           0x02    // Bit 7 = motion flag
#define PAW3204_REG_DELTA_X          0x03    // Signed 8-bit X delta
#define PAW3204_REG_DELTA_Y          0x04    // Signed 8-bit Y delta
#define PAW3204_REG_SQUAL            0x05    // Surface quality (0-255)
#define PAW3204_REG_CONFIG           0x06    // Configuration
#define PAW3204_REG_WRITE_PROTECT    0x09    // Write protect
#define PAW3204_REG_SLEEP1           0x0A    // Sleep1 enter time
#define PAW3204_REG_SLEEP2           0x0B    // Sleep2 enter time
#define PAW3204_REG_RESOLUTION       0x0D    // CPI setting

// PAW3204 expected product ID
#define PAW3204_PRODUCT_ID          0x30

// Serial protocol timing (microseconds)
#define PAW3204_T_SCLK_HIGH_US      1       // min SCLK high time
#define PAW3204_T_SCLK_LOW_US       1       // min SCLK low time
#define PAW3204_T_SDIO_SETUP_US     1       // data setup before SCLK rising
#define PAW3204_T_READ_DELAY_US     4       // delay between address and data phase (read)
#define PAW3204_T_WRITE_DELAY_US    20      // delay between address and data phase (write)
#define PAW3204_T_RESYNC_US         4       // delay between register operations (datasheet min)
#define PAW3204_T_POWERUP_MS        50      // power-up time before first register access

// Resolution settings (CPI)
#define PAW3204_RES_400CPI          0x00
#define PAW3204_RES_500CPI          0x01
#define PAW3204_RES_600CPI          0x02
#define PAW3204_RES_800CPI          0x03
#define PAW3204_RES_1000CPI         0x04
#define PAW3204_RES_1200CPI         0x05
#define PAW3204_RES_1600CPI         0x06
