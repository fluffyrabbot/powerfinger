// SPDX-License-Identifier: MIT
// PowerFinger HAL — GPIO interface

#pragma once

#include "hal_types.h"

typedef enum {
    HAL_GPIO_INPUT,
    HAL_GPIO_OUTPUT,
    HAL_GPIO_INPUT_PULLUP,
    HAL_GPIO_INPUT_PULLDOWN,
} hal_gpio_mode_t;

typedef enum {
    HAL_GPIO_INTR_NONE,
    HAL_GPIO_INTR_RISING,
    HAL_GPIO_INTR_FALLING,
    HAL_GPIO_INTR_ANY_EDGE,
} hal_gpio_intr_t;

// Initialize a GPIO pin with the given mode
hal_status_t hal_gpio_init(hal_pin_t pin, hal_gpio_mode_t mode);

// Set output level (true = high, false = low)
hal_status_t hal_gpio_set(hal_pin_t pin, bool level);

// Read input level
bool hal_gpio_get(hal_pin_t pin);

// Configure edge interrupt on a pin. Pass INTR_NONE to disable.
// Callback is invoked from ISR context — keep it short (set a flag).
hal_status_t hal_gpio_set_interrupt(hal_pin_t pin, hal_gpio_intr_t edge,
                                    hal_isr_callback_t cb, void *arg);

// Deep sleep wake via GPIO is configured through hal_sleep.h:
// hal_sleep_configure_wake_gpio(pin, level)
