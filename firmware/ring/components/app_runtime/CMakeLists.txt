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

idf_component_register(
    SRCS "app_runtime.c"
    INCLUDE_DIRS "include"
    REQUIRES ${APP_RUNTIME_REQUIRES}
    PRIV_REQUIRES app_update nvs_flash
)

# IDF expands REQUIRES before CONFIG_* exists. The pen application explicitly
# includes pen_wake; bind its private headers/library after component discovery.
idf_component_optional_requires(PRIVATE pen_wake)
