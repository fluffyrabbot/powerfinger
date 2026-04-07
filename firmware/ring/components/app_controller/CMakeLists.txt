# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "app_controller.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_hid state_machine diagnostics power runtime_health click
)
