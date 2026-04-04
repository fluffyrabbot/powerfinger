# SPDX-License-Identifier: MIT

set(SENSOR_SRCS "sensor_none.c")

if(CONFIG_SENSOR_PAW3204)
    set(SENSOR_SRCS "paw3204.c")
elseif(CONFIG_SENSOR_PMW3360)
    set(SENSOR_SRCS "pmw3360.c")
elseif(CONFIG_SENSOR_BALL_HALL)
    set(SENSOR_SRCS "ball_hall.c")
endif()

idf_component_register(
    SRCS ${SENSOR_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES hal
)
