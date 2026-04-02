// SPDX-License-Identifier: MIT
// PowerFinger — Hub control unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "hub_control.h"
#include "event_composer.h"

static const uint8_t MAC_A[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };

static void reset(void)
{
    mock_hal_reset();
    mock_ble_central_clear_connected_rings();
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(HAL_OK, event_composer_init());
}

void test_set_role_persists_for_disconnected_ring(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(HAL_OK, hub_control_set_role(MAC_A, ROLE_MODIFIER));
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));
}

void test_set_role_updates_live_event_composer_cache(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_feed(0, 0x01, 12, 8);
    mock_ble_central_set_connected_ring(0, MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK, hub_control_set_role(MAC_A, ROLE_SCROLL));

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, report.scroll_h);
    TEST_ASSERT_EQUAL(0, report.scroll_v);

    event_composer_feed(0, 0x01, 3, -7);
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0x02, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(3, report.scroll_h);
    TEST_ASSERT_EQUAL(-7, report.scroll_v);
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_A));
}

void test_set_role_rejects_unknown_mac(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_ERR_NOT_FOUND, hub_control_set_role(MAC_A, ROLE_SCROLL));
}

void run_hub_control_tests(void)
{
    printf("Hub control tests:\n");
    RUN_TEST(test_set_role_persists_for_disconnected_ring);
    RUN_TEST(test_set_role_updates_live_event_composer_cache);
    RUN_TEST(test_set_role_rejects_unknown_mac);
}
