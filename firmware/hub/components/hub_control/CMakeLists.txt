# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "hub_control.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_central role_engine event_composer gesture_engine hub_settings
)
