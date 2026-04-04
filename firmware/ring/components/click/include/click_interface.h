// SPDX-License-Identifier: MIT
// PowerFinger — Common click input interface
//
// All click mechanisms (snap dome, piezo+LRA) implement this interface.

#pragma once

#include "hal_types.h"
#include <stdbool.h>

typedef enum {
    CLICK_SOURCE_PRIMARY = 0,
    CLICK_SOURCE_SECONDARY,
} click_source_t;

// Initialize click hardware (GPIO for dome, ADC+PWM for piezo).
hal_status_t click_init(void);

// Poll click state. Returns true if button is currently pressed.
// Call from main loop, not ISR.
bool click_is_pressed(click_source_t source);

// Deinitialize (for power-down)
hal_status_t click_deinit(void);
