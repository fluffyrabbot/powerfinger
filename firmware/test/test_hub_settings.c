// SPDX-License-Identifier: MIT
// PowerFinger — Hub settings unit tests

#include "unity.h"
#include "mock_hal.h"
#include "hub_settings.h"

static void reset(void)
{
    mock_hal_reset();
    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_init());
}

void test_hub_settings_defaults_when_storage_empty(void)
{
    reset();

    hub_settings_snapshot_t snapshot = {0};
    hub_settings_get(&snapshot);

    TEST_ASSERT_EQUAL(1, snapshot.usb_poll_ms);
    TEST_ASSERT_EQUAL(1, snapshot.scan_policy);
    TEST_ASSERT_EQUAL(2, snapshot.expected_rings);
}

void test_hub_settings_persist_after_flush_and_reload(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_set_usb_poll_ms(4));
    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_set_scan_policy(2));
    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_set_expected_rings(3));
    hub_settings_flush_if_dirty();

    TEST_ASSERT_EQUAL(HAL_OK, hub_settings_init());

    hub_settings_snapshot_t snapshot = {0};
    hub_settings_get(&snapshot);

    TEST_ASSERT_EQUAL(4, snapshot.usb_poll_ms);
    TEST_ASSERT_EQUAL(2, snapshot.scan_policy);
    TEST_ASSERT_EQUAL(3, snapshot.expected_rings);
}

void test_hub_settings_reject_invalid_values(void)
{
    reset();

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, hub_settings_set_usb_poll_ms(3));
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, hub_settings_set_scan_policy(9));
    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG, hub_settings_set_expected_rings(0));

    hub_settings_snapshot_t snapshot = {0};
    hub_settings_get(&snapshot);
    TEST_ASSERT_EQUAL(1, snapshot.usb_poll_ms);
    TEST_ASSERT_EQUAL(1, snapshot.scan_policy);
    TEST_ASSERT_EQUAL(2, snapshot.expected_rings);
}

void test_hub_settings_init_propagates_storage_failure(void)
{
    mock_hal_reset();
    mock_hal_inject_storage_init_failure(HAL_ERR_IO, 1);

    TEST_ASSERT_EQUAL(HAL_ERR_IO, hub_settings_init());
}

void run_hub_settings_tests(void)
{
    printf("Hub settings tests:\n");
    RUN_TEST(test_hub_settings_defaults_when_storage_empty);
    RUN_TEST(test_hub_settings_persist_after_flush_and_reload);
    RUN_TEST(test_hub_settings_reject_invalid_values);
    RUN_TEST(test_hub_settings_init_propagates_storage_failure);
}
