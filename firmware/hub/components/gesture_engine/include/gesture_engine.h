// SPDX-License-Identifier: MIT
// PowerFinger Hub — Gesture mapping interface
//
// Stores hub-owned gesture mappings that require multi-ring awareness.

#pragma once

#include "hal_types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GESTURE_ENGINE_MAX_ENTRIES 8U

typedef enum {
    GESTURE_TRIGGER_CURSOR_SCROLL_CLICK = 0x01,
    GESTURE_TRIGGER_CURSOR_MODIFIER_CLICK = 0x02,
    GESTURE_TRIGGER_SCROLL_MODIFIER_CLICK = 0x03,
    GESTURE_TRIGGER_ALL_THREE_CLICK = 0x04,
    GESTURE_TRIGGER_CURSOR_DOUBLE_CLICK = 0x05,
    GESTURE_TRIGGER_SCROLL_DOUBLE_CLICK = 0x06,
    GESTURE_TRIGGER_MAX = GESTURE_TRIGGER_SCROLL_DOUBLE_CLICK,
} gesture_trigger_t;

typedef enum {
    GESTURE_ACTION_NONE = 0x00,
    GESTURE_ACTION_MIDDLE_CLICK = 0x01,
    GESTURE_ACTION_BACK = 0x02,
    GESTURE_ACTION_FORWARD = 0x03,
    GESTURE_ACTION_SCROLL_LOCK_TOGGLE = 0x04,
    GESTURE_ACTION_DPI_CYCLE = 0x05,
    GESTURE_ACTION_MAX = GESTURE_ACTION_DPI_CYCLE,
} gesture_action_t;

typedef struct {
    uint8_t trigger;
    uint8_t action;
} gesture_engine_entry_t;

hal_status_t gesture_engine_init(void);
gesture_action_t gesture_engine_get_action(uint8_t trigger);
hal_status_t gesture_engine_get_all(gesture_engine_entry_t *entries_out,
                                    size_t max_entries,
                                    size_t *count_out);
hal_status_t gesture_engine_set_action(uint8_t trigger, gesture_action_t action);
void gesture_engine_flush_if_dirty(void);

bool gesture_engine_trigger_known(uint8_t trigger);
bool gesture_engine_trigger_supported(uint8_t trigger);
bool gesture_engine_action_known(uint8_t action);
bool gesture_engine_action_supported(uint8_t action);

const char *gesture_engine_trigger_name(uint8_t trigger);
const char *gesture_engine_action_name(uint8_t action);
