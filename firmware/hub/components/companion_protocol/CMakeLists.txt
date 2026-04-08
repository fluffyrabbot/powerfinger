# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "companion_protocol.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_central role_engine gesture_engine hub_control hub_settings
)
