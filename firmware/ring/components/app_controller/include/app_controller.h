// SPDX-License-Identifier: MIT
// PowerFinger — Ring app controller
//
// Coordinates ring-state dispatch, action execution, and transition side
// effects so the entrypoint can stay focused on polling and I/O.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "dead_zone.h"
#include "power_manager.h"
#include "ring_diagnostics.h"
#include "ring_runtime_health.h"
#include "ring_state.h"

typedef enum {
    APP_CONTROLLER_EVT_RING_STATE = 0,
    APP_CONTROLLER_EVT_BOND_RESTORED,
    APP_CONTROLLER_EVT_BOND_FAILED,
    APP_CONTROLLER_EVT_CONN_PARAMS_UPDATED,
    APP_CONTROLLER_EVT_CONN_PARAMS_REJECTED,
} app_controller_event_type_t;

typedef struct {
    app_controller_event_type_t type;
    ring_event_t ring_evt;
    uint16_t conn_interval_1_25ms;
} app_controller_event_t;

typedef struct {
    ring_diagnostics_t *diagnostics;
    ring_runtime_health_t *runtime_health;
    dead_zone_ctx_t *primary_dead_zone;
    dead_zone_ctx_t *secondary_dead_zone;
    uint32_t advertising_started_ms;
    uint8_t previous_buttons;
} app_controller_t;

void app_controller_init(app_controller_t *controller,
                         ring_diagnostics_t *diagnostics,
                         ring_runtime_health_t *runtime_health,
                         dead_zone_ctx_t *primary_dead_zone,
                         dead_zone_ctx_t *secondary_dead_zone);

void app_controller_log_diagnostics_snapshot(const char *reason,
                                             const ring_diagnostics_t *diagnostics);

void app_controller_dispatch_ring_event(app_controller_t *controller,
                                        ring_event_t event,
                                        uint16_t conn_interval_1_25ms,
                                        uint32_t now_ms,
                                        const char *reason);

void app_controller_handle_event(app_controller_t *controller,
                                 const app_controller_event_t *event,
                                 uint32_t now_ms);

void app_controller_reconcile_ble_event_drops(app_controller_t *controller,
                                              unsigned int dropped,
                                              bool connected,
                                              uint32_t now_ms);

void app_controller_check_advertising_timeout(app_controller_t *controller,
                                              uint32_t now_ms);

void app_controller_handle_power_event(app_controller_t *controller,
                                       power_event_t event,
                                       uint32_t now_ms);

uint8_t app_controller_previous_buttons(const app_controller_t *controller);
void app_controller_set_previous_buttons(app_controller_t *controller,
                                         uint8_t buttons);
