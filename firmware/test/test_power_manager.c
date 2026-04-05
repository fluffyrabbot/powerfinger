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

    power_event_t evt = power_manager_tick(IDLE_TRANSITION_MS - 1);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_conn_param_request_count());

    evt = power_manager_tick(IDLE_TRANSITION_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_IDLE_TIMEOUT, evt);
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
    power_manager_on_connect();

    power_event_t evt = power_manager_tick(SLEEP_TIMEOUT_MS - 1);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);

    mock_hal_set_time_ms(SLEEP_TIMEOUT_MS - 1);
    power_manager_on_click();

    evt = power_manager_tick(SLEEP_TIMEOUT_MS + 10);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);
    evt = power_manager_tick((SLEEP_TIMEOUT_MS - 1) + IDLE_TRANSITION_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_IDLE_TIMEOUT, evt);
    evt = power_manager_tick((SLEEP_TIMEOUT_MS - 1) + SLEEP_TIMEOUT_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_SLEEP_TIMEOUT, evt);
}

void test_sleep_timeout_requires_idle_first(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_on_connect();
    power_manager_on_motion();

    power_event_t evt = power_manager_tick(IDLE_TRANSITION_MS - 1);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);

    evt = power_manager_tick(IDLE_TRANSITION_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_IDLE_TIMEOUT, evt);

    evt = power_manager_tick(SLEEP_TIMEOUT_MS - 1);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);
}

void test_disconnect_suppresses_connected_timeouts(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_on_connect();
    power_manager_on_motion();
    power_manager_on_disconnect();

    power_event_t evt = power_manager_tick(SLEEP_TIMEOUT_MS + 100);
    TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);
}

void test_low_battery_cutoff_triggers_shutdown(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    mock_hal_set_adc_mv(LOW_VOLTAGE_CUTOFF_MV - 1);
    power_event_t evt = power_manager_tick(BATTERY_CHECK_INTERVAL_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_LOW_BATTERY, evt);
}

void test_adc_failure_threshold_forces_shutdown(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    mock_hal_set_adc_status(HAL_ERR_IO);

    for (int i = 1; i < 5; i++) {
        mock_hal_set_time_ms(((uint32_t)i * BATTERY_CHECK_INTERVAL_MS) - 1U);
        power_manager_on_connect();
        power_event_t evt = power_manager_tick((uint32_t)i * BATTERY_CHECK_INTERVAL_MS);
        TEST_ASSERT_EQUAL(POWER_EVT_NONE, evt);
    }

    mock_hal_set_time_ms((5U * BATTERY_CHECK_INTERVAL_MS) - 1U);
    power_manager_on_connect();
    power_event_t evt = power_manager_tick(5U * BATTERY_CHECK_INTERVAL_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_LOW_BATTERY, evt);
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
    power_event_t evt = power_manager_tick(IDLE_TRANSITION_MS);
    TEST_ASSERT_EQUAL(POWER_EVT_IDLE_TIMEOUT, evt);

    mock_hal_set_ble_conn_param_status(HAL_OK);
    power_manager_on_connect();
    power_manager_on_motion();
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_conn_param_request_count());
}

void test_sensor_power_can_be_gated_for_calibration_failure(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    hal_pin_t pin = HAL_PIN_NONE;
    bool level = false;
    mock_hal_get_last_gpio_set(&pin, &level);
    TEST_ASSERT_EQUAL(9, pin);
    TEST_ASSERT_TRUE(level);

    TEST_ASSERT_EQUAL(HAL_OK, power_manager_set_sensor_power(false));
    mock_hal_get_last_gpio_set(&pin, &level);
    TEST_ASSERT_EQUAL(9, pin);
    TEST_ASSERT_FALSE(level);
}

void test_click_activity_restores_sensor_power_after_gate_off(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_set_sensor_power(false));

    power_manager_on_click();

    hal_pin_t pin = HAL_PIN_NONE;
    bool level = false;
    mock_hal_get_last_gpio_set(&pin, &level);
    TEST_ASSERT_EQUAL(9, pin);
    TEST_ASSERT_TRUE(level);
}

void test_deep_sleep_uses_configured_wake_gpio_mask(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_enter_sleep(true);

    uint64_t mask = 0;
    bool level = true;
    mock_hal_get_last_wake_gpio_mask(&mask, &level);
    TEST_ASSERT_TRUE(mask == 0x100ULL);
    TEST_ASSERT_FALSE(level);
    TEST_ASSERT_EQUAL(1, mock_hal_get_wake_gpio_config_count());
    TEST_ASSERT_EQUAL(HAL_SLEEP_DEEP, mock_hal_get_last_sleep_mode());
    TEST_ASSERT_EQUAL(1, mock_hal_get_sleep_enter_count());
}

void test_runtime_wake_gpio_mask_override_is_applied(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    power_manager_set_wake_gpio_mask(0x30ULL);
    power_manager_enter_sleep(true);

    uint64_t mask = 0;
    bool level = true;
    mock_hal_get_last_wake_gpio_mask(&mask, &level);
    TEST_ASSERT_TRUE(mask == 0x30ULL);
    TEST_ASSERT_FALSE(level);
}

void run_power_manager_tests(void)
{
    printf("Power manager tests:\n");
    RUN_TEST(test_init_primes_battery_cache_from_adc);
    RUN_TEST(test_motion_requests_active_conn_params_once_per_connection);
    RUN_TEST(test_idle_transition_requests_default_conn_params);
    RUN_TEST(test_click_resets_sleep_timer);
    RUN_TEST(test_sleep_timeout_requires_idle_first);
    RUN_TEST(test_disconnect_suppresses_connected_timeouts);
    RUN_TEST(test_low_battery_cutoff_triggers_shutdown);
    RUN_TEST(test_adc_failure_threshold_forces_shutdown);
    RUN_TEST(test_rejected_active_params_retry_after_new_connection);
    RUN_TEST(test_sensor_power_can_be_gated_for_calibration_failure);
    RUN_TEST(test_click_activity_restores_sensor_power_after_gate_off);
    RUN_TEST(test_deep_sleep_uses_configured_wake_gpio_mask);
    RUN_TEST(test_runtime_wake_gpio_mask_override_is_applied);
}
