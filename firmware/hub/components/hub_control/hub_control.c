// SPDX-License-Identifier: MIT
// PowerFinger Hub — Control helpers

#include "hub_control.h"

#include "ble_central.h"
#include "event_composer.h"

#include <string.h>

static hal_status_t get_assigned_role(const uint8_t mac[6], ring_role_t *role_out)
{
    if (!mac || !role_out) {
        return HAL_ERR_INVALID_ARG;
    }

    role_engine_entry_t entries[HUB_MAX_RINGS] = {0};
    size_t count = 0;
    hal_status_t rc = role_engine_get_all(entries, HUB_MAX_RINGS, &count);
    if (rc != HAL_OK) {
        return rc;
    }

    for (size_t i = 0; i < count; i++) {
        if (memcmp(entries[i].mac, mac, 6) == 0) {
            *role_out = entries[i].role;
            return HAL_OK;
        }
    }

    return HAL_ERR_NOT_FOUND;
}

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

hal_status_t hub_control_swap_roles(const uint8_t mac_a[6], const uint8_t mac_b[6])
{
    if (!mac_a || !mac_b || memcmp(mac_a, mac_b, 6) == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    ring_role_t role_a = ROLE_CURSOR;
    ring_role_t role_b = ROLE_CURSOR;
    hal_status_t rc = get_assigned_role(mac_a, &role_a);
    if (rc != HAL_OK) {
        return rc;
    }

    rc = get_assigned_role(mac_b, &role_b);
    if (rc != HAL_OK) {
        return rc;
    }

    if (role_a == role_b) {
        return HAL_OK;
    }

    rc = role_engine_swap(mac_a, mac_b);
    if (rc != HAL_OK) {
        return rc;
    }

    uint8_t ring_index_a = 0;
    uint8_t ring_index_b = 0;
    hal_status_t ring_a_rc = ble_central_find_ring_index_by_mac(mac_a, &ring_index_a);
    hal_status_t ring_b_rc = ble_central_find_ring_index_by_mac(mac_b, &ring_index_b);

    if (ring_a_rc == HAL_OK && ring_b_rc == HAL_OK) {
        event_composer_swap_roles(ring_index_a, role_b, ring_index_b, role_a);
        return HAL_OK;
    }

    if (ring_a_rc == HAL_OK) {
        event_composer_set_role(ring_index_a, role_b);
    } else if (ring_a_rc != HAL_ERR_NOT_FOUND) {
        return ring_a_rc;
    }

    if (ring_b_rc == HAL_OK) {
        event_composer_set_role(ring_index_b, role_a);
    } else if (ring_b_rc != HAL_ERR_NOT_FOUND) {
        return ring_b_rc;
    }

    return HAL_OK;
}

hal_status_t hub_control_forget_ring(const uint8_t mac[6])
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

    ring_role_t existing_role = ROLE_CURSOR;
    hal_status_t rc = get_assigned_role(mac, &existing_role);
    if (rc != HAL_OK) {
        return rc;
    }

    uint8_t ring_index = 0;
    rc = ble_central_find_ring_index_by_mac(mac, &ring_index);
    if (rc == HAL_OK) {
        // Drop live contribution immediately so the forget path cannot leave
        // stale input active while the asynchronous BLE disconnect completes.
        event_composer_ring_disconnected(ring_index);

        hal_status_t disconnect_rc = ble_central_disconnect_ring_by_mac(mac);
        if (disconnect_rc != HAL_OK && disconnect_rc != HAL_ERR_NOT_FOUND) {
            return disconnect_rc;
        }
    } else if (rc != HAL_ERR_NOT_FOUND) {
        return rc;
    }

    rc = ble_central_delete_bond_by_mac(mac);
    if (rc != HAL_OK) {
        return rc;
    }

    return role_engine_forget(mac);
}
