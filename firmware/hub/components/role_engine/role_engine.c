// SPDX-License-Identifier: MIT
// PowerFinger Hub — Role engine implementation
//
// Maps each connected ring's BLE MAC address to a role.
// Default assignment: first ring = CURSOR, second = SCROLL.
// Reassignment is atomic (mutex-protected).
// Roles persist to NVS across power cycles.
//
// NVS writes happen outside the mutex to avoid blocking the NimBLE task
// for up to 200ms during flash sector erase/garbage collection.

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

// NVS blob format: version byte + array of role_entry_t
#define ROLE_NVS_KEY     "roles"
#define ROLE_NVS_VERSION 1

// Role assignment entry
typedef struct {
    uint8_t mac[6];
    ring_role_t role;
} role_entry_t;

// NVS blob layout
typedef struct {
    uint8_t version;
    uint8_t count;
    role_entry_t entries[HUB_MAX_RINGS];
} role_blob_t;

static role_entry_t s_entries[HUB_MAX_RINGS];
static int s_entry_count = 0;
static bool s_dirty = false;          // deferred NVS write pending
static role_blob_t s_pending_blob;    // snapshot to write when s_dirty is consumed

static const char *s_role_names[] = {
    [ROLE_CURSOR]   = "CURSOR",
    [ROLE_SCROLL]   = "SCROLL",
    [ROLE_MODIFIER] = "MODIFIER",
};

// Assign the first unoccupied role in priority order.
// Called while mutex is held, before s_entry_count is incremented, so
// s_entries[0..s_entry_count-1] are the existing (already assigned) entries.
static ring_role_t default_role_for_new_ring(void)
{
    static const ring_role_t priority[] = {
        ROLE_CURSOR, ROLE_SCROLL, ROLE_MODIFIER,
    };
    for (int r = 0; r < (int)(sizeof(priority) / sizeof(priority[0])); r++) {
        bool occupied = false;
        for (int i = 0; i < s_entry_count; i++) {
            if (s_entries[i].role == priority[r]) {
                occupied = true;
                break;
            }
        }
        if (!occupied) return priority[r];
    }
    return ROLE_MODIFIER;  // all named roles occupied
}

// Write to NVS from a pre-captured snapshot (avoids reading s_entries without mutex).
// M8: returns true on success, false on failure (caller should re-set dirty flag).
static bool flush_to_nvs(const role_blob_t *snapshot)
{
    size_t len = sizeof(uint8_t) * 2 + sizeof(role_entry_t) * snapshot->count;
    hal_status_t rc = hal_storage_set(ROLE_NVS_KEY, snapshot, len);
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS role write failed: %d — roles not persisted", (int)rc);
#endif
        return false;
    }
    rc = hal_storage_commit();
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS commit failed: %d — roles may not survive power loss", (int)rc);
#endif
        return false;
    }
    return true;
}

// Capture s_entries into a blob while mutex is held. Caller must hold the lock.
static role_blob_t snapshot_entries_locked(int count)
{
    role_blob_t blob;
    blob.version = ROLE_NVS_VERSION;
    blob.count = (uint8_t)count;
    memcpy(blob.entries, s_entries, sizeof(role_entry_t) * count);
    return blob;
}

hal_status_t role_engine_init(void)
{
#ifdef ESP_PLATFORM
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) return HAL_ERR_NO_MEM;
#endif

    hal_storage_init();

    // Try to load saved roles from NVS
    role_blob_t blob;
    size_t len = sizeof(blob);
    if (hal_storage_get(ROLE_NVS_KEY, &blob, &len) == HAL_OK) {
        if (len >= 2 && blob.version == ROLE_NVS_VERSION && blob.count <= HUB_MAX_RINGS) {
            // Validate each entry
            s_entry_count = 0;
            for (int i = 0; i < blob.count; i++) {
                if (blob.entries[i].role < ROLE_COUNT) {
                    s_entries[s_entry_count++] = blob.entries[i];
                }
            }
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "loaded %d role assignments from NVS (v%d)",
                     s_entry_count, blob.version);
#endif
        } else {
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "NVS role data version mismatch or corrupt, resetting");
#endif
            s_entry_count = 0;
        }
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
    ring_role_t result;

    LOCK();

    // Search for existing assignment
    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            result = s_entries[i].role;
            UNLOCK();
            return result;
        }
    }

    // Not found — assign default role
    if (s_entry_count < HUB_MAX_RINGS) {
        int idx = s_entry_count;
        memcpy(s_entries[idx].mac, mac, 6);
        s_entries[idx].role = default_role_for_new_ring();
        s_entry_count++;
        result = s_entries[idx].role;
        // Capture snapshot and mark dirty while lock is held.
        // Actual NVS write is deferred to role_engine_flush_if_dirty(),
        // called from the main loop — keeps the NimBLE task unblocked.
        s_pending_blob = snapshot_entries_locked(s_entry_count);
        s_dirty = true;
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "assigned %s to new ring (index %d)", s_role_names[result], idx);
#endif
    } else {
        result = ROLE_CURSOR;  // fallback: all slots full
    }

    UNLOCK();

    return result;
}

hal_status_t role_engine_set_role(const uint8_t mac[6], ring_role_t role)
{
    if (role >= ROLE_COUNT) return HAL_ERR_INVALID_ARG;

    bool found = false;

    LOCK();

    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            s_entries[i].role = role;
            found = true;
            s_pending_blob = snapshot_entries_locked(s_entry_count);
            s_dirty = true;
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "reassigned ring %d to %s", i, s_role_names[role]);
#endif
            break;
        }
    }

    UNLOCK();

    return found ? HAL_OK : HAL_ERR_NOT_FOUND;
}

hal_status_t role_engine_forget(const uint8_t mac[6])
{
    bool found = false;

    LOCK();

    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            // Shift remaining entries down
            for (int j = i; j < s_entry_count - 1; j++) {
                s_entries[j] = s_entries[j + 1];
            }
            s_entry_count--;
            found = true;
            s_pending_blob = snapshot_entries_locked(s_entry_count);
            s_dirty = true;
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "forgot ring at index %d, %d entries remain", i, s_entry_count);
#endif
            break;
        }
    }

    UNLOCK();

    return found ? HAL_OK : HAL_ERR_NOT_FOUND;
}

const char *role_engine_role_name(ring_role_t role)
{
    if (role < ROLE_COUNT) return s_role_names[role];
    return "UNKNOWN";
}

void role_engine_flush_if_dirty(void)
{
    bool need_flush = false;
    role_blob_t snapshot;

    LOCK();
    if (s_dirty) {
        snapshot = s_pending_blob;
        s_dirty = false;
        need_flush = true;
    }
    UNLOCK();

    // NVS write outside mutex — blocks ~200ms on flash erase.
    // Must be called from main loop task, never from NimBLE task context.
    if (need_flush) {
        if (!flush_to_nvs(&snapshot)) {
            // M8: re-set dirty so the next flush interval retries.
            // Without this, a transient NVS failure permanently loses
            // the role assignment — it works in RAM but never persists.
            LOCK();
            s_dirty = true;
            // s_pending_blob is still valid (snapshot captured under lock
            // earlier), so we don't need to re-snapshot.
            UNLOCK();
        }
    }
}
