// SPDX-License-Identifier: MIT
// PowerFinger — Power manager unit tests

#include "unity.h"
#include "power_manager.h"
#include "mock_hal.h"
#include "ble_config.h"
#include "ring_config.h"

static void reset(void)
{
    mock_hal_reset();
}

void test_init_primes_battery_cache_from_adc(void)
{
    reset();
    mock_hal_set_adc_mv(3700);

    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(3700, power_manager_get_last_battery_mv());
    TEST_ASSERT_EQUAL(50, power_manager_get_battery_level());
}

void test_motion_requests_active_conn_params_once_per_connection(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_on_connect();
    power_manager_on_motion();

    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_conn_param_request_count());
    uint16_t min_itvl = 0;
    uint16_t max_itvl = 0;
    mock_hal_get_last_ble_conn_param_request(&min_itvl, &max_itvl);
    TEST_ASSERT_EQUAL(BLE_CONN_ITVL_MIN_ACTIVE, min_itvl);
    TEST_ASSERT_EQUAL(BLE_CONN_ITVL_MAX_ACTIVE, max_itvl);

    power_manager_on_motion();
    power_manager_on_click();
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_conn_param_request_count());
}

void test_idle_transition_requests_default_conn_params(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_on_connect();
    power_manager_on_motion();

    TEST_ASSERT_EQUAL(POWER_EVT_NONE, power_manager_tick(IDLE_TRANSITION_MS - 1));
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_conn_param_request_count());

    TEST_ASSERT_EQUAL(POWER_EVT_NONE, power_manager_tick(IDLE_TRANSITION_MS));
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_conn_param_request_count());

    uint16_t min_itvl = 0;
    uint16_t max_itvl = 0;
    mock_hal_get_last_ble_conn_param_request(&min_itvl, &max_itvl);
    TEST_ASSERT_EQUAL(BLE_CONN_ITVL_MIN_DEFAULT, min_itvl);
    TEST_ASSERT_EQUAL(BLE_CONN_ITVL_MAX_DEFAULT, max_itvl);
}

void test_click_resets_sleep_timer(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    TEST_ASSERT_EQUAL(POWER_EVT_NONE, power_manager_tick(SLEEP_TIMEOUT_MS - 1));

    mock_hal_set_time_ms(SLEEP_TIMEOUT_MS - 1);
    power_manager_on_click();

    TEST_ASSERT_EQUAL(POWER_EVT_NONE, power_manager_tick(SLEEP_TIMEOUT_MS + 10));
    TEST_ASSERT_EQUAL(POWER_EVT_SLEEP_TIMEOUT,
                      power_manager_tick((SLEEP_TIMEOUT_MS - 1) + SLEEP_TIMEOUT_MS));
}

void test_low_battery_cutoff_triggers_shutdown(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    mock_hal_set_adc_mv(LOW_VOLTAGE_CUTOFF_MV - 1);
    TEST_ASSERT_EQUAL(POWER_EVT_LOW_BATTERY,
                      power_manager_tick(BATTERY_CHECK_INTERVAL_MS));
}

void test_adc_failure_threshold_forces_shutdown(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    mock_hal_set_adc_status(HAL_ERR_IO);

    for (int i = 1; i < 5; i++) {
        mock_hal_set_time_ms(((uint32_t)i * BATTERY_CHECK_INTERVAL_MS) - 1U);
        power_manager_on_connect();
        TEST_ASSERT_EQUAL(POWER_EVT_NONE,
                          power_manager_tick((uint32_t)i * BATTERY_CHECK_INTERVAL_MS));
    }

    mock_hal_set_time_ms((5U * BATTERY_CHECK_INTERVAL_MS) - 1U);
    power_manager_on_connect();
    TEST_ASSERT_EQUAL(POWER_EVT_LOW_BATTERY,
                      power_manager_tick(5U * BATTERY_CHECK_INTERVAL_MS));
}

void test_rejected_active_params_retry_after_new_connection(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    mock_hal_set_ble_conn_param_status(HAL_ERR_REJECTED);
    power_manager_on_connect();
    power_manager_on_motion();
    power_manager_on_motion();
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_conn_param_request_count());

    mock_hal_set_ble_conn_param_status(HAL_OK);
    power_manager_on_connect();
    power_manager_on_motion();
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_conn_param_request_count());
}

void run_power_manager_tests(void)
{
    printf("Power manager tests:\n");
    RUN_TEST(test_init_primes_battery_cache_from_adc);
    RUN_TEST(test_motion_requests_active_conn_params_once_per_connection);
    RUN_TEST(test_idle_transition_requests_default_conn_params);
    RUN_TEST(test_click_resets_sleep_timer);
    RUN_TEST(test_low_battery_cutoff_triggers_shutdown);
    RUN_TEST(test_adc_failure_threshold_forces_shutdown);
    RUN_TEST(test_rejected_active_params_retry_after_new_connection);
}
