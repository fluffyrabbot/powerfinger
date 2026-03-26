# SPDX-License-Identifier: MIT

set(CLICK_SRCS "dead_zone.c" "calibration.c")

if(CONFIG_CLICK_SNAP_DOME)
    list(APPEND CLICK_SRCS "click_dome.c")
elseif(CONFIG_CLICK_PIEZO_LRA)
    list(APPEND CLICK_SRCS "click_piezo.c")
else()
    list(APPEND CLICK_SRCS "click_none.c")
endif()

idf_component_register(
    SRCS ${CLICK_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES hal sensors
)
