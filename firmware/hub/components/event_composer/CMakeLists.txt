# SPDX-License-Identifier: MIT
idf_component_register(
    SRCS "event_composer.c"
    INCLUDE_DIRS "include"
    REQUIRES hal role_engine
)
