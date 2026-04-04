// SPDX-License-Identifier: MIT
// PowerFinger — Ball+Hall sensor driver (4x DRV5053 via ADC)
//
// Four DRV5053 analog Hall sensors read magnetic roller spindles around a
// 5mm steel ball. Opposing pairs provide differential X and Y rotation.
//
// The driver does NOT control the Hall power gate MOSFET — the power
// manager owns that via hall_power_set(). This driver assumes sensors
// are powered before sensor_init() or sensor_wake() is called.

#include "sensor_interface.h"
#include "sensor_ball_hall.h"
#include "hal_adc.h"
#include "hal_timer.h"
#include "hal_types.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "ball_hall";
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#else
#define LOG_I(fmt, ...)
#define LOG_W(fmt, ...)
#define LOG_E(fmt, ...)
#endif

// --- Kconfig pin mapping (ESP-IDF build) or test defaults ---

#ifdef CONFIG_POWERFINGER_HALL_ADC_CH_X_POS
#define HALL_CH_X_POS   ((hal_adc_channel_t)CONFIG_POWERFINGER_HALL_ADC_CH_X_POS)
#define HALL_CH_X_NEG   ((hal_adc_channel_t)CONFIG_POWERFINGER_HALL_ADC_CH_X_NEG)
#define HALL_CH_Y_POS   ((hal_adc_channel_t)CONFIG_POWERFINGER_HALL_ADC_CH_Y_POS)
#define HALL_CH_Y_NEG   ((hal_adc_channel_t)CONFIG_POWERFINGER_HALL_ADC_CH_Y_NEG)
#else
// Host test defaults
#define HALL_CH_X_POS   ((hal_adc_channel_t)1)
#define HALL_CH_X_NEG   ((hal_adc_channel_t)2)
#define HALL_CH_Y_POS   ((hal_adc_channel_t)3)
#define HALL_CH_Y_NEG   ((hal_adc_channel_t)4)
#endif

#ifdef CONFIG_POWERFINGER_HALL_SENSITIVITY
#define HALL_SENSITIVITY  CONFIG_POWERFINGER_HALL_SENSITIVITY
#else
#define HALL_SENSITIVITY  10  // 1.0x in 0.1x units
#endif

// --- Static state ---

static hal_adc_channel_t s_channels[BALL_HALL_NUM_CH];
static uint32_t s_baseline_mv[BALL_HALL_NUM_CH];
static bool s_initialized = false;

// --- Internal helpers ---

static hal_status_t read_channel_mv(int ch_idx, uint32_t *out_mv)
{
    return hal_adc_read_mv(s_channels[ch_idx], out_mv);
}

static hal_status_t capture_baselines(void)
{
    uint64_t accum[BALL_HALL_NUM_CH] = {0};
    uint64_t accum_sq[BALL_HALL_NUM_CH] = {0};

    for (int s = 0; s < BALL_HALL_CAL_SAMPLES; s++) {
        for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
            uint32_t mv = 0;
            hal_status_t rc = read_channel_mv(ch, &mv);
            if (rc != HAL_OK) {
                LOG_E("ADC read failed on ch %d during calibration: %d",
                      ch, (int)rc);
                return rc;
            }
            accum[ch] += mv;
            accum_sq[ch] += (uint64_t)mv * mv;
        }
        if (s < BALL_HALL_CAL_SAMPLES - 1) {
            hal_timer_delay_ms(BALL_HALL_CAL_PERIOD_MS);
        }
    }

    // Compute mean and check variance
    for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
        uint32_t mean = (uint32_t)(accum[ch] / BALL_HALL_CAL_SAMPLES);
        uint64_t mean_sq = (uint64_t)mean * mean;
        uint64_t avg_sq = accum_sq[ch] / BALL_HALL_CAL_SAMPLES;
        // Variance = E[X^2] - E[X]^2
        uint32_t variance = (avg_sq > mean_sq) ? (uint32_t)(avg_sq - mean_sq) : 0;

        // Approximate stddev via integer sqrt (good enough for threshold check)
        uint32_t stddev = 0;
        if (variance > 0) {
            stddev = 1;
            while (stddev * stddev < variance) {
                stddev++;
            }
        }

        if (stddev > BALL_HALL_CAL_MOTION_MV) {
            LOG_W("ch %d: stddev %lu mV exceeds threshold %d — ball was moving",
                  ch, (unsigned long)stddev, BALL_HALL_CAL_MOTION_MV);
            return HAL_ERR_BUSY;  // motion during calibration
        }

        s_baseline_mv[ch] = mean;
        LOG_I("ch %d: baseline %lu mV (stddev %lu)",
              ch, (unsigned long)mean, (unsigned long)stddev);
    }

    return HAL_OK;
}

// --- sensor_interface.h implementation ---

hal_status_t sensor_init(void)
{
    s_channels[BALL_HALL_CH_X_POS] = HALL_CH_X_POS;
    s_channels[BALL_HALL_CH_X_NEG] = HALL_CH_X_NEG;
    s_channels[BALL_HALL_CH_Y_POS] = HALL_CH_Y_POS;
    s_channels[BALL_HALL_CH_Y_NEG] = HALL_CH_Y_NEG;

    // Initialize ADC for all four channels
    for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
        hal_status_t rc = hal_adc_init(s_channels[ch]);
        if (rc != HAL_OK) {
            LOG_E("ADC init failed for channel %d: %d",
                  (int)s_channels[ch], (int)rc);
            return rc;
        }
    }

    // Wait for sensor settling after power-on
    hal_timer_delay_ms(BALL_HALL_POWER_ON_MS);

    // Capture at-rest baselines
    hal_status_t cal_rc = capture_baselines();
    if (cal_rc != HAL_OK) {
        LOG_E("baseline capture failed: %d", (int)cal_rc);
        return cal_rc;
    }

    s_initialized = true;
    LOG_I("ball+Hall sensor initialized (4x DRV5053)");
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

    // Read all four channels
    uint32_t mv[BALL_HALL_NUM_CH] = {0};
    for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
        hal_status_t rc = read_channel_mv(ch, &mv[ch]);
        if (rc != HAL_OK) {
            return rc;
        }
    }

    // Compute differential deltas (signed, in mV)
    // Differential cancels common-mode noise (temperature, supply variation).
    int32_t x_diff = ((int32_t)mv[BALL_HALL_CH_X_POS] - (int32_t)s_baseline_mv[BALL_HALL_CH_X_POS])
                   - ((int32_t)mv[BALL_HALL_CH_X_NEG] - (int32_t)s_baseline_mv[BALL_HALL_CH_X_NEG]);

    int32_t y_diff = ((int32_t)mv[BALL_HALL_CH_Y_POS] - (int32_t)s_baseline_mv[BALL_HALL_CH_Y_POS])
                   - ((int32_t)mv[BALL_HALL_CH_Y_NEG] - (int32_t)s_baseline_mv[BALL_HALL_CH_Y_NEG]);

    // Scale by sensitivity factor (0.1x units: 10 = 1.0x)
    int32_t dx_scaled = (x_diff * HALL_SENSITIVITY) / 10;
    int32_t dy_scaled = (y_diff * HALL_SENSITIVITY) / 10;

    // Clamp to int16_t range
    if (dx_scaled > 32767) dx_scaled = 32767;
    if (dx_scaled < -32767) dx_scaled = -32767;
    if (dy_scaled > 32767) dy_scaled = 32767;
    if (dy_scaled < -32767) dy_scaled = -32767;

    out->dx = (int16_t)dx_scaled;
    out->dy = (int16_t)dy_scaled;

    // Motion detection: either axis above noise threshold
    int32_t abs_dx = dx_scaled < 0 ? -dx_scaled : dx_scaled;
    int32_t abs_dy = dy_scaled < 0 ? -dy_scaled : dy_scaled;
    out->motion_detected = (abs_dx > BALL_HALL_NOISE_MV || abs_dy > BALL_HALL_NOISE_MV);

    // Surface confidence: how close readings are to baseline.
    // Max single-channel deviation from baseline across all 4 channels.
    uint32_t max_dev = 0;
    for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
        int32_t dev = (int32_t)mv[ch] - (int32_t)s_baseline_mv[ch];
        uint32_t abs_dev = dev < 0 ? (uint32_t)(-dev) : (uint32_t)dev;
        if (abs_dev > max_dev) {
            max_dev = abs_dev;
        }
    }
    // Scale: 0mV deviation = 255 confidence, >=255mV deviation = 0
    out->surface_confidence = (max_dev >= 255) ? 0 : (uint8_t)(255 - max_dev);

    return HAL_OK;
}

hal_status_t sensor_power_down(void)
{
    // The driver does not control the Hall power MOSFET — that is the
    // power manager's responsibility. We just mark ourselves as powered down.
    s_initialized = false;

    for (int ch = 0; ch < BALL_HALL_NUM_CH; ch++) {
        hal_adc_deinit(s_channels[ch]);
    }

    LOG_I("ball+Hall sensor powered down");
    return HAL_OK;
}

hal_status_t sensor_wake(void)
{
    // Re-init ADC channels and recapture baselines.
    // Thermal drift during sleep may have shifted the zero point.
    return sensor_init();
}
