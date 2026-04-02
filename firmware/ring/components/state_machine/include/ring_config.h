// SPDX-License-Identifier: MIT
// PowerFinger — Ring configuration constants
//
// Every threshold and timeout is a named constant with rationale.
// No magic numbers in firmware code.

#pragma once

#include <stdint.h>

// --- Sensor noise ---
// Sensor deltas below this do not count as motion and do not reset the idle timer.
// Without this, sensor jitter prevents the device from ever entering idle.
#define NOISE_THRESHOLD_OPTICAL     2    // counts (PAW3204/ADNS-2080)
#define NOISE_THRESHOLD_HALL        5    // counts (DRV5053, TBD after calibration)

// --- Main loop ---
// Sensor poll rate. Matches the minimum BLE connection interval (7.5ms rounds
// up to 15ms on most hosts) and keeps CPU load proportional to HID report rate.
#define SENSOR_POLL_INTERVAL_MS     15

// Repeated sensor read failures should degrade motion input instead of
// immediately rebooting the device out from under the user. 50 polls at 15ms
// is about 750ms of continuous failure before falling back to click-only mode.
#define SENSOR_READ_FAIL_DEGRADE_THRESHOLD 50

// Once motion input is unavailable, attempt sensor wake/reinit at a human
// timescale instead of hammering the bus on every 15ms main-loop iteration.
#define SENSOR_RECOVERY_ATTEMPT_MS 1000

// If HID notifications fail continuously for this long, restart to recover the
// BLE stack instead of silently staying connected but unusable.
#define BLE_HID_SEND_RESTART_MS    5000

// --- State machine timing ---
// Duration of no motion before transitioning from ACTIVE to IDLE.
// At this point, connection interval reverts from 7.5ms to 15ms.
#define IDLE_TRANSITION_MS          250

// Duration of no activity (no motion above noise threshold, no click)
// before entering deep sleep from IDLE.
#define SLEEP_TIMEOUT_MS            45000  // 45 seconds

// Maximum time to advertise before giving up and entering deep sleep.
// Prevents battery drain from indefinite advertising.
#define RECONNECT_TIMEOUT_MS        60000  // 60 seconds

// --- Battery ---
// LiPo cells suffer permanent damage below ~3.0V. Cut off above the knee.
#define LOW_VOLTAGE_CUTOFF_MV       3200

// How often to sample VBAT ADC during connected states.
// NVS writes for logging are deferred to idle periods.
#define BATTERY_CHECK_INTERVAL_MS   60000  // 60 seconds

// --- Click dead zone ---
// Pressing the finger down causes micro-movement of the sensor.
// Dead zone suppresses X/Y deltas while the dome is actuated.
// Dead zone exit requires BOTH conditions to be met. These are the persisted
// first-boot defaults; runtime values may be changed over BLE and stored in NVS.
#define DEAD_ZONE_TIME_MS           50   // minimum hold time before exit
#define DEAD_ZONE_TIME_MS_MAX       2000 // BLE-configurable upper bound
#define DEAD_ZONE_DISTANCE          10   // minimum accumulated counts before exit
#define DEAD_ZONE_DISTANCE_MAX      255  // BLE-configurable upper bound
#define DEAD_ZONE_DISTANCE_PRO      15   // Pro: account for LRA vibration contribution (A6 analysis)

// Additional suppression during haptic pulse (Pro tier, piezo+LRA click)
// 35ms covers Sharp Click waveform 17 (~20ms) + mechanical decay margin (A6 analysis)
#define HAPTIC_SUPPRESS_MS          35

// Click debounce (dome switch bounce is typically 1-5ms)
#define CLICK_DEBOUNCE_MS           5

// --- Calibration ---
// Boot-time calibration samples sensor at rest to establish zero offset.
#define CALIBRATION_SAMPLE_COUNT    50     // number of readings
#define CALIBRATION_SAMPLE_PERIOD_MS 2     // ms between readings (~100ms total)

// If standard deviation of sensor readings during calibration exceeds this,
// the device was moving during boot — retry calibration.
#define CALIBRATION_MOTION_THRESHOLD 5     // stddev in counts

// Delay between calibration retries
#define CALIBRATION_RETRY_DELAY_MS  500

// Maximum calibration attempts before using zero offset
#define CALIBRATION_MAX_RETRIES     3

// --- Runtime settings ---
// DPI multiplier uses 0.1x units so the ring can scale deltas without float.
#define DPI_MULTIPLIER_DEFAULT      10   // 1.0x native sensor scale
#define DPI_MULTIPLIER_MIN          1    // 0.1x
#define DPI_MULTIPLIER_MAX          255  // 25.5x

// Flash erase/commit can take tens to hundreds of milliseconds, so settings
// writes stay in RAM until the ring leaves the active HID path.
#define SETTINGS_FLUSH_RETRY_MS     1000
