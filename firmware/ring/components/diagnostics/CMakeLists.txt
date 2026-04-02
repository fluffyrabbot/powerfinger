# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "ring_diagnostics.c"
    INCLUDE_DIRS "include"
    REQUIRES state_machine
)
