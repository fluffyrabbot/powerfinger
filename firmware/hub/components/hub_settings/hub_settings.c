// SPDX-License-Identifier: MIT
// PowerFinger Hub — Persisted hub settings implementation

#include "hub_settings.h"

#include "hal_storage.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
static const char *TAG = "hub_settings";
static SemaphoreHandle_t s_mutex = NULL;
#define LOCK()   if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY)
#define UNLOCK() if (s_mutex) xSemaphoreGive(s_mutex)
#else
#define LOCK()   (void)0
#define UNLOCK() (void)0
#endif

#include <string.h>

#define HUB_SETTINGS_NVS_KEY      "hub_settings"
#define HUB_SETTINGS_NVS_VERSION  1U
#define HUB_SETTINGS_BLOB_LEN     4U

#ifdef ESP_PLATFORM
#define HUB_SETTINGS_FLUSH_TASK_STACK_BYTES 3072
#define HUB_SETTINGS_FLUSH_TASK_PRIORITY    1
#define HUB_SETTINGS_FLUSH_RETRY_DELAY_MS   250
#endif

typedef struct {
    uint8_t version;
    uint8_t usb_poll_ms;
    uint8_t scan_policy;
    uint8_t expected_rings;
} hub_settings_blob_t;

typedef enum {
    HUB_SETTINGS_FLUSH_NONE,
    HUB_SETTINGS_FLUSH_OK,
    HUB_SETTINGS_FLUSH_RETRY,
} hub_settings_flush_result_t;

static hub_settings_snapshot_t s_settings = {
    .usb_poll_ms = HUB_SETTINGS_USB_POLL_MS_DEFAULT,
    .scan_policy = HUB_SETTINGS_SCAN_POLICY_CONTINUOUS,
    .expected_rings = HUB_SETTINGS_EXPECTED_RINGS_DEFAULT,
};
static bool s_dirty = false;
static hub_settings_blob_t s_pending_blob = {
    .version = HUB_SETTINGS_NVS_VERSION,
    .usb_poll_ms = HUB_SETTINGS_USB_POLL_MS_DEFAULT,
    .scan_policy = HUB_SETTINGS_SCAN_POLICY_CONTINUOUS,
    .expected_rings = HUB_SETTINGS_EXPECTED_RINGS_DEFAULT,
};

#ifdef ESP_PLATFORM
static TaskHandle_t s_flush_task = NULL;
#endif

bool hub_settings_usb_poll_ms_supported(uint8_t usb_poll_ms)
{
    return usb_poll_ms == 1U ||
           usb_poll_ms == 2U ||
           usb_poll_ms == 4U ||
           usb_poll_ms == 8U;
}

bool hub_settings_scan_policy_supported(uint8_t scan_policy)
{
    return scan_policy == HUB_SETTINGS_SCAN_POLICY_BOOT_ONLY ||
           scan_policy == HUB_SETTINGS_SCAN_POLICY_CONTINUOUS ||
           scan_policy == HUB_SETTINGS_SCAN_POLICY_EXPECTED;
}

bool hub_settings_expected_rings_supported(uint8_t expected_rings)
{
    return expected_rings >= 1U && expected_rings <= HUB_MAX_RINGS;
}

static hub_settings_snapshot_t default_settings(void)
{
    return (hub_settings_snapshot_t) {
        .usb_poll_ms = HUB_SETTINGS_USB_POLL_MS_DEFAULT,
        .scan_policy = HUB_SETTINGS_SCAN_POLICY_CONTINUOUS,
        .expected_rings = HUB_SETTINGS_EXPECTED_RINGS_DEFAULT,
    };
}

static bool snapshot_valid(const hub_settings_snapshot_t *snapshot)
{
    return snapshot &&
           hub_settings_usb_poll_ms_supported(snapshot->usb_poll_ms) &&
           hub_settings_scan_policy_supported(snapshot->scan_policy) &&
           hub_settings_expected_rings_supported(snapshot->expected_rings);
}

static hub_settings_blob_t snapshot_to_blob(const hub_settings_snapshot_t *snapshot)
{
    hub_settings_blob_t blob = {
        .version = HUB_SETTINGS_NVS_VERSION,
        .usb_poll_ms = HUB_SETTINGS_USB_POLL_MS_DEFAULT,
        .scan_policy = HUB_SETTINGS_SCAN_POLICY_CONTINUOUS,
        .expected_rings = HUB_SETTINGS_EXPECTED_RINGS_DEFAULT,
    };

    if (snapshot_valid(snapshot)) {
        blob.usb_poll_ms = snapshot->usb_poll_ms;
        blob.scan_policy = snapshot->scan_policy;
        blob.expected_rings = snapshot->expected_rings;
    }

    return blob;
}

static bool decode_blob(const uint8_t *buf,
                        size_t len,
                        hub_settings_snapshot_t *snapshot_out)
{
    if (!buf || !snapshot_out || len != HUB_SETTINGS_BLOB_LEN) {
        return false;
    }

    hub_settings_blob_t blob = {
        .version = buf[0],
        .usb_poll_ms = buf[1],
        .scan_policy = buf[2],
        .expected_rings = buf[3],
    };

    hub_settings_snapshot_t snapshot = {
        .usb_poll_ms = blob.usb_poll_ms,
        .scan_policy = blob.scan_policy,
        .expected_rings = blob.expected_rings,
    };

    if (blob.version != HUB_SETTINGS_NVS_VERSION || !snapshot_valid(&snapshot)) {
        return false;
    }

    *snapshot_out = snapshot;
    return true;
}

static bool flush_to_nvs(const hub_settings_blob_t *blob)
{
    if (!blob) {
        return false;
    }

    uint8_t raw[HUB_SETTINGS_BLOB_LEN] = {
        blob->version,
        blob->usb_poll_ms,
        blob->scan_policy,
        blob->expected_rings,
    };

    hal_status_t rc = hal_storage_set(HUB_SETTINGS_NVS_KEY, raw, sizeof(raw));
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS hub-settings write failed: %d", (int)rc);
#endif
        return false;
    }

    rc = hal_storage_commit();
    if (rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NVS hub-settings commit failed: %d", (int)rc);
#endif
        return false;
    }

    return true;
}

static void signal_flush_task(void)
{
#ifdef ESP_PLATFORM
    if (s_flush_task) {
        xTaskNotifyGive(s_flush_task);
    }
#endif
}

static hub_settings_flush_result_t flush_pending_once(void)
{
    bool need_flush = false;
    hub_settings_blob_t blob = {0};

    LOCK();
    if (s_dirty) {
        blob = s_pending_blob;
        s_dirty = false;
        need_flush = true;
    }
    UNLOCK();

    if (!need_flush) {
        return HUB_SETTINGS_FLUSH_NONE;
    }

    if (!flush_to_nvs(&blob)) {
        LOCK();
        s_dirty = true;
        UNLOCK();
        return HUB_SETTINGS_FLUSH_RETRY;
    }

    return HUB_SETTINGS_FLUSH_OK;
}

#ifdef ESP_PLATFORM
static void hub_settings_flush_task(void *arg)
{
    (void)arg;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) {
            hub_settings_flush_result_t result = flush_pending_once();
            if (result == HUB_SETTINGS_FLUSH_NONE) {
                break;
            }
            if (result == HUB_SETTINGS_FLUSH_RETRY) {
                vTaskDelay(pdMS_TO_TICKS(HUB_SETTINGS_FLUSH_RETRY_DELAY_MS));
            }
        }
    }
}
#endif

static hal_status_t update_settings_locked(const hub_settings_snapshot_t *next_snapshot)
{
    if (!snapshot_valid(next_snapshot)) {
        return HAL_ERR_INVALID_ARG;
    }

    if (memcmp(&s_settings, next_snapshot, sizeof(*next_snapshot)) == 0) {
        return HAL_OK;
    }

    s_settings = *next_snapshot;
    s_pending_blob = snapshot_to_blob(next_snapshot);
    s_dirty = true;
    return HAL_OK;
}

hal_status_t hub_settings_init(void)
{
#ifdef ESP_PLATFORM
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
        if (!s_mutex) {
            return HAL_ERR_NO_MEM;
        }
    }
#endif

    s_settings = default_settings();
    s_pending_blob = snapshot_to_blob(&s_settings);
    s_dirty = false;

    hal_status_t storage_rc = hal_storage_init();
    if (storage_rc != HAL_OK) {
        return storage_rc;
    }

    uint8_t raw_blob[HUB_SETTINGS_BLOB_LEN] = {0};
    size_t len = sizeof(raw_blob);
    hub_settings_snapshot_t loaded = default_settings();
    if (hal_storage_get(HUB_SETTINGS_NVS_KEY, raw_blob, &len) == HAL_OK) {
        if (decode_blob(raw_blob, len, &loaded)) {
            s_settings = loaded;
#ifdef ESP_PLATFORM
            ESP_LOGI(TAG, "loaded hub settings from NVS: poll=%u scan=%u expected=%u",
                     loaded.usb_poll_ms,
                     loaded.scan_policy,
                     loaded.expected_rings);
#endif
        } else {
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "hub settings blob invalid, using defaults");
#endif
        }
    }

    s_pending_blob = snapshot_to_blob(&s_settings);

#ifdef ESP_PLATFORM
    if (!s_flush_task) {
        BaseType_t task_rc = xTaskCreate(hub_settings_flush_task,
                                         "hub_settings_flush",
                                         HUB_SETTINGS_FLUSH_TASK_STACK_BYTES / sizeof(StackType_t),
                                         NULL,
                                         HUB_SETTINGS_FLUSH_TASK_PRIORITY,
                                         &s_flush_task);
        if (task_rc != pdPASS) {
            s_flush_task = NULL;
            return HAL_ERR_NO_MEM;
        }
    }
#endif

    return HAL_OK;
}

void hub_settings_get(hub_settings_snapshot_t *snapshot_out)
{
    if (!snapshot_out) {
        return;
    }

    LOCK();
    *snapshot_out = s_settings;
    UNLOCK();
}

uint8_t hub_settings_get_usb_poll_ms(void)
{
    LOCK();
    uint8_t usb_poll_ms = s_settings.usb_poll_ms;
    UNLOCK();
    return usb_poll_ms;
}

uint8_t hub_settings_get_scan_policy(void)
{
    LOCK();
    uint8_t scan_policy = s_settings.scan_policy;
    UNLOCK();
    return scan_policy;
}

uint8_t hub_settings_get_expected_rings(void)
{
    LOCK();
    uint8_t expected_rings = s_settings.expected_rings;
    UNLOCK();
    return expected_rings;
}

hal_status_t hub_settings_set_usb_poll_ms(uint8_t usb_poll_ms)
{
    hub_settings_snapshot_t next = {0};

    LOCK();
    next = s_settings;
    next.usb_poll_ms = usb_poll_ms;
    hal_status_t rc = update_settings_locked(&next);
    UNLOCK();

    if (rc == HAL_OK) {
        signal_flush_task();
    }
    return rc;
}

hal_status_t hub_settings_set_scan_policy(uint8_t scan_policy)
{
    hub_settings_snapshot_t next = {0};

    LOCK();
    next = s_settings;
    next.scan_policy = scan_policy;
    hal_status_t rc = update_settings_locked(&next);
    UNLOCK();

    if (rc == HAL_OK) {
        signal_flush_task();
    }
    return rc;
}

hal_status_t hub_settings_set_expected_rings(uint8_t expected_rings)
{
    hub_settings_snapshot_t next = {0};

    LOCK();
    next = s_settings;
    next.expected_rings = expected_rings;
    hal_status_t rc = update_settings_locked(&next);
    UNLOCK();

    if (rc == HAL_OK) {
        signal_flush_task();
    }
    return rc;
}

void hub_settings_flush_if_dirty(void)
{
    while (flush_pending_once() == HUB_SETTINGS_FLUSH_RETRY) {
#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(HUB_SETTINGS_FLUSH_RETRY_DELAY_MS));
#endif
    }
}
