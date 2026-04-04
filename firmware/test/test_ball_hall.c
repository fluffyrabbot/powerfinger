// SPDX-License-Identifier: MIT
// PowerFinger — Ball+Hall sensor driver unit tests

#include "unity.h"
#include "sensor_interface.h"
#include "sensor_ball_hall.h"
#include "mock_hal.h"
#include "ring_config.h"

// Default test channels (must match ball_hall.c host defaults)
#define CH_X_POS 1
#define CH_X_NEG 2
#define CH_Y_POS 3
#define CH_Y_NEG 4

static void set_all_channels_mv(uint32_t mv)
{
    mock_hal_set_adc_mv_channel(CH_X_POS, mv);
    mock_hal_set_adc_mv_channel(CH_X_NEG, mv);
    mock_hal_set_adc_mv_channel(CH_Y_POS, mv);
    mock_hal_set_adc_mv_channel(CH_Y_NEG, mv);
}

static void reset(void)
{
    mock_hal_reset();
    // Set all channels to nominal zero field (~1650mV)
    set_all_channels_mv(BALL_HALL_NOMINAL_ZERO_MV);
}

// --- Init tests ---

void test_ball_hall_init_success(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, sensor_init());
}

void test_ball_hall_init_adc_failure(void)
{
    reset();
    mock_hal_set_adc_status(HAL_ERR_IO);
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_init());
}

// --- Zero motion ---

void test_ball_hall_zero_motion(void)
{
    reset();
    sensor_init();

    // All channels at baseline — no motion
    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_EQUAL(0, r.dx);
    TEST_ASSERT_EQUAL(0, r.dy);
    TEST_ASSERT_FALSE(r.motion_detected);
    TEST_ASSERT_EQUAL(255, r.surface_confidence);
}

// --- X-axis motion ---

void test_ball_hall_x_positive_motion(void)
{
    reset();
    sensor_init();

    // X+ elevated, X- depressed (differential = positive X)
    mock_hal_set_adc_mv_channel(CH_X_POS, BALL_HALL_NOMINAL_ZERO_MV + 50);
    mock_hal_set_adc_mv_channel(CH_X_NEG, BALL_HALL_NOMINAL_ZERO_MV - 50);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_TRUE(r.dx > 0);
    TEST_ASSERT_EQUAL(0, r.dy);
    TEST_ASSERT_TRUE(r.motion_detected);
}

void test_ball_hall_x_negative_motion(void)
{
    reset();
    sensor_init();

    // X+ depressed, X- elevated (differential = negative X)
    mock_hal_set_adc_mv_channel(CH_X_POS, BALL_HALL_NOMINAL_ZERO_MV - 50);
    mock_hal_set_adc_mv_channel(CH_X_NEG, BALL_HALL_NOMINAL_ZERO_MV + 50);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_TRUE(r.dx < 0);
    TEST_ASSERT_TRUE(r.motion_detected);
}

// --- Y-axis motion ---

void test_ball_hall_y_positive_motion(void)
{
    reset();
    sensor_init();

    mock_hal_set_adc_mv_channel(CH_Y_POS, BALL_HALL_NOMINAL_ZERO_MV + 50);
    mock_hal_set_adc_mv_channel(CH_Y_NEG, BALL_HALL_NOMINAL_ZERO_MV - 50);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_EQUAL(0, r.dx);
    TEST_ASSERT_TRUE(r.dy > 0);
    TEST_ASSERT_TRUE(r.motion_detected);
}

// --- Diagonal motion ---

void test_ball_hall_diagonal_motion(void)
{
    reset();
    sensor_init();

    mock_hal_set_adc_mv_channel(CH_X_POS, BALL_HALL_NOMINAL_ZERO_MV + 30);
    mock_hal_set_adc_mv_channel(CH_X_NEG, BALL_HALL_NOMINAL_ZERO_MV - 30);
    mock_hal_set_adc_mv_channel(CH_Y_POS, BALL_HALL_NOMINAL_ZERO_MV + 40);
    mock_hal_set_adc_mv_channel(CH_Y_NEG, BALL_HALL_NOMINAL_ZERO_MV - 40);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_TRUE(r.dx > 0);
    TEST_ASSERT_TRUE(r.dy > 0);
    TEST_ASSERT_TRUE(r.motion_detected);
}

// --- Noise rejection ---

void test_ball_hall_noise_below_threshold(void)
{
    reset();
    sensor_init();

    // Tiny deflection below noise threshold
    mock_hal_set_adc_mv_channel(CH_X_POS, BALL_HALL_NOMINAL_ZERO_MV + 2);
    mock_hal_set_adc_mv_channel(CH_X_NEG, BALL_HALL_NOMINAL_ZERO_MV - 2);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    // Differential is 4mV * sensitivity(1.0) = 4 counts, below NOISE_MV (8)
    TEST_ASSERT_FALSE(r.motion_detected);
}

// --- Surface confidence ---

void test_ball_hall_surface_confidence_full_at_baseline(void)
{
    reset();
    sensor_init();

    sensor_reading_t r;
    sensor_read(&r);
    TEST_ASSERT_EQUAL(255, r.surface_confidence);
}

void test_ball_hall_surface_confidence_degrades_with_deviation(void)
{
    reset();
    sensor_init();

    mock_hal_set_adc_mv_channel(CH_X_POS, BALL_HALL_NOMINAL_ZERO_MV + 100);

    sensor_reading_t r;
    sensor_read(&r);
    TEST_ASSERT_TRUE(r.surface_confidence < 255);
    TEST_ASSERT_TRUE(r.surface_confidence > 0);
}

// --- Power down / wake ---

void test_ball_hall_power_cycle(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, sensor_init());
    TEST_ASSERT_EQUAL(HAL_OK, sensor_power_down());

    // After power down, reads should fail
    sensor_reading_t r;
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_read(&r));

    // Wake re-initializes
    TEST_ASSERT_EQUAL(HAL_OK, sensor_wake());
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
}

// --- Null argument ---

void test_ball_hall_null_out_returns_error(void)
{
    reset();
    sensor_init();
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, sensor_read(NULL));
}

// --- ADC failure during read ---

void test_ball_hall_adc_failure_during_read(void)
{
    reset();
    sensor_init();

    // Now inject failure for reads
    mock_hal_set_adc_status(HAL_ERR_IO);
    sensor_reading_t r;
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_read(&r));
}

// --- Test runner ---

int main(void)
{
    UNITY_BEGIN();

    printf("Ball+Hall sensor tests:\n");
    RUN_TEST(test_ball_hall_init_success);
    RUN_TEST(test_ball_hall_init_adc_failure);
    RUN_TEST(test_ball_hall_zero_motion);
    RUN_TEST(test_ball_hall_x_positive_motion);
    RUN_TEST(test_ball_hall_x_negative_motion);
    RUN_TEST(test_ball_hall_y_positive_motion);
    RUN_TEST(test_ball_hall_diagonal_motion);
    RUN_TEST(test_ball_hall_noise_below_threshold);
    RUN_TEST(test_ball_hall_surface_confidence_full_at_baseline);
    RUN_TEST(test_ball_hall_surface_confidence_degrades_with_deviation);
    RUN_TEST(test_ball_hall_power_cycle);
    RUN_TEST(test_ball_hall_null_out_returns_error);
    RUN_TEST(test_ball_hall_adc_failure_during_read);

    printf("\n--- Results: %d tests, %d failures ---\n",
           unity_tests, unity_failures);
    return UNITY_END();
}
