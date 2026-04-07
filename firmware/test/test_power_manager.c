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

    // After init, sensor power should be on. Verify by gating off and checking.
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_set_sensor_power(false));
    hal_pin_t pin = HAL_PIN_NONE;
    bool level = true;
    mock_hal_get_last_gpio_set(&pin, &level);
    TEST_ASSERT_EQUAL(9, pin);
    TEST_ASSERT_FALSE(level);

    // Gate back on
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_set_sensor_power(true));
    mock_hal_get_last_gpio_set(&pin, &level);
    TEST_ASSERT_EQUAL(9, pin);
    TEST_ASSERT_TRUE(level);
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

// --- NTC ADC test values (computed from B3950 equation with R0=10k, Rdiv=10k, VCC=3300) ---
// These are mV values that map to specific temperatures via the NTC beta equation.
#define NTC_ADC_MV_25C  1650   // 25°C — room temperature (safe)
#define NTC_ADC_MV_38C  1204   // 38°C — below resume threshold (safe to resume)
#define NTC_ADC_MV_43C  1050   // ~43°C — between cutoff and resume (still locked out)
#define NTC_ADC_MV_46C   973   // 46°C — above cutoff threshold (disable charging)
#define NTC_ADC_MV_62C   620   // 62°C — above emergency threshold (force shutdown)
#define NTC_ADC_MV_NEG2C 2603  // −2°C — below cold cutoff (disable charging)
#define NTC_ADC_MV_7C   2312   // 7°C — above cold resume (safe to resume)

// Test Kconfig pin assignments (must match CMakeLists.txt compile definitions):
// NTC_ADC_CHANNEL=1, CHARGE_ENABLE_PIN=3, VBUS_DETECT_PIN=4
#define TEST_NTC_CH       1
#define TEST_CHARGE_PIN   3
#define TEST_VBUS_PIN     4

// Helper: set NTC to a safe temperature and VBUS present, then tick to
// evaluate charge state. Returns the power event from the tick.
static power_event_t setup_charging(uint32_t tick_ms)
{
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    return power_manager_tick(tick_ms);
}

void test_init_disables_charging_by_default(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Charge enable GPIO should be HIGH (MOSFET off = charging disabled).
    // VBUS is not present at init (pull-down default).
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
}

void test_charging_enabled_when_vbus_present_and_temp_safe(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Simulate VBUS present + safe temperature
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);

    // Tick past thermal check interval
    power_manager_tick(THERMAL_CHECK_INTERVAL_MS + 1);

    TEST_ASSERT_TRUE(power_manager_is_vbus_present());
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());
}

void test_charging_disabled_on_high_temp(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Start charging normally
    setup_charging(THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());

    // Temperature rises above cutoff (46°C > 45°C)
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_46C);
    power_manager_tick(2 * THERMAL_CHECK_INTERVAL_MS + 1);

    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
}

void test_charging_resumes_with_hysteresis(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Start charging, then overheat
    setup_charging(THERMAL_CHECK_INTERVAL_MS + 1);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_46C);
    power_manager_tick(2 * THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());

    // Cool to ~43°C (between cutoff and resume) — should NOT resume yet
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_43C);
    power_manager_tick(3 * THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());

    // Cool to 38°C (below resume threshold) — should resume
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_38C);
    power_manager_tick(4 * THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());
}

void test_thermal_emergency_forces_shutdown(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Set extreme temperature and VBUS present (charging scenario)
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_62C);

    power_event_t evt = power_manager_tick(THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_EQUAL(POWER_EVT_THERMAL_SHUTDOWN, evt);

    // Charging should be disabled
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
}

void test_thermal_emergency_overrides_vbus_sleep_suppression(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Connect and go idle with VBUS present
    power_manager_on_connect();
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);

    // Should NOT get sleep timeout while VBUS is present
    power_event_t evt = power_manager_tick(SLEEP_TIMEOUT_MS + 1);
    TEST_ASSERT_NOT_EQUAL(POWER_EVT_SLEEP_TIMEOUT, evt);

    // But thermal emergency MUST fire even with VBUS
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_62C);
    evt = power_manager_tick(SLEEP_TIMEOUT_MS + THERMAL_CHECK_INTERVAL_MS + 2);
    TEST_ASSERT_EQUAL(POWER_EVT_THERMAL_SHUTDOWN, evt);
}

void test_vbus_suppresses_sleep_timeout(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Connect with VBUS present
    power_manager_on_connect();
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);

    // Sleep timeout would normally fire, but VBUS suppresses it
    power_event_t evt = power_manager_tick(SLEEP_TIMEOUT_MS + 1);
    TEST_ASSERT_NOT_EQUAL(POWER_EVT_SLEEP_TIMEOUT, evt);

    // Unplug VBUS — sleep timeout should now fire
    mock_hal_set_gpio_input(TEST_VBUS_PIN, false);
    mock_hal_set_time_ms(SLEEP_TIMEOUT_MS + 1);
    evt = power_manager_tick(2 * SLEEP_TIMEOUT_MS + 2);
    TEST_ASSERT_EQUAL(POWER_EVT_SLEEP_TIMEOUT, evt);
}

void test_cold_cutoff_disables_charging(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // VBUS present but cell is cold
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_NEG2C);

    power_manager_tick(THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());

    // Warm up to above cold resume
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_7C);
    power_manager_tick(2 * THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());
}

void test_overvoltage_disables_charging(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Start charging normally
    setup_charging(THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());

    // VBAT exceeds overvoltage threshold during battery check
    mock_hal_set_adc_mv(CHARGE_OVERVOLTAGE_MV + 10);
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);

    power_manager_tick(BATTERY_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
}

void test_cell_temp_api_returns_last_reading(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);
    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    power_manager_tick(THERMAL_CHECK_INTERVAL_MS + 1);

    int8_t temp = power_manager_get_cell_temp_c();
    // Allow ±2°C tolerance for integer truncation in NTC conversion
    TEST_ASSERT_TRUE(temp >= 23 && temp <= 27);
}

void test_charging_api_reports_state(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
    TEST_ASSERT_FALSE(power_manager_is_vbus_present());

    mock_hal_set_gpio_input(TEST_VBUS_PIN, true);
    mock_hal_set_adc_mv_channel(TEST_NTC_CH, NTC_ADC_MV_25C);
    power_manager_tick(THERMAL_CHECK_INTERVAL_MS + 1);

    TEST_ASSERT_TRUE(power_manager_is_vbus_present());
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());
}

void test_deep_sleep_disables_charging(void)
{
    reset();
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());

    // Enable charging
    setup_charging(THERMAL_CHECK_INTERVAL_MS + 1);
    TEST_ASSERT_TRUE(power_manager_is_charging_enabled());

    // Enter deep sleep — charging must be disabled for safety
    power_manager_enter_sleep(true);
    TEST_ASSERT_FALSE(power_manager_is_charging_enabled());
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

    printf("\nThermal safety + charge control tests:\n");
    RUN_TEST(test_init_disables_charging_by_default);
    RUN_TEST(test_charging_enabled_when_vbus_present_and_temp_safe);
    RUN_TEST(test_charging_disabled_on_high_temp);
    RUN_TEST(test_charging_resumes_with_hysteresis);
    RUN_TEST(test_thermal_emergency_forces_shutdown);
    RUN_TEST(test_thermal_emergency_overrides_vbus_sleep_suppression);
    RUN_TEST(test_vbus_suppresses_sleep_timeout);
    RUN_TEST(test_cold_cutoff_disables_charging);
    RUN_TEST(test_overvoltage_disables_charging);
    RUN_TEST(test_cell_temp_api_returns_last_reading);
    RUN_TEST(test_charging_api_reports_state);
    RUN_TEST(test_deep_sleep_disables_charging);
}
