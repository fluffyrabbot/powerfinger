// SPDX-License-Identifier: MIT
// PowerFinger — Hub identity unit tests

#include <string.h>

#include "unity.h"
#include "hub_identity.h"

void test_hub_identity_strings_are_truthful(void)
{
    TEST_ASSERT_TRUE(strcmp("0.1.0", hub_identity_firmware_revision()) == 0);
    TEST_ASSERT_TRUE(strcmp("DEVBOARD-S3", hub_identity_hardware_revision()) == 0);
}

void test_hub_identity_binary_version_matches_string_contract(void)
{
    uint8_t version[3] = { 0xFF, 0xFF, 0xFF };

    hub_identity_firmware_version(version);

    TEST_ASSERT_EQUAL(0, version[0]);
    TEST_ASSERT_EQUAL(1, version[1]);
    TEST_ASSERT_EQUAL(0, version[2]);
}

void run_hub_identity_tests(void)
{
    printf("Hub identity tests:\n");
    RUN_TEST(test_hub_identity_strings_are_truthful);
    RUN_TEST(test_hub_identity_binary_version_matches_string_contract);
}
