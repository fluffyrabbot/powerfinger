// SPDX-License-Identifier: MIT
// PowerFinger HAL — Software timer interface

#pragma once

#include "hal_types.h"

// Create a periodic or one-shot software timer.
// callback is called from task context (not ISR).
hal_status_t hal_timer_create(const char *name, uint32_t period_ms, bool periodic,
                              hal_callback_t callback, void *arg,
                              hal_timer_handle_t *out_handle);

// Start (or restart) the timer
hal_status_t hal_timer_start(hal_timer_handle_t handle);

// Stop the timer
hal_status_t hal_timer_stop(hal_timer_handle_t handle);

// Change period while timer is running or stopped
hal_status_t hal_timer_set_period(hal_timer_handle_t handle, uint32_t period_ms);

// Delete the timer and free resources
hal_status_t hal_timer_delete(hal_timer_handle_t handle);

// Get current system time in milliseconds (monotonic, wraps at UINT32_MAX)
uint32_t hal_timer_get_ms(void);

// Delay current task for the given duration
hal_status_t hal_timer_delay_ms(uint32_t ms);
