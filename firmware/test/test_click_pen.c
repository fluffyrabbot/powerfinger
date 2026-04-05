// SPDX-License-Identifier: MIT
// PowerFinger — PowerPen click driver unit tests

#include "unity.h"
#include "click_interface.h"
#include "mock_hal.h"
#include "ring_config.h"

#define PIN_BARREL 4
#define PIN_TIP_DOME 5

static void reset(void)
{
    mock_hal_reset();
    mock_hal_set_time_ms(0);
    TEST_ASSERT_EQUAL(HAL_OK, click_init());
}

static void set_pressed(hal_pin_t pin, bool pressed)
{
    mock_hal_set_gpio_input(pin, !pressed);
}

void test_pen_click_defaults_to_unpressed(void)
{
    reset();
    TEST_ASSERT_FALSE(click_is_pressed(CLICK_SOURCE_PRIMARY));
    TEST_ASSERT_FALSE(click_is_pressed(CLICK_SOURCE_SECONDARY));
}

void test_pen_click_primary_and_secondary_are_independent(void)
{
    reset();

    set_pressed(PIN_BARREL, true);
    TEST_ASSERT_FALSE(click_is_pressed(CLICK_SOURCE_PRIMARY));
    mock_hal_advance_time_ms(CLICK_DEBOUNCE_MS);
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_PRIMARY));
    TEST_ASSERT_FALSE(click_is_pressed(CLICK_SOURCE_SECONDARY));

    set_pressed(PIN_TIP_DOME, true);
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_PRIMARY));
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_SECONDARY));
}

void test_pen_click_release_is_debounced_per_source(void)
{
    reset();

    set_pressed(PIN_TIP_DOME, true);
    mock_hal_advance_time_ms(CLICK_DEBOUNCE_MS);
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_SECONDARY));

    set_pressed(PIN_TIP_DOME, false);
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_SECONDARY));
    mock_hal_advance_time_ms(CLICK_DEBOUNCE_MS);
    TEST_ASSERT_FALSE(click_is_pressed(CLICK_SOURCE_SECONDARY));
}

void test_pen_click_simultaneous_press_preserves_both_sources(void)
{
    reset();

    set_pressed(PIN_BARREL, true);
    set_pressed(PIN_TIP_DOME, true);
    mock_hal_advance_time_ms(CLICK_DEBOUNCE_MS);

    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_PRIMARY));
    TEST_ASSERT_TRUE(click_is_pressed(CLICK_SOURCE_SECONDARY));
}

int main(void)
{
    UNITY_BEGIN();

    printf("PowerPen click tests:\n");
    RUN_TEST(test_pen_click_defaults_to_unpressed);
    RUN_TEST(test_pen_click_primary_and_secondary_are_independent);
    RUN_TEST(test_pen_click_release_is_debounced_per_source);
    RUN_TEST(test_pen_click_simultaneous_press_preserves_both_sources);

    printf("\n--- Results: %d tests, %d failures ---\n",
           unity_tests, unity_failures);
    return UNITY_END();
}
