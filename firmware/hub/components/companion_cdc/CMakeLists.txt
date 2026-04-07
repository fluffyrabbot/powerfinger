# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "companion_cdc.c"
    INCLUDE_DIRS "include"
    REQUIRES pf_hal companion_session
)
