# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "ring_runtime_health.c"
    INCLUDE_DIRS "include"
    REQUIRES hal state_machine
)
