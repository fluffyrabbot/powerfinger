// SPDX-License-Identifier: MIT
// PowerFinger — Ring runtime health policy implementation

#include "ring_runtime_health.h"
#include "ring_config.h"

#include <string.h>

void ring_runtime_health_init(ring_runtime_health_t *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
}

void ring_runtime_health_mark_sensor_unavailable(
    ring_runtime_health_t *state,
    uint32_t now_ms)
{
    if (!state) {
        return;
    }

    state->sensor_unavailable = true;
    state->next_sensor_recovery_attempt_ms = now_ms;
}

ring_sensor_health_update_t ring_runtime_health_note_sensor_result(
    ring_runtime_health_t *state,
    hal_status_t status,
    uint32_t now_ms)
{
    ring_sensor_health_update_t update = {
        .event = RING_SENSOR_HEALTH_STABLE,
        .consecutive_failures = 0,
    };

    if (!state) {
        return update;
    }

    if (status == HAL_OK) {
        bool was_recovering = state->sensor_degraded || state->sensor_unavailable;
        state->consecutive_sensor_failures = 0;
        state->sensor_degraded = false;
        state->sensor_unavailable = false;
        state->next_sensor_recovery_attempt_ms = 0;
        update.event = was_recovering ? RING_SENSOR_HEALTH_RECOVERED
                                      : RING_SENSOR_HEALTH_STABLE;
        return update;
    }

    if (state->consecutive_sensor_failures < UINT16_MAX) {
        state->consecutive_sensor_failures++;
    }
    update.consecutive_failures = state->consecutive_sensor_failures;

    if (!state->sensor_degraded &&
        state->consecutive_sensor_failures >= SENSOR_READ_FAIL_DEGRADE_THRESHOLD) {
        state->sensor_degraded = true;
        state->sensor_unavailable = true;
        state->next_sensor_recovery_attempt_ms = now_ms;
        update.event = RING_SENSOR_HEALTH_DEGRADED;
    }

    return update;
}

ring_sensor_health_update_t ring_runtime_health_note_sensor_recovery_attempt(
    ring_runtime_health_t *state,
    hal_status_t status,
    uint32_t now_ms)
{
    ring_sensor_health_update_t update = {
        .event = RING_SENSOR_HEALTH_STABLE,
        .consecutive_failures = 0,
    };

    if (!state) {
        return update;
    }

    if (status == HAL_OK) {
        bool was_recovering = state->sensor_degraded || state->sensor_unavailable;
        state->consecutive_sensor_failures = 0;
        state->sensor_degraded = false;
        state->sensor_unavailable = false;
        state->next_sensor_recovery_attempt_ms = 0;
        update.event = was_recovering ? RING_SENSOR_HEALTH_RECOVERED
                                      : RING_SENSOR_HEALTH_STABLE;
        return update;
    }

    state->sensor_unavailable = true;
    state->next_sensor_recovery_attempt_ms = now_ms + SENSOR_RECOVERY_ATTEMPT_MS;
    update.consecutive_failures = state->consecutive_sensor_failures;
    return update;
}

bool ring_runtime_health_sensor_recovery_due(
    const ring_runtime_health_t *state,
    uint32_t now_ms)
{
    if (!state) {
        return false;
    }

    return (state->sensor_unavailable || state->sensor_degraded) &&
           now_ms >= state->next_sensor_recovery_attempt_ms;
}

ring_hid_health_update_t ring_runtime_health_note_hid_send_result(
    ring_runtime_health_t *state,
    hal_status_t status,
    uint32_t now_ms)
{
    ring_hid_health_update_t update = {
        .event = RING_HID_HEALTH_STABLE,
        .status = status,
        .fault_elapsed_ms = 0,
    };

    if (!state) {
        return update;
    }

    if (status == HAL_OK || status == HAL_ERR_BUSY) {
        bool was_fault_active = state->hid_fault_active;
        ring_runtime_health_reset_hid_send(state);
        update.event = was_fault_active ? RING_HID_HEALTH_RECOVERED
                                        : RING_HID_HEALTH_STABLE;
        return update;
    }

    if (!state->hid_fault_active) {
        state->hid_fault_active = true;
        state->hid_fault_start_ms = now_ms;
        state->hid_fault_status = status;
        update.event = RING_HID_HEALTH_FAULT_STARTED;
        return update;
    }

    state->hid_fault_status = status;
    update.fault_elapsed_ms = now_ms - state->hid_fault_start_ms;

    if (update.fault_elapsed_ms >= BLE_HID_SEND_RESTART_MS) {
        update.event = RING_HID_HEALTH_RESTART_REQUIRED;
    }

    return update;
}

void ring_runtime_health_reset_hid_send(ring_runtime_health_t *state)
{
    if (!state) {
        return;
    }
    state->hid_fault_active = false;
    state->hid_fault_start_ms = 0;
    state->hid_fault_status = HAL_OK;
}

bool ring_runtime_health_sensor_degraded(const ring_runtime_health_t *state)
{
    return state ? state->sensor_degraded : false;
}

bool ring_runtime_health_sensor_unavailable(const ring_runtime_health_t *state)
{
    return state ? state->sensor_unavailable : false;
}
