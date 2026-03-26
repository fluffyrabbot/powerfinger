// SPDX-License-Identifier: MIT
// PowerFinger HAL — SPI implementation for ESP-IDF
//
// Used by PMW3360 (standard 4-wire SPI). PAW3204 uses bit-banged GPIO.

#include "hal_spi.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <stdlib.h>
#include <string.h>

static const char *TAG = "hal_spi";

typedef struct {
    spi_device_handle_t dev;
    hal_pin_t cs;
} spi_ctx_t;

hal_status_t hal_spi_init(const hal_spi_config_t *config, hal_spi_handle_t *out_handle)
{
    if (!config || !out_handle) return HAL_ERR_INVALID_ARG;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = (int)config->mosi,
        .miso_io_num = (int)config->miso,
        .sclk_io_num = (int)config->clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 256,
    };

    // Use SPI2 (the general-purpose SPI host on ESP32-C3)
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = (int)config->clock_hz,
        .mode = config->mode,
        .spics_io_num = (int)config->cs,
        .queue_size = 1,
    };

    spi_ctx_t *ctx = calloc(1, sizeof(spi_ctx_t));
    if (!ctx) return HAL_ERR_NO_MEM;

    ctx->cs = config->cs;

    ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &ctx->dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device failed: %s", esp_err_to_name(ret));
        free(ctx);
        return HAL_ERR_IO;
    }

    *out_handle = (hal_spi_handle_t)ctx;
    return HAL_OK;
}

hal_status_t hal_spi_transfer(hal_spi_handle_t handle,
                              const uint8_t *tx_buf, uint8_t *rx_buf,
                              size_t len)
{
    if (!handle || len == 0) return HAL_ERR_INVALID_ARG;

    spi_ctx_t *ctx = (spi_ctx_t *)handle;
    spi_transaction_t txn = {
        .length = len * 8,
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };

    esp_err_t ret = spi_device_transmit(ctx->dev, &txn);
    return (ret == ESP_OK) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_spi_write_reg(hal_spi_handle_t handle, uint8_t addr, uint8_t value)
{
    uint8_t tx[2] = { addr | 0x80, value };  // MSB set = write
    return hal_spi_transfer(handle, tx, NULL, 2);
}

hal_status_t hal_spi_read_reg(hal_spi_handle_t handle, uint8_t addr, uint8_t *value)
{
    if (!value) return HAL_ERR_INVALID_ARG;

    uint8_t tx[2] = { addr & 0x7F, 0x00 };  // MSB clear = read
    uint8_t rx[2] = { 0 };

    hal_status_t ret = hal_spi_transfer(handle, tx, rx, 2);
    if (ret == HAL_OK) {
        *value = rx[1];
    }
    return ret;
}

hal_status_t hal_spi_deinit(hal_spi_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;

    spi_ctx_t *ctx = (spi_ctx_t *)handle;
    spi_bus_remove_device(ctx->dev);
    spi_bus_free(SPI2_HOST);
    free(ctx);
    return HAL_OK;
}
