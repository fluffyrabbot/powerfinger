// SPDX-License-Identifier: MIT
// PowerFinger — Ball+Hall sensor constants (4x DRV5053)
//
// The ball+Hall sensor uses four DRV5053 analog Hall sensors arranged in
// opposing pairs around a 5mm steel ball with magnetic roller spindles.
// Each pair reads differential rotation on one axis.

#pragma once

// --- DRV5053 characteristics ---

// DRV5053 output at zero field: nominally VCC/2 (~1650mV at 3.3V).
// Actual baseline is captured during calibration.
#define BALL_HALL_NOMINAL_ZERO_MV   1650

// DRV5053 sensitivity: ~23mV/mT (EADR variant, ratiometric at 3.3V).
// At finger-pressure rotation speeds, expect 10-100mV differentials.
#define BALL_HALL_SENSITIVITY_MV_PER_MT  23

// --- Calibration ---

// Number of ADC samples per channel during baseline capture.
// Must match CALIBRATION_SAMPLE_COUNT in ring_config.h.
#define BALL_HALL_CAL_SAMPLES       50

// Delay between calibration samples (ms)
#define BALL_HALL_CAL_PERIOD_MS     2

// Maximum acceptable standard deviation during baseline capture (mV).
// If exceeded, the ball was moving during calibration.
#define BALL_HALL_CAL_MOTION_MV     15

// --- Runtime ---

// Minimum millivolt differential to register as motion.
// Below this, sensor noise is dominant. Mapped from NOISE_THRESHOLD_HALL
// in ring_config.h but expressed in the driver's native unit (mV).
#define BALL_HALL_NOISE_MV          8

// ADC settling time after power-on (ms). DRV5053 power-on time is ~30us;
// this covers MOSFET gate + bypass cap + ADC unit init.
#define BALL_HALL_POWER_ON_MS       2

// Channel indices for the four Hall sensors
#define BALL_HALL_CH_X_POS  0
#define BALL_HALL_CH_X_NEG  1
#define BALL_HALL_CH_Y_POS  2
#define BALL_HALL_CH_Y_NEG  3
#define BALL_HALL_NUM_CH    4
