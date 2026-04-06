# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "ring_runtime_health.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal state_machine
)
