// SPDX-License-Identifier: MIT
// PowerFinger — Power management interface
//
// Manages adaptive connection intervals, sleep entry/exit,
// battery monitoring, and watchdog.
//
// Decoupled from ring_state.h — uses its own event enum so the power
// manager can be reused in wand firmware without pulling ring state types.

#pragma once

#include "hal_types.h"
#include <stdbool.h>

// Power management events (mapped to ring_event_t by the caller)
typedef enum {
    POWER_EVT_NONE = 0,         // No event this tick
    POWER_EVT_LOW_BATTERY,      // VBAT below cutoff — must enter deep sleep
    POWER_EVT_SLEEP_TIMEOUT,    // No activity for SLEEP_TIMEOUT_MS
} power_event_t;

// Initialize power management subsystem.
// Sets up battery ADC, watchdog timer.
hal_status_t power_manager_init(void);

// Called when a BLE connection is established.
// Resets connection parameter rejection state so active params (7.5ms)
// can be re-requested — the new central may accept them even if the last one didn't.
void power_manager_on_connect(void);

// Called from main loop on sensor motion events.
// Manages adaptive connection interval transitions.
void power_manager_on_motion(void);

// Called from main loop while the click is held.
// Keeps the device awake and preserves low-latency button release behavior.
void power_manager_on_click(void);

// Called from main loop periodically.
// Checks battery voltage, manages idle/sleep timers.
power_event_t power_manager_tick(uint32_t now_ms);

// Last sampled battery status, derived from VBAT.
// Battery percentage is an approximate loaded-voltage estimate.
uint8_t power_manager_get_battery_level(void);
uint32_t power_manager_get_last_battery_mv(void);

// Feed the watchdog timer. Must be called periodically from main loop.
void power_manager_feed_watchdog(void);

// Enter sleep mode as directed by state machine actions.
void power_manager_enter_sleep(bool deep);
