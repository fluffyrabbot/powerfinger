# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES app_runtime pen_wake
)
