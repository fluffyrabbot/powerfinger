// SPDX-License-Identifier: MIT
// PowerFinger — Common sensor interface
//
// All sensor drivers (optical, ball+Hall, IMU) implement this interface.
// The main application code never includes driver-specific headers.

#pragma once

#include "hal_types.h"

typedef struct {
    int16_t dx;                 // X delta since last read
    int16_t dy;                 // Y delta since last read
    uint8_t surface_confidence; // 0-255: SQUAL for optical, baseline deviation for Hall, 0 for IMU
    bool    motion_detected;    // true if any delta above noise floor
} sensor_reading_t;

// Initialize the sensor. Called once at boot after HAL init.
hal_status_t sensor_init(void);

// Read accumulated deltas since last read.
// For optical sensors, the sensor accumulates internally.
// For Hall/IMU, the driver accumulates in a buffer.
hal_status_t sensor_read(sensor_reading_t *out);

// Power down the sensor (for sleep modes).
hal_status_t sensor_power_down(void);

// Wake the sensor from power-down.
hal_status_t sensor_wake(void);
