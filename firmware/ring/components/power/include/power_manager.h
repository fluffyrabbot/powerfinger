// SPDX-License-Identifier: MIT
// PowerFinger — Power management interface
//
// Manages adaptive connection intervals, sleep entry/exit,
// battery monitoring, and watchdog.

#pragma once

#include "hal_types.h"
#include "ring_state.h"
#include <stdbool.h>

// Initialize power management subsystem.
// Sets up battery ADC, watchdog timer.
hal_status_t power_manager_init(void);

// Called from main loop on sensor motion events.
// Manages adaptive connection interval transitions.
void power_manager_on_motion(void);

// Called from main loop periodically.
// Checks battery voltage, manages idle/sleep timers.
// Returns LOW_BATTERY event if voltage below cutoff.
ring_event_t power_manager_tick(uint32_t now_ms);

// Feed the watchdog timer. Must be called periodically from main loop.
void power_manager_feed_watchdog(void);

// Enter sleep mode as directed by state machine actions.
void power_manager_enter_sleep(bool deep);
