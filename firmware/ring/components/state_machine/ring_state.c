// SPDX-License-Identifier: MIT
// PowerFinger — Ring state machine implementation
//
// Table-driven state machine. Each (state, event) pair maps to a handler
// that returns the new state and sets action flags.
//
// Invariants enforced at dispatch level:
//   - LOW_BATTERY forces DEEP_SLEEP from any state
//   - HID reports are never enabled before CONNECTED_ACTIVE

#include "ring_state.h"
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "ring_state";
#define LOG_TRANSITION(from, to, evt) \
    ESP_LOGI(TAG, "%s + %s -> %s", ring_state_name(from), ring_event_name(evt), ring_state_name(to))
#else
#define LOG_TRANSITION(from, to, evt) (void)0
#endif

static ring_state_t s_state = RING_STATE_DEEP_SLEEP;

// --- State names (for logging) ---

static const char *s_state_names[] = {
    [RING_STATE_DEEP_SLEEP]       = "DEEP_SLEEP",
    [RING_STATE_BOOTING]          = "BOOTING",
    [RING_STATE_ADVERTISING]      = "ADVERTISING",
    [RING_STATE_CONNECTED_ACTIVE] = "CONNECTED_ACTIVE",
    [RING_STATE_CONNECTED_IDLE]   = "CONNECTED_IDLE",
};

static const char *s_event_names[] = {
    [RING_EVT_GPIO_WAKE]          = "GPIO_WAKE",
    [RING_EVT_CALIBRATION_DONE]   = "CALIBRATION_DONE",
    [RING_EVT_CALIBRATION_FAILED] = "CALIBRATION_FAILED",
    [RING_EVT_BLE_CONNECTED]      = "BLE_CONNECTED",
    [RING_EVT_BLE_DISCONNECTED]   = "BLE_DISCONNECTED",
    [RING_EVT_BLE_ADV_TIMEOUT]    = "BLE_ADV_TIMEOUT",
    [RING_EVT_MOTION_DETECTED]    = "MOTION_DETECTED",
    [RING_EVT_IDLE_TIMEOUT]       = "IDLE_TIMEOUT",
    [RING_EVT_SLEEP_TIMEOUT]      = "SLEEP_TIMEOUT",
    [RING_EVT_LOW_BATTERY]        = "LOW_BATTERY",
};

const char *ring_state_name(ring_state_t state)
{
    if (state < RING_STATE_COUNT) return s_state_names[state];
    return "UNKNOWN";
}

const char *ring_event_name(ring_event_t event)
{
    if (event < RING_EVT_COUNT) return s_event_names[event];
    return "UNKNOWN";
}

// --- Handler type ---
// Returns new state and populates actions. NULL in the table = no transition.
typedef ring_state_t (*handler_t)(ring_actions_t *actions);

// --- Handlers ---

static ring_state_t h_deep_sleep_wake(ring_actions_t *a)
{
    (void)a;
    return RING_STATE_BOOTING;
}

static ring_state_t h_boot_cal_done(ring_actions_t *a)
{
    a->start_advertising = true;
    return RING_STATE_ADVERTISING;
}

static ring_state_t h_boot_cal_failed(ring_actions_t *a)
{
    // Calibration failed but we proceed anyway with zero offset
    a->start_advertising = true;
    return RING_STATE_ADVERTISING;
}

static ring_state_t h_adv_connected(ring_actions_t *a)
{
    a->stop_advertising = true;
    a->request_idle_conn_params = true;
    return RING_STATE_CONNECTED_IDLE;
}

static ring_state_t h_adv_timeout(ring_actions_t *a)
{
    a->stop_advertising = true;
    a->enter_deep_sleep = true;
    return RING_STATE_DEEP_SLEEP;
}

static ring_state_t h_active_idle_timeout(ring_actions_t *a)
{
    a->request_idle_conn_params = true;
    a->disable_hid_reports = true;
    return RING_STATE_CONNECTED_IDLE;
}

static ring_state_t h_active_disconnected(ring_actions_t *a)
{
    a->disable_hid_reports = true;
    a->start_advertising = true;
    return RING_STATE_ADVERTISING;
}

static ring_state_t h_idle_motion(ring_actions_t *a)
{
    a->request_active_conn_params = true;
    a->enable_hid_reports = true;
    return RING_STATE_CONNECTED_ACTIVE;
}

static ring_state_t h_idle_sleep_timeout(ring_actions_t *a)
{
    a->enter_deep_sleep = true;
    return RING_STATE_DEEP_SLEEP;
}

static ring_state_t h_idle_disconnected(ring_actions_t *a)
{
    a->start_advertising = true;
    return RING_STATE_ADVERTISING;
}

static ring_state_t h_force_deep_sleep(ring_actions_t *a)
{
    a->enter_deep_sleep = true;
    a->disable_hid_reports = true;
    a->stop_advertising = true;
    return RING_STATE_DEEP_SLEEP;
}

// --- Transition table ---
// table[state][event] = handler. NULL = event ignored in this state.

static const handler_t s_table[RING_STATE_COUNT][RING_EVT_COUNT] = {
    // DEEP_SLEEP
    [RING_STATE_DEEP_SLEEP] = {
        [RING_EVT_GPIO_WAKE]        = h_deep_sleep_wake,
    },
    // BOOTING
    [RING_STATE_BOOTING] = {
        [RING_EVT_CALIBRATION_DONE]   = h_boot_cal_done,
        [RING_EVT_CALIBRATION_FAILED] = h_boot_cal_failed,
    },
    // ADVERTISING
    [RING_STATE_ADVERTISING] = {
        [RING_EVT_BLE_CONNECTED]    = h_adv_connected,
        [RING_EVT_BLE_ADV_TIMEOUT]  = h_adv_timeout,
    },
    // CONNECTED_ACTIVE
    [RING_STATE_CONNECTED_ACTIVE] = {
        [RING_EVT_IDLE_TIMEOUT]     = h_active_idle_timeout,
        [RING_EVT_BLE_DISCONNECTED] = h_active_disconnected,
    },
    // CONNECTED_IDLE
    [RING_STATE_CONNECTED_IDLE] = {
        [RING_EVT_MOTION_DETECTED]  = h_idle_motion,
        [RING_EVT_SLEEP_TIMEOUT]    = h_idle_sleep_timeout,
        [RING_EVT_BLE_DISCONNECTED] = h_idle_disconnected,
    },
};

// --- Public API ---

void ring_state_init(void)
{
    s_state = RING_STATE_BOOTING;
}

ring_state_t ring_state_dispatch(ring_event_t event, ring_actions_t *out_actions)
{
    ring_actions_t actions;
    memset(&actions, 0, sizeof(actions));

    // Invariant: LOW_BATTERY forces DEEP_SLEEP from any state
    if (event == RING_EVT_LOW_BATTERY) {
        ring_state_t prev = s_state;
        s_state = h_force_deep_sleep(&actions);
        LOG_TRANSITION(prev, s_state, event);
        if (out_actions) *out_actions = actions;
        return s_state;
    }

    if (event >= RING_EVT_COUNT || s_state >= RING_STATE_COUNT) {
        if (out_actions) *out_actions = actions;
        return s_state;
    }

    handler_t handler = s_table[s_state][event];
    if (handler) {
        ring_state_t prev = s_state;
        s_state = handler(&actions);
        LOG_TRANSITION(prev, s_state, event);
    }

    if (out_actions) *out_actions = actions;
    return s_state;
}

ring_state_t ring_state_get(void)
{
    return s_state;
}
