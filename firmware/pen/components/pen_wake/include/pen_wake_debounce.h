// SPDX-License-Identifier: MIT
// PowerFinger — PowerPen DRV5032 wake debounce

#pragma once

#include "hal_types.h"
#include <stdbool.h>
#include <stdint.h>

hal_status_t pen_wake_debounce_init(void);
void pen_wake_debounce_tick(uint32_t now_ms);
void pen_wake_debounce_note_motion(void);
void pen_wake_debounce_note_barrel_press(void);
void pen_wake_debounce_note_validation_failure(void);
bool pen_wake_debounce_drv5032_enabled(void);
uint8_t pen_wake_debounce_spurious_wake_count(void);
bool pen_wake_debounce_validation_pending(void);
