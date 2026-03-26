// SPDX-License-Identifier: MIT
// PowerFinger — BLE GAP connection parameter management

#include "ble_gap_handler.h"
#include "ble_config.h"
#include "hal_ble.h"

hal_status_t ble_gap_request_active_params(void)
{
    return hal_ble_request_conn_params(
        BLE_CONN_ITVL_MIN_ACTIVE,
        BLE_CONN_ITVL_MAX_ACTIVE
    );
}

hal_status_t ble_gap_request_idle_params(void)
{
    return hal_ble_request_conn_params(
        BLE_CONN_ITVL_MIN_DEFAULT,
        BLE_CONN_ITVL_MAX_DEFAULT
    );
}
