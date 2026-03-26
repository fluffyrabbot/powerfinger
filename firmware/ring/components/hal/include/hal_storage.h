// SPDX-License-Identifier: MIT
// PowerFinger HAL — Non-volatile storage interface
//
// Key-value storage for settings, calibration data, and bonding info.
// Wraps NVS on ESP-IDF, settings subsystem on Zephyr.

#pragma once

#include "hal_types.h"

// Initialize storage subsystem
hal_status_t hal_storage_init(void);

// Store a blob by key. Overwrites if key exists.
hal_status_t hal_storage_set(const char *key, const void *data, size_t len);

// Retrieve a blob by key. On entry, *len is buffer size. On exit, *len is
// actual data size. Returns HAL_ERR_NOT_FOUND if key doesn't exist.
hal_status_t hal_storage_get(const char *key, void *data, size_t *len);

// Delete a key
hal_status_t hal_storage_delete(const char *key);

// Commit pending writes to flash. NVS writes are cheap but flash erase
// can take 20-200ms — call during idle periods, not in hot paths.
hal_status_t hal_storage_commit(void);
