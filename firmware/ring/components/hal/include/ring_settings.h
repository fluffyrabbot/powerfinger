// SPDX-License-Identifier: MIT
// PowerFinger — Persisted ring runtime settings
//
// Stores user-facing runtime parameters in NVS and exposes lock-free getters
// for the main loop and BLE config callbacks.

#pragma once

#include "hal_types.h"

typedef struct {
    uint8_t dpi_multiplier;
    uint16_t dead_zone_time_ms;
    uint8_t dead_zone_distance;
} ring_settings_snapshot_t;

// Load persisted settings from storage. Falls back to defaults when unset.
hal_status_t ring_settings_init(void);

// Return the current in-memory settings.
ring_settings_snapshot_t ring_settings_snapshot(void);

uint8_t ring_settings_get_dpi_multiplier(void);
uint16_t ring_settings_get_dead_zone_time_ms(void);
uint8_t ring_settings_get_dead_zone_distance(void);

// Apply a new runtime value and mark it dirty for deferred persistence.
hal_status_t ring_settings_set_dpi_multiplier(uint8_t value);
hal_status_t ring_settings_set_dead_zone_time_ms(uint16_t value);
hal_status_t ring_settings_set_dead_zone_distance(uint8_t value);

// Settings persistence is deferred to idle/disconnected states.
bool ring_settings_needs_flush(void);
hal_status_t ring_settings_flush(void);

// Apply the current DPI multiplier to a raw sensor delta.
int16_t ring_settings_scale_delta(int16_t raw_delta);
