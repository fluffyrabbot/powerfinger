// SPDX-License-Identifier: MIT
// PowerFinger — Ring diagnostics snapshot implementation

#include "ring_diagnostics.h"

#include <limits.h>
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
    state->snapshot.drv5032_wake_enabled = true;
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

void ring_diagnostics_note_pen_wake(ring_diagnostics_t *state,
                                    bool drv5032_wake_enabled,
                                    uint8_t spurious_wake_count)
{
    if (!state) {
        return;
    }

    state->snapshot.drv5032_wake_enabled = drv5032_wake_enabled;
    state->snapshot.spurious_wake_count = spurious_wake_count;
}

ring_diag_snapshot_t ring_diagnostics_snapshot(const ring_diagnostics_t *state)
{
    ring_diag_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.ring_state = RING_STATE_BOOTING;
    snapshot.bond_state = RING_DIAG_BOND_UNKNOWN;
    snapshot.sensor_state = RING_DIAG_SENSOR_UNAVAILABLE;
    snapshot.drv5032_wake_enabled = true;

    if (!state) {
        return snapshot;
    }
    return state->snapshot;
}

size_t ring_diagnostics_encode_ble_payload(const ring_diagnostics_t *state,
                                           uint8_t *buf,
                                           size_t buf_len)
{
    ring_diag_snapshot_t snapshot = ring_diagnostics_snapshot(state);
    uint16_t battery_mv = (snapshot.battery_mv > UINT16_MAX)
        ? UINT16_MAX
        : (uint16_t)snapshot.battery_mv;
    uint8_t flags = 0;

    if (!buf || buf_len < RING_DIAG_BLE_PAYLOAD_LEN) {
        return 0;
    }

    if (snapshot.connected) {
        flags |= 0x01;
    }
    if (snapshot.calibration_valid) {
        flags |= 0x02;
    }
    if (snapshot.conn_param_rejected) {
        flags |= 0x04;
    }

    buf[0] = RING_DIAG_BLE_PAYLOAD_VERSION;
    buf[1] = (uint8_t)snapshot.ring_state;
    buf[2] = (uint8_t)snapshot.sensor_state;
    buf[3] = (uint8_t)snapshot.bond_state;
    buf[4] = flags;
    buf[5] = snapshot.battery_pct;
    buf[6] = (uint8_t)(battery_mv & 0xFFU);
    buf[7] = (uint8_t)((battery_mv >> 8) & 0xFFU);
    buf[8] = (uint8_t)(snapshot.conn_interval_1_25ms & 0xFFU);
    buf[9] = (uint8_t)((snapshot.conn_interval_1_25ms >> 8) & 0xFFU);

    return RING_DIAG_BLE_PAYLOAD_LEN;
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
