// SPDX-License-Identifier: MIT
// PowerFinger — Gesture engine unit tests

#include "unity.h"
#include "gesture_engine.h"
#include "hal_storage.h"
#include "mock_hal.h"

#include <string.h>

#define GESTURE_NVS_TEST_KEY "gestures"
#define GESTURE_NVS_VERSION  1

static void reset(void)
{
    mock_hal_reset();
    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());
}

void test_gestures_persist_after_flush_and_reload(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK,
                                                GESTURE_ACTION_MIDDLE_CLICK));
    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_SCROLL_MODIFIER_CLICK,
                                                GESTURE_ACTION_BACK));

    gesture_engine_flush_if_dirty();

    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());
    TEST_ASSERT_EQUAL(GESTURE_ACTION_MIDDLE_CLICK,
                      gesture_engine_get_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK));
    TEST_ASSERT_EQUAL(GESTURE_ACTION_BACK,
                      gesture_engine_get_action(GESTURE_TRIGGER_SCROLL_MODIFIER_CLICK));
    TEST_ASSERT_EQUAL(GESTURE_ACTION_NONE,
                      gesture_engine_get_action(GESTURE_TRIGGER_CURSOR_MODIFIER_CLICK));
}

void test_gesture_flush_writes_compact_explicit_blob_format(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK,
                                                GESTURE_ACTION_MIDDLE_CLICK));
    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_ALL_THREE_CLICK,
                                                GESTURE_ACTION_FORWARD));

    gesture_engine_flush_if_dirty();

    uint8_t blob[32] = {0};
    size_t len = sizeof(blob);
    uint8_t expected[] = {
        GESTURE_NVS_VERSION,
        2,
        GESTURE_TRIGGER_CURSOR_SCROLL_CLICK, GESTURE_ACTION_MIDDLE_CLICK,
        GESTURE_TRIGGER_ALL_THREE_CLICK, GESTURE_ACTION_FORWARD,
    };

    TEST_ASSERT_EQUAL(HAL_OK, hal_storage_get(GESTURE_NVS_TEST_KEY, blob, &len));
    TEST_ASSERT_EQUAL(sizeof(expected), len);
    TEST_ASSERT_TRUE(memcmp(expected, blob, sizeof(expected)) == 0);
}

void test_set_action_none_removes_entry(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK,
                                                GESTURE_ACTION_MIDDLE_CLICK));
    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK,
                                                GESTURE_ACTION_NONE));

    gesture_engine_entry_t entries[GESTURE_ENGINE_MAX_ENTRIES] = {0};
    size_t count = 0;
    TEST_ASSERT_EQUAL(HAL_OK,
                      gesture_engine_get_all(entries, GESTURE_ENGINE_MAX_ENTRIES, &count));
    TEST_ASSERT_EQUAL(0, count);
    TEST_ASSERT_EQUAL(GESTURE_ACTION_NONE,
                      gesture_engine_get_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK));
}

void test_set_action_rejects_unsupported_trigger_and_action(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_ERR_NOT_SUPPORTED,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_DOUBLE_CLICK,
                                                GESTURE_ACTION_MIDDLE_CLICK));
    TEST_ASSERT_EQUAL(HAL_ERR_NOT_SUPPORTED,
                      gesture_engine_set_action(GESTURE_TRIGGER_CURSOR_SCROLL_CLICK,
                                                GESTURE_ACTION_DPI_CYCLE));
}

void test_gesture_init_propagates_storage_init_failure(void)
{
    mock_hal_reset();
    mock_hal_inject_storage_init_failure(HAL_ERR_IO, 1);

    TEST_ASSERT_EQUAL(HAL_ERR_IO, gesture_engine_init());
    TEST_ASSERT_EQUAL(HAL_OK, gesture_engine_init());
}

void run_gesture_engine_tests(void)
{
    printf("Gesture engine tests:\n");
    RUN_TEST(test_gestures_persist_after_flush_and_reload);
    RUN_TEST(test_gesture_flush_writes_compact_explicit_blob_format);
    RUN_TEST(test_set_action_none_removes_entry);
    RUN_TEST(test_set_action_rejects_unsupported_trigger_and_action);
    RUN_TEST(test_gesture_init_propagates_storage_init_failure);
}
