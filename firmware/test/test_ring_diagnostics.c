// SPDX-License-Identifier: MIT
// PowerFinger — Ring diagnostics unit tests

#include "unity.h"
#include "ring_diagnostics.h"

void test_init_defaults_to_booting_and_unavailable(void)
{
    ring_diagnostics_t state;
    ring_diagnostics_init(&state);

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(RING_STATE_BOOTING, snapshot.ring_state);
    TEST_ASSERT_FALSE(snapshot.connected);
    TEST_ASSERT_EQUAL(0, snapshot.conn_interval_1_25ms);
    TEST_ASSERT_FALSE(snapshot.conn_param_rejected);
    TEST_ASSERT_EQUAL(RING_DIAG_BOND_UNKNOWN, snapshot.bond_state);
    TEST_ASSERT_EQUAL(RING_DIAG_SENSOR_UNAVAILABLE, snapshot.sensor_state);
    TEST_ASSERT_FALSE(snapshot.calibration_valid);
}

void test_connected_session_tracks_interval_bond_and_rejection(void)
{
    ring_diagnostics_t state;
    ring_diagnostics_init(&state);

    ring_diagnostics_note_connected(&state, 12);
    ring_diagnostics_note_conn_param_rejected(&state);
    ring_diagnostics_note_bond_failed(&state);

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_TRUE(snapshot.connected);
    TEST_ASSERT_EQUAL(12, snapshot.conn_interval_1_25ms);
    TEST_ASSERT_TRUE(snapshot.conn_param_rejected);
    TEST_ASSERT_EQUAL(RING_DIAG_BOND_FAILED, snapshot.bond_state);

    ring_diagnostics_note_conn_params_updated(&state, 6);
    ring_diagnostics_note_bond_restored(&state);
    snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(6, snapshot.conn_interval_1_25ms);
    TEST_ASSERT_FALSE(snapshot.conn_param_rejected);
    TEST_ASSERT_EQUAL(RING_DIAG_BOND_RESTORED, snapshot.bond_state);

    ring_diagnostics_note_disconnected(&state);
    snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_FALSE(snapshot.connected);
    TEST_ASSERT_EQUAL(0, snapshot.conn_interval_1_25ms);
    TEST_ASSERT_FALSE(snapshot.conn_param_rejected);
    TEST_ASSERT_EQUAL(RING_DIAG_BOND_RESTORED, snapshot.bond_state);
}

void test_sensor_path_distinguishes_pending_and_ready(void)
{
    ring_diagnostics_t state;
    ring_diagnostics_init(&state);

    ring_diagnostics_note_sensor_path(&state, false, false);
    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(RING_DIAG_SENSOR_UNAVAILABLE, snapshot.sensor_state);
    TEST_ASSERT_FALSE(snapshot.calibration_valid);

    ring_diagnostics_note_sensor_path(&state, true, false);
    snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(RING_DIAG_SENSOR_CALIBRATION_PENDING, snapshot.sensor_state);
    TEST_ASSERT_FALSE(snapshot.calibration_valid);

    ring_diagnostics_note_sensor_path(&state, true, true);
    snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(RING_DIAG_SENSOR_READY, snapshot.sensor_state);
    TEST_ASSERT_TRUE(snapshot.calibration_valid);
}

void test_ring_state_and_battery_snapshot_are_updated(void)
{
    ring_diagnostics_t state;
    ring_diagnostics_init(&state);

    ring_diagnostics_note_ring_state(&state, RING_STATE_CONNECTED_IDLE);
    ring_diagnostics_note_battery(&state, 3725, 52);

    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(&state);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_IDLE, snapshot.ring_state);
    TEST_ASSERT_EQUAL(3725, snapshot.battery_mv);
    TEST_ASSERT_EQUAL(52, snapshot.battery_pct);
}

void test_name_helpers_return_stable_strings(void)
{
    TEST_ASSERT_TRUE(ring_diag_sensor_state_name(RING_DIAG_SENSOR_UNAVAILABLE) != NULL);
    TEST_ASSERT_TRUE(ring_diag_sensor_state_name(RING_DIAG_SENSOR_CALIBRATION_PENDING) != NULL);
    TEST_ASSERT_TRUE(ring_diag_sensor_state_name(RING_DIAG_SENSOR_READY) != NULL);
    TEST_ASSERT_TRUE(ring_diag_bond_state_name(RING_DIAG_BOND_UNKNOWN) != NULL);
    TEST_ASSERT_TRUE(ring_diag_bond_state_name(RING_DIAG_BOND_RESTORED) != NULL);
    TEST_ASSERT_TRUE(ring_diag_bond_state_name(RING_DIAG_BOND_FAILED) != NULL);
}

void test_ble_payload_encodes_snapshot_flags_and_intervals(void)
{
    ring_diagnostics_t state;
    uint8_t payload[RING_DIAG_BLE_PAYLOAD_LEN] = {0};

    ring_diagnostics_init(&state);
    ring_diagnostics_note_ring_state(&state, RING_STATE_CONNECTED_ACTIVE);
    ring_diagnostics_note_connected(&state, 12);
    ring_diagnostics_note_conn_param_rejected(&state);
    ring_diagnostics_note_bond_failed(&state);
    ring_diagnostics_note_sensor_path(&state, true, true);
    ring_diagnostics_note_battery(&state, 3712, 48);

    TEST_ASSERT_EQUAL(RING_DIAG_BLE_PAYLOAD_LEN,
                      ring_diagnostics_encode_ble_payload(&state,
                                                          payload,
                                                          sizeof(payload)));
    TEST_ASSERT_EQUAL(RING_DIAG_BLE_PAYLOAD_VERSION, payload[0]);
    TEST_ASSERT_EQUAL(RING_STATE_CONNECTED_ACTIVE, payload[1]);
    TEST_ASSERT_EQUAL(RING_DIAG_SENSOR_READY, payload[2]);
    TEST_ASSERT_EQUAL(RING_DIAG_BOND_FAILED, payload[3]);
    TEST_ASSERT_EQUAL(0x07, payload[4]);
    TEST_ASSERT_EQUAL(48, payload[5]);
    TEST_ASSERT_EQUAL(0x80, payload[6]);
    TEST_ASSERT_EQUAL(0x0E, payload[7]);
    TEST_ASSERT_EQUAL(12, payload[8]);
    TEST_ASSERT_EQUAL(0, payload[9]);
}

void test_ble_payload_rejects_small_buffer_and_clamps_battery_mv(void)
{
    ring_diagnostics_t state;
    uint8_t payload[RING_DIAG_BLE_PAYLOAD_LEN] = {0};

    ring_diagnostics_init(&state);
    ring_diagnostics_note_battery(&state, 70000, 100);

    TEST_ASSERT_EQUAL(0,
                      ring_diagnostics_encode_ble_payload(&state,
                                                          payload,
                                                          RING_DIAG_BLE_PAYLOAD_LEN - 1));
    TEST_ASSERT_EQUAL(RING_DIAG_BLE_PAYLOAD_LEN,
                      ring_diagnostics_encode_ble_payload(&state,
                                                          payload,
                                                          sizeof(payload)));
    TEST_ASSERT_EQUAL(0xFF, payload[6]);
    TEST_ASSERT_EQUAL(0xFF, payload[7]);
}

void run_ring_diagnostics_tests(void)
{
    printf("Ring diagnostics tests:\n");
    RUN_TEST(test_init_defaults_to_booting_and_unavailable);
    RUN_TEST(test_connected_session_tracks_interval_bond_and_rejection);
    RUN_TEST(test_sensor_path_distinguishes_pending_and_ready);
    RUN_TEST(test_ring_state_and_battery_snapshot_are_updated);
    RUN_TEST(test_name_helpers_return_stable_strings);
    RUN_TEST(test_ble_payload_encodes_snapshot_flags_and_intervals);
    RUN_TEST(test_ble_payload_rejects_small_buffer_and_clamps_battery_mv);
}
