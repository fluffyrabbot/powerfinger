// SPDX-License-Identifier: MIT
// PowerFinger — PMW3360 sensor driver unit tests

#include "unity.h"
#include "sensor_interface.h"
#include "sensor_pmw3360.h"
#include "mock_hal.h"

static void seed_valid_pmw3360(void)
{
    // Seed registers for a successful init
    mock_hal_spi_set_register(PMW3360_REG_PRODUCT_ID, PMW3360_PRODUCT_ID);
    mock_hal_spi_set_register(PMW3360_REG_SROM_ID, PMW3360_SROM_ID_EXPECTED);
    mock_hal_spi_set_register(PMW3360_REG_INVERSE_PROD_ID, PMW3360_INVERSE_PROD_ID);
}

static void reset(void)
{
    mock_hal_reset();
    seed_valid_pmw3360();
}

// --- Init tests ---

void test_pmw3360_init_success(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, sensor_init());
}

void test_pmw3360_init_wrong_product_id(void)
{
    reset();
    mock_hal_spi_set_register(PMW3360_REG_PRODUCT_ID, 0xFF);
    TEST_ASSERT_EQUAL(HAL_ERR_IO, sensor_init());
}

void test_pmw3360_init_srom_verification_failure(void)
{
    reset();
    mock_hal_spi_set_register(PMW3360_REG_SROM_ID, 0x00);
    TEST_ASSERT_EQUAL(HAL_ERR_IO, sensor_init());
}

void test_pmw3360_init_spi_failure(void)
{
    reset();
    // Fail on SPI init
    mock_hal_inject_spi_failure(HAL_ERR_IO, 1);
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_init());
}

// --- Motion read ---

void test_pmw3360_read_zero_motion(void)
{
    reset();
    sensor_init();

    // Burst read registers: all zeros (no motion)
    // MOTION_BURST address = 0x50, burst starts at that register
    mock_hal_spi_set_register(PMW3360_REG_MOTION_BURST, 0x00);

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_EQUAL(0, r.dx);
    TEST_ASSERT_EQUAL(0, r.dy);
    TEST_ASSERT_FALSE(r.motion_detected);
}

void test_pmw3360_read_positive_motion(void)
{
    reset();
    sensor_init();

    // Seed burst registers with known motion
    // Burst order: MOTION, Obs, DX_L, DX_H, DY_L, DY_H, SQUAL, ...
    uint8_t burst_base = PMW3360_REG_MOTION_BURST;
    mock_hal_spi_set_register(burst_base + 0, 0x80);  // MOTION: bit 7 set
    mock_hal_spi_set_register(burst_base + 1, 0x00);  // Observation
    mock_hal_spi_set_register(burst_base + 2, 0x0A);  // DX_L = 10
    mock_hal_spi_set_register(burst_base + 3, 0x00);  // DX_H = 0
    mock_hal_spi_set_register(burst_base + 4, 0x05);  // DY_L = 5
    mock_hal_spi_set_register(burst_base + 5, 0x00);  // DY_H = 0
    mock_hal_spi_set_register(burst_base + 6, 0x80);  // SQUAL = 128

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_EQUAL(10, r.dx);
    TEST_ASSERT_EQUAL(5, r.dy);
    TEST_ASSERT_EQUAL(128, r.surface_confidence);
    TEST_ASSERT_TRUE(r.motion_detected);
}

void test_pmw3360_read_negative_motion(void)
{
    reset();
    sensor_init();

    uint8_t burst_base = PMW3360_REG_MOTION_BURST;
    mock_hal_spi_set_register(burst_base + 0, 0x80);  // MOTION flag
    mock_hal_spi_set_register(burst_base + 2, 0xF6);  // DX_L = -10 (0xFFF6)
    mock_hal_spi_set_register(burst_base + 3, 0xFF);  // DX_H
    mock_hal_spi_set_register(burst_base + 4, 0xFB);  // DY_L = -5 (0xFFFB)
    mock_hal_spi_set_register(burst_base + 5, 0xFF);  // DY_H

    sensor_reading_t r;
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
    TEST_ASSERT_EQUAL(-10, r.dx);
    TEST_ASSERT_EQUAL(-5, r.dy);
    TEST_ASSERT_TRUE(r.motion_detected);
}

// --- Power down / wake ---

void test_pmw3360_power_cycle(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, sensor_init());

    TEST_ASSERT_EQUAL(HAL_OK, sensor_power_down());

    // After power down, read should fail
    sensor_reading_t r;
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_read(&r));

    // Wake re-initializes (needs valid registers again)
    seed_valid_pmw3360();
    TEST_ASSERT_EQUAL(HAL_OK, sensor_wake());
    TEST_ASSERT_EQUAL(HAL_OK, sensor_read(&r));
}

// --- SPI failure during read ---

void test_pmw3360_spi_failure_during_read(void)
{
    reset();
    sensor_init();

    mock_hal_inject_spi_failure(HAL_ERR_IO, 1);
    sensor_reading_t r;
    TEST_ASSERT_NOT_EQUAL(HAL_OK, sensor_read(&r));
}

// --- Null argument ---

void test_pmw3360_null_out_returns_error(void)
{
    reset();
    sensor_init();
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, sensor_read(NULL));
}

// --- Write count tracks init sequence ---

void test_pmw3360_init_writes_registers(void)
{
    reset();
    sensor_init();
    // Init should have written at least: power-up reset, SROM_Enable x2,
    // CONFIG2, CONFIG1, and drained 5 motion registers
    TEST_ASSERT_TRUE(mock_hal_spi_get_write_count() > 0);
}

// --- Test runner ---

int main(void)
{
    UNITY_BEGIN();

    printf("PMW3360 sensor tests:\n");
    RUN_TEST(test_pmw3360_init_success);
    RUN_TEST(test_pmw3360_init_wrong_product_id);
    RUN_TEST(test_pmw3360_init_srom_verification_failure);
    RUN_TEST(test_pmw3360_init_spi_failure);
    RUN_TEST(test_pmw3360_read_zero_motion);
    RUN_TEST(test_pmw3360_read_positive_motion);
    RUN_TEST(test_pmw3360_read_negative_motion);
    RUN_TEST(test_pmw3360_power_cycle);
    RUN_TEST(test_pmw3360_spi_failure_during_read);
    RUN_TEST(test_pmw3360_null_out_returns_error);
    RUN_TEST(test_pmw3360_init_writes_registers);

    printf("\n--- Results: %d tests, %d failures ---\n",
           unity_tests, unity_failures);
    return UNITY_END();
}
