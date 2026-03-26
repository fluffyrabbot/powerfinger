// SPDX-License-Identifier: MIT
// PowerFinger — Dead zone unit tests

#include "unity.h"
#include "dead_zone.h"
#include "ring_config.h"
#include "mock_hal.h"

// --- Helper ---
static void reset(void)
{
    mock_hal_reset();
    dead_zone_init();
}

// --- Tests ---

void test_no_suppression_without_click(void)
{
    reset();
    int16_t dx = 10, dy = 5;
    bool active = dead_zone_update(false, &dx, &dy, 0);
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_EQUAL(10, dx);
    TEST_ASSERT_EQUAL(5, dy);
}

void test_suppression_on_click_press(void)
{
    reset();
    int16_t dx = 10, dy = 5;
    bool active = dead_zone_update(true, &dx, &dy, 0);
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_EQUAL(0, dx);
    TEST_ASSERT_EQUAL(0, dy);
}

void test_suppression_continues_during_time_window(void)
{
    reset();
    int16_t dx, dy;

    // Click press
    dx = 3; dy = 2;
    dead_zone_update(true, &dx, &dy, 0);

    // Still pressed, only 10ms later, small movement
    dx = 3; dy = 2;
    bool active = dead_zone_update(true, &dx, &dy, 10);
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_EQUAL(0, dx);
    TEST_ASSERT_EQUAL(0, dy);
}

void test_suppression_continues_until_both_conditions_met(void)
{
    reset();
    int16_t dx, dy;

    // Click press at t=0
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);

    // Time met (60ms > 50ms) but distance NOT met (only 5 < 10)
    dx = 3; dy = 2;
    bool active = dead_zone_update(true, &dx, &dy, 60);
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_EQUAL(0, dx);

    // Now distance also met (accumulated: 5 + 8 = 13 > 10)
    dx = 5; dy = 3;
    active = dead_zone_update(true, &dx, &dy, 70);
    TEST_ASSERT_FALSE(active);  // both conditions met → exit
    TEST_ASSERT_EQUAL(5, dx);   // deltas flow through (drag)
    TEST_ASSERT_EQUAL(3, dy);
}

void test_distance_alone_not_enough(void)
{
    reset();
    int16_t dx, dy;

    // Click press at t=0
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);

    // Big movement at t=10ms (distance met, but only 10ms < 50ms)
    dx = 20; dy = 20;
    bool active = dead_zone_update(true, &dx, &dy, 10);
    TEST_ASSERT_TRUE(active);  // time not met yet
    TEST_ASSERT_EQUAL(0, dx);
}

void test_click_release_during_dead_zone(void)
{
    reset();
    int16_t dx, dy;

    // Click press
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);

    // Release during dead zone
    dx = 3; dy = 2;
    bool active = dead_zone_update(false, &dx, &dy, 20);
    TEST_ASSERT_FALSE(active);

    // Next tick should be normal
    dx = 5; dy = 5;
    active = dead_zone_update(false, &dx, &dy, 30);
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_EQUAL(5, dx);
}

void test_drag_mode_passes_deltas(void)
{
    reset();
    int16_t dx, dy;

    // Click press at t=0
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);

    // Accumulate enough distance and time to exit dead zone
    dx = 6; dy = 5;
    dead_zone_update(true, &dx, &dy, 60);

    // Should be in drag mode now — deltas flow through
    dx = 10; dy = 10;
    bool active = dead_zone_update(true, &dx, &dy, 70);
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_EQUAL(10, dx);
    TEST_ASSERT_EQUAL(10, dy);
}

void test_drag_ends_on_release(void)
{
    reset();
    int16_t dx, dy;

    // Enter drag mode
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);
    dx = 6; dy = 5;
    dead_zone_update(true, &dx, &dy, 60);

    // Release from drag
    dx = 5; dy = 5;
    dead_zone_update(false, &dx, &dy, 80);

    // New click should start fresh dead zone
    dx = 3; dy = 3;
    bool active = dead_zone_update(true, &dx, &dy, 100);
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_EQUAL(0, dx);
}

void test_reset_clears_state(void)
{
    reset();
    int16_t dx, dy;

    // Start a dead zone
    dx = 0; dy = 0;
    dead_zone_update(true, &dx, &dy, 0);

    // Reset
    dead_zone_reset();

    // Should not be in dead zone anymore
    dx = 5; dy = 5;
    bool active = dead_zone_update(false, &dx, &dy, 10);
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_EQUAL(5, dx);
}

// --- Test runner ---

void run_dead_zone_tests(void)
{
    printf("Dead zone tests:\n");
    RUN_TEST(test_no_suppression_without_click);
    RUN_TEST(test_suppression_on_click_press);
    RUN_TEST(test_suppression_continues_during_time_window);
    RUN_TEST(test_suppression_continues_until_both_conditions_met);
    RUN_TEST(test_distance_alone_not_enough);
    RUN_TEST(test_click_release_during_dead_zone);
    RUN_TEST(test_drag_mode_passes_deltas);
    RUN_TEST(test_drag_ends_on_release);
    RUN_TEST(test_reset_clears_state);
}
