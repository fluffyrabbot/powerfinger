// SPDX-License-Identifier: MIT
// PowerFinger HAL — Sleep / power management interface

#pragma once

#include "hal_types.h"

typedef enum {
    HAL_SLEEP_LIGHT,    // CPU halts, peripherals may stay on, fast wake
    HAL_SLEEP_DEEP,     // Full power-down, wake from GPIO only, loses RAM
} hal_sleep_mode_t;

typedef enum {
    HAL_WAKE_CAUSE_COLD_BOOT = 0,
    HAL_WAKE_CAUSE_GPIO,
    HAL_WAKE_CAUSE_TIMER,
    HAL_WAKE_CAUSE_OTHER,
} hal_wake_cause_t;

// Enter sleep mode. For DEEP sleep, this function does not return —
// the device reboots on wake. For LIGHT sleep, returns after wakeup.
hal_status_t hal_sleep_enter(hal_sleep_mode_t mode);

// Configure a GPIO pin as a wake source for deep sleep.
// Must be called before hal_sleep_enter(HAL_SLEEP_DEEP).
// level: the GPIO level that triggers wake (true = high, false = low).
hal_status_t hal_sleep_configure_wake_gpio(hal_pin_t pin, bool level);

// Configure one or more GPIOs as a deep sleep wake source using a raw pin
// bitmask. Bits correspond to GPIO numbers.
hal_status_t hal_sleep_configure_wake_gpio_mask(uint64_t pin_mask, bool level);

// Enable timer-based wake from light sleep (e.g. for next BLE event).
hal_status_t hal_sleep_configure_wake_timer(uint32_t us);

// Return the wake reason for the current boot.
hal_wake_cause_t hal_sleep_get_wake_cause(void);
