// SPDX-License-Identifier: MIT
// PowerFinger HAL — ADC interface
//
// Used for battery voltage monitoring and piezo click detection.

#pragma once

#include "hal_types.h"

// Initialize ADC for the given channel
hal_status_t hal_adc_init(uint8_t channel);

// Read ADC value in millivolts (calibrated)
hal_status_t hal_adc_read_mv(uint8_t channel, uint32_t *out_mv);

// Deinitialize ADC channel
hal_status_t hal_adc_deinit(uint8_t channel);
