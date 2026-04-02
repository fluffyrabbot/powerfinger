// SPDX-License-Identifier: MIT
// PowerFinger — Ring identity unit tests

#include <string.h>

#include "unity.h"
#include "ring_identity.h"

void test_default_model_and_revision_strings_are_truthful(void)
{
    TEST_ASSERT_TRUE(strcmp("Ring-S", ring_identity_model_number()) == 0);
    TEST_ASSERT_TRUE(strcmp("0.1.0", ring_identity_firmware_revision()) == 0);
    TEST_ASSERT_TRUE(strcmp("DEVBOARD-C3", ring_identity_hardware_revision()) == 0);
}

void test_binary_firmware_version_matches_string_contract(void)
{
    uint8_t version[3] = {0xFF, 0xFF, 0xFF};

    ring_identity_firmware_version(version);

    TEST_ASSERT_EQUAL(0, version[0]);
    TEST_ASSERT_EQUAL(1, version[1]);
    TEST_ASSERT_EQUAL(0, version[2]);
}

void test_serial_formats_as_uppercase_hex(void)
{
    const uint8_t mac[6] = { 0xAA, 0x01, 0xB2, 0x03, 0xC4, 0x05 };
    char serial[13] = {0};

    TEST_ASSERT_EQUAL(HAL_OK,
                      ring_identity_format_serial(mac, serial, sizeof(serial)));
    TEST_ASSERT_TRUE(strcmp("AA01B203C405", serial) == 0);
}

void test_serial_rejects_too_small_buffer(void)
{
    const uint8_t mac[6] = { 0, 1, 2, 3, 4, 5 };
    char serial[12] = {0};

    TEST_ASSERT_EQUAL(HAL_ERR_INVALID_ARG,
                      ring_identity_format_serial(mac, serial, sizeof(serial)));
}

void run_ring_identity_tests(void)
{
    printf("Ring identity tests:\n");
    RUN_TEST(test_default_model_and_revision_strings_are_truthful);
    RUN_TEST(test_binary_firmware_version_matches_string_contract);
    RUN_TEST(test_serial_formats_as_uppercase_hex);
    RUN_TEST(test_serial_rejects_too_small_buffer);
}
