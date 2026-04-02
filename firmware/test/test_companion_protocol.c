// SPDX-License-Identifier: MIT
// PowerFinger — Companion protocol unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "companion_protocol.h"
#include "ble_central.h"
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
    };
    return info;
}

static void reset(void)
{
    mock_hal_reset();
    mock_ble_central_clear_connected_rings();
    mock_ble_central_clear_bonds();
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
