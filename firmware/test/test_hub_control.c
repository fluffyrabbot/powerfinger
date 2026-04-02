// SPDX-License-Identifier: MIT
// PowerFinger — Hub control unit tests

#include <string.h>

#include "unity.h"
#include "mock_hal.h"
#include "ble_central.h"
#include "hub_control.h"
#include "event_composer.h"

static const uint8_t MAC_A[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };
static const uint8_t MAC_B[6] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25 };

static void reset(void)
{
    mock_hal_reset();
    mock_ble_central_clear_connected_rings();
    mock_ble_central_clear_bonds();
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(HAL_OK, event_composer_init());
}

static size_t role_entry_count(void)
{
    size_t count = 0;
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_get_all(NULL, 0, &count));
    return count;
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

void test_swap_roles_persists_for_disconnected_rings(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));

    TEST_ASSERT_EQUAL(HAL_OK, hub_control_swap_roles(MAC_A, MAC_B));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_B));
}

void test_swap_roles_updates_live_event_composer_cache_for_two_active_rings(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_mark_connected(1, ROLE_SCROLL);
    event_composer_feed(0, 0x01, 8, 4);
    event_composer_feed(1, 0x01, 2, -5);
    mock_ble_central_set_connected_ring(0, MAC_A);
    mock_ble_central_set_connected_ring(1, MAC_B);

    TEST_ASSERT_EQUAL(HAL_OK, hub_control_swap_roles(MAC_A, MAC_B));

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
}

void test_swap_roles_rejects_identical_mac(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, hub_control_swap_roles(MAC_A, MAC_A));
}

void test_forget_ring_removes_assignment_and_bond_for_disconnected_ring(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    mock_ble_central_seed_bond(MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK, hub_control_forget_ring(MAC_A));
    TEST_ASSERT_EQUAL(0, role_entry_count());
    TEST_ASSERT_FALSE(mock_ble_central_has_bond(MAC_A));
}

void test_forget_ring_drops_live_input_before_disconnect_completes(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    event_composer_mark_connected(0, ROLE_CURSOR);
    event_composer_feed(0, 0x01, 7, 4);
    mock_ble_central_set_connected_ring(0, MAC_A);
    mock_ble_central_seed_bond(MAC_A);

    TEST_ASSERT_EQUAL(HAL_OK, hub_control_forget_ring(MAC_A));

    composed_report_t report;
    event_composer_compose(&report);
    TEST_ASSERT_EQUAL(0, report.buttons);
    TEST_ASSERT_EQUAL(0, report.cursor_dx);
    TEST_ASSERT_EQUAL(0, report.cursor_dy);
    TEST_ASSERT_EQUAL(0, role_entry_count());
    TEST_ASSERT_FALSE(mock_ble_central_has_bond(MAC_A));
    TEST_ASSERT_EQUAL(HAL_ERR_NOT_FOUND, ble_central_find_ring_index_by_mac(MAC_A, &(uint8_t){0}));
}

void test_forget_ring_rejects_unknown_mac(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_ERR_NOT_FOUND, hub_control_forget_ring(MAC_A));
}

void run_hub_control_tests(void)
{
    printf("Hub control tests:\n");
    RUN_TEST(test_set_role_persists_for_disconnected_ring);
    RUN_TEST(test_set_role_updates_live_event_composer_cache);
    RUN_TEST(test_set_role_rejects_unknown_mac);
    RUN_TEST(test_swap_roles_persists_for_disconnected_rings);
    RUN_TEST(test_swap_roles_updates_live_event_composer_cache_for_two_active_rings);
    RUN_TEST(test_swap_roles_rejects_identical_mac);
    RUN_TEST(test_forget_ring_removes_assignment_and_bond_for_disconnected_ring);
    RUN_TEST(test_forget_ring_drops_live_input_before_disconnect_completes);
    RUN_TEST(test_forget_ring_rejects_unknown_mac);
}
