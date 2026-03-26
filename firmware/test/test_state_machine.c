// SPDX-License-Identifier: MIT
// PowerFinger — State machine unit tests

#include "unity.h"
#include "ring_state.h"
#include "mock_hal.h"

// --- Helper ---
static void reset(void)
{
    mock_hal_reset();
    ring_state_init();
}

// --- Tests ---

void test_initial_state_is_booting(void)
{
    reset();
    TEST_ASSERT_EQUAL(RING_STATE_BOOTING, ring_state_get());
}

void test_calibration_done_transitions_to_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_t s = ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, s);
    TEST_ASSERT_TRUE(a.start_advertising);
}

void test_calibration_failed_still_transitions_to_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_t s = ring_state_dispatch(RING_EVT_CALIBRATION_FAILED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, s);
    TEST_ASSERT_TRUE(a.start_advertising);
}

void test_ble_connected_transitions_to_idle(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_t s = ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, s);
    TEST_ASSERT_TRUE(a.stop_advertising);
    TEST_ASSERT_TRUE(a.request_idle_conn_params);
}

void test_motion_transitions_idle_to_active(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_MOTION_DETECTED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_ACTIVE, s);
    TEST_ASSERT_TRUE(a.request_active_conn_params);
    TEST_ASSERT_TRUE(a.enable_hid_reports);
}

void test_idle_timeout_transitions_active_to_idle(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);
    ring_state_dispatch(RING_EVT_MOTION_DETECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_IDLE_TIMEOUT, &a);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, s);
    TEST_ASSERT_TRUE(a.request_idle_conn_params);
    TEST_ASSERT_TRUE(a.disable_hid_reports);
}

void test_sleep_timeout_transitions_idle_to_deep_sleep(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_SLEEP_TIMEOUT, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
}

void test_adv_timeout_transitions_to_deep_sleep(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_BLE_ADV_TIMEOUT, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
    TEST_ASSERT_TRUE(a.stop_advertising);
}

void test_disconnect_from_active_goes_to_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);
    ring_state_dispatch(RING_EVT_MOTION_DETECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_BLE_DISCONNECTED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, s);
    TEST_ASSERT_TRUE(a.start_advertising);
    TEST_ASSERT_TRUE(a.disable_hid_reports);
}

void test_disconnect_from_idle_goes_to_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_BLE_DISCONNECTED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, s);
    TEST_ASSERT_TRUE(a.start_advertising);
}

// --- LOW_BATTERY invariant: forces DEEP_SLEEP from any state ---

void test_low_battery_from_booting(void)
{
    reset();
    ring_actions_t a;
    ring_state_t s = ring_state_dispatch(RING_EVT_LOW_BATTERY, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
}

void test_low_battery_from_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_LOW_BATTERY, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
}

void test_low_battery_from_connected_active(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);
    ring_state_dispatch(RING_EVT_MOTION_DETECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_LOW_BATTERY, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
    TEST_ASSERT_TRUE(a.disable_hid_reports);
}

void test_low_battery_from_connected_idle(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);
    ring_state_dispatch(RING_EVT_BLE_CONNECTED, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_LOW_BATTERY, &a);
    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, s);
    TEST_ASSERT_TRUE(a.enter_deep_sleep);
}

// --- Ignored events don't change state ---

void test_motion_ignored_in_advertising(void)
{
    reset();
    ring_actions_t a;
    ring_state_dispatch(RING_EVT_CALIBRATION_DONE, &a);

    ring_state_t s = ring_state_dispatch(RING_EVT_MOTION_DETECTED, &a);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, s);
}

void test_state_names_not_null(void)
{
    for (int i = 0; i < RING_STATE_COUNT; i++) {
        TEST_ASSERT_TRUE(ring_state_name((ring_state_t)i) != NULL);
    }
    for (int i = 0; i < RING_EVT_COUNT; i++) {
        TEST_ASSERT_TRUE(ring_event_name((ring_event_t)i) != NULL);
    }
}

// --- Test runner ---

void run_state_machine_tests(void)
{
    printf("State machine tests:\n");
    RUN_TEST(test_initial_state_is_booting);
    RUN_TEST(test_calibration_done_transitions_to_advertising);
    RUN_TEST(test_calibration_failed_still_transitions_to_advertising);
    RUN_TEST(test_ble_connected_transitions_to_idle);
    RUN_TEST(test_motion_transitions_idle_to_active);
    RUN_TEST(test_idle_timeout_transitions_active_to_idle);
    RUN_TEST(test_sleep_timeout_transitions_idle_to_deep_sleep);
    RUN_TEST(test_adv_timeout_transitions_to_deep_sleep);
    RUN_TEST(test_disconnect_from_active_goes_to_advertising);
    RUN_TEST(test_disconnect_from_idle_goes_to_advertising);
    RUN_TEST(test_low_battery_from_booting);
    RUN_TEST(test_low_battery_from_advertising);
    RUN_TEST(test_low_battery_from_connected_active);
    RUN_TEST(test_low_battery_from_connected_idle);
    RUN_TEST(test_motion_ignored_in_advertising);
    RUN_TEST(test_state_names_not_null);
}
