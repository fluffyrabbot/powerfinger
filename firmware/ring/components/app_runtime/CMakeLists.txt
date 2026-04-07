# SPDX-License-Identifier: MIT

set(APP_RUNTIME_REQUIRES
    app_controller
    pf_hal
    ble_hid
    state_machine
    sensors
    click
    power
    runtime_health
    diagnostics
)

if(CONFIG_POWERFINGER_FORM_FACTOR_PEN)
    list(APPEND APP_RUNTIME_REQUIRES pen_wake)
endif()

idf_component_register(
    SRCS "app_runtime.c"
    INCLUDE_DIRS "include"
    REQUIRES ${APP_RUNTIME_REQUIRES}
    PRIV_REQUIRES app_update nvs_flash
)
