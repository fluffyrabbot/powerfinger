// SPDX-License-Identifier: MIT
// PowerFinger HAL — Sleep implementation for ESP-IDF

#include "hal_sleep.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_log.h"

static const char *TAG = "hal_sleep";
static const gpio_int_type_t GPIO_WAKE_HIGH = GPIO_INTR_HIGH_LEVEL;
static const gpio_int_type_t GPIO_WAKE_LOW = GPIO_INTR_LOW_LEVEL;

static hal_status_t configure_wake_pin(hal_pin_t pin, bool level)
{
    gpio_int_type_t intr_type = level ? GPIO_WAKE_HIGH : GPIO_WAKE_LOW;

    if (!GPIO_IS_VALID_GPIO((gpio_num_t)pin)) {
        return HAL_ERR_INVALID_ARG;
    }

    esp_err_t ret = gpio_wakeup_enable((gpio_num_t)pin, intr_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_wakeup_enable failed for pin %lu: %s",
                 (unsigned long)pin, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

#if SOC_GPIO_SUPPORT_HP_PERIPH_PD_SLEEP_WAKEUP
    ret = gpio_wakeup_enable_on_hp_periph_powerdown_sleep((gpio_num_t)pin, intr_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_wakeup_enable_on_hp_periph_powerdown_sleep failed for pin %lu: %s",
                 (unsigned long)pin, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
#endif

    return HAL_OK;
}

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
    return hal_sleep_configure_wake_gpio_mask((1ULL << pin), level);
}

hal_status_t hal_sleep_configure_wake_gpio_mask(uint64_t pin_mask, bool level)
{
    if (pin_mask == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    for (uint8_t pin = 0; pin < 64; ++pin) {
        if ((pin_mask & (1ULL << pin)) == 0) {
            continue;
        }

        hal_status_t pin_rc = configure_wake_pin((hal_pin_t)pin, level);
        if (pin_rc != HAL_OK) {
            return pin_rc;
        }
    }

    esp_err_t ret = esp_sleep_enable_gpio_wakeup();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "configure wake GPIO mask 0x%llx failed: %s",
                 (unsigned long long)pin_mask, esp_err_to_name(ret));
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

hal_wake_cause_t hal_sleep_get_wake_cause(void)
{
    uint32_t causes = esp_sleep_get_wakeup_causes();
    const uint32_t gpio_causes = (1U << ESP_SLEEP_WAKEUP_GPIO) |
                                 (1U << ESP_SLEEP_WAKEUP_EXT0) |
                                 (1U << ESP_SLEEP_WAKEUP_EXT1);

    if (causes & (1U << ESP_SLEEP_WAKEUP_UNDEFINED)) {
        return HAL_WAKE_CAUSE_COLD_BOOT;
    }
    // Preserve user-triggered wake classification when a timer also fires.
    if (causes & gpio_causes) {
        return HAL_WAKE_CAUSE_GPIO;
    }
    if (causes & (1U << ESP_SLEEP_WAKEUP_TIMER)) {
        return HAL_WAKE_CAUSE_TIMER;
    }
    return HAL_WAKE_CAUSE_OTHER;
}
