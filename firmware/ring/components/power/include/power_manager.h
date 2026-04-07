// SPDX-License-Identifier: MIT
// PowerFinger — Power management interface
//
// Manages adaptive connection intervals, sleep entry/exit,
// battery monitoring, thermal safety, charge control, and watchdog.
//
// Decoupled from ring_state.h — uses its own event enum so the power
// manager can be reused in wand firmware without pulling ring state types.

#pragma once

#include "hal_types.h"
#include <stdbool.h>
#include <stdint.h>

// Power management events (mapped to ring_event_t by the caller)
typedef enum {
    POWER_EVT_NONE = 0,         // No event this tick
    POWER_EVT_IDLE_TIMEOUT,     // No activity for IDLE_TRANSITION_MS while active
    POWER_EVT_LOW_BATTERY,      // VBAT below cutoff — must enter deep sleep
    POWER_EVT_SLEEP_TIMEOUT,    // No activity for SLEEP_TIMEOUT_MS while idle
    POWER_EVT_THERMAL_SHUTDOWN, // Cell temp dangerously high — must enter deep sleep immediately
} power_event_t;

// Initialize power management subsystem.
// Sets up battery ADC, NTC ADC, charge control GPIO, and watchdog timer.
// Charge remains disabled until the first successful thermal + VBUS check.
hal_status_t power_manager_init(void);

// Called when a BLE connection is established.
// Resets connection parameter rejection state so active params (7.5ms)
// can be re-requested — the new central may accept them even if the last one didn't.
void power_manager_on_connect(void);

// Called when the BLE connection is lost.
// Clears active/idle tracking so connected-only timeouts stop firing.
void power_manager_on_disconnect(void);

// Called from main loop on sensor motion events.
// Marks the link active and requests low-latency connection parameters.
void power_manager_on_motion(void);

// Called from main loop while the click is held.
// Marks the link active and preserves low-latency button release behavior.
void power_manager_on_click(void);

// Called from main loop periodically.
// Checks battery voltage, cell temperature, and emits idle/sleep timeout events.
// Thermal checks run every tick when charging; VBAT checks run on BATTERY_CHECK_INTERVAL_MS.
// Returns the highest-priority event for this tick.
power_event_t power_manager_tick(uint32_t now_ms);

// Last sampled battery status, derived from VBAT.
// Battery percentage is an approximate loaded-voltage estimate.
uint8_t power_manager_get_battery_level(void);
uint32_t power_manager_get_last_battery_mv(void);

// Last sampled cell temperature in degrees Celsius (integer).
// Returns INT8_MIN if NTC is not configured or has never been read.
int8_t power_manager_get_cell_temp_c(void);

// Returns true if USB VBUS is detected (device is plugged in).
// Returns false if VBUS detection is not configured.
bool power_manager_is_vbus_present(void);

// Returns true if charging is currently enabled (MOSFET on).
bool power_manager_is_charging_enabled(void);

// Feed the watchdog timer. Must be called periodically from main loop.
void power_manager_feed_watchdog(void);

// Power-gate the motion sensor rail when supported by the form factor.
// No-op for variants without a dedicated sensor power pin.
hal_status_t power_manager_set_sensor_power(bool enabled);

// Override the deep-sleep wake GPIO bitmask for the current boot.
// Used by form-factor-specific wake debounce policies.
void power_manager_set_wake_gpio_mask(uint64_t pin_mask);

// Enter sleep mode as directed by state machine actions.
void power_manager_enter_sleep(bool deep);
