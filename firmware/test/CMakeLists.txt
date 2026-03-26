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

# Test executable
add_executable(powerfinger_tests
    test_state_machine.c
    test_dead_zone.c
    test_main.c
)
target_link_libraries(powerfinger_tests
    unity
    mock_hal
    state_machine
    dead_zone
)
target_include_directories(powerfinger_tests PRIVATE
    ../ring/components/state_machine/include
    ../ring/components/click/include
    ../ring/components/hal/include
    mocks
)
