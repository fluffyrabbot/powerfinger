// Unity Test Framework — minimal subset for PowerFinger tests
// Full Unity: https://github.com/ThrowTheSwitch/Unity (MIT license)
//
// This is a minimal single-header implementation sufficient for our tests.
// Replace with full Unity if more features are needed.

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int unity_failures;
extern int unity_tests;

#define TEST_ASSERT(condition) do { \
    unity_tests++; \
    if (!(condition)) { \
        printf("  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        unity_failures++; \
    } \
} while(0)

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))

#define TEST_ASSERT_EQUAL(expected, actual) do { \
    unity_tests++; \
    if ((expected) != (actual)) { \
        printf("  FAIL: %s:%d: expected %d, got %d\n", __FILE__, __LINE__, (int)(expected), (int)(actual)); \
        unity_failures++; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_EQUAL(expected, actual)

#define TEST_ASSERT_NOT_EQUAL(expected, actual) do { \
    unity_tests++; \
    if ((expected) == (actual)) { \
        printf("  FAIL: %s:%d: values should differ, both are %d\n", __FILE__, __LINE__, (int)(expected)); \
        unity_failures++; \
    } \
} while(0)

#define RUN_TEST(func) do { \
    printf("  %s...", #func); \
    func(); \
    printf(" ok\n"); \
} while(0)

#define UNITY_BEGIN() do { \
    unity_failures = 0; \
    unity_tests = 0; \
} while(0)

#define UNITY_END() (unity_failures)
