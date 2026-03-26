// SPDX-License-Identifier: MIT
// PowerFinger — PAW3204 optical sensor driver
//
// Bit-banged 2-wire serial via hal_gpio. Not SPI — the PAW3204 uses a
// proprietary protocol with a bidirectional SDIO line.

#include "sensor_interface.h"
#include "sensor_paw3204.h"
#include "hal_gpio.h"
#include "hal_timer.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "sdkconfig.h"
static const char *TAG = "paw3204";
#define LOG_I(...) ESP_LOGI(TAG, __VA_ARGS__)
#define LOG_E(...) ESP_LOGE(TAG, __VA_ARGS__)
#define LOG_W(...) ESP_LOGW(TAG, __VA_ARGS__)
// Use ROM delay for sub-millisecond timing
#include "rom/ets_sys.h"
#define delay_us(us) ets_delay_us(us)
#else
#define LOG_I(...) (void)0
#define LOG_E(...) (void)0
#define LOG_W(...) (void)0
#define delay_us(us) (void)0
#endif

#ifdef CONFIG_POWERFINGER_SENSOR_SCLK_PIN
#define PIN_SCLK ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_SCLK_PIN)
#define PIN_SDIO ((hal_pin_t)CONFIG_POWERFINGER_SENSOR_SDIO_PIN)
#else
#define PIN_SCLK 4
#define PIN_SDIO 5
#endif

// --- Bit-bang serial protocol ---

// Set SDIO as output and drive a value
static void sdio_output(bool level)
{
    hal_gpio_init(PIN_SDIO, HAL_GPIO_OUTPUT);
    hal_gpio_set(PIN_SDIO, level);
}

// Set SDIO as input and read
static bool sdio_input(void)
{
    hal_gpio_init(PIN_SDIO, HAL_GPIO_INPUT_PULLUP);
    return hal_gpio_get(PIN_SDIO);
}

// Clock one bit out (MSB first). SDIO must already be set.
static void clock_pulse(void)
{
    hal_gpio_set(PIN_SCLK, true);
    delay_us(PAW3204_T_SCLK_HIGH_US);
    hal_gpio_set(PIN_SCLK, false);
    delay_us(PAW3204_T_SCLK_LOW_US);
}

// Write one byte to the sensor (MSB first)
static void write_byte(uint8_t data)
{
    for (int i = 7; i >= 0; i--) {
        sdio_output((data >> i) & 0x01);
        delay_us(PAW3204_T_SDIO_SETUP_US);
        clock_pulse();
    }
}

// Read one byte from the sensor (MSB first)
static uint8_t read_byte(void)
{
    uint8_t data = 0;
    sdio_input(); // switch SDIO to input

    for (int i = 7; i >= 0; i--) {
        hal_gpio_set(PIN_SCLK, true);
        delay_us(PAW3204_T_SCLK_HIGH_US);
        if (hal_gpio_get(PIN_SDIO)) {
            data |= (1 << i);
        }
        hal_gpio_set(PIN_SCLK, false);
        delay_us(PAW3204_T_SCLK_LOW_US);
    }

    return data;
}

// Read a register
static uint8_t reg_read(uint8_t addr)
{
    // Address phase: bit 7 = 0 for read
    write_byte(addr & 0x7F);
    delay_us(PAW3204_T_READ_DELAY_US);

    // Data phase: sensor drives SDIO
    uint8_t value = read_byte();
    delay_us(PAW3204_T_RESYNC_US);

    return value;
}

// Write a register
static void reg_write(uint8_t addr, uint8_t value)
{
    // Address phase: bit 7 = 1 for write
    write_byte(addr | 0x80);
    delay_us(PAW3204_T_WRITE_DELAY_US);

    // Data phase
    write_byte(value);
    delay_us(PAW3204_T_RESYNC_US);
}

// --- Sensor interface implementation ---

hal_status_t sensor_init(void)
{
    // Initialize GPIO pins
    hal_gpio_init(PIN_SCLK, HAL_GPIO_OUTPUT);
    hal_gpio_set(PIN_SCLK, false);  // SCLK idle low
    hal_gpio_init(PIN_SDIO, HAL_GPIO_OUTPUT);

    // Wait for sensor power-up
    hal_timer_delay_ms(PAW3204_T_POWERUP_MS);

    // Perform a re-sync: toggle SCLK with SDIO high
    sdio_output(true);
    for (int i = 0; i < 20; i++) {
        clock_pulse();
    }
    delay_us(10);

    // Verify product ID
    uint8_t pid = reg_read(PAW3204_REG_PRODUCT_ID);
    if (pid != PAW3204_PRODUCT_ID) {
        LOG_E("unexpected product ID: 0x%02X (expected 0x%02X)", pid, PAW3204_PRODUCT_ID);
        return HAL_ERR_IO;
    }
    LOG_I("PAW3204 detected (product ID: 0x%02X)", pid);

    // Disable write protection for configuration
    reg_write(PAW3204_REG_WRITE_PROTECT, 0x5A);

    // Set resolution: 800 CPI (default for Standard tier)
    reg_write(PAW3204_REG_RESOLUTION, PAW3204_RES_800CPI);

    // Re-enable write protection
    reg_write(PAW3204_REG_WRITE_PROTECT, 0x00);

    // Read and discard any stale delta values
    reg_read(PAW3204_REG_MOTION);
    reg_read(PAW3204_REG_DELTA_X);
    reg_read(PAW3204_REG_DELTA_Y);

    LOG_I("PAW3204 initialized at 800 CPI");
    return HAL_OK;
}

hal_status_t sensor_read(sensor_reading_t *out)
{
    if (!out) return HAL_ERR_INVALID_ARG;

    // Read motion register first (bit 7 = motion flag)
    uint8_t motion = reg_read(PAW3204_REG_MOTION);

    // Read deltas (must read both even if no motion, to clear registers)
    int8_t dx = (int8_t)reg_read(PAW3204_REG_DELTA_X);
    int8_t dy = (int8_t)reg_read(PAW3204_REG_DELTA_Y);

    // Read surface quality
    uint8_t squal = reg_read(PAW3204_REG_SQUAL);

    out->dx = dx;
    out->dy = dy;
    out->surface_confidence = squal;
    out->motion_detected = (motion & 0x80) != 0;

    return HAL_OK;
}

hal_status_t sensor_power_down(void)
{
    // PAW3204 enters sleep automatically based on SLEEP1/SLEEP2 registers.
    // For forced power-down, we can set the configuration register.
    // In practice, the LDO power gate handles this for deep sleep.
    reg_write(PAW3204_REG_WRITE_PROTECT, 0x5A);
    reg_write(PAW3204_REG_CONFIG, 0x01);  // Force sleep
    reg_write(PAW3204_REG_WRITE_PROTECT, 0x00);
    return HAL_OK;
}

hal_status_t sensor_wake(void)
{
    // Drive SDIO high during resync clocks (matches sensor_init pattern).
    // Without this, SDIO may be left as input from a prior reg_read,
    // and the resync pulses would run with SDIO floating.
    sdio_output(true);
    for (int i = 0; i < 20; i++) {
        clock_pulse();
    }
    hal_timer_delay_ms(10);

    // Verify sensor responds
    uint8_t pid = reg_read(PAW3204_REG_PRODUCT_ID);
    if (pid != PAW3204_PRODUCT_ID) {
        LOG_W("sensor wake failed, product ID: 0x%02X", pid);
        return HAL_ERR_IO;
    }

    // Drain stale deltas
    reg_read(PAW3204_REG_MOTION);
    reg_read(PAW3204_REG_DELTA_X);
    reg_read(PAW3204_REG_DELTA_Y);

    return HAL_OK;
}
