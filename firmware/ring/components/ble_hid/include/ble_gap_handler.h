// SPDX-License-Identifier: MIT
// PowerFinger — BLE GAP event handler interface

#pragma once

#include "hal_ble.h"

// Request active (low-latency) connection parameters.
// Called when sensor detects motion.
hal_status_t ble_gap_request_active_params(void);

// Request idle (power-saving) connection parameters.
// Called after idle timeout with no motion.
hal_status_t ble_gap_request_idle_params(void);
