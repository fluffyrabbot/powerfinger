# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "pen_wake_debounce.c"
    INCLUDE_DIRS "include"
    REQUIRES hal power
)
