// SPDX-License-Identifier: MIT
// PowerFinger — Test runner entry point

#include <stdio.h>
#include "unity.h"

// Test suite entry points
extern void run_state_machine_tests(void);
extern void run_dead_zone_tests(void);
extern void run_ring_diagnostics_tests(void);
extern void run_calibration_tests(void);
extern void run_power_manager_tests(void);
extern void run_ring_runtime_health_tests(void);
extern void run_ring_settings_tests(void);
extern void run_ring_identity_tests(void);
extern void run_hub_identity_tests(void);
extern void run_role_engine_tests(void);
extern void run_companion_protocol_tests(void);
extern void run_event_composer_tests(void);
extern void run_usb_hid_tests(void);

int main(void)
{
    UNITY_BEGIN();

    run_state_machine_tests();
    run_dead_zone_tests();
    run_ring_diagnostics_tests();
    run_calibration_tests();
    run_power_manager_tests();
    run_ring_runtime_health_tests();
    run_ring_settings_tests();
    run_ring_identity_tests();
    run_hub_identity_tests();
    run_role_engine_tests();
    run_companion_protocol_tests();
    run_event_composer_tests();
    run_usb_hid_tests();

    printf("\n--- Results: %d tests, %d failures ---\n",
           unity_tests, unity_failures);

    return UNITY_END();
}
