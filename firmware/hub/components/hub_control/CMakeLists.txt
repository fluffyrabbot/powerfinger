# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "hub_control.c"
    INCLUDE_DIRS "include"
    REQUIRES hal ble_central role_engine event_composer
)
