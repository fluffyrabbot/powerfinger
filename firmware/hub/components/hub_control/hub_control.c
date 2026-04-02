// SPDX-License-Identifier: MIT
// PowerFinger Hub — Control helpers

#include "hub_control.h"

#include "ble_central.h"
#include "event_composer.h"

hal_status_t hub_control_set_role(const uint8_t mac[6], ring_role_t role)
{
    if (!mac || role >= ROLE_COUNT) {
        return HAL_ERR_INVALID_ARG;
    }

    hal_status_t rc = role_engine_set_role(mac, role);
    if (rc != HAL_OK) {
        return rc;
    }

    uint8_t ring_index = 0;
    rc = ble_central_find_ring_index_by_mac(mac, &ring_index);
    if (rc == HAL_OK) {
        event_composer_set_role(ring_index, role);
        return HAL_OK;
    }

    // A persisted role can be updated even when the ring is not currently
    // connected; in that case there is no live cache to synchronize.
    if (rc == HAL_ERR_NOT_FOUND) {
        return HAL_OK;
    }

    return rc;
}
