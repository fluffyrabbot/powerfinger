// SPDX-License-Identifier: MIT
// PowerFinger Hub — Role engine implementation
//
// Maps each connected ring's BLE MAC address to a role.
// Default assignment: first ring = CURSOR, second = SCROLL.
// Reassignment is atomic (mutex-protected).
// Roles persist to NVS across power cycles.
//
// NVS writes happen outside the mutex to avoid blocking the NimBLE task.
// On ESP targets, a low-priority background task performs the flash commit so
// the hub's 1ms USB HID loop never absorbs the ~20-200ms erase stall.

#include "role_engine.h"
#include "ble_central.h"
#include "hal_storage.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
static const char *TAG = "role_engine";
static SemaphoreHandle_t s_mutex = NULL;
#define LOCK()   if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY)
#define UNLOCK() if (s_mutex) xSemaphoreGive(s_mutex)
#else
#define LOCK()   (void)0
#define UNLOCK() (void)0
#endif

#include <string.h>

// NVS blob format: version byte + array of role_engine_entry_t
#define ROLE_NVS_KEY     "roles"
#define ROLE_NVS_VERSION 1

#ifdef ESP_PLATFORM
// Background NVS flush task. Low priority is intentional: USB HID delivery and
// NimBLE should preempt this worker, because role persistence is important but
// not latency-critical once the in-memory assignment exists.
#define ROLE_FLUSH_TASK_STACK_BYTES 3072
#define ROLE_FLUSH_TASK_PRIORITY    1
#define ROLE_FLUSH_RETRY_DELAY_MS   250
#endif

// Role assignment entry
// NVS blob layout
typedef struct {
    uint8_t version;
    uint8_t count;
    role_engine_entry_t entries[HUB_MAX_RINGS];
} role_blob_t;

static role_engine_entry_t s_entries[HUB_MAX_RINGS];
static int s_entry_count = 0;
static bool s_dirty = false;          // deferred NVS write pending
static role_blob_t s_pending_blob;    // snapshot to write when s_dirty is consumed

#ifdef ESP_PLATFORM
static TaskHandle_t s_flush_task = NULL;
#endif

static const char *s_role_names[] = {
    [ROLE_CURSOR]   = "CURSOR",
    [ROLE_SCROLL]   = "SCROLL",
    [ROLE_MODIFIER] = "MODIFIER",
};

typedef enum {
    ROLE_FLUSH_NONE,
    ROLE_FLUSH_OK,
    ROLE_FLUSH_RETRY,
} role_flush_result_t;

static void signal_flush_task(void)
{
#ifdef ESP_PLATFORM
    if (s_flush_task) {
        xTaskNotifyGive(s_flush_task);
    }
#endif
}

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
    size_t len = sizeof(uint8_t) * 2 +
                 sizeof(role_engine_entry_t) * snapshot->count;
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
    memcpy(blob.entries, s_entries, sizeof(role_engine_entry_t) * count);
    return blob;
}

static void stage_pending_blob_locked(int count)
{
    s_pending_blob = snapshot_entries_locked(count);
    s_dirty = true;
}

static int find_entry_index_locked(const uint8_t mac[6])
{
    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            return i;
        }
    }
    return -1;
}

static role_flush_result_t flush_pending_once(void)
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

    if (!need_flush) {
        return ROLE_FLUSH_NONE;
    }

    if (!flush_to_nvs(&snapshot)) {
        // M8: re-set dirty so the next flush attempt retries.
        // Without this, a transient NVS failure permanently loses the role
        // assignment — it works in RAM but never persists.
        LOCK();
        s_dirty = true;
        UNLOCK();
        return ROLE_FLUSH_RETRY;
    }

    return ROLE_FLUSH_OK;
}

#ifdef ESP_PLATFORM
static void role_flush_task(void *arg)
{
    (void)arg;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) {
            role_flush_result_t result = flush_pending_once();
            if (result == ROLE_FLUSH_NONE) {
                break;
            }
            if (result == ROLE_FLUSH_RETRY) {
                vTaskDelay(pdMS_TO_TICKS(ROLE_FLUSH_RETRY_DELAY_MS));
            }
        }
    }
}
#endif

hal_status_t role_engine_init(void)
{
#ifdef ESP_PLATFORM
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
        if (!s_mutex) return HAL_ERR_NO_MEM;
    }
#endif

    // Reset process-local state so host tests and warm re-inits do not inherit
    // stale in-memory assignments or dirty flags from a prior session.
    memset(s_entries, 0, sizeof(s_entries));
    s_entry_count = 0;
    s_dirty = false;
    memset(&s_pending_blob, 0, sizeof(s_pending_blob));

    hal_storage_init();

    // Try to load saved roles from NVS
    role_blob_t blob;
    size_t len = sizeof(blob);
    if (hal_storage_get(ROLE_NVS_KEY, &blob, &len) == HAL_OK) {
        if (len >= 2 && blob.version == ROLE_NVS_VERSION && blob.count <= HUB_MAX_RINGS) {
            // Validate each entry and deduplicate by MAC, keeping the last
            // entry encountered so the most recently written assignment wins.
            for (int i = 0; i < blob.count; i++) {
                if (blob.entries[i].role >= ROLE_COUNT) {
                    continue;
                }

                int existing = -1;
                for (int j = 0; j < s_entry_count; j++) {
                    if (memcmp(s_entries[j].mac, blob.entries[i].mac, sizeof(blob.entries[i].mac)) == 0) {
                        existing = j;
                        break;
                    }
                }

                if (existing >= 0) {
                    s_entries[existing] = blob.entries[i];
#ifdef ESP_PLATFORM
                    ESP_LOGW(TAG, "duplicate MAC in NVS role blob, keeping last entry");
#endif
                } else {
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

#ifdef ESP_PLATFORM
    if (!s_flush_task) {
        BaseType_t task_rc = xTaskCreate(
            role_flush_task,
            "role_flush",
            ROLE_FLUSH_TASK_STACK_BYTES,
            NULL,
            ROLE_FLUSH_TASK_PRIORITY,
            &s_flush_task
        );
        if (task_rc != pdPASS) {
            s_flush_task = NULL;
            return HAL_ERR_NO_MEM;
        }
    }
#endif

    return HAL_OK;
}

ring_role_t role_engine_get_role(const uint8_t mac[6])
{
    ring_role_t result;
    bool marked_dirty = false;

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
        // The background flush worker persists it without blocking the caller.
        stage_pending_blob_locked(s_entry_count);
        marked_dirty = true;
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "assigned %s to new ring (index %d)", s_role_names[result], idx);
#endif
    } else {
        result = ROLE_CURSOR;  // fallback: all slots full
    }

    UNLOCK();

    if (marked_dirty) {
        signal_flush_task();
    }

    return result;
}

hal_status_t role_engine_get_all(role_engine_entry_t *entries_out,
                                 size_t max_entries,
                                 size_t *count_out)
{
    if (!count_out) {
        return HAL_ERR_INVALID_ARG;
    }

    LOCK();

    size_t entry_count = (size_t)s_entry_count;
    *count_out = entry_count;

    if (!entries_out) {
        if (max_entries != 0) {
            UNLOCK();
            return HAL_ERR_INVALID_ARG;
        }
    } else if (max_entries < entry_count) {
        UNLOCK();
        return HAL_ERR_INVALID_ARG;
    }

    if (entries_out && entry_count > 0) {
        memcpy(entries_out, s_entries, sizeof(s_entries[0]) * entry_count);
    }

    UNLOCK();
    return HAL_OK;
}

hal_status_t role_engine_set_role(const uint8_t mac[6], ring_role_t role)
{
    if (role >= ROLE_COUNT) return HAL_ERR_INVALID_ARG;

    bool found = false;
    bool marked_dirty = false;

    LOCK();

    int idx = find_entry_index_locked(mac);
    if (idx >= 0) {
        s_entries[idx].role = role;
        found = true;
        stage_pending_blob_locked(s_entry_count);
        marked_dirty = true;
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "reassigned ring %d to %s", idx, s_role_names[role]);
#endif
    }

    UNLOCK();

    if (marked_dirty) {
        signal_flush_task();
    }

    return found ? HAL_OK : HAL_ERR_NOT_FOUND;
}

hal_status_t role_engine_swap(const uint8_t mac_a[6], const uint8_t mac_b[6])
{
    if (!mac_a || !mac_b || memcmp(mac_a, mac_b, 6) == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    bool swapped = false;
    bool marked_dirty = false;

    LOCK();

    int idx_a = find_entry_index_locked(mac_a);
    int idx_b = find_entry_index_locked(mac_b);
    if (idx_a >= 0 && idx_b >= 0) {
        swapped = true;
        if (s_entries[idx_a].role != s_entries[idx_b].role) {
            ring_role_t tmp = s_entries[idx_a].role;
            s_entries[idx_a].role = s_entries[idx_b].role;
            s_entries[idx_b].role = tmp;
            stage_pending_blob_locked(s_entry_count);
            marked_dirty = true;
        }
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "swapped roles for rings %d and %d", idx_a, idx_b);
#endif
    }

    UNLOCK();

    if (marked_dirty) {
        signal_flush_task();
    }

    return swapped ? HAL_OK : HAL_ERR_NOT_FOUND;
}

hal_status_t role_engine_forget(const uint8_t mac[6])
{
    bool found = false;
    bool marked_dirty = false;

    LOCK();

    for (int i = 0; i < s_entry_count; i++) {
        if (memcmp(s_entries[i].mac, mac, 6) == 0) {
            // Shift remaining entries down
            for (int j = i; j < s_entry_count - 1; j++) {
                s_entries[j] = s_entries[j + 1];
            }
            s_entry_count--;
            found = true;
            stage_pending_blob_locked(s_entry_count);
            marked_dirty = true;
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "forgot ring at index %d, %d entries remain", i, s_entry_count);
#endif
            break;
        }
    }

    UNLOCK();

    if (marked_dirty) {
        signal_flush_task();
    }

    return found ? HAL_OK : HAL_ERR_NOT_FOUND;
}

const char *role_engine_role_name(ring_role_t role)
{
    if (role < ROLE_COUNT) return s_role_names[role];
    return "UNKNOWN";
}

void role_engine_flush_if_dirty(void)
{
    (void)flush_pending_once();
}
