# SPDX-License-Identifier: MIT
# PowerPen Firmware — ESP-IDF Project

cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/components"
    "${CMAKE_CURRENT_SOURCE_DIR}/../ring/components"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(powerfinger_pen)
