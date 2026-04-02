// SPDX-License-Identifier: MIT
// PowerFinger — Event composer unit tests
//
// These tests cover the accessibility-critical stuck-button invariant:
// after disconnect, all buttons contributed by that ring must be released.

#include "unity.h"
#include "event_composer.h"

static void reset(void)
{
    event_composer_init();
}

static void connect_ring(uint8_t ring_index, ring_role_t role)
{
    event_composer_mark_connected(ring_index, role);
}

static void feed_ring(uint8_t ring_index, uint8_t buttons, int8_t dx, int8_t dy)
{
    event_composer_feed(ring_index, buttons, dx, dy);
}

// --- Basic composition ---

void test_empty_compose_is_zero(void)
{
    reset();
    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.cursor_dy);
    TEST_ASSERT_EQUAL(0, r.scroll_v);
    TEST_ASSERT_EQUAL(0, r.scroll_h);
}

void test_single_cursor_ring(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 10, -5);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x01, r.buttons);   // left click
    TEST_ASSERT_EQUAL(10, r.cursor_dx);
    TEST_ASSERT_EQUAL(-5, r.cursor_dy);
    TEST_ASSERT_EQUAL(0, r.scroll_v);
}

void test_single_scroll_ring(void)
{
    reset();
    connect_ring(0, ROLE_SCROLL);
    feed_ring(0, 0x01, 3, -7);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x02, r.buttons);   // right click (scroll ring click -> right)
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(-7, r.scroll_v);
    TEST_ASSERT_EQUAL(3, r.scroll_h);
}

void test_two_ring_composition(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    connect_ring(1, ROLE_SCROLL);
    feed_ring(0, 0x01, 5, 3);
    feed_ring(1, 0x01, 0, 10);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x03, r.buttons);   // left + right
    TEST_ASSERT_EQUAL(5, r.cursor_dx);
    TEST_ASSERT_EQUAL(3, r.cursor_dy);
    TEST_ASSERT_EQUAL(10, r.scroll_v);
}

// --- Accumulator clearing ---

void test_compose_clears_accumulators(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x00, 10, 10);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(10, r.cursor_dx);

    // Second compose without feed — deltas must be zero
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.cursor_dy);
}

// --- CRITICAL: Disconnect releases buttons (accessibility invariant) ---

void test_disconnect_releases_buttons(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 5, 3);

    // Disconnect while button held
    event_composer_ring_disconnected(0);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.cursor_dy);
}

void test_disconnect_zeroes_deltas(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x00, 50, 50);

    event_composer_ring_disconnected(0);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.cursor_dy);
}

void test_disconnect_one_ring_keeps_other(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    connect_ring(1, ROLE_SCROLL);
    feed_ring(0, 0x01, 5, 5);
    feed_ring(1, 0x01, 0, 10);

    // Disconnect ring 0 only
    event_composer_ring_disconnected(0);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x02, r.buttons);   // only right (from ring 1 scroll)
    TEST_ASSERT_EQUAL(0, r.cursor_dx);    // ring 0 contribution gone
    TEST_ASSERT_EQUAL(10, r.scroll_v);    // ring 1 still there
}

// --- CRITICAL: Stale BLE notification after disconnect is rejected ---

void test_feed_after_disconnect_rejected(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 5, 3);

    event_composer_ring_disconnected(0);

    // Stale notification arrives after disconnect
    feed_ring(0, 0x01, 2, 1);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);      // must NOT resurrect the ring
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
}

// --- Feed without mark_connected is rejected ---

void test_feed_without_connect_rejected(void)
{
    reset();
    // Never called mark_connected — feed should be silently dropped
    feed_ring(0, 0x01, 10, 10);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
}

// --- Delta accumulation ---

void test_deltas_accumulate_across_feeds(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x00, 10, 5);
    feed_ring(0, 0x00, 10, 5);
    feed_ring(0, 0x00, 10, 5);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(30, r.cursor_dx);
    TEST_ASSERT_EQUAL(15, r.cursor_dy);
}

// --- Saturation (no overflow) ---

void test_accumulator_saturates_positive(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);

    // Feed 300 times with max positive delta — should saturate, not wrap
    for (int i = 0; i < 300; i++) {
        feed_ring(0, 0x00, 127, 0);
    }

    composed_report_t r;
    event_composer_compose(&r);
    // 300 * 127 = 38100, which would overflow int16_t (max 32767).
    // Saturating addition should cap at INT16_MAX.
    TEST_ASSERT_TRUE(r.cursor_dx > 0);  // must not wrap negative
}

void test_accumulator_saturates_negative(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);

    for (int i = 0; i < 300; i++) {
        feed_ring(0, 0x00, -128, 0);
    }

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_TRUE(r.cursor_dx < 0);  // must not wrap positive
}

// --- Modifier ring ---

void test_modifier_ring_middle_click(void)
{
    reset();
    connect_ring(0, ROLE_MODIFIER);
    feed_ring(0, 0x01, 10, 10);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x04, r.buttons);   // middle click
    TEST_ASSERT_EQUAL(0, r.cursor_dx);    // modifier ignores deltas
}

// --- Bounds check ---

void test_invalid_ring_index_ignored(void)
{
    reset();
    event_composer_mark_connected(99, ROLE_CURSOR);  // out of bounds — should be no-op
    event_composer_feed(99, 0x01, 10, 10);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
}

// --- Reconnect lifecycle ---

void test_reconnect_after_disconnect_works(void)
{
    reset();
    // First connection
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 10, 10);

    // Disconnect
    event_composer_ring_disconnected(0);

    // Reconnect
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x00, 5, 5);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);      // new connection has no click
    TEST_ASSERT_EQUAL(5, r.cursor_dx);    // new deltas work
}

void test_buttons_reflect_latest_feed(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 0, 0);  // press
    feed_ring(0, 0x00, 0, 0);  // release

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);  // last feed wins for buttons
}

void test_role_change_clears_old_mapping_before_new_role_applies(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    feed_ring(0, 0x01, 5, 3);

    event_composer_set_role(0, ROLE_SCROLL);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.scroll_v);

    feed_ring(0, 0x01, 0, 7);
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x02, r.buttons);  // scroll-role click -> right click
    TEST_ASSERT_EQUAL(7, r.scroll_v);
}

void test_mark_connected_seeds_cached_role(void)
{
    reset();
    connect_ring(0, ROLE_SCROLL);
    feed_ring(0, 0x01, 4, -6);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x02, r.buttons);
    TEST_ASSERT_EQUAL(4, r.scroll_h);
    TEST_ASSERT_EQUAL(-6, r.scroll_v);
}

void test_role_swap_clears_old_mapping_before_new_roles_apply(void)
{
    reset();
    connect_ring(0, ROLE_CURSOR);
    connect_ring(1, ROLE_SCROLL);
    feed_ring(0, 0x01, 5, 3);
    feed_ring(1, 0x01, 2, -4);

    event_composer_swap_roles(0, ROLE_SCROLL, 1, ROLE_CURSOR);

    composed_report_t r;
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0, r.buttons);
    TEST_ASSERT_EQUAL(0, r.cursor_dx);
    TEST_ASSERT_EQUAL(0, r.cursor_dy);
    TEST_ASSERT_EQUAL(0, r.scroll_h);
    TEST_ASSERT_EQUAL(0, r.scroll_v);

    feed_ring(0, 0x01, 6, -2);
    feed_ring(1, 0x01, -3, 9);
    event_composer_compose(&r);
    TEST_ASSERT_EQUAL(0x03, r.buttons);
    TEST_ASSERT_EQUAL(-3, r.cursor_dx);
    TEST_ASSERT_EQUAL(9, r.cursor_dy);
    TEST_ASSERT_EQUAL(6, r.scroll_h);
    TEST_ASSERT_EQUAL(-2, r.scroll_v);
}

// --- Test runner ---

void run_event_composer_tests(void)
{
    printf("Event composer tests:\n");
    RUN_TEST(test_empty_compose_is_zero);
    RUN_TEST(test_single_cursor_ring);
    RUN_TEST(test_single_scroll_ring);
    RUN_TEST(test_two_ring_composition);
    RUN_TEST(test_compose_clears_accumulators);
    RUN_TEST(test_disconnect_releases_buttons);
    RUN_TEST(test_disconnect_zeroes_deltas);
    RUN_TEST(test_disconnect_one_ring_keeps_other);
    RUN_TEST(test_feed_after_disconnect_rejected);
    RUN_TEST(test_feed_without_connect_rejected);
    RUN_TEST(test_deltas_accumulate_across_feeds);
    RUN_TEST(test_accumulator_saturates_positive);
    RUN_TEST(test_accumulator_saturates_negative);
    RUN_TEST(test_modifier_ring_middle_click);
    RUN_TEST(test_invalid_ring_index_ignored);
    RUN_TEST(test_reconnect_after_disconnect_works);
    RUN_TEST(test_buttons_reflect_latest_feed);
    RUN_TEST(test_role_change_clears_old_mapping_before_new_role_applies);
    RUN_TEST(test_role_swap_clears_old_mapping_before_new_roles_apply);
    RUN_TEST(test_mark_connected_seeds_cached_role);
}
