# SPDX-License-Identifier: MIT
# PowerFinger USB Hub Dongle Firmware — ESP-IDF Project

cmake_minimum_required(VERSION 3.16)

# Share HAL component from ring project
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/components"
    "${CMAKE_CURRENT_SOURCE_DIR}/../ring/components/hal"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(powerfinger_hub)
