// SPDX-License-Identifier: MIT
// PowerFinger — Companion protocol unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "companion_protocol.h"
#include "ble_central.h"
#include "event_composer.h"
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
    TEST_ASSERT_EQUAL(HAL_OK, event_composer_init());
}

static void expect_response(const char *line,
                            const companion_protocol_hub_info_t *info,
                            char *response,
                            size_t response_len,
                            const char *expected)
{
    memset(response, 0, response_len);
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line(line,
                                                     info,
                                                     response,
                                                     response_len));
    TEST_ASSERT_TRUE(strcmp(expected, response) == 0);
}

static size_t role_entry_count(void)
{
    size_t count = 0;
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_get_all(NULL, 0, &count));
    return count;
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

void test_set_gesture_updates_live_composer_and_persisted_readback(void)
{
    reset();
    char response[256] = {0};
    companion_protocol_hub_info_t info = default_hub_info(2);

    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_mark_connected(1, ROLE_SCROLL);
    event_composer_feed(0, 0x01, 5, 3);
    event_composer_feed(1, 0x01, 2, -4);

    expect_response("SET_GESTURE 0x01 0x03",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0x10, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, report.scroll_h);
    TEST_ASSERT_EQUAL(0, report.scroll_v);

    gesture_engine_flush_if_dirty();
    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());

    expect_response("GET_GESTURES",
                    &info,
                    response,
                    sizeof(response),
                    "+ 0x01 0x03 cursor+scroll=forward\n"
                    "+ 0x02 0x00 cursor+modifier=disabled\n"
                    "+ 0x03 0x00 scroll+modifier=disabled\n"
                    "+ 0x04 0x00 all_three=disabled\n"
                    "OK\n");
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

void test_set_hub_reports_live_apply_failures_without_mutating_settings(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    mock_ble_central_inject_scan_policy_failure(HAL_ERR_IO, 1);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("SET_HUB scan_policy 2",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 500 hub_settings_write_failed\n", response) == 0);
    TEST_ASSERT_EQUAL(1, hub_settings_get_scan_policy());
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

void test_ring_settings_and_diagnostics_readback_follow_live_command_sequence(void)
{
    reset();
    char response[320] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    expect_response("SET_RING_DPI 10:11:12:13:14:15 20",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");
    expect_response("SET_RING_DEAD_ZONE_TIME 10:11:12:13:14:15 75",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");
    expect_response("SET_RING_DEAD_ZONE_DISTANCE 10:11:12:13:14:15 12",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");

    expect_response("GET_RING_SETTINGS 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
                    "+ mac=10:11:12:13:14:15\n"
                    "+ dpi_multiplier=20\n"
                    "+ dead_zone_time_ms=75\n"
                    "+ dead_zone_distance=12\n"
                    "+ firmware_version=0.1.0\n"
                    "OK\n");

    expect_response("GET_RING_DIAGNOSTICS 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
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
                    "OK\n");
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

void test_set_role_persists_and_get_rings_uses_reloaded_assignment(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));

    expect_response("SET_ROLE 10:11:12:13:14:15 MODIFIER",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");
    role_engine_flush_if_dirty();

    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);

    expect_response("GET_RINGS",
                    &info,
                    response,
                    sizeof(response),
                    "+ 10:11:12:13:14:15 MODIFIER connected\n"
                    "OK\n");
}

void test_set_role_while_disconnected_applies_on_reconnect_without_stale_input(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_feed(0, 0x01, 7, 4);

    event_composer_ring_disconnected(0);
    mock_ble_central_clear_connected_rings();
    event_composer_feed(0, 0x01, 9, 9);

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);

    expect_response("SET_ROLE 10:11:12:13:14:15 SCROLL",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_A));

    mock_ble_central_set_connected_ring(0, MAC_A);
    event_composer_mark_connected(0, role_engine_get_role(MAC_A));

    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, report.scroll_h);
    TEST_ASSERT_EQUAL(0, report.scroll_v);

    event_composer_feed(0, 0x01, 4, -6);
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0x02, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(4, report.scroll_h);
    TEST_ASSERT_EQUAL(-6, report.scroll_v);

    expect_response("GET_RING_INFO 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
                    "+ mac=10:11:12:13:14:15\n"
                    "+ role=SCROLL\n"
                    "+ connected=1\n"
                    "+ rssi_dbm=-54\n"
                    "OK\n");
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

void test_swap_roles_command_clears_live_cached_input_before_new_roles_apply(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(2);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));
    mock_ble_central_set_connected_ring(0, MAC_A);
    mock_ble_central_set_connected_ring(1, MAC_B);
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_mark_connected(1, ROLE_SCROLL);
    event_composer_feed(0, 0x01, 8, 4);
    event_composer_feed(1, 0x01, 2, -5);

    expect_response("SWAP_ROLES 10:11:12:13:14:15 20:21:22:23:24:25",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, report.scroll_h);
    TEST_ASSERT_EQUAL(0, report.scroll_v);

    event_composer_feed(0, 0x01, 6, -2);
    event_composer_feed(1, 0x01, -3, 9);
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0x03, report.buttons);
    TEST_ASSERT_EQUAL(-3, report.cursor_dx);
    TEST_ASSERT_EQUAL(9, report.cursor_dy);
    TEST_ASSERT_EQUAL(6, report.scroll_h);
    TEST_ASSERT_EQUAL(-2, report.scroll_v);

    expect_response("GET_RINGS",
                    &info,
                    response,
                    sizeof(response),
                    "+ 10:11:12:13:14:15 SCROLL connected\n"
                    "+ 20:21:22:23:24:25 CURSOR connected\n"
                    "OK\n");
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

void test_forget_ring_command_tears_down_live_ring_and_readback_state(void)
{
    reset();
    char response[192] = {0};
    companion_protocol_hub_info_t info = default_hub_info(1);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_set_connected_ring(0, MAC_A);
    mock_ble_central_seed_bond(MAC_A);
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_feed(0, 0x01, 7, 4);

    expect_response("FORGET_RING 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");

    event_composer_feed(0, 0x01, 9, 9);

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, role_entry_count());
    TEST_ASSERT_FALSE(mock_ble_central_has_bond(MAC_A));
    TEST_ASSERT_EQUAL(HAL_ERR_NOT_FOUND,
                      ble_central_find_ring_index_by_mac(MAC_A, &(uint8_t){0}));

    expect_response("GET_RINGS",
                    &info,
                    response,
                    sizeof(response),
                    "OK\n");
    expect_response("GET_RING_SETTINGS 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
                    "ERR 404 unknown_mac\n");
    expect_response("GET_RING_DIAGNOSTICS 10:11:12:13:14:15",
                    &info,
                    response,
                    sizeof(response),
                    "ERR 404 unknown_mac\n");
}

void test_forget_ring_reports_bond_delete_failures(void)
{
    reset();
    char response[64] = {0};
    companion_protocol_hub_info_t info = default_hub_info(0);

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_seed_bond(MAC_A);
    mock_ble_central_inject_delete_bond_failure(HAL_ERR_IO, 1);

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_protocol_handle_line("FORGET_RING 10:11:12:13:14:15",
                                                     &info,
                                                     response,
                                                     sizeof(response)));
    TEST_ASSERT_TRUE(strcmp("ERR 500 forget_failed\n", response) == 0);
    TEST_ASSERT_TRUE(mock_ble_central_has_bond(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
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
    RUN_TEST(test_set_gesture_updates_live_composer_and_persisted_readback);
    RUN_TEST(test_set_gesture_rejects_known_but_unsupported_values);
    RUN_TEST(test_set_hub_updates_persisted_settings_and_live_scan_policy);
    RUN_TEST(test_set_hub_reports_live_apply_failures_without_mutating_settings);
    RUN_TEST(test_set_hub_rejects_unknown_parameter);
    RUN_TEST(test_set_hub_rejects_invalid_value);
    RUN_TEST(test_set_ring_setting_commands_update_live_ring_state);
    RUN_TEST(test_ring_settings_and_diagnostics_readback_follow_live_command_sequence);
    RUN_TEST(test_set_ring_dpi_rejects_invalid_value);
    RUN_TEST(test_set_ring_dpi_reports_known_but_disconnected_ring);
    RUN_TEST(test_unknown_command_returns_protocol_error);
    RUN_TEST(test_set_role_reassigns_known_ring);
    RUN_TEST(test_set_role_persists_and_get_rings_uses_reloaded_assignment);
    RUN_TEST(test_set_role_while_disconnected_applies_on_reconnect_without_stale_input);
    RUN_TEST(test_command_set_role_rejects_unknown_mac);
    RUN_TEST(test_set_role_rejects_invalid_role_name);
    RUN_TEST(test_set_role_rejects_invalid_mac_format);
    RUN_TEST(test_swap_roles_swaps_known_assignments);
    RUN_TEST(test_swap_roles_command_clears_live_cached_input_before_new_roles_apply);
    RUN_TEST(test_role_swap_alias_is_accepted);
    RUN_TEST(test_swap_roles_rejects_identical_macs);
    RUN_TEST(test_swap_roles_rejects_unknown_mac);
    RUN_TEST(test_forget_ring_forgets_known_ring);
    RUN_TEST(test_forget_ring_command_tears_down_live_ring_and_readback_state);
    RUN_TEST(test_forget_ring_reports_bond_delete_failures);
    RUN_TEST(test_forget_ring_rejects_invalid_mac_format);
    RUN_TEST(test_command_forget_ring_rejects_unknown_mac);
    RUN_TEST(test_extra_args_are_rejected);
}
