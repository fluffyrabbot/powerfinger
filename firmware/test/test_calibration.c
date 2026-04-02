// SPDX-License-Identifier: MIT
// PowerFinger — Calibration unit tests

#include "unity.h"
#include "calibration.h"
#include "sensor_interface.h"
#include "mock_hal.h"
#include "ring_config.h"
#include "hal_timer.h"

#include <string.h>

#define SENSOR_SCRIPT_MAX 256

static hal_status_t s_script_status[SENSOR_SCRIPT_MAX];
static sensor_reading_t s_script_readings[SENSOR_SCRIPT_MAX];
static int s_script_len = 0;
static int s_script_pos = 0;

static void reset(void)
{
    mock_hal_reset();
    calibration_reset();
    memset(s_script_status, 0, sizeof(s_script_status));
    memset(s_script_readings, 0, sizeof(s_script_readings));
    s_script_len = 0;
    s_script_pos = 0;
}

static void script_push(hal_status_t status, int16_t dx, int16_t dy)
{
    TEST_ASSERT_TRUE(s_script_len < SENSOR_SCRIPT_MAX);
    s_script_status[s_script_len] = status;
    s_script_readings[s_script_len].dx = dx;
    s_script_readings[s_script_len].dy = dy;
    s_script_readings[s_script_len].motion_detected = (dx != 0 || dy != 0);
    s_script_len++;
}

static void script_push_many_ok(int count, int16_t dx, int16_t dy)
{
    for (int i = 0; i < count; i++) {
        script_push(HAL_OK, dx, dy);
    }
}

static void script_push_many_error(int count, hal_status_t status)
{
    for (int i = 0; i < count; i++) {
        script_push(status, 0, 0);
    }
}

hal_status_t sensor_read(sensor_reading_t *out)
{
    if (s_script_pos >= s_script_len) {
        return HAL_ERR_IO;
    }

    hal_status_t status = s_script_status[s_script_pos];
    if (status == HAL_OK && out) {
        *out = s_script_readings[s_script_pos];
    }
    s_script_pos++;
    return status;
}

void test_calibration_attempt_once_sets_offsets_and_validity(void)
{
    reset();
    script_push_many_ok(CALIBRATION_SAMPLE_COUNT, 5, -3);

    TEST_ASSERT_EQUAL(HAL_OK, calibration_attempt_once());
    TEST_ASSERT_TRUE(calibration_is_valid());

    int16_t offset_x = 0;
    int16_t offset_y = 0;
    calibration_get_offsets(&offset_x, &offset_y);
    TEST_ASSERT_EQUAL(5, offset_x);
    TEST_ASSERT_EQUAL(-3, offset_y);

    int16_t dx = 8;
    int16_t dy = -1;
    calibration_apply(&dx, &dy);
    TEST_ASSERT_EQUAL(3, dx);
    TEST_ASSERT_EQUAL(2, dy);
}

void test_calibration_failure_clears_previous_offsets(void)
{
    reset();
    script_push_many_ok(CALIBRATION_SAMPLE_COUNT, 4, 1);
    TEST_ASSERT_EQUAL(HAL_OK, calibration_attempt_once());
    TEST_ASSERT_TRUE(calibration_is_valid());

    script_push_many_error(CALIBRATION_SAMPLE_COUNT, HAL_ERR_IO);
    TEST_ASSERT_EQUAL(HAL_ERR_TIMEOUT, calibration_attempt_once());
    TEST_ASSERT_FALSE(calibration_is_valid());

    int16_t offset_x = 123;
    int16_t offset_y = 456;
    calibration_get_offsets(&offset_x, &offset_y);
    TEST_ASSERT_EQUAL(0, offset_x);
    TEST_ASSERT_EQUAL(0, offset_y);

    int16_t dx = 8;
    int16_t dy = -1;
    calibration_apply(&dx, &dy);
    TEST_ASSERT_EQUAL(8, dx);
    TEST_ASSERT_EQUAL(-1, dy);
}

void test_calibration_attempt_once_rejects_motion_during_sampling(void)
{
    reset();
    for (int i = 0; i < (CALIBRATION_SAMPLE_COUNT / 2); i++) {
        script_push(HAL_OK, 0, 0);
    }
    for (int i = CALIBRATION_SAMPLE_COUNT / 2; i < CALIBRATION_SAMPLE_COUNT; i++) {
        script_push(HAL_OK, 12, 0);
    }

    TEST_ASSERT_EQUAL(HAL_ERR_TIMEOUT, calibration_attempt_once());
    TEST_ASSERT_FALSE(calibration_is_valid());
}

void test_calibration_run_retries_until_success(void)
{
    reset();
    script_push_many_error(CALIBRATION_SAMPLE_COUNT, HAL_ERR_IO);
    script_push_many_ok(CALIBRATION_SAMPLE_COUNT, 2, 2);

    TEST_ASSERT_EQUAL(HAL_OK, calibration_run());
    TEST_ASSERT_TRUE(calibration_is_valid());
    TEST_ASSERT_EQUAL((CALIBRATION_SAMPLE_COUNT * CALIBRATION_SAMPLE_PERIOD_MS * 2) +
                          CALIBRATION_RETRY_DELAY_MS,
                      hal_timer_get_ms());
}

void run_calibration_tests(void)
{
    printf("Calibration tests:\n");
    RUN_TEST(test_calibration_attempt_once_sets_offsets_and_validity);
    RUN_TEST(test_calibration_failure_clears_previous_offsets);
    RUN_TEST(test_calibration_attempt_once_rejects_motion_during_sampling);
    RUN_TEST(test_calibration_run_retries_until_success);
}
