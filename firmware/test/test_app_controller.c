// SPDX-License-Identifier: MIT
// PowerFinger — App controller unit tests

#include "unity.h"
#include "mock_hal.h"
#include "app_controller.h"
#include "ble_config.h"
#include "power_manager.h"

static app_controller_t s_controller;
static ring_diagnostics_t s_diagnostics;
static ring_runtime_health_t s_runtime_health;
static dead_zone_ctx_t s_primary_dead_zone;
static dead_zone_ctx_t s_secondary_dead_zone;

static void reset(void)
{
    mock_hal_reset();
    ring_state_init();
    ring_diagnostics_init(&s_diagnostics);
    ring_runtime_health_init(&s_runtime_health);
    dead_zone_init(&s_primary_dead_zone);
    dead_zone_init(&s_secondary_dead_zone);
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    app_controller_init(&s_controller,
                        &s_diagnostics,
                        &s_runtime_health,
                        &s_primary_dead_zone,
                        &s_secondary_dead_zone);
}

void test_calibration_done_starts_advertising_from_boot(void)
{
    reset();

    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_CALIBRATION_DONE,
                                       0,
                                       100,
                                       NULL);

    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, ring_state_get());
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_adv_start_count());
    TEST_ASSERT_EQUAL(0, mock_hal_get_ble_adv_stop_count());
    TEST_ASSERT_EQUAL(BLE_ADVERTISE_TIMEOUT_MS,
                      mock_hal_get_last_ble_adv_timeout_ms());

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&s_diagnostics);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, snapshot.ring_state);
}

void test_connected_event_updates_snapshot_and_clears_button_state(void)
{
    reset();
    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_CALIBRATION_DONE,
                                       0,
                                       10,
                                       NULL);
    app_controller_set_previous_buttons(&s_controller, 0x01);

    app_controller_event_t event = {
        .type = APP_CONTROLLER_EVT_RING_STATE,
        .ring_evt = RING_EVT_BLE_CONNECTED,
        .conn_interval_1_25ms = 12,
    };
    app_controller_handle_event(&s_controller, &event, 20);

    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, ring_state_get());
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_adv_start_count());
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_adv_stop_count());
    TEST_ASSERT_EQUAL(0, app_controller_previous_buttons(&s_controller));

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&s_diagnostics);
    TEST_ASSERT_TRUE(snapshot.connected);
    TEST_ASSERT_EQUAL(12, snapshot.conn_interval_1_25ms);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, snapshot.ring_state);
}

void test_disconnect_event_returns_to_advertising_and_resets_buttons(void)
{
    reset();
    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_CALIBRATION_DONE,
                                       0,
                                       10,
                                       NULL);
    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_BLE_CONNECTED,
                                       12,
                                       20,
                                       "connect");
    app_controller_set_previous_buttons(&s_controller, 0x03);

    app_controller_event_t event = {
        .type = APP_CONTROLLER_EVT_RING_STATE,
        .ring_evt = RING_EVT_BLE_DISCONNECTED,
    };
    app_controller_handle_event(&s_controller, &event, 50);

    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, ring_state_get());
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_adv_start_count());
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_adv_stop_count());
    TEST_ASSERT_EQUAL(0, app_controller_previous_buttons(&s_controller));

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&s_diagnostics);
    TEST_ASSERT_FALSE(snapshot.connected);
    TEST_ASSERT_EQUAL(RING_STATE_ADVERTISING, snapshot.ring_state);
}

void test_reconcile_repairs_missed_connect_event(void)
{
    reset();
    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_CALIBRATION_DONE,
                                       0,
                                       10,
                                       NULL);
    mock_hal_set_ble_connected(true);

    app_controller_reconcile_ble_event_drops(&s_controller, 1, true, 100);

    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, ring_state_get());
    TEST_ASSERT_EQUAL(1, mock_hal_get_ble_adv_stop_count());

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&s_diagnostics);
    TEST_ASSERT_TRUE(snapshot.connected);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, snapshot.ring_state);
}

void test_low_battery_event_forces_sleep(void)
{
    reset();
    app_controller_dispatch_ring_event(&s_controller,
                                       RING_EVT_CALIBRATION_DONE,
                                       0,
                                       10,
                                       NULL);

    app_controller_handle_power_event(&s_controller, POWER_EVT_LOW_BATTERY, 200);

    TEST_ASSERT_EQUAL(RING_STATE_DEEP_SLEEP, ring_state_get());
    TEST_ASSERT_EQUAL(1, mock_hal_get_sleep_enter_count());
}

void test_conn_update_forwarded_to_power_manager_without_diagnostics(void)
{
    reset();
    app_controller_t controller;
    app_controller_init(&controller, NULL, NULL, NULL, NULL);
    app_controller_dispatch_ring_event(&controller,
                                       RING_EVT_BLE_CONNECTED,
                                       BLE_CONN_ITVL_15MS,
                                       0,
                                       NULL);
    app_controller_event_t update = {
        .type = APP_CONTROLLER_EVT_CONN_PARAMS_UPDATED,
        .conn_interval_1_25ms = BLE_CONN_ITVL_15MS,
    };
    app_controller_handle_event(&controller, &update, 1);
    power_manager_on_motion();
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_conn_param_request_count());
}

void test_conn_rejection_forwarded_to_power_manager_without_diagnostics(void)
{
    reset();
    app_controller_t controller;
    app_controller_init(&controller, NULL, NULL, NULL, NULL);
    app_controller_dispatch_ring_event(&controller,
                                       RING_EVT_BLE_CONNECTED,
                                       BLE_CONN_ITVL_15MS,
                                       0,
                                       NULL);
    app_controller_event_t rejected = {
        .type = APP_CONTROLLER_EVT_CONN_PARAMS_REJECTED,
    };
    app_controller_handle_event(&controller, &rejected, 1);
    power_manager_on_motion();
    TEST_ASSERT_EQUAL(2, mock_hal_get_ble_conn_param_request_count());
}

void run_app_controller_tests(void)
{
    printf("App controller tests:\n");
    RUN_TEST(test_calibration_done_starts_advertising_from_boot);
    RUN_TEST(test_connected_event_updates_snapshot_and_clears_button_state);
    RUN_TEST(test_disconnect_event_returns_to_advertising_and_resets_buttons);
    RUN_TEST(test_reconcile_repairs_missed_connect_event);
    RUN_TEST(test_low_battery_event_forces_sleep);
    RUN_TEST(test_conn_update_forwarded_to_power_manager_without_diagnostics);
    RUN_TEST(test_conn_rejection_forwarded_to_power_manager_without_diagnostics);
}
