# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "event_composer.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal ble_central role_engine
)
