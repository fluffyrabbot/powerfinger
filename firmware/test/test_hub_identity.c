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

void test_hub_identity_formats_serial_as_uppercase_hex(void)
{
    const uint8_t mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34 };
    char serial[13] = {0};

    TEST_ASSERT_EQUAL(HAL_OK,
                      hub_identity_format_serial(mac, serial, sizeof(serial)));
    TEST_ASSERT_TRUE(strcmp("DEADBEEF1234", serial) == 0);
}

void test_hub_identity_serial_rejects_too_small_buffer(void)
{
    const uint8_t mac[6] = { 0, 1, 2, 3, 4, 5 };
    char serial[12] = {0};

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG,
                      hub_identity_format_serial(mac, serial, sizeof(serial)));
}

void run_hub_identity_tests(void)
{
    printf("Hub identity tests:\n");
    RUN_TEST(test_hub_identity_strings_are_truthful);
    RUN_TEST(test_hub_identity_binary_version_matches_string_contract);
    RUN_TEST(test_hub_identity_formats_serial_as_uppercase_hex);
    RUN_TEST(test_hub_identity_serial_rejects_too_small_buffer);
}
