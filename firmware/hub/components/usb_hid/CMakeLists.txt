# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "usb_hid_mouse.c" "hid_report_descriptor.c"
    INCLUDE_DIRS "include"
    REQUIRES hal event_composer
)
