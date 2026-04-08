# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "hub_settings.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_central
)
