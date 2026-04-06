# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES pf_hal ble_hid state_machine sensors click power runtime_health diagnostics pen_wake
    PRIV_REQUIRES app_update
)
