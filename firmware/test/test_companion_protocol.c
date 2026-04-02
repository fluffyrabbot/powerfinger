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
    RUN_TEST(test_extra_args_are_rejected);
}
