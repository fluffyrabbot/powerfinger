# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "companion_protocol.c"
    INCLUDE_DIRS "include"
    REQUIRES hal ble_central role_engine hub_control
)
