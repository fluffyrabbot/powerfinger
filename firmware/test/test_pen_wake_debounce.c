// SPDX-License-Identifier: MIT
// PowerFinger — PowerPen wake debounce unit tests

#include "unity.h"
#include "mock_hal.h"
#include "power_manager.h"
#include "pen_wake_debounce.h"

static void cold_boot(void)
{
    mock_hal_reset();
    mock_hal_set_wake_cause(HAL_WAKE_CAUSE_COLD_BOOT);
    mock_hal_set_time_ms(0);
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(HAL_OK, pen_wake_debounce_init());
}

static void drv5032_gpio_wake(uint32_t now_ms)
{
    mock_hal_reset();
    mock_hal_set_wake_cause(HAL_WAKE_CAUSE_GPIO);
    mock_hal_set_time_ms(now_ms);
    mock_hal_set_gpio_input(4, true);
    mock_hal_set_gpio_input(5, true);
    mock_hal_set_gpio_input(8, false);
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(HAL_OK, pen_wake_debounce_init());
}

static void tip_gpio_wake(uint32_t now_ms)
{
    mock_hal_reset();
    mock_hal_set_wake_cause(HAL_WAKE_CAUSE_GPIO);
    mock_hal_set_time_ms(now_ms);
    mock_hal_set_gpio_input(4, true);
    mock_hal_set_gpio_input(5, false);
    mock_hal_set_gpio_input(8, true);
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(HAL_OK, pen_wake_debounce_init());
}

void test_false_drv5032_wakes_disable_gpio8_after_limit(void)
{
    cold_boot();

    drv5032_gpio_wake(1000);
    TEST_ASSERT_TRUE(pen_wake_debounce_validation_pending());
    pen_wake_debounce_note_validation_failure();
    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(1, pen_wake_debounce_spurious_wake_count());

    drv5032_gpio_wake(2000);
    pen_wake_debounce_note_validation_failure();
    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(2, pen_wake_debounce_spurious_wake_count());

    drv5032_gpio_wake(3000);
    pen_wake_debounce_note_validation_failure();
    TEST_ASSERT_FALSE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(3, pen_wake_debounce_spurious_wake_count());

    power_manager_enter_sleep(true);

    uint64_t mask = 0;
    bool level = true;
    mock_hal_get_last_wake_gpio_mask(&mask, &level);
    TEST_ASSERT_TRUE(mask == 0x30ULL);
    TEST_ASSERT_FALSE(level);
}

void test_barrel_press_reenables_drv5032_wake(void)
{
    cold_boot();

    for (uint32_t now_ms = 1000; now_ms <= 3000; now_ms += 1000) {
        drv5032_gpio_wake(now_ms);
        pen_wake_debounce_note_validation_failure();
    }

    TEST_ASSERT_FALSE(pen_wake_debounce_drv5032_enabled());

    mock_hal_reset();
    mock_hal_set_wake_cause(HAL_WAKE_CAUSE_GPIO);
    mock_hal_set_time_ms(5000);
    mock_hal_set_gpio_input(4, false);
    mock_hal_set_gpio_input(5, true);
    mock_hal_set_gpio_input(8, true);
    TEST_ASSERT_EQUAL(HAL_OK, power_manager_init());
    TEST_ASSERT_EQUAL(HAL_OK, pen_wake_debounce_init());

    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(0, pen_wake_debounce_spurious_wake_count());

    power_manager_enter_sleep(true);

    uint64_t mask = 0;
    mock_hal_get_last_wake_gpio_mask(&mask, NULL);
    TEST_ASSERT_TRUE(mask == 0x130ULL);
}

void test_spurious_wake_window_expiry_resets_counter_before_disable(void)
{
    cold_boot();

    drv5032_gpio_wake(1000);
    pen_wake_debounce_note_validation_failure();
    TEST_ASSERT_EQUAL(1, pen_wake_debounce_spurious_wake_count());

    drv5032_gpio_wake(32050);
    pen_wake_debounce_note_validation_failure();

    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(1, pen_wake_debounce_spurious_wake_count());
}

void test_motion_within_validation_window_clears_pending_without_counting_spurious(void)
{
    cold_boot();

    drv5032_gpio_wake(1000);
    TEST_ASSERT_TRUE(pen_wake_debounce_validation_pending());

    pen_wake_debounce_note_motion();

    TEST_ASSERT_FALSE(pen_wake_debounce_validation_pending());
    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(0, pen_wake_debounce_spurious_wake_count());
}

void test_tip_dome_wake_does_not_arm_drv5032_validation(void)
{
    cold_boot();

    tip_gpio_wake(1000);

    TEST_ASSERT_FALSE(pen_wake_debounce_validation_pending());
    TEST_ASSERT_TRUE(pen_wake_debounce_drv5032_enabled());
    TEST_ASSERT_EQUAL(0, pen_wake_debounce_spurious_wake_count());
}

void run_pen_wake_debounce_tests(void)
{
    printf("Pen wake debounce tests:\n");
    RUN_TEST(test_false_drv5032_wakes_disable_gpio8_after_limit);
    RUN_TEST(test_barrel_press_reenables_drv5032_wake);
    RUN_TEST(test_spurious_wake_window_expiry_resets_counter_before_disable);
    RUN_TEST(test_motion_within_validation_window_clears_pending_without_counting_spurious);
    RUN_TEST(test_tip_dome_wake_does_not_arm_drv5032_validation);
}

int main(void)
{
    UNITY_BEGIN();
    run_pen_wake_debounce_tests();
    return UNITY_END();
}
