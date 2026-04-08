// SPDX-License-Identifier: MIT
// PowerFinger — Companion protocol unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "companion_protocol.h"
#include "ble_central.h"
#include "gesture_engine.h"
#include "hub_settings.h"
#include "hub_identity.h"
#include "role_engine.h"

static const uint8_t MAC_A[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };
static const uint8_t MAC_B[6] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25 };

static companion_protocol_hub_info_t default_hub_info(uint8_t connected_rings)
{
    companion_protocol_hub_info_t info = {
        .firmware_revision = hub_identity_firmware_revision(),
        .hardware_revision = hub_identity_hardware_revision(),
        .connected_rings = connected_rings,
        .max_rings = HUB_MAX_RINGS,
        .usb_poll_ms = 1,
        .scan_policy = 1,
        .expected_rings = 2,
    };
    return info;
}

static void reset(void)
{
    mock_hal_reset();
    mock_ble_central_clear_connected_rings();
    mock_ble_central_clear_bonds();
    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());
    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_init());
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
}

void test_get_hub_info_formats_truthful_snapshot(void)
{
    reset();

    char response[160] = {0};
    companion_protocol_hub_info_t info = default_hub_info(2);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_HUB_INFO",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ fw=0.1.0\n"
        "+ hw=DEVBOARD-S3\n"
        "+ rings=2\n"
        "+ max_rings=4\n"
        "+ usb_poll_ms=1\n"
        "+ scan_policy=1\n"
        "+ expected_rings=2\n"
        "OK\n",
        response) == 0);
}

void test_get_roles_formats_known_assignments(void)
{
    reset();
    char response[160] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("get_roles",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ 10:11:12:13:14:15 CURSOR\n"
        "+ 20:21:22:23:24:25 SCROLL\n"
        "OK\n",
        response) == 0);
}

void test_get_roles_returns_ok_when_empty(void)
{
    reset();
    char response[32] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_ROLES",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
}

void test_get_rings_merges_role_map_with_live_connection_status(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RINGS",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ 10:11:12:13:14:15 CURSOR connected\n"
        "+ 20:21:22:23:24:25 SCROLL disconnected\n"
        "OK\n",
        response) == 0);
}

void test_get_ring_info_reports_known_ring_snapshot(void)
{
    reset();
    char response[160] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);
    mock_ble_central_set_ring_rssi(0, -61);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_INFO 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ mac=10:11:12:13:14:15\n"
        "+ role=CURSOR\n"
        "+ connected=1\n"
        "+ rssi_dbm=-61\n"
        "OK\n",
        response) == 0);
}

void test_get_ring_info_rejects_unknown_mac(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_INFO 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 404 unknown_mac\n", response) == 0);
}

void test_get_ring_settings_reads_live_config_from_connected_ring(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_SETTINGS 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ mac=10:11:12:13:14:15\n"
        "+ dpi_multiplier=10\n"
        "+ dead_zone_time_ms=50\n"
        "+ dead_zone_distance=10\n"
        "+ firmware_version=0.1.0\n"
        "OK\n",
        response) == 0);
}

void test_get_ring_settings_reports_known_but_disconnected_ring(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_SETTINGS 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 409 ring_not_connected\n", response) == 0);
}

void test_get_ring_diagnostics_reads_live_snapshot_from_connected_ring(void)
{
    reset();
    char response[320] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_DIAGNOSTICS 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ mac=10:11:12:13:14:15\n"
        "+ battery_pct=50\n"
        "+ battery_mv=3700\n"
        "+ ring_state=CONNECTED_IDLE\n"
        "+ sensor_state=READY\n"
        "+ bond_state=RESTORED\n"
        "+ connected=1\n"
        "+ calibration_valid=1\n"
        "+ conn_param_rejected=0\n"
        "+ conn_interval_1_25ms=12\n"
        "+ diagnostics_version=1\n"
        "OK\n",
        response) == 0);
}

void test_get_ring_diagnostics_reports_known_but_disconnected_ring(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_RING_DIAGNOSTICS 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 409 ring_not_connected\n", response) == 0);
}

void test_get_gestures_returns_supported_trigger_table(void)
{
    reset();
    char response[256] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_GESTURES",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp(
        "+ 0x01 0x00 cursor+scroll=disabled\n"
        "+ 0x02 0x00 cursor+modifier=disabled\n"
        "+ 0x03 0x00 scroll+modifier=disabled\n"
        "+ 0x04 0x00 all_three=disabled\n"
        "OK\n",
        response) == 0);
}

void test_set_gesture_updates_persisted_mapping(void)
{
    reset();
    char response[256] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_GESTURE 0x01 0x02",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(GESTURE_ACTION_BACK,
                      gesture_engine_get_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_GESTURES",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strstr(response, "+ 0x01 0x02 cursor+scroll=back\n") != NULL);
}

void test_set_gesture_rejects_known_but_unsupported_values(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_GESTURE 0x05 0x01",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 501 unsupported_gesture_trigger\n", response) == 0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_GESTURE 0x01 0x05",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 501 unsupported_gesture_action\n", response) == 0);
}

void test_set_hub_updates_persisted_settings_and_live_scan_policy(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB usb_poll_ms 4",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(4, hub_settings_get_usb_poll_ms());

    memset(response, 0, sizeof(response));
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB scan_policy 2",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(2, mock_ble_central_get_scan_policy());

    memset(response, 0, sizeof(response));
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB expected_rings 3",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(3, mock_ble_central_get_expected_rings());
}

void test_set_hub_rejects_unknown_parameter(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB nope 1",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_param\n", response) == 0);
}

void test_set_hub_rejects_invalid_value(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB usb_poll_ms 3",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_value\n", response) == 0);
}

void test_set_ring_setting_commands_update_live_ring_state(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);
    hub_ring_settings_t settings = {0};

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_RING_DPI 10:11:12:13:14:15 20",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_RING_DEAD_ZONE_TIME 10:11:12:13:14:15 75",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_RING_DEAD_ZONE_DISTANCE 10:11:12:13:14:15 12",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);

    TEST_ASSERT_EQUAL(HAL_OK, ble_central_get_ring_settings_by_mac(MAC_A, &settings));
    TEST_ASSERT_EQUAL(20, settings.dpi_multiplier);
    TEST_ASSERT_EQUAL(75, settings.dead_zone_time_ms);
    TEST_ASSERT_EQUAL(12, settings.dead_zone_distance);
    TEST_ASSERT_EQUAL(0, settings.firmware_version[0]);
    TEST_ASSERT_EQUAL(1, settings.firmware_version[1]);
    TEST_ASSERT_EQUAL(0, settings.firmware_version[2]);
}

void test_set_ring_dpi_rejects_invalid_value(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_RING_DPI 10:11:12:13:14:15 0",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_value\n", response) == 0);
}

void test_set_ring_dpi_reports_known_but_disconnected_ring(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_RING_DPI 10:11:12:13:14:15 20",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 409 ring_not_connected\n", response) == 0);
}

void test_unknown_command_returns_protocol_error(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("PING",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 unknown_command\n", response) == 0);
}

void test_set_role_reassigns_known_ring(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_ROLE 10:11:12:13:14:15 modifier",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));
}

void test_command_set_role_rejects_unknown_mac(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_ROLE 10:11:12:13:14:15 SCROLL",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 404 unknown_mac\n", response) == 0);
}

void test_set_role_rejects_invalid_role_name(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_ROLE 10:11:12:13:14:15 FOOBAR",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_role\n", response) == 0);
}

void test_set_role_rejects_invalid_mac_format(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_ROLE 10-11-12-13-14-15 SCROLL",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_mac\n", response) == 0);
}

void test_swap_roles_swaps_known_assignments(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SWAP_ROLES 10:11:12:13:14:15 20:21:22:23:24:25",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_B));
}

void test_role_swap_alias_is_accepted(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("ROLE_SWAP 10:11:12:13:14:15 20:21:22:23:24:25",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
}

void test_swap_roles_rejects_identical_macs(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SWAP_ROLES 10:11:12:13:14:15 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 identical_macs\n", response) == 0);
}

void test_swap_roles_rejects_unknown_mac(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SWAP_ROLES 10:11:12:13:14:15 20:21:22:23:24:25",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 404 unknown_mac\n", response) == 0);
}

void test_forget_ring_forgets_known_ring(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_seed_bond(MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("FORGET_RING 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("OK\n", response) == 0);
    TEST_ASSERT_FALSE(mock_ble_central_has_bond(MAC_A));
}

void test_forget_ring_rejects_invalid_mac_format(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("FORGET_RING 10-11-12-13-14-15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 invalid_mac\n", response) == 0);
}

void test_command_forget_ring_rejects_unknown_mac(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("FORGET_RING 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 404 unknown_mac\n", response) == 0);
}

void test_extra_args_are_rejected(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("GET_HUB_INFO now",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 400 unexpected_args\n", response) == 0);
}

void run_companion_protocol_tests(void)
{
    printf("Companion protocol tests:\n");
    RUN_TEST(test_get_hub_info_formats_truthful_snapshot);
    RUN_TEST(test_get_roles_formats_known_assignments);
    RUN_TEST(test_get_roles_returns_ok_when_empty);
    RUN_TEST(test_get_rings_merges_role_map_with_live_connection_status);
    RUN_TEST(test_get_ring_info_reports_known_ring_snapshot);
    RUN_TEST(test_get_ring_info_rejects_unknown_mac);
    RUN_TEST(test_get_ring_settings_reads_live_config_from_connected_ring);
    RUN_TEST(test_get_ring_settings_reports_known_but_disconnected_ring);
    RUN_TEST(test_get_ring_diagnostics_reads_live_snapshot_from_connected_ring);
    RUN_TEST(test_get_ring_diagnostics_reports_known_but_disconnected_ring);
    RUN_TEST(test_get_gestures_returns_supported_trigger_table);
    RUN_TEST(test_set_gesture_updates_persisted_mapping);
    RUN_TEST(test_set_gesture_rejects_known_but_unsupported_values);
    RUN_TEST(test_set_hub_updates_persisted_settings_and_live_scan_policy);
    RUN_TEST(test_set_hub_rejects_unknown_parameter);
    RUN_TEST(test_set_hub_rejects_invalid_value);
    RUN_TEST(test_set_ring_setting_commands_update_live_ring_state);
    RUN_TEST(test_set_ring_dpi_rejects_invalid_value);
    RUN_TEST(test_set_ring_dpi_reports_known_but_disconnected_ring);
    RUN_TEST(test_unknown_command_returns_protocol_error);
    RUN_TEST(test_set_role_reassigns_known_ring);
    RUN_TEST(test_command_set_role_rejects_unknown_mac);
    RUN_TEST(test_set_role_rejects_invalid_role_name);
    RUN_TEST(test_set_role_rejects_invalid_mac_format);
    RUN_TEST(test_swap_roles_swaps_known_assignments);
    RUN_TEST(test_role_swap_alias_is_accepted);
    RUN_TEST(test_swap_roles_rejects_identical_macs);
    RUN_TEST(test_swap_roles_rejects_unknown_mac);
    RUN_TEST(test_forget_ring_forgets_known_ring);
    RUN_TEST(test_forget_ring_rejects_invalid_mac_format);
    RUN_TEST(test_command_forget_ring_rejects_unknown_mac);
    RUN_TEST(test_extra_args_are_rejected);
}
