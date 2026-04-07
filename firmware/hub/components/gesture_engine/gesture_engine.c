// SPDX-License-Identifier: MIT
// PowerFinger Hub — Gesture mapping implementation
//
// Persists the hub-owned gesture table in NVS so companion configuration
// survives reboots and active multi-ring behavior can be updated atomically.

#include "gesture_engine.h"

#include "hal_storage.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
static const char *TAG = "gesture_engine";
static SemaphoreHandle_t s_mutex = NULL;
#define LOCK()   if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY)
#define UNLOCK() if (s_mutex) xSemaphoreGive(s_mutex)
#else
#define LOCK()   (void)0
#define UNLOCK() (void)0
#endif

#include <string.h>

#define GESTURE_NVS_KEY         "gestures"
#define GESTURE_NVS_VERSION     1U
#define GESTURE_BLOB_HEADER_LEN 2U
#define GESTURE_BLOB_ENTRY_LEN  2U
#define GESTURE_BLOB_MAX_LEN    (GESTURE_BLOB_HEADER_LEN + \
                                 (GESTURE_BLOB_ENTRY_LEN * GESTURE_ENGINE_MAX_ENTRIES))

#ifdef ESP_PLATFORM
#define GESTURE_FLUSH_TASK_STACK_BYTES 3072
#define GESTURE_FLUSH_TASK_PRIORITY    1
#define GESTURE_FLUSH_RETRY_DELAY_MS   250
#endif

typedef struct {
    uint8_t version;
    uint8_t count;
    gesture_engine_entry_t entries[GESTURE_ENGINE_MAX_ENTRIES];
} gesture_blob_t;

static gesture_engine_entry_t s_entries[GESTURE_ENGINE_MAX_ENTRIES];
static int s_entry_count = 0;
static bool s_dirty = false;
static gesture_blob_t s_pending_blob;

#ifdef ESP_PLATFORM
static TaskHandle_t s_flush_task = NULL;
#endif

typedef enum {
    GESTURE_FLUSH_NONE,
    GESTURE_FLUSH_OK,
    GESTURE_FLUSH_RETRY,
} gesture_flush_result_t;

static void signal_flush_task(void)
{
#ifdef ESP_PLATFORM
    if (s_flush_task) {
        xTaskNotifyGive(s_flush_task);
    }
#endif
}

bool gesture_engine_trigger_known(uint8_t trigger)
{
    return trigger >= GESTURE_TRIGGER_CURSOR_SCROLL_CLICK &&
           trigger <= GESTURE_TRIGGER_SCROLL_DOUBLE_CLICK;
}

bool gesture_engine_trigger_supported(uint8_t trigger)
{
    return trigger >= GESTURE_TRIGGER_CURSOR_SCROLL_CLICK &&
           trigger <= GESTURE_TRIGGER_ALL_THREE_CLICK;
}

bool gesture_engine_action_known(uint8_t action)
{
    return action <= GESTURE_ACTION_DPI_CYCLE;
}

bool gesture_engine_action_supported(uint8_t action)
{
    return action <= GESTURE_ACTION_FORWARD;
}

const char *gesture_engine_trigger_name(uint8_t trigger)
{
    switch (trigger) {
    case GESTURE_TRIGGER_CURSOR_SCROLL_CLICK:
        return "cursor+scroll";
    case GESTURE_TRIGGER_CURSOR_MODIFIER_CLICK:
        return "cursor+modifier";
    case GESTURE_TRIGGER_SCROLL_MODIFIER_CLICK:
        return "scroll+modifier";
    case GESTURE_TRIGGER_ALL_THREE_CLICK:
        return "all_three";
    case GESTURE_TRIGGER_CURSOR_DOUBLE_CLICK:
        return "double_click_cursor";
    case GESTURE_TRIGGER_SCROLL_DOUBLE_CLICK:
        return "double_click_scroll";
    default:
        return "unknown_trigger";
    }
}

const char *gesture_engine_action_name(uint8_t action)
{
    switch (action) {
    case GESTURE_ACTION_NONE:
        return "disabled";
    case GESTURE_ACTION_MIDDLE_CLICK:
        return "middle_click";
    case GESTURE_ACTION_BACK:
        return "back";
    case GESTURE_ACTION_FORWARD:
        return "forward";
    case GESTURE_ACTION_SCROLL_LOCK_TOGGLE:
        return "toggle_scroll_lock";
    case GESTURE_ACTION_DPI_CYCLE:
        return "dpi_cycle";
    default:
        return "unknown_action";
    }
}

static size_t encode_gesture_blob(const gesture_blob_t *snapshot,
                                  uint8_t *buf,
                                  size_t buf_len)
{
    if (!snapshot || !buf || snapshot->count > GESTURE_ENGINE_MAX_ENTRIES) {
        return 0;
    }

    size_t required_len = GESTURE_BLOB_HEADER_LEN +
                          ((size_t)snapshot->count * GESTURE_BLOB_ENTRY_LEN);
    if (buf_len < required_len) {
        return 0;
    }

    buf[0] = snapshot->version;
    buf[1] = snapshot->count;

    size_t offset = GESTURE_BLOB_HEADER_LEN;
    for (size_t i = 0; i < snapshot->count; i++) {
        buf[offset] = snapshot->entries[i].trigger;
        buf[offset + 1] = snapshot->entries[i].action;
        offset += GESTURE_BLOB_ENTRY_LEN;
    }

    return required_len;
}

static bool decode_gesture_blob(const uint8_t *buf,
                                size_t len,
                                gesture_blob_t *blob_out)
{
    if (!buf || !blob_out || len < GESTURE_BLOB_HEADER_LEN) {
        return false;
    }

    memset(blob_out, 0, sizeof(*blob_out));
    blob_out->version = buf[0];
    blob_out->count = buf[1];

    if (blob_out->version != GESTURE_NVS_VERSION ||
        blob_out->count > GESTURE_ENGINE_MAX_ENTRIES) {
        return false;
    }

    size_t required_len = GESTURE_BLOB_HEADER_LEN +
                          ((size_t)blob_out->count * GESTURE_BLOB_ENTRY_LEN);
    if (len != required_len) {
        return false;
    }

    size_t offset = GESTURE_BLOB_HEADER_LEN;
    for (size_t i = 0; i < blob_out->count; i++) {
        blob_out->entries[i].trigger = buf[offset];
        blob_out->entries[i].action = buf[offset + 1];
        offset += GESTURE_BLOB_ENTRY_LEN;
    }

    return true;
}

static bool flush_to_nvs(const gesture_blob_t *snapshot)
{
    uint8_t buf[GESTURE_BLOB_MAX_LEN] = {0};
    size_t len = encode_gesture_blob(snapshot, buf, sizeof(buf));
    if (len == 0) {
        return false;
    }

    hal_status_t rc = hal_storage_set(GESTURE_NVS_KEY, buf, len);
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS gesture write failed: %d — gestures not persisted", (int)rc);
#endif
        return false;
    }
    rc = hal_storage_commit();
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS gesture commit failed: %d — gestures may not persist", (int)rc);
#endif
        return false;
    }
    return true;
}

static gesture_blob_t snapshot_entries_locked(int count)
{
    gesture_blob_t blob = {0};
    blob.version = GESTURE_NVS_VERSION;
    blob.count = (uint8_t)count;
    memcpy(blob.entries, s_entries, sizeof(gesture_engine_entry_t) * (size_t)count);
    return blob;
}

static void stage_pending_blob_locked(int count)
{
    s_pending_blob = snapshot_entries_locked(count);
    s_dirty = true;
}

static int find_entry_index_locked(uint8_t trigger)
{
    for (int i = 0; i < s_entry_count; i++) {
        if (s_entries[i].trigger == trigger) {
            return i;
        }
    }
    return -1;
}

static int insertion_index_for_trigger_locked(uint8_t trigger)
{
    int insert_at = s_entry_count;
    for (int i = 0; i < s_entry_count; i++) {
        if (s_entries[i].trigger > trigger) {
            insert_at = i;
            break;
        }
    }
    return insert_at;
}

static gesture_flush_result_t flush_pending_once(void)
{
    bool need_flush = false;
    gesture_blob_t snapshot;

    LOCK();
    if (s_dirty) {
        snapshot = s_pending_blob;
        s_dirty = false;
        need_flush = true;
    }
    UNLOCK();

    if (!need_flush) {
        return GESTURE_FLUSH_NONE;
    }

    if (!flush_to_nvs(&snapshot)) {
        LOCK();
        s_dirty = true;
        UNLOCK();
        return GESTURE_FLUSH_RETRY;
    }

    return GESTURE_FLUSH_OK;
}

#ifdef ESP_PLATFORM
static void gesture_flush_task(void *arg)
{
    (void)arg;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) {
            gesture_flush_result_t result = flush_pending_once();
            if (result == GESTURE_FLUSH_NONE) {
                break;
            }
            if (result == GESTURE_FLUSH_RETRY) {
                vTaskDelay(pdMS_TO_TICKS(GESTURE_FLUSH_RETRY_DELAY_MS));
            }
        }
    }
}
#endif

hal_status_t gesture_engine_init(void)
{
#ifdef ESP_PLATFORM
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
        if (!s_mutex) {
            return HAL_ERR_NO_MEM;
        }
    }
#endif

    memset(s_entries, 0, sizeof(s_entries));
    s_entry_count = 0;
    s_dirty = false;
    memset(&s_pending_blob, 0, sizeof(s_pending_blob));

    hal_status_t storage_rc = hal_storage_init();
    if (storage_rc != HAL_OK) {
        return storage_rc;
    }

    uint8_t raw_blob[GESTURE_BLOB_MAX_LEN] = {0};
    size_t len = sizeof(raw_blob);
    gesture_blob_t blob;
    if (hal_storage_get(GESTURE_NVS_KEY, raw_blob, &len) == HAL_OK) {
        if (decode_gesture_blob(raw_blob, len, &blob)) {
            for (int i = 0; i < blob.count; i++) {
                uint8_t trigger = blob.entries[i].trigger;
                uint8_t action = blob.entries[i].action;
                if (!gesture_engine_trigger_known(trigger) ||
                    !gesture_engine_action_known(action) ||
                    !gesture_engine_trigger_supported(trigger) ||
                    !gesture_engine_action_supported(action)) {
                    continue;
                }

                int existing = find_entry_index_locked(trigger);
                if (existing >= 0) {
                    s_entries[existing] = blob.entries[i];
                } else if (s_entry_count < (int)GESTURE_ENGINE_MAX_ENTRIES) {
                    s_entries[s_entry_count++] = blob.entries[i];
                }
            }
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "loaded %d gesture mappings from NVS (v%d)",
                     s_entry_count, blob.version);
#endif
        } else {
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "NVS gesture data version mismatch or corrupt, resetting");
#endif
        }
    } else {
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "no saved gestures, using defaults");
#endif
    }

#ifdef ESP_PLATFORM
    if (!s_flush_task) {
        BaseType_t task_rc = xTaskCreate(gesture_flush_task,
                                         "gesture_flush",
                                         GESTURE_FLUSH_TASK_STACK_BYTES / sizeof(StackType_t),
                                         NULL,
                                         GESTURE_FLUSH_TASK_PRIORITY,
                                         &s_flush_task);
        if (task_rc != pdPASS) {
            s_flush_task = NULL;
            return HAL_ERR_NO_MEM;
        }
    }
#endif

    return HAL_OK;
}

gesture_action_t gesture_engine_get_action(uint8_t trigger)
{
    if (!gesture_engine_trigger_known(trigger)) {
        return GESTURE_ACTION_NONE;
    }

    LOCK();
    for (int i = 0; i < s_entry_count; i++) {
        if (s_entries[i].trigger == trigger) {
            gesture_action_t action = (gesture_action_t)s_entries[i].action;
            UNLOCK();
            return action;
        }
    }
    UNLOCK();

    return GESTURE_ACTION_NONE;
}

hal_status_t gesture_engine_get_all(gesture_engine_entry_t *entries_out,
                                    size_t max_entries,
                                    size_t *count_out)
{
    if (!count_out) {
        return HAL_ERR_INVALID_ARG;
    }

    LOCK();
    size_t count = (size_t)s_entry_count;
    *count_out = count;

    if (!entries_out) {
        UNLOCK();
        return (max_entries == 0) ? HAL_OK : HAL_ERR_INVALID_ARG;
    }
    if (count > max_entries) {
        UNLOCK();
        return HAL_ERR_INVALID_ARG;
    }

    memcpy(entries_out, s_entries, sizeof(gesture_engine_entry_t) * count);
    UNLOCK();
    return HAL_OK;
}

hal_status_t gesture_engine_set_action(uint8_t trigger, gesture_action_t action)
{
    if (!gesture_engine_trigger_known(trigger) || !gesture_engine_action_known((uint8_t)action)) {
        return HAL_ERR_INVALID_ARG;
    }
    if (!gesture_engine_trigger_supported(trigger) ||
        !gesture_engine_action_supported((uint8_t)action)) {
        return HAL_ERR_NOT_SUPPORTED;
    }

    LOCK();

    int idx = find_entry_index_locked(trigger);
    if (action == GESTURE_ACTION_NONE) {
        if (idx >= 0) {
            memmove(&s_entries[idx],
                    &s_entries[idx + 1],
                    sizeof(s_entries[0]) * (size_t)(s_entry_count - idx - 1));
            s_entry_count--;
            memset(&s_entries[s_entry_count], 0, sizeof(s_entries[s_entry_count]));
            stage_pending_blob_locked(s_entry_count);
        }
        UNLOCK();
        signal_flush_task();
        return HAL_OK;
    }

    if (idx >= 0) {
        if (s_entries[idx].action != (uint8_t)action) {
            s_entries[idx].action = (uint8_t)action;
            stage_pending_blob_locked(s_entry_count);
        }
        UNLOCK();
        signal_flush_task();
        return HAL_OK;
    }

    if (s_entry_count >= (int)GESTURE_ENGINE_MAX_ENTRIES) {
        UNLOCK();
        return HAL_ERR_NO_MEM;
    }

    int insert_at = insertion_index_for_trigger_locked(trigger);
    memmove(&s_entries[insert_at + 1],
            &s_entries[insert_at],
            sizeof(s_entries[0]) * (size_t)(s_entry_count - insert_at));
    s_entries[insert_at].trigger = trigger;
    s_entries[insert_at].action = (uint8_t)action;
    s_entry_count++;
    stage_pending_blob_locked(s_entry_count);
    UNLOCK();

    signal_flush_task();
    return HAL_OK;
}

void gesture_engine_flush_if_dirty(void)
{
    while (flush_pending_once() == GESTURE_FLUSH_RETRY) {
#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(GESTURE_FLUSH_RETRY_DELAY_MS));
#endif
    }
}
