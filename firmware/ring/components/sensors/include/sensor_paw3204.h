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
#define PAW3204_REG_PRODUCT_ID2     0x01    // High nibble identifies TJ3L
#define PAW3204_REG_MOTION           0x02    // Bit 7 = motion flag
#define PAW3204_REG_DELTA_X          0x03    // Signed 8-bit X delta
#define PAW3204_REG_DELTA_Y          0x04    // Signed 8-bit Y delta
#define PAW3204_REG_IMAGE_QUALITY    0x07    // Surface quality (0-255)
#define PAW3204_REG_CONFIG           0x06    // Configuration
#define PAW3204_REG_WRITE_PROTECT    0x09    // Write protect
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

// Write protection
#define PAW3204_WRITE_PROTECT_UNLOCK 0x5A   // Unlock value for REG_WRITE_PROTECT
#define PAW3204_WRITE_PROTECT_LOCK   0x00   // Re-lock value

// Section 3.1 of the TJ3L datasheet gives 0x02 as the reset configuration.
#define PAW3204_CONFIG_DEFAULT      0x02
#define PAW3204_CONFIG_POWER_DOWN   0x08
#define PAW3204_PRODUCT_ID2_MASK    0xF0
#define PAW3204_PRODUCT_ID2_TJ3L    0xD0
