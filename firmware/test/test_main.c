// SPDX-License-Identifier: MIT
// PowerFinger — Test runner entry point

#include <stdio.h>
#include "unity.h"

// Test suite entry points
extern void run_state_machine_tests(void);
extern void run_dead_zone_tests(void);

int main(void)
{
    UNITY_BEGIN();

    run_state_machine_tests();
    run_dead_zone_tests();

    printf("\n--- Results: %d tests, %d failures ---\n",
           unity_tests, unity_failures);

    return UNITY_END();
}
