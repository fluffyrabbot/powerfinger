// SPDX-License-Identifier: MIT
// PowerFinger HAL — SPI interface
//
// Used by PMW3360 (standard SPI). The PAW3204 uses a proprietary 2-wire
// serial protocol and bit-bangs via hal_gpio instead.

#pragma once

#include "hal_types.h"

typedef enum {
    HAL_SPI_MODE_0 = 0,    // CPOL=0, CPHA=0 (clock idle low, sample on rising)
    HAL_SPI_MODE_1 = 1,    // CPOL=0, CPHA=1
    HAL_SPI_MODE_2 = 2,    // CPOL=1, CPHA=0
    HAL_SPI_MODE_3 = 3,    // CPOL=1, CPHA=1 (clock idle high, sample on falling)
} hal_spi_mode_t;

typedef struct {
    hal_pin_t clk;
    hal_pin_t mosi;
    hal_pin_t miso;
    hal_pin_t cs;
    uint32_t       clock_hz;   // SPI clock frequency
    hal_spi_mode_t mode;       // SPI mode (0-3)
} hal_spi_config_t;

// Initialize SPI bus and return handle
hal_status_t hal_spi_init(const hal_spi_config_t *config, hal_spi_handle_t *out_handle);

// Full-duplex transfer: tx_buf and rx_buf may be NULL for one-direction transfers
hal_status_t hal_spi_transfer(hal_spi_handle_t handle,
                              const uint8_t *tx_buf, uint8_t *rx_buf,
                              size_t len);

// Write a single register (address byte + data byte, CS asserted)
hal_status_t hal_spi_write_reg(hal_spi_handle_t handle, uint8_t addr, uint8_t value);

// Read a single register (address byte out, data byte in, CS asserted)
hal_status_t hal_spi_read_reg(hal_spi_handle_t handle, uint8_t addr, uint8_t *value);

// Deinitialize SPI
hal_status_t hal_spi_deinit(hal_spi_handle_t handle);
