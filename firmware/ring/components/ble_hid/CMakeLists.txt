# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "ble_hid_mouse.c" "ble_gap_handler.c" "hid_report_descriptor.c"
    INCLUDE_DIRS "include"
    REQUIRES hal
)
