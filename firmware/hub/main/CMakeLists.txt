# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES hal ble_central role_engine event_composer usb_hid
)
