// SPDX-License-Identifier: MIT
// PowerFinger HAL — OTA implementation for ESP-IDF

#include "hal_ota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "hal_ota";

hal_status_t hal_ota_begin(hal_ota_handle_t *out_handle)
{
    if (!out_handle) return HAL_ERR_INVALID_ARG;

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "no OTA update partition found");
        return HAL_ERR_NOT_FOUND;
    }

    esp_ota_handle_t *ota_handle = malloc(sizeof(esp_ota_handle_t));
    if (!ota_handle) return HAL_ERR_NO_MEM;

    esp_err_t ret = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(ret));
        free(ota_handle);
        return HAL_ERR_IO;
    }

    *out_handle = (hal_ota_handle_t)ota_handle;
    return HAL_OK;
}

hal_status_t hal_ota_write(hal_ota_handle_t handle, const void *data, size_t len)
{
    if (!handle || !data) return HAL_ERR_INVALID_ARG;

    esp_ota_handle_t *ota_handle = (esp_ota_handle_t *)handle;
    esp_err_t ret = esp_ota_write(*ota_handle, data, len);
    return (ret == ESP_OK) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_ota_finish(hal_ota_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;

    esp_ota_handle_t *ota_handle = (esp_ota_handle_t *)handle;
    esp_err_t ret = esp_ota_end(*ota_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(ret));
        free(ota_handle);
        return HAL_ERR_IO;
    }

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    ret = esp_ota_set_boot_partition(update_partition);
    free(ota_handle);

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
