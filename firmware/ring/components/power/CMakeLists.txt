# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "power_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_hid
    PRIV_REQUIRES state_machine
)
