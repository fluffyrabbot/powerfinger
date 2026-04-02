// SPDX-License-Identifier: MIT
// PowerFinger — Ring runtime health unit tests

#include "unity.h"
#include "ring_runtime_health.h"
#include "ring_config.h"

void test_sensor_degrades_after_threshold_failures(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    for (uint16_t i = 1; i < SENSOR_READ_FAIL_DEGRADE_THRESHOLD; i++) {
        ring_sensor_health_update_t update =
            ring_runtime_health_note_sensor_result(&state, HAL_ERR_IO);
        TEST_ASSERT_EQUAL(RING_SENSOR_HEALTH_STABLE, update.event);
        TEST_ASSERT_EQUAL(i, update.consecutive_failures);
        TEST_ASSERT_FALSE(ring_runtime_health_sensor_degraded(&state));
    }

    ring_sensor_health_update_t update =
        ring_runtime_health_note_sensor_result(&state, HAL_ERR_IO);
    TEST_ASSERT_EQUAL(RING_SENSOR_HEALTH_DEGRADED, update.event);
    TEST_ASSERT_EQUAL(SENSOR_READ_FAIL_DEGRADE_THRESHOLD, update.consecutive_failures);
    TEST_ASSERT_TRUE(ring_runtime_health_sensor_degraded(&state));
}

void test_sensor_success_resets_failure_count_before_degrade(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    ring_sensor_health_update_t update =
        ring_runtime_health_note_sensor_result(&state, HAL_ERR_IO);
    TEST_ASSERT_EQUAL(1, update.consecutive_failures);

    update = ring_runtime_health_note_sensor_result(&state, HAL_OK);
    TEST_ASSERT_EQUAL(RING_SENSOR_HEALTH_STABLE, update.event);
    TEST_ASSERT_EQUAL(0, update.consecutive_failures);
    TEST_ASSERT_FALSE(ring_runtime_health_sensor_degraded(&state));

    for (uint16_t i = 1; i < SENSOR_READ_FAIL_DEGRADE_THRESHOLD; i++) {
        update = ring_runtime_health_note_sensor_result(&state, HAL_ERR_IO);
        TEST_ASSERT_EQUAL(i, update.consecutive_failures);
        TEST_ASSERT_FALSE(ring_runtime_health_sensor_degraded(&state));
    }
}

void test_sensor_success_recovers_after_degrade(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    for (uint16_t i = 0; i < SENSOR_READ_FAIL_DEGRADE_THRESHOLD; i++) {
        ring_runtime_health_note_sensor_result(&state, HAL_ERR_IO);
    }

    ring_sensor_health_update_t update =
        ring_runtime_health_note_sensor_result(&state, HAL_OK);
    TEST_ASSERT_EQUAL(RING_SENSOR_HEALTH_RECOVERED, update.event);
    TEST_ASSERT_EQUAL(0, update.consecutive_failures);
    TEST_ASSERT_FALSE(ring_runtime_health_sensor_degraded(&state));
}

void test_hid_send_persistent_errors_require_restart(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    ring_hid_health_update_t update =
        ring_runtime_health_note_hid_send_result(&state, HAL_ERR_IO, 100);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_FAULT_STARTED, update.event);
    TEST_ASSERT_EQUAL(HAL_ERR_IO, update.status);
    TEST_ASSERT_EQUAL(0, update.fault_elapsed_ms);

    update = ring_runtime_health_note_hid_send_result(
        &state, HAL_ERR_IO, 100 + BLE_HID_SEND_RESTART_MS - 1);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_STABLE, update.event);
    TEST_ASSERT_EQUAL(BLE_HID_SEND_RESTART_MS - 1, update.fault_elapsed_ms);

    update = ring_runtime_health_note_hid_send_result(
        &state, HAL_ERR_IO, 100 + BLE_HID_SEND_RESTART_MS);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_RESTART_REQUIRED, update.event);
    TEST_ASSERT_EQUAL(BLE_HID_SEND_RESTART_MS, update.fault_elapsed_ms);
}

void test_hid_send_success_clears_fault_window(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    ring_runtime_health_note_hid_send_result(&state, HAL_ERR_IO, 50);

    ring_hid_health_update_t update =
        ring_runtime_health_note_hid_send_result(&state, HAL_OK, 75);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_RECOVERED, update.event);
    TEST_ASSERT_EQUAL(HAL_OK, update.status);

    update = ring_runtime_health_note_hid_send_result(
        &state, HAL_ERR_IO, 75 + BLE_HID_SEND_RESTART_MS);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_FAULT_STARTED, update.event);
    TEST_ASSERT_EQUAL(0, update.fault_elapsed_ms);
}

void test_hid_send_busy_is_treated_as_recovery(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    ring_runtime_health_note_hid_send_result(&state, HAL_ERR_IO, 10);

    ring_hid_health_update_t update =
        ring_runtime_health_note_hid_send_result(&state, HAL_ERR_BUSY, 20);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_RECOVERED, update.event);
    TEST_ASSERT_EQUAL(HAL_ERR_BUSY, update.status);

    update = ring_runtime_health_note_hid_send_result(&state, HAL_ERR_BUSY, 30);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_STABLE, update.event);
}

void test_reset_hid_send_clears_fault_window(void)
{
    ring_runtime_health_t state;
    ring_runtime_health_init(&state);

    ring_runtime_health_note_hid_send_result(&state, HAL_ERR_NO_MEM, 10);
    ring_runtime_health_reset_hid_send(&state);

    ring_hid_health_update_t update =
        ring_runtime_health_note_hid_send_result(
            &state, HAL_ERR_IO, 10 + BLE_HID_SEND_RESTART_MS);
    TEST_ASSERT_EQUAL(RING_HID_HEALTH_FAULT_STARTED, update.event);
    TEST_ASSERT_EQUAL(0, update.fault_elapsed_ms);
}

void run_ring_runtime_health_tests(void)
{
    printf("Ring runtime health tests:\n");
    RUN_TEST(test_sensor_degrades_after_threshold_failures);
    RUN_TEST(test_sensor_success_resets_failure_count_before_degrade);
    RUN_TEST(test_sensor_success_recovers_after_degrade);
    RUN_TEST(test_hid_send_persistent_errors_require_restart);
    RUN_TEST(test_hid_send_success_clears_fault_window);
    RUN_TEST(test_hid_send_busy_is_treated_as_recovery);
    RUN_TEST(test_reset_hid_send_clears_fault_window);
}
