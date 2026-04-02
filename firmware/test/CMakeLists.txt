# SPDX-License-Identifier: MIT
# PowerFinger — Host-side unit tests
#
# Builds on host machine (no ESP32 needed). Tests pure-logic components
# with a mock HAL.
#
# Usage:
#   cmake -B build firmware/test
#   cmake --build build
#   ./build/powerfinger_tests

cmake_minimum_required(VERSION 3.16)
project(powerfinger_tests C)

set(CMAKE_C_STANDARD 11)
enable_testing()

# Unity test framework (single-file, vendored)
add_library(unity STATIC unity/unity.c)
target_include_directories(unity PUBLIC unity)

# Mock HAL
add_library(mock_hal STATIC mocks/mock_hal.c)
target_include_directories(mock_hal PUBLIC
    mocks
    ../ring/components/hal/include
)

# Components under test (compiled for host, not ESP32)
add_library(state_machine STATIC
    ../ring/components/state_machine/ring_state.c
)
target_include_directories(state_machine PUBLIC
    ../ring/components/state_machine/include
    ../ring/components/hal/include
)

add_library(dead_zone STATIC
    ../ring/components/click/dead_zone.c
)
target_include_directories(dead_zone PUBLIC
    ../ring/components/click/include
    ../ring/components/state_machine/include
    ../ring/components/hal/include
)

add_library(power_manager STATIC
    ../ring/components/power/power_manager.c
    ../ring/components/ble_hid/ble_gap_handler.c
)
target_include_directories(power_manager PUBLIC
    ../ring/components/power/include
    ../ring/components/ble_hid/include
    ../ring/components/state_machine/include
    ../ring/components/hal/include
)

add_library(ring_runtime_health STATIC
    ../ring/components/runtime_health/ring_runtime_health.c
)
target_include_directories(ring_runtime_health PUBLIC
    ../ring/components/runtime_health/include
    ../ring/components/state_machine/include
    ../ring/components/hal/include
)

add_library(role_engine STATIC
    ../hub/components/role_engine/role_engine.c
)
target_include_directories(role_engine PUBLIC
    ../hub/components/role_engine/include
    ../hub/components/ble_central/include
    ../ring/components/hal/include
)

# Event composer — compiled for host with lock macros as no-ops (no ESP_PLATFORM)
add_library(event_composer STATIC
    ../hub/components/event_composer/event_composer.c
)
target_include_directories(event_composer PUBLIC
    ../hub/components/event_composer/include
    ../hub/components/role_engine/include
    ../hub/components/ble_central/include
    ../ring/components/hal/include
)

# USB HID mouse — compiled for host (mock path, no TinyUSB)
add_library(usb_hid STATIC
    ../hub/components/usb_hid/usb_hid_mouse.c
    ../hub/components/usb_hid/hid_report_descriptor.c
)
target_include_directories(usb_hid PUBLIC
    ../hub/components/usb_hid/include
    ../hub/components/event_composer/include
    ../hub/components/role_engine/include
    ../hub/components/ble_central/include
    ../ring/components/hal/include
)

# Test executable
add_executable(powerfinger_tests
    test_state_machine.c
    test_dead_zone.c
    test_power_manager.c
    test_ring_runtime_health.c
    test_role_engine.c
    test_event_composer.c
    test_usb_hid.c
    test_main.c
)
target_link_libraries(powerfinger_tests
    unity
    mock_hal
    state_machine
    dead_zone
    power_manager
    ring_runtime_health
    role_engine
    event_composer
    usb_hid
)
target_include_directories(powerfinger_tests PRIVATE
    ../ring/components/state_machine/include
    ../ring/components/click/include
    ../ring/components/power/include
    ../ring/components/runtime_health/include
    ../ring/components/ble_hid/include
    ../ring/components/hal/include
    ../hub/components/role_engine/include
    ../hub/components/ble_central/include
    ../hub/components/event_composer/include
    ../hub/components/usb_hid/include
    mocks
)

add_test(NAME powerfinger_tests COMMAND powerfinger_tests)
