# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "usb_hid_mouse.c" "hid_report_descriptor.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal event_composer gesture_engine hub_identity esp_hw_support
)
