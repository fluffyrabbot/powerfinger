// SPDX-License-Identifier: MIT
// PowerFinger — Companion session unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "ble_central.h"
#include "companion_session.h"
#include "gesture_engine.h"
#include "hub_identity.h"
#include "role_engine.h"

static const uint8_t MAC_A[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };

static char s_emitted[1024];
static size_t s_emitted_len = 0;
static uint8_t s_connected_rings = 0;

static void fill_hub_info(companion_protocol_hub_info_t *info_out, void *arg)
{
    (void)arg;

    TEST_ASSERT_TRUE(info_out != NULL);
    *info_out = (companion_protocol_hub_info_t) {
        .firmware_revision = hub_identity_firmware_revision(),
        .hardware_revision = hub_identity_hardware_revision(),
        .connected_rings = s_connected_rings,
        .max_rings = HUB_MAX_RINGS,
        .usb_poll_ms = 1,
        .scan_policy = 1,
    };
}

static hal_status_t collect_response(const char *response, void *arg)
{
    (void)arg;

    size_t response_len = strlen(response);
    TEST_ASSERT_TRUE(response_len <= (sizeof(s_emitted) - s_emitted_len - 1));
    memcpy(s_emitted + s_emitted_len, response, response_len);
    s_emitted_len += response_len;
    s_emitted[s_emitted_len] = '\0';
    return HAL_OK;
}

static companion_session_t new_session(void)
{
    companion_session_t session;
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_init(&session,
                                             fill_hub_info,
                                             collect_response,
                                             NULL));
    return session;
}

static void reset(void)
{
    mock_hal_reset();
    mock_ble_central_clear_connected_rings();
    mock_ble_central_clear_bonds();
    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    memset(s_emitted, 0, sizeof(s_emitted));
    s_emitted_len = 0;
    s_connected_rings = 0;
}

void test_companion_session_processes_chunked_commands(void)
{
    reset();
    s_connected_rings = 2;
    companion_session_t session = new_session();

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)"GET_HU",
                                                   strlen("GET_HU")));
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)"B_INFO\n",
                                                   strlen("B_INFO\n")));

    TEST_ASSERT_TRUE(strcmp(
        "+ fw=0.1.0\n"
        "+ hw=DEVBOARD-S3\n"
        "+ rings=2\n"
        "+ max_rings=4\n"
        "+ usb_poll_ms=1\n"
        "+ scan_policy=1\n"
        "OK\n",
        s_emitted) == 0);
}

void test_companion_session_ignores_carriage_return_and_handles_empty_lines(void)
{
    reset();
    companion_session_t session = new_session();

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)"\r\n",
                                                   2));

    TEST_ASSERT_TRUE(strcmp("ERR 400 empty_command\n", s_emitted) == 0);
}

void test_companion_session_rejects_overlong_line_and_recovers(void)
{
    reset();
    s_connected_rings = 1;
    companion_session_t session = new_session();

    char long_command[COMPANION_PROTOCOL_COMMAND_LINE_MAX_LEN + 3] = {0};
    memset(long_command, 'A', sizeof(long_command) - 2);
    long_command[sizeof(long_command) - 2] = '\n';

    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)long_command,
                                                   sizeof(long_command) - 1));
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)"GET_HUB_INFO\n",
                                                   strlen("GET_HUB_INFO\n")));

    TEST_ASSERT_TRUE(strcmp(
        "ERR 400 command_too_long\n"
        "+ fw=0.1.0\n"
        "+ hw=DEVBOARD-S3\n"
        "+ rings=1\n"
        "+ max_rings=4\n"
        "+ usb_poll_ms=1\n"
        "+ scan_policy=1\n"
        "OK\n",
        s_emitted) == 0);
}

void test_companion_session_routes_mutating_commands(void)
{
    reset();
    companion_session_t session = new_session();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(HAL_OK,
                      companion_session_feed_bytes(&session,
                                                   (const uint8_t *)"SET_ROLE 10:11:12:13:14:15 MODIFIER\n",
                                                   strlen("SET_ROLE 10:11:12:13:14:15 MODIFIER\n")));

    TEST_ASSERT_TRUE(strcmp("OK\n", s_emitted) == 0);
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));
}

void run_companion_session_tests(void)
{
    RUN_TEST(test_companion_session_processes_chunked_commands);
    RUN_TEST(test_companion_session_ignores_carriage_return_and_handles_empty_lines);
    RUN_TEST(test_companion_session_rejects_overlong_line_and_recovers);
    RUN_TEST(test_companion_session_routes_mutating_commands);
}
