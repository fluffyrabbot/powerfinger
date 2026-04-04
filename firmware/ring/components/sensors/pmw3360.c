// SPDX-License-Identifier: MIT
// PowerFinger — PMW3360 optical-on-ball sensor driver (SPI)
//
// Standard SPI Mode 3 driver for the PMW3360DM-T2QU.
// Requires SROM firmware upload on every power-on (see pmw3360_srom.h).
// Uses burst motion read for low-latency delta capture.

#include "sensor_interface.h"
#include "sensor_pmw3360.h"
#include "pmw3360_srom.h"
#include "hal_spi.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_types.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "rom/ets_sys.h"
static const char *TAG = "pmw3360";
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#define delay_us(us) ets_delay_us(us)
#else
#define LOG_I(fmt, ...)
#define LOG_W(fmt, ...)
#define LOG_E(fmt, ...)
#define delay_us(us) ((void)0)
#endif

// --- Kconfig pin mapping (ESP-IDF build) or test defaults ---

#ifdef CONFIG_POWERFINGER_SENSOR_SCLK_PIN
#define PIN_SCLK    ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_SCLK_PIN)
#define PIN_MOSI    ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_SDIO_PIN)
#define PIN_MISO    ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_MISO_PIN)
#define PIN_NCS     ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_NCS_PIN)
#else
// Host test defaults
#define PIN_SCLK    ((hal_pin_t)4)
#define PIN_MOSI    ((hal_pin_t)5)
#define PIN_MISO    ((hal_pin_t)7)
#define PIN_NCS     ((hal_pin_t)6)
#endif

// --- Static state ---

static hal_spi_handle_t s_spi = NULL;
static bool s_initialized = false;

// --- Internal helpers ---

static hal_status_t write_reg(uint8_t addr, uint8_t value)
{
    hal_status_t rc = hal_spi_write_reg(s_spi, addr, value);
    delay_us(PMW3360_T_SWW_US);
    return rc;
}

static hal_status_t read_reg(uint8_t addr, uint8_t *value)
{
    // The HAL SPI read_reg handles address masking (addr & 0x7F)
    // and the HAL SPI write_reg handles write bit (addr | 0x80).
    hal_status_t rc = hal_spi_read_reg(s_spi, addr, value);
    delay_us(PMW3360_T_SRR_US);
    return rc;
}

static hal_status_t upload_srom(void)
{
    hal_status_t rc;

    // Step 1: Write 0x00 to SROM_Enable
    rc = write_reg(PMW3360_REG_SROM_ENABLE, 0x00);
    if (rc != HAL_OK) return rc;

    // Step 2: Wait for SROM to be ready
    hal_timer_delay_ms(PMW3360_T_SROM_LOAD_MS);

    // Step 3: Write 0x1D to SROM_Enable to start download
    rc = write_reg(PMW3360_REG_SROM_ENABLE, 0x1D);
    if (rc != HAL_OK) return rc;

    delay_us(PMW3360_T_SRAD_US);

    // Step 4: Burst-write SROM data byte by byte.
    // Write to SROM_Load_Burst (0x62) — first byte sets the address,
    // subsequent bytes auto-increment.
    uint8_t tx_buf[2];
    tx_buf[0] = 0x62 | 0x80;  // Write to burst address
    tx_buf[1] = pmw3360_srom_data[0];
    rc = hal_spi_transfer(s_spi, tx_buf, NULL, 2);
    if (rc != HAL_OK) return rc;

    for (uint16_t i = 1; i < PMW3360_SROM_LENGTH; i++) {
        delay_us(15);  // Inter-byte delay per datasheet
        tx_buf[0] = pmw3360_srom_data[i];
        rc = hal_spi_transfer(s_spi, tx_buf, NULL, 1);
        if (rc != HAL_OK) return rc;
    }

    delay_us(200);  // Wait for SROM to execute

    // Step 5: Verify SROM ID
    uint8_t srom_id = 0;
    rc = read_reg(PMW3360_REG_SROM_ID, &srom_id);
    if (rc != HAL_OK) return rc;

    if (srom_id != PMW3360_SROM_ID_EXPECTED) {
        LOG_E("SROM verification failed: got 0x%02x, expected 0x%02x",
              srom_id, PMW3360_SROM_ID_EXPECTED);
        return HAL_ERR_IO;
    }

    LOG_I("SROM uploaded successfully (ID=0x%02x)", srom_id);
    return HAL_OK;
}

static hal_status_t set_cpi(uint16_t cpi)
{
    if (cpi < PMW3360_CPI_MIN) cpi = PMW3360_CPI_MIN;
    if (cpi > PMW3360_CPI_MAX) cpi = PMW3360_CPI_MAX;

    // Round to nearest step
    cpi = ((cpi + PMW3360_CPI_STEP / 2) / PMW3360_CPI_STEP) * PMW3360_CPI_STEP;

    uint16_t reg_val = (cpi / PMW3360_CPI_STEP) - 1;

    hal_status_t rc = write_reg(PMW3360_REG_CONFIG1, (uint8_t)(reg_val & 0xFF));
    if (rc != HAL_OK) return rc;

    // CONFIG2 bit 0 is the high bit of CPI (for values > 12700)
    // For CPI <= 12000, this is always 0.
    rc = write_reg(PMW3360_REG_CONFIG2, 0x00);
    return rc;
}

static hal_status_t drain_motion(void)
{
    // Read and discard motion registers to clear any stale data
    uint8_t discard;
    hal_status_t rc;

    rc = read_reg(PMW3360_REG_MOTION, &discard);
    if (rc != HAL_OK) return rc;
    rc = read_reg(PMW3360_REG_DELTA_X_L, &discard);
    if (rc != HAL_OK) return rc;
    rc = read_reg(PMW3360_REG_DELTA_X_H, &discard);
    if (rc != HAL_OK) return rc;
    rc = read_reg(PMW3360_REG_DELTA_Y_L, &discard);
    if (rc != HAL_OK) return rc;
    rc = read_reg(PMW3360_REG_DELTA_Y_H, &discard);
    return rc;
}

// --- sensor_interface.h implementation ---

hal_status_t sensor_init(void)
{
    hal_status_t rc;

    // Initialize SPI bus
    hal_spi_config_t spi_cfg = {
        .clk = PIN_SCLK,
        .mosi = PIN_MOSI,
        .miso = PIN_MISO,
        .cs = PIN_NCS,
        .clock_hz = PMW3360_SPI_CLOCK_HZ,
        .mode = (hal_spi_mode_t)PMW3360_SPI_MODE,
    };

    rc = hal_spi_init(&spi_cfg, &s_spi);
    if (rc != HAL_OK) {
        LOG_E("SPI init failed: %d", (int)rc);
        return rc;
    }

    // Power-up reset sequence
    rc = write_reg(PMW3360_REG_POWER_UP_RESET, PMW3360_RESET_VALUE);
    if (rc != HAL_OK) {
        LOG_E("power-up reset write failed: %d", (int)rc);
        return rc;
    }
    hal_timer_delay_ms(PMW3360_T_POWERUP_MS);

    // Verify product ID
    uint8_t product_id = 0;
    rc = read_reg(PMW3360_REG_PRODUCT_ID, &product_id);
    if (rc != HAL_OK) {
        LOG_E("product ID read failed: %d", (int)rc);
        return rc;
    }
    if (product_id != PMW3360_PRODUCT_ID) {
        LOG_E("wrong product ID: got 0x%02x, expected 0x%02x",
              product_id, PMW3360_PRODUCT_ID);
        return HAL_ERR_IO;
    }

    // Upload SROM firmware
    rc = upload_srom();
    if (rc != HAL_OK) {
        return rc;
    }

    // Disable rest mode for consistent tracking during active use.
    // The firmware power manager handles sleep transitions.
    rc = write_reg(PMW3360_REG_CONFIG2, 0x00);
    if (rc != HAL_OK) return rc;

    // Set CPI
    rc = set_cpi(PMW3360_CPI_DEFAULT);
    if (rc != HAL_OK) {
        LOG_W("CPI set failed: %d — using sensor default", (int)rc);
        // Non-fatal: sensor works at its power-on default CPI
    }

    // Drain any stale motion data
    rc = drain_motion();
    if (rc != HAL_OK) {
        LOG_W("motion drain failed: %d", (int)rc);
        // Non-fatal
    }

    s_initialized = true;
    LOG_I("PMW3360 initialized (product_id=0x%02x, CPI=%d)",
          product_id, (int)PMW3360_CPI_DEFAULT);
    return HAL_OK;
}

hal_status_t sensor_read(sensor_reading_t *out)
{
    if (!out) {
        return HAL_ERR_INVALID_ARG;
    }
    if (!s_initialized) {
        return HAL_ERR_IO;
    }

    memset(out, 0, sizeof(*out));

    // Burst motion read: write MOTION_BURST address, then read 12 bytes
    // in a single SPI transaction for minimum latency.
    uint8_t tx[1 + PMW3360_BURST_LEN] = {0};
    uint8_t rx[1 + PMW3360_BURST_LEN] = {0};

    tx[0] = PMW3360_REG_MOTION_BURST;  // Read (bit 7 clear)

    hal_status_t rc = hal_spi_transfer(s_spi, tx, rx, sizeof(tx));
    if (rc != HAL_OK) {
        return rc;
    }

    // Parse burst data (offset by 1 for the address byte)
    uint8_t *burst = &rx[1];

    uint8_t motion = burst[PMW3360_BURST_MOTION];
    bool motion_flag = (motion & 0x80) != 0;

    int16_t dx = (int16_t)((uint16_t)burst[PMW3360_BURST_DELTA_X_L] |
                            ((uint16_t)burst[PMW3360_BURST_DELTA_X_H] << 8));
    int16_t dy = (int16_t)((uint16_t)burst[PMW3360_BURST_DELTA_Y_L] |
                            ((uint16_t)burst[PMW3360_BURST_DELTA_Y_H] << 8));
    uint8_t squal = burst[PMW3360_BURST_SQUAL];

    out->dx = dx;
    out->dy = dy;
    out->surface_confidence = squal;
    out->motion_detected = motion_flag && (dx != 0 || dy != 0);

    return HAL_OK;
}

hal_status_t sensor_power_down(void)
{
    if (!s_initialized) {
        return HAL_OK;
    }

    // Write shutdown register
    hal_status_t rc = write_reg(PMW3360_REG_SHUTDOWN, PMW3360_SHUTDOWN_VALUE);
    s_initialized = false;

    LOG_I("PMW3360 powered down");
    return rc;
}

hal_status_t sensor_wake(void)
{
    if (!s_spi) {
        return sensor_init();
    }

    // PMW3360 loses SROM contents on shutdown.
    // Full re-init is required (reset + SROM upload).
    s_initialized = false;

    // Power-up reset
    hal_status_t rc = write_reg(PMW3360_REG_POWER_UP_RESET, PMW3360_RESET_VALUE);
    if (rc != HAL_OK) {
        LOG_E("wake reset failed: %d", (int)rc);
        return rc;
    }
    hal_timer_delay_ms(PMW3360_T_WAKEUP_MS);

    // Verify product ID
    uint8_t product_id = 0;
    rc = read_reg(PMW3360_REG_PRODUCT_ID, &product_id);
    if (rc != HAL_OK || product_id != PMW3360_PRODUCT_ID) {
        LOG_E("wake product ID check failed");
        return HAL_ERR_IO;
    }

    // Re-upload SROM
    rc = upload_srom();
    if (rc != HAL_OK) {
        return rc;
    }

    // Restore CPI and drain stale motion
    set_cpi(PMW3360_CPI_DEFAULT);
    drain_motion();

    s_initialized = true;
    LOG_I("PMW3360 wake complete");
    return HAL_OK;
}
