// SPDX-License-Identifier: MIT
// PowerFinger Hub — Role engine implementation
//
// Maps each connected ring's BLE MAC address to a role.
// Default assignment: first ring = CURSOR, second = SCROLL.
// Reassignment is atomic (mutex-protected).
// Roles persist to NVS across power cycles.

#include "role_engine.h"
#include "ble_central.h"
#include "hal_storage.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static const char *TAG = "role_engine";
static SemaphoreHandle_t s_mutex = NULL;
#define LOCK()   if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY)
#define UNLOCK() if (s_mutex) xSemaphoreGive(s_mutex)
#else
#define LOCK()   (void)0
#define UNLOCK() (void)0
#endif

#include <string.h>

// NVS key prefix for role storage
#define ROLE_NVS_KEY "roles"

// Role assignment entry
typedef struct {
    uint8_t mac[6];
    ring_role_t role;
} role_entry_t;

static role_entry_t s_entries[HUB_MAX_RINGS];
static int s_entry_count = 0;

static const char *s_role_names[] = {
    [ROLE_CURSOR]   = "CURSOR",
    [ROLE_SCROLL]   = "SCROLL",
    [ROLE_MODIFIER] = "MODIFIER",
};

// Default role based on assignment order
static ring_role_t default_role_for_index(int index)
{
    switch (index) {
    case 0: return ROLE_CURSOR;
    case 1: return ROLE_SCROLL;
    default: return ROLE_MODIFIER;
    }
}

static void save_to_nvs(void)
{
    hal_storage_set(ROLE_NVS_KEY, s_entries, sizeof(role_entry_t) * s_entry_count);
    hal_storage_commit();
}

hal_status_t role_engine_init(void)
{
#ifdef ESP_PLATFORM
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) return HAL_ERR_NO_MEM;
#endif

    hal_storage_init();

    // Try to load saved roles from NVS
    size_t len = sizeof(s_entries);
    if (hal_storage_get(ROLE_NVS_KEY, s_entries, &len) == HAL_OK) {
        s_entry_count = (int)(len / sizeof(role_entry_t));
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "loaded %d role assignments from NVS", s_entry_count);
#endif
    } else {
        s_entry_count = 0;
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "no saved roles, will use defaults");
#endif
    }

    return HAL_OK;
}

ring_role_t role_engine_get_role(const uint8_t mac[6])
{
    LOCK();

    // Search for existing assignment
    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            ring_role_t role = s_entries[i].role;
            UNLOCK();
            return role;
        }
    }

    // Not found — assign default role and persist
    if (s_entry_count < HUB_MAX_RINGS) {
        int idx = s_entry_count;
        memcpy(s_entries[idx].mac, mac, 6);
        s_entries[idx].role = default_role_for_index(idx);
        s_entry_count++;

        ring_role_t role = s_entries[idx].role;
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "assigned %s to new ring (index %d)", s_role_names[role], idx);
#endif
        save_to_nvs();
        UNLOCK();
        return role;
    }

    UNLOCK();
    return ROLE_CURSOR;  // fallback
}

hal_status_t role_engine_set_role(const uint8_t mac[6], ring_role_t role)
{
    if (role >= ROLE_COUNT) return HAL_ERR_INVALID_ARG;

    LOCK();

    // Find existing entry
    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            s_entries[i].role = role;
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "reassigned ring %d to %s", i, s_role_names[role]);
#endif
            save_to_nvs();
            UNLOCK();
            return HAL_OK;
        }
    }

    UNLOCK();
    return HAL_ERR_NOT_FOUND;
}

const char *role_engine_role_name(ring_role_t role)
{
    if (role < ROLE_COUNT) return s_role_names[role];
    return "UNKNOWN";
}
