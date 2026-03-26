// SPDX-License-Identifier: MIT
// PowerFinger HAL — Sleep implementation for ESP-IDF

#include "hal_sleep.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_log.h"

static const char *TAG = "hal_sleep";

hal_status_t hal_sleep_enter(hal_sleep_mode_t mode)
{
    esp_err_t ret;

    switch (mode) {
    case HAL_SLEEP_LIGHT:
        ret = esp_light_sleep_start();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "light sleep failed: %s", esp_err_to_name(ret));
            return HAL_ERR_IO;
        }
        return HAL_OK;

    case HAL_SLEEP_DEEP:
        ESP_LOGI(TAG, "entering deep sleep");
        esp_deep_sleep_start();
        // Does not return — device reboots on wake
        return HAL_OK;  // unreachable

    default:
        return HAL_ERR_INVALID_ARG;
    }
}

hal_status_t hal_sleep_configure_wake_gpio(hal_pin_t pin, bool level)
{
    esp_err_t ret = esp_deep_sleep_enable_gpio_wakeup(
        (1ULL << pin),
        level ? ESP_GPIO_WAKEUP_GPIO_HIGH : ESP_GPIO_WAKEUP_GPIO_LOW
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "configure wake GPIO %lu failed: %s",
                 (unsigned long)pin, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}

hal_status_t hal_sleep_configure_wake_timer(uint32_t us)
{
    esp_err_t ret = esp_sleep_enable_timer_wakeup((uint64_t)us);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "configure wake timer failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}
