// SPDX-License-Identifier: MIT
// PowerFinger — Role engine unit tests

#include "unity.h"
#include "role_engine.h"
#include "ble_central.h"
#include "hal_storage.h"
#include "mock_hal.h"

#include <string.h>

#define ROLE_NVS_TEST_KEY "roles"
#define ROLE_NVS_VERSION  1

typedef struct {
    uint8_t mac[6];
    ring_role_t role;
} role_entry_t;

typedef struct {
    uint8_t version;
    uint8_t count;
    role_entry_t entries[HUB_MAX_RINGS];
} role_blob_t;

static const uint8_t MAC_A[6] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };
static const uint8_t MAC_B[6] = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25 };

static void reset(void)
{
    mock_hal_reset();
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
}

void test_roles_persist_after_flush_and_reload(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_set_role(MAC_A, ROLE_MODIFIER));

    role_engine_flush_if_dirty();

    // Simulate a restart by reloading from committed storage.
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_B));
}

void test_flush_failure_retries_dirty_snapshot(void)
{
    reset();

    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_A));
    TEST_ASSERT_EQUAL(HAL_OK, role_engine_set_role(MAC_A, ROLE_SCROLL));

    mock_hal_inject_storage_commit_failure(HAL_ERR_IO, 1);
    role_engine_flush_if_dirty();

    uint8_t blob[64];
    size_t len = sizeof(blob);
    TEST_ASSERT_EQUAL(HAL_ERR_NOT_FOUND,
                      hal_storage_get(ROLE_NVS_TEST_KEY, blob, &len));

    role_engine_flush_if_dirty();

    len = sizeof(blob);
    TEST_ASSERT_EQUAL(HAL_OK, hal_storage_get(ROLE_NVS_TEST_KEY, blob, &len));

    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(ROLE_SCROLL, role_engine_get_role(MAC_A));
}

void test_duplicate_mac_load_keeps_last_entry(void)
{
    mock_hal_reset();

    role_blob_t blob = {
        .version = ROLE_NVS_VERSION,
        .count = 2,
        .entries = {
            { .mac = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 }, .role = ROLE_CURSOR },
            { .mac = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 }, .role = ROLE_MODIFIER },
        },
    };
    mock_hal_storage_seed(ROLE_NVS_TEST_KEY, &blob, sizeof(uint8_t) * 2 + (sizeof(role_entry_t) * 2));

    TEST_ASSERT_EQUAL(HAL_OK, role_engine_init());
    TEST_ASSERT_EQUAL(ROLE_MODIFIER, role_engine_get_role(MAC_A));

    // Dedup must collapse the duplicate so the next new ring sees CURSOR as
    // the first unoccupied default role, not SCROLL.
    TEST_ASSERT_EQUAL(ROLE_CURSOR, role_engine_get_role(MAC_B));
}

void run_role_engine_tests(void)
{
    printf("Role engine tests:\n");
    RUN_TEST(test_roles_persist_after_flush_and_reload);
    RUN_TEST(test_flush_failure_retries_dirty_snapshot);
    RUN_TEST(test_duplicate_mac_load_keeps_last_entry);
}
