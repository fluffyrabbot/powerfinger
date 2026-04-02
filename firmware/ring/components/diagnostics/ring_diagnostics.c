// SPDX-License-Identifier: MIT
// PowerFinger — Ring diagnostics snapshot implementation

#include "ring_diagnostics.h"

#include <string.h>

static const char *s_sensor_state_names[] = {
    [RING_DIAG_SENSOR_UNAVAILABLE] = "unavailable",
    [RING_DIAG_SENSOR_CALIBRATION_PENDING] = "calibration_pending",
    [RING_DIAG_SENSOR_READY] = "ready",
};

static const char *s_bond_state_names[] = {
    [RING_DIAG_BOND_UNKNOWN] = "unknown",
    [RING_DIAG_BOND_RESTORED] = "restored",
    [RING_DIAG_BOND_FAILED] = "failed",
};

void ring_diagnostics_init(ring_diagnostics_t *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->snapshot.ring_state = RING_STATE_BOOTING;
    state->snapshot.bond_state = RING_DIAG_BOND_UNKNOWN;
    state->snapshot.sensor_state = RING_DIAG_SENSOR_UNAVAILABLE;
}

void ring_diagnostics_note_ring_state(ring_diagnostics_t *state, ring_state_t ring_state)
{
    if (!state) {
        return;
    }
    state->snapshot.ring_state = ring_state;
}

void ring_diagnostics_note_connected(ring_diagnostics_t *state, uint16_t conn_interval_1_25ms)
{
    if (!state) {
        return;
    }

    state->snapshot.connected = true;
    state->snapshot.conn_interval_1_25ms = conn_interval_1_25ms;
    state->snapshot.conn_param_rejected = false;
    state->snapshot.bond_state = RING_DIAG_BOND_UNKNOWN;
}

void ring_diagnostics_note_disconnected(ring_diagnostics_t *state)
{
    if (!state) {
        return;
    }

    state->snapshot.connected = false;
    state->snapshot.conn_interval_1_25ms = 0;
    state->snapshot.conn_param_rejected = false;
}

void ring_diagnostics_note_conn_params_updated(ring_diagnostics_t *state,
                                               uint16_t conn_interval_1_25ms)
{
    if (!state) {
        return;
    }

    state->snapshot.conn_interval_1_25ms = conn_interval_1_25ms;
    state->snapshot.conn_param_rejected = false;
}

void ring_diagnostics_note_conn_param_rejected(ring_diagnostics_t *state)
{
    if (!state) {
        return;
    }
    state->snapshot.conn_param_rejected = true;
}

void ring_diagnostics_note_bond_restored(ring_diagnostics_t *state)
{
    if (!state) {
        return;
    }
    state->snapshot.bond_state = RING_DIAG_BOND_RESTORED;
}

void ring_diagnostics_note_bond_failed(ring_diagnostics_t *state)
{
    if (!state) {
        return;
    }
    state->snapshot.bond_state = RING_DIAG_BOND_FAILED;
}

void ring_diagnostics_note_sensor_path(ring_diagnostics_t *state,
                                       bool sensor_available,
                                       bool calibration_valid)
{
    if (!state) {
        return;
    }

    state->snapshot.calibration_valid = calibration_valid;

    if (!sensor_available) {
        state->snapshot.sensor_state = RING_DIAG_SENSOR_UNAVAILABLE;
        return;
    }

    state->snapshot.sensor_state = calibration_valid
        ? RING_DIAG_SENSOR_READY
        : RING_DIAG_SENSOR_CALIBRATION_PENDING;
}

void ring_diagnostics_note_battery(ring_diagnostics_t *state,
                                   uint32_t battery_mv,
                                   uint8_t battery_pct)
{
    if (!state) {
        return;
    }

    state->snapshot.battery_mv = battery_mv;
    state->snapshot.battery_pct = battery_pct;
}

ring_diag_snapshot_t ring_diagnostics_snapshot(const ring_diagnostics_t *state)
{
    ring_diag_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.ring_state = RING_STATE_BOOTING;
    snapshot.bond_state = RING_DIAG_BOND_UNKNOWN;
    snapshot.sensor_state = RING_DIAG_SENSOR_UNAVAILABLE;

    if (!state) {
        return snapshot;
    }
    return state->snapshot;
}

const char *ring_diag_sensor_state_name(ring_diag_sensor_state_t state)
{
    if ((int)state >= 0 && state <= RING_DIAG_SENSOR_READY) {
        return s_sensor_state_names[state];
    }
    return "unknown";
}

const char *ring_diag_bond_state_name(ring_diag_bond_state_t state)
{
    if ((int)state >= 0 && state <= RING_DIAG_BOND_FAILED) {
        return s_bond_state_names[state];
    }
    return "unknown";
}
