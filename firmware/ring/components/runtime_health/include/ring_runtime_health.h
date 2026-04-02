// SPDX-License-Identifier: MIT
// PowerFinger — Ring runtime health policy
//
// Tracks recoverable sensor degradation and sustained BLE HID send failures.

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hal_types.h"

typedef enum {
    RING_SENSOR_HEALTH_STABLE = 0,
    RING_SENSOR_HEALTH_DEGRADED,
    RING_SENSOR_HEALTH_RECOVERED,
} ring_sensor_health_event_t;

typedef struct {
    ring_sensor_health_event_t event;
    uint16_t consecutive_failures;
} ring_sensor_health_update_t;

typedef enum {
    RING_HID_HEALTH_STABLE = 0,
    RING_HID_HEALTH_FAULT_STARTED,
    RING_HID_HEALTH_RECOVERED,
    RING_HID_HEALTH_RESTART_REQUIRED,
} ring_hid_health_event_t;

typedef struct {
    ring_hid_health_event_t event;
    hal_status_t status;
    uint32_t fault_elapsed_ms;
} ring_hid_health_update_t;

typedef struct {
    uint16_t consecutive_sensor_failures;
    bool sensor_degraded;
    bool hid_fault_active;
    uint32_t hid_fault_start_ms;
    hal_status_t hid_fault_status;
} ring_runtime_health_t;

void ring_runtime_health_init(ring_runtime_health_t *state);

ring_sensor_health_update_t ring_runtime_health_note_sensor_result(
    ring_runtime_health_t *state,
    hal_status_t status);

ring_hid_health_update_t ring_runtime_health_note_hid_send_result(
    ring_runtime_health_t *state,
    hal_status_t status,
    uint32_t now_ms);

void ring_runtime_health_reset_hid_send(ring_runtime_health_t *state);
bool ring_runtime_health_sensor_degraded(const ring_runtime_health_t *state);
