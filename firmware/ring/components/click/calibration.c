// SPDX-License-Identifier: MIT
// PowerFinger — Boot-time sensor calibration implementation

#include "calibration.h"
#include "sensor_interface.h"
#include "ring_config.h"
#include "hal_timer.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "calibration";
#define LOG_I(...) ESP_LOGI(TAG, __VA_ARGS__)
#define LOG_W(...) ESP_LOGW(TAG, __VA_ARGS__)
#else
#define LOG_I(...) (void)0
#define LOG_W(...) (void)0
#endif

#include <string.h>
#include <math.h>

static int16_t s_offset_x = 0;
static int16_t s_offset_y = 0;
static bool s_calibration_valid = false;

void calibration_reset(void)
{
    s_offset_x = 0;
    s_offset_y = 0;
    s_calibration_valid = false;
}

bool calibration_is_valid(void)
{
    return s_calibration_valid;
}

hal_status_t calibration_attempt_once(void)
{
    int32_t sum_x = 0, sum_y = 0;
    int32_t sum_x2 = 0, sum_y2 = 0;
    int valid_samples = 0;

    for (int i = 0; i < CALIBRATION_SAMPLE_COUNT; i++) {
        sensor_reading_t reading;
        if (sensor_read(&reading) == HAL_OK) {
            sum_x += reading.dx;
            sum_y += reading.dy;
            sum_x2 += (int32_t)reading.dx * reading.dx;
            sum_y2 += (int32_t)reading.dy * reading.dy;
            valid_samples++;
        }
        hal_timer_delay_ms(CALIBRATION_SAMPLE_PERIOD_MS);
    }

    if (valid_samples == 0) {
        LOG_W("no valid sensor readings during calibration");
        calibration_reset();
        return HAL_ERR_TIMEOUT;
    }

    // Compute mean
    int16_t mean_x = (int16_t)(sum_x / valid_samples);
    int16_t mean_y = (int16_t)(sum_y / valid_samples);

    // Compute variance (integer approximation)
    int32_t var_x = (sum_x2 / valid_samples) - ((int32_t)mean_x * mean_x);
    int32_t var_y = (sum_y2 / valid_samples) - ((int32_t)mean_y * mean_y);

    // Check if device was moving during calibration
    // Use sum of variances as a simple motion metric
    int32_t total_var = var_x + var_y;
    int32_t threshold_sq = (int32_t)CALIBRATION_MOTION_THRESHOLD *
                           CALIBRATION_MOTION_THRESHOLD;

    if (total_var > threshold_sq) {
        LOG_W("motion detected during calibration (var=%ld)", (long)total_var);
        calibration_reset();
        return HAL_ERR_TIMEOUT;
    }

    // Calibration succeeded
    s_offset_x = mean_x;
    s_offset_y = mean_y;
    s_calibration_valid = true;
    LOG_I("calibration complete: offset_x=%d, offset_y=%d", s_offset_x, s_offset_y);
    return HAL_OK;
}

hal_status_t calibration_run(void)
{
    calibration_reset();

    for (int attempt = 0; attempt <= CALIBRATION_MAX_RETRIES; attempt++) {
        hal_status_t rc = calibration_attempt_once();
        if (rc == HAL_OK) {
            return HAL_OK;
        }

        if (attempt < CALIBRATION_MAX_RETRIES) {
            LOG_W("calibration attempt %d/%d failed, retrying",
                  attempt + 1, CALIBRATION_MAX_RETRIES + 1);
            hal_timer_delay_ms(CALIBRATION_RETRY_DELAY_MS);
        }
    }

    LOG_W("calibration failed after %d attempts, using zero offset fallback",
          CALIBRATION_MAX_RETRIES + 1);
    calibration_reset();
    return HAL_ERR_TIMEOUT;
}

void calibration_apply(int16_t *dx, int16_t *dy)
{
    if (!s_calibration_valid) {
        return;
    }
    if (dx) *dx -= s_offset_x;
    if (dy) *dy -= s_offset_y;
}

void calibration_get_offsets(int16_t *out_offset_x, int16_t *out_offset_y)
{
    if (out_offset_x) *out_offset_x = s_offset_x;
    if (out_offset_y) *out_offset_y = s_offset_y;
}
