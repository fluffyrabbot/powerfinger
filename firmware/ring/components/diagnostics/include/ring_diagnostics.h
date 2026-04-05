// SPDX-License-Identifier: MIT
// PowerFinger — Ring diagnostics snapshot
//
// Captures bring-up relevant state in one place so logs, future BLE
// diagnostics, and host tests all read the same facts.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ring_state.h"

#define RING_DIAG_BLE_PAYLOAD_VERSION 1U
#define RING_DIAG_BLE_PAYLOAD_LEN     10U

typedef enum {
    RING_DIAG_SENSOR_UNAVAILABLE = 0,
    RING_DIAG_SENSOR_CALIBRATION_PENDING,
    RING_DIAG_SENSOR_READY,
} ring_diag_sensor_state_t;

typedef enum {
    RING_DIAG_BOND_UNKNOWN = 0,
    RING_DIAG_BOND_RESTORED,
    RING_DIAG_BOND_FAILED,
} ring_diag_bond_state_t;

typedef struct {
    ring_state_t ring_state;
    bool connected;
    uint16_t conn_interval_1_25ms;
    bool conn_param_rejected;
    ring_diag_bond_state_t bond_state;
    ring_diag_sensor_state_t sensor_state;
    bool calibration_valid;
    uint8_t battery_pct;
    uint32_t battery_mv;
    bool drv5032_wake_enabled;
    uint8_t spurious_wake_count;
} ring_diag_snapshot_t;

typedef struct {
    ring_diag_snapshot_t snapshot;
} ring_diagnostics_t;

void ring_diagnostics_init(ring_diagnostics_t *state);
void ring_diagnostics_note_ring_state(ring_diagnostics_t *state, ring_state_t ring_state);
void ring_diagnostics_note_connected(ring_diagnostics_t *state, uint16_t conn_interval_1_25ms);
void ring_diagnostics_note_disconnected(ring_diagnostics_t *state);
void ring_diagnostics_note_conn_params_updated(ring_diagnostics_t *state,
                                               uint16_t conn_interval_1_25ms);
void ring_diagnostics_note_conn_param_rejected(ring_diagnostics_t *state);
void ring_diagnostics_note_bond_restored(ring_diagnostics_t *state);
void ring_diagnostics_note_bond_failed(ring_diagnostics_t *state);
void ring_diagnostics_note_sensor_path(ring_diagnostics_t *state,
                                       bool sensor_available,
                                       bool calibration_valid);
void ring_diagnostics_note_battery(ring_diagnostics_t *state,
                                   uint32_t battery_mv,
                                   uint8_t battery_pct);
void ring_diagnostics_note_pen_wake(ring_diagnostics_t *state,
                                    bool drv5032_wake_enabled,
                                    uint8_t spurious_wake_count);

ring_diag_snapshot_t ring_diagnostics_snapshot(const ring_diagnostics_t *state);
size_t ring_diagnostics_encode_ble_payload(const ring_diagnostics_t *state,
                                           uint8_t *buf,
                                           size_t buf_len);
const char *ring_diag_sensor_state_name(ring_diag_sensor_state_t state);
const char *ring_diag_bond_state_name(ring_diag_bond_state_t state);
