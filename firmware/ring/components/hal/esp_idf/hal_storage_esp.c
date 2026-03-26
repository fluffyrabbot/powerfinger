// SPDX-License-Identifier: MIT
// PowerFinger HAL — Storage implementation for ESP-IDF (NVS)

#include "hal_storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "hal_storage";
static const char *NVS_NAMESPACE = "powerfinger";
static nvs_handle_t s_nvs_handle = 0;
static bool s_opened = false;

hal_status_t hal_storage_init(void)
{
    if (s_opened) return HAL_OK;

    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &s_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    s_opened = true;
    return HAL_OK;
}

hal_status_t hal_storage_set(const char *key, const void *data, size_t len)
{
    if (!s_opened || !key || !data) return HAL_ERR_INVALID_ARG;

    esp_err_t ret = nvs_set_blob(s_nvs_handle, key, data, len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob '%s' failed: %s", key, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}

hal_status_t hal_storage_get(const char *key, void *data, size_t *len)
{
    if (!s_opened || !key || !len) return HAL_ERR_INVALID_ARG;

    esp_err_t ret = nvs_get_blob(s_nvs_handle, key, data, len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) return HAL_ERR_NOT_FOUND;
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob '%s' failed: %s", key, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}

hal_status_t hal_storage_delete(const char *key)
{
    if (!s_opened || !key) return HAL_ERR_INVALID_ARG;

    esp_err_t ret = nvs_erase_key(s_nvs_handle, key);
    if (ret == ESP_ERR_NVS_NOT_FOUND) return HAL_ERR_NOT_FOUND;
    if (ret != ESP_OK) return HAL_ERR_IO;
    return HAL_OK;
}

hal_status_t hal_storage_commit(void)
{
    if (!s_opened) return HAL_ERR_INVALID_ARG;

    esp_err_t ret = nvs_commit(s_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}
