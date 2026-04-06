// SPDX-License-Identifier: MIT
// PowerFinger HAL — ADC interface
//
// Used for battery voltage monitoring and piezo click detection.

#pragma once

#include "hal_types.h"

// Typed ADC channel identifier (prevents confusion with other uint8_t values)
typedef uint8_t hal_adc_channel_t;

// Initialize ADC for the given channel
hal_status_t hal_adc_init(hal_adc_channel_t channel);

// Read ADC value in millivolts (calibrated)
hal_status_t hal_adc_read_mv(hal_adc_channel_t channel, uint32_t *out_mv);

// Deinitialize ADC channel
hal_status_t hal_adc_deinit(hal_adc_channel_t channel);
