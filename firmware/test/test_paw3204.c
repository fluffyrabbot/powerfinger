// SPDX-License-Identifier: MIT
// PowerFinger — PAW3204 optical sensor driver unit tests
//
// The PAW3204 uses a bit-banged 2-wire serial protocol where the MCU drives
// SCLK and the SDIO line is bidirectional. During register reads, the sensor
// drives SDIO which the MCU samples on each SCLK rising edge.
//
// These tests use mock_hal_set_gpio_read_sequence() to feed bit-level
// responses for SDIO reads, verifying the init, read, power cycle, and
// error handling paths of the driver.

#include "unity.h"
#include "sensor_interface.h"
#include "sensor_paw3204.h"
#include "mock_hal.h"

#include <string.h>

// Pin assignments must match the PAW3204 driver defaults (no Kconfig override)
#define PIN_SCLK 4
#define PIN_SDIO 5

// Maximum GPIO read sequence length for a full init+read cycle
#define SEQ_MAX 256

static bool s_sdio_seq[SEQ_MAX];
static int s_sdio_seq_len = 0;

static void reset(void)
{
    mock_hal_reset();
    memset(s_sdio_seq, 0, sizeof(s_sdio_seq));
    s_sdio_seq_len = 0;
}

// Append one byte to the SDIO read sequence (MSB first, 8 bits).
// This represents the data the sensor would drive during a read phase.
static void seq_push_byte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--) {
        s_sdio_seq[s_sdio_seq_len++] = (byte >> i) & 0x01;
    }
}

// Commit the SDIO sequence to the GPIO read mock for PIN_SDIO.
static void seq_commit(void)
{
    mock_hal_set_gpio_read_sequence(PIN_SDIO, s_sdio_seq, s_sdio_seq_len);
}

// Seed the SDIO responses for a successful sensor_init().
// Init reads: product_id, then drains motion, delta_x, delta_y.
static void seed_valid_init(void)
{
    seq_push_byte(PAW3204_PRODUCT_ID);  // product ID read
    seq_push_byte(0x00);  // motion drain
    seq_push_byte(0x00);  // delta_x drain
    seq_push_byte(0x00);  // delta_y drain
    seq_commit();
}

// Seed SDIO responses for a sensor_read() after init.
// sensor_read reads: motion, delta_x, delta_y, squal (4 registers).
static void seed_motion_read(uint8_t motion, int8_t dx, int8_t dy, uint8_t squal)
{
    seq_push_byte(motion);
    seq_push_byte((uint8_t)dx);
    seq_push_byte((uint8_t)dy);
    seq_push_byte(squal);
    seq_commit();
}

// --- Init tests ---

void test_paw3204_init_success(void)
{
    reset();
    seed_valid_init();

    TEST_ASSERT_EQUAL(HAL_OK, sensor_init());
}

void test_paw3204_init_wrong_product_id(void)
{
    reset();
    seq_push_byte(0xFF);  // wrong product ID
    seq_push_byte(0x00);  // motion drain (in case init reads further)
    seq_push_byte(0x00);
    seq_push_byte(0x00);
    seq_commit();

    TEST_ASSERT_EQUAL(HAL_ERR_IO, sensor_init());
}

// --- Motion read tests ---

void test_paw3204_read_zero_motion(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    // Reset sequence for the read phase
    s_sdio_seq_len = 0;
    seed_motion_read(0x00, 0, 0, 200);

    sensor_reading_t reading = {0};
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&reading));
    TEST_ASSERT_EQUAL(0, reading.dx);
    TEST_ASSERT_EQUAL(0, reading.dy);
    TEST_ASSERT_FALSE(reading.motion_detected);
    TEST_ASSERT_EQUAL(200, reading.surface_confidence);
}

void test_paw3204_read_positive_motion(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    s_sdio_seq_len = 0;
    seed_motion_read(0x80, 10, 5, 150);  // motion flag set

    sensor_reading_t reading = {0};
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&reading));
    TEST_ASSERT_EQUAL(10, reading.dx);
    TEST_ASSERT_EQUAL(5, reading.dy);
    TEST_ASSERT_TRUE(reading.motion_detected);
}

void test_paw3204_read_negative_motion(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    s_sdio_seq_len = 0;
    seed_motion_read(0x80, -20, -15, 100);

    sensor_reading_t reading = {0};
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&reading));
    TEST_ASSERT_EQUAL(-20, reading.dx);
    TEST_ASSERT_EQUAL(-15, reading.dy);
    TEST_ASSERT_TRUE(reading.motion_detected);
}

void test_paw3204_read_null_out_returns_error(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, sensor_read(NULL));
}

// --- Power cycle tests ---

void test_paw3204_power_down_success(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    // power_down only writes (no reads needed in sequence)
    TEST_ASSERT_EQUAL(HAL_OK, sensor_power_down());
}

void test_paw3204_wake_success(void)
{
    reset();
    seed_valid_init();
    sensor_init();
    sensor_power_down();

    // Wake reads product_id + drains 3 registers
    s_sdio_seq_len = 0;
    seq_push_byte(PAW3204_PRODUCT_ID);  // product ID verify
    seq_push_byte(0x00);  // motion drain
    seq_push_byte(0x00);  // delta_x drain
    seq_push_byte(0x00);  // delta_y drain
    seq_commit();

    TEST_ASSERT_EQUAL(HAL_OK, sensor_wake());
}

void test_paw3204_wake_failure_wrong_id(void)
{
    reset();
    seed_valid_init();
    sensor_init();
    sensor_power_down();

    // Wake: sensor returns wrong product ID
    s_sdio_seq_len = 0;
    seq_push_byte(0xFF);
    seq_push_byte(0x00);
    seq_push_byte(0x00);
    seq_push_byte(0x00);
    seq_commit();

    TEST_ASSERT_EQUAL(HAL_ERR_IO, sensor_wake());
}

// --- Surface quality ---

void test_paw3204_surface_quality_passthrough(void)
{
    reset();
    seed_valid_init();
    sensor_init();

    // SQUAL=0 means surface is not trackable
    s_sdio_seq_len = 0;
    seed_motion_read(0x00, 0, 0, 0);

    sensor_reading_t reading = {0};
    sensor_read(&reading);
    TEST_ASSERT_EQUAL(0, reading.surface_confidence);

    // SQUAL=255 is the highest quality
    s_sdio_seq_len = 0;
    seed_motion_read(0x00, 0, 0, 255);

    sensor_read(&reading);
    TEST_ASSERT_EQUAL(255, reading.surface_confidence);
}

// --- Runner ---

int main(void)
{
    UNITY_BEGIN();

    printf("PAW3204 sensor tests:\n");
    RUN_TEST(test_paw3204_init_success);
    RUN_TEST(test_paw3204_init_wrong_product_id);
    RUN_TEST(test_paw3204_read_zero_motion);
    RUN_TEST(test_paw3204_read_positive_motion);
    RUN_TEST(test_paw3204_read_negative_motion);
    RUN_TEST(test_paw3204_read_null_out_returns_error);
    RUN_TEST(test_paw3204_power_down_success);
    RUN_TEST(test_paw3204_wake_success);
    RUN_TEST(test_paw3204_wake_failure_wrong_id);
    RUN_TEST(test_paw3204_surface_quality_passthrough);

    return UNITY_END();
}
