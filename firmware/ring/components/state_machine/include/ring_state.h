// SPDX-License-Identifier: MIT
// PowerFinger — Ring state machine
//
// Pure-logic state machine with no hardware dependencies.
// Takes events in, produces action flags out. Testable on host.

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Ring states (exactly one active at any time)
typedef enum {
    RING_STATE_DEEP_SLEEP = 0,
    RING_STATE_BOOTING,
    RING_STATE_ADVERTISING,
    RING_STATE_CONNECTED_ACTIVE,
    RING_STATE_CONNECTED_IDLE,
    RING_STATE_COUNT,
} ring_state_t;

// Events that trigger state transitions
typedef enum {
    RING_EVT_NONE = -1,             // No event (sentinel, not a real event)
    RING_EVT_GPIO_WAKE = 0,         // GPIO interrupt woke from deep sleep
    RING_EVT_CALIBRATION_DONE,      // Boot calibration completed successfully
    RING_EVT_CALIBRATION_FAILED,    // Calibration failed after max retries
    RING_EVT_BLE_CONNECTED,         // BLE link established
    RING_EVT_BLE_DISCONNECTED,      // BLE link lost
    RING_EVT_BLE_ADV_TIMEOUT,       // Advertising timed out with no connection
    RING_EVT_MOTION_DETECTED,       // Sensor delta above noise threshold
    RING_EVT_CLICK_ACTIVITY,        // Click press while connected idle
    RING_EVT_IDLE_TIMEOUT,          // No motion for IDLE_TRANSITION_MS
    RING_EVT_SLEEP_TIMEOUT,         // No activity for SLEEP_TIMEOUT_MS
    RING_EVT_LOW_BATTERY,           // VBAT below LOW_VOLTAGE_CUTOFF_MV
    RING_EVT_COUNT,
} ring_event_t;

// Action flags — what the main loop should do after a transition.
// Multiple flags can be set simultaneously.
typedef struct {
    bool start_advertising;
    bool stop_advertising;
    bool enter_deep_sleep;
    bool request_active_conn_params;    // 7.5ms
    bool request_idle_conn_params;      // 15ms
    bool enable_hid_reports;
    bool disable_hid_reports;
} ring_actions_t;

// Initialize the state machine. Call once at startup.
void ring_state_init(void);

// Dispatch an event and get the resulting actions.
// Returns the new state (may be same as current).
ring_state_t ring_state_dispatch(ring_event_t event, ring_actions_t *out_actions);

// Get current state (read-only)
ring_state_t ring_state_get(void);

// Get state name as string (for logging)
const char *ring_state_name(ring_state_t state);
const char *ring_event_name(ring_event_t event);
