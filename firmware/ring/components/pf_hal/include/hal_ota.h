// SPDX-License-Identifier: MIT
// PowerFinger HAL — Over-the-air update interface
//
// A/B partition scheme with mandatory boot confirmation.
// Wraps esp_ota_ops on ESP-IDF, MCUboot on Zephyr.

#pragma once

#include "hal_types.h"

// Opaque OTA session handle — distinct from other handle types
typedef struct hal_ota_ctx *hal_ota_handle_t;

// Begin an OTA update. Erases the inactive partition.
hal_status_t hal_ota_begin(hal_ota_handle_t *out_handle);

// Write a chunk of firmware data to the inactive partition.
hal_status_t hal_ota_write(hal_ota_handle_t handle, const void *data, size_t len);

// Finalize the OTA update. Validates checksum, sets inactive partition
// as boot target, but does NOT reboot.
hal_status_t hal_ota_finish(hal_ota_handle_t handle);

// Reboot into the newly written partition.
// After reboot, hal_ota_confirm() must be called within 30 seconds
// or the bootloader will roll back to the previous partition.
void hal_ota_reboot(void);

// Confirm that the current firmware is working. Must be called after
// every OTA reboot. If not called within the watchdog timeout, the
// bootloader rolls back.
hal_status_t hal_ota_confirm(void);

// Explicitly roll back to the previous partition and reboot.
void hal_ota_rollback(void);
