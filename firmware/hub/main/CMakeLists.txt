# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES pf_hal ble_central role_engine gesture_engine hub_settings event_composer usb_hid hub_identity companion_cdc
    PRIV_REQUIRES app_update
)
