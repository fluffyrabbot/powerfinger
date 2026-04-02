// SPDX-License-Identifier: MIT
// PowerFinger — Ring settings unit tests

#include <limits.h>

#include "unity.h"
#include "mock_hal.h"
#include "ring_config.h"
#include "ring_settings.h"

static void seed_settings_blob(uint8_t version,
                               uint8_t dpi_multiplier,
                               uint16_t dead_zone_time_ms,
                               uint8_t dead_zone_distance)
{
    uint8_t blob[5] = {
        version,
        dpi_multiplier,
        (uint8_t)(dead_zone_time_ms & 0xFFU),
        (uint8_t)((dead_zone_time_ms >> 8) & 0xFFU),
        dead_zone_distance,
    };
    mock_hal_storage_seed("ring_cfg", blob, sizeof(blob));
}

static void reset(void)
{
    mock_hal_reset();
}

void test_defaults_loaded_when_storage_empty(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());

    ring_settings_snapshot_t snapshot = ring_settings_snapshot();
    TEST_ASSERT_EQUAL(DPI_MULTIPLIER_DEFAULT, snapshot.dpi_multiplier);
    TEST_ASSERT_EQUAL(DEAD_ZONE_TIME_MS, snapshot.dead_zone_time_ms);
    TEST_ASSERT_EQUAL(DEAD_ZONE_DISTANCE, snapshot.dead_zone_distance);
    TEST_ASSERT_FALSE(ring_settings_needs_flush());
}

void test_persisted_blob_loads_successfully(void)
{
    reset();
    seed_settings_blob(1, 22, 125, 33);

    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());

    ring_settings_snapshot_t snapshot = ring_settings_snapshot();
    TEST_ASSERT_EQUAL(22, snapshot.dpi_multiplier);
    TEST_ASSERT_EQUAL(125, snapshot.dead_zone_time_ms);
    TEST_ASSERT_EQUAL(33, snapshot.dead_zone_distance);
    TEST_ASSERT_FALSE(ring_settings_needs_flush());
}

void test_invalid_blob_falls_back_to_defaults_and_marks_dirty(void)
{
    reset();
    seed_settings_blob(2, 22, 125, 33);

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, ring_settings_init());

    ring_settings_snapshot_t snapshot = ring_settings_snapshot();
    TEST_ASSERT_EQUAL(DPI_MULTIPLIER_DEFAULT, snapshot.dpi_multiplier);
    TEST_ASSERT_EQUAL(DEAD_ZONE_TIME_MS, snapshot.dead_zone_time_ms);
    TEST_ASSERT_EQUAL(DEAD_ZONE_DISTANCE, snapshot.dead_zone_distance);
    TEST_ASSERT_TRUE(ring_settings_needs_flush());
}

void test_setters_validate_input_and_scale_deltas(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, ring_settings_set_dpi_multiplier(0));
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG,
                      ring_settings_set_dead_zone_time_ms(DEAD_ZONE_TIME_MS_MAX + 1));

    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dpi_multiplier(25));
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dead_zone_time_ms(200));
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dead_zone_distance(40));

    TEST_ASSERT_EQUAL(10, ring_settings_scale_delta(4));
    TEST_ASSERT_EQUAL(-15, ring_settings_scale_delta(-6));
    TEST_ASSERT_EQUAL(INT16_MAX, ring_settings_scale_delta(INT16_MAX));
}

void test_flush_persists_dirty_settings(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dpi_multiplier(31));
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dead_zone_time_ms(140));
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dead_zone_distance(12));

    TEST_ASSERT_TRUE(ring_settings_needs_flush());
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_flush());
    TEST_ASSERT_FALSE(ring_settings_needs_flush());

    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());

    ring_settings_snapshot_t snapshot = ring_settings_snapshot();
    TEST_ASSERT_EQUAL(31, snapshot.dpi_multiplier);
    TEST_ASSERT_EQUAL(140, snapshot.dead_zone_time_ms);
    TEST_ASSERT_EQUAL(12, snapshot.dead_zone_distance);
}

void test_commit_failure_keeps_settings_dirty_for_retry(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_init());
    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_set_dpi_multiplier(18));
    mock_hal_inject_storage_commit_failure(HAL_ERR_IO, 1);

    TEST_ASSERT_EQUAL(HAL_ERR_IO, ring_settings_flush());
    TEST_ASSERT_TRUE(ring_settings_needs_flush());

    TEST_ASSERT_EQUAL(HAL_OK, ring_settings_flush());
    TEST_ASSERT_FALSE(ring_settings_needs_flush());
}

void run_ring_settings_tests(void)
{
    printf("Ring settings tests:\n");
    RUN_TEST(test_defaults_loaded_when_storage_empty);
    RUN_TEST(test_persisted_blob_loads_successfully);
    RUN_TEST(test_invalid_blob_falls_back_to_defaults_and_marks_dirty);
    RUN_TEST(test_setters_validate_input_and_scale_deltas);
    RUN_TEST(test_flush_persists_dirty_settings);
    RUN_TEST(test_commit_failure_keeps_settings_dirty_for_retry);
}
