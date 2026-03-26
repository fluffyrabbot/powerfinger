// SPDX-License-Identifier: MIT
// PowerFinger — Null sensor driver (Phase 0: no physical sensor)
//
// Returns no motion. Used during BLE HID validation when the
// Phase 0 fake-motion loop generates its own deltas.

#include "sensor_interface.h"
#include <string.h>

hal_status_t sensor_init(void)
{
    return HAL_OK;
}

hal_status_t sensor_read(sensor_reading_t *out)
{
    if (!out) return HAL_ERR_INVALID_ARG;
    memset(out, 0, sizeof(*out));
    return HAL_OK;
}

hal_status_t sensor_power_down(void)
{
    return HAL_OK;
}

hal_status_t sensor_wake(void)
{
    return HAL_OK;
}
