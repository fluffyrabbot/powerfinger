// SPDX-License-Identifier: MIT
// PowerFinger HAL — OTA implementation for ESP-IDF

#include "hal_ota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_log.h"

#include <stdlib.h>

static const char *TAG = "hal_ota";

// Context stores both the ESP OTA handle and the partition pointer,
// so hal_ota_finish doesn't need to re-query the partition table.
typedef struct {
    esp_ota_handle_t handle;
    const esp_partition_t *partition;
} ota_ctx_t;

hal_status_t hal_ota_begin(hal_ota_handle_t *out_handle)
{
    if (!out_handle) return HAL_ERR_INVALID_ARG;

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "no OTA update partition found");
        return HAL_ERR_NOT_FOUND;
    }

    ota_ctx_t *ctx = calloc(1, sizeof(ota_ctx_t));
    if (!ctx) return HAL_ERR_NO_MEM;

    ctx->partition = update_partition;

    esp_err_t ret = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ctx->handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(ret));
        free(ctx);
        return HAL_ERR_IO;
    }

    *out_handle = (hal_ota_handle_t)ctx;
    return HAL_OK;
}

hal_status_t hal_ota_write(hal_ota_handle_t handle, const void *data, size_t len)
{
    if (!handle || !data) return HAL_ERR_INVALID_ARG;

    ota_ctx_t *ctx = (ota_ctx_t *)handle;
    esp_err_t ret = esp_ota_write(ctx->handle, data, len);
    return (ret == ESP_OK) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_ota_finish(hal_ota_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;

    ota_ctx_t *ctx = (ota_ctx_t *)handle;
    esp_err_t ret = esp_ota_end(ctx->handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(ret));
        free(ctx);
        return HAL_ERR_IO;
    }

    // Use the same partition from begin(), not re-queried
    ret = esp_ota_set_boot_partition(ctx->partition);
    free(ctx);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    return HAL_OK;
}

void hal_ota_reboot(void)
{
    ESP_LOGI(TAG, "rebooting into new firmware");
    esp_restart();
}

hal_status_t hal_ota_confirm(void)
{
    esp_err_t ret = esp_ota_mark_app_valid_cancel_rollback();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_mark_app_valid failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    ESP_LOGI(TAG, "OTA firmware confirmed");
    return HAL_OK;
}

void hal_ota_rollback(void)
{
    ESP_LOGW(TAG, "rolling back to previous firmware");
    esp_ota_mark_app_invalid_rollback_and_reboot();
}
