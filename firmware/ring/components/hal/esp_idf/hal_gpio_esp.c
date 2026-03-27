// SPDX-License-Identifier: MIT
// PowerFinger HAL — GPIO implementation for ESP-IDF

#include "hal_gpio.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"

static const char *TAG = "hal_gpio";

hal_status_t hal_gpio_init(hal_pin_t pin, hal_gpio_mode_t mode)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << pin),
        .intr_type = GPIO_INTR_DISABLE,
    };

    switch (mode) {
    case HAL_GPIO_INPUT:
        cfg.mode = GPIO_MODE_INPUT;
        cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case HAL_GPIO_OUTPUT:
        cfg.mode = GPIO_MODE_OUTPUT;
        cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case HAL_GPIO_INPUT_PULLUP:
        cfg.mode = GPIO_MODE_INPUT;
        cfg.pull_up_en = GPIO_PULLUP_ENABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        break;
    case HAL_GPIO_INPUT_PULLDOWN:
        cfg.mode = GPIO_MODE_INPUT;
        cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
        break;
    default:
        return HAL_ERR_INVALID_ARG;
    }

    esp_err_t ret = gpio_config(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config failed for pin %lu: %s", (unsigned long)pin, esp_err_to_name(ret));
        return HAL_ERR_IO;
    }
    return HAL_OK;
}

hal_status_t hal_gpio_set(hal_pin_t pin, bool level)
{
    esp_err_t ret = gpio_set_level((gpio_num_t)pin, level ? 1 : 0);
    return (ret == ESP_OK) ? HAL_OK : HAL_ERR_IO;
}

bool hal_gpio_get(hal_pin_t pin)
{
    return gpio_get_level((gpio_num_t)pin) != 0;
}

static bool s_isr_service_installed = false;

hal_status_t hal_gpio_set_interrupt(hal_pin_t pin, hal_gpio_intr_t edge,
                                    hal_isr_callback_t cb, void *arg)
{
    // Install ISR service on first use
    if (!s_isr_service_installed) {
        esp_err_t ret = gpio_install_isr_service(0);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "gpio_install_isr_service failed: %s", esp_err_to_name(ret));
            return HAL_ERR_IO;
        }
        s_isr_service_installed = true;
    }

    gpio_int_type_t itype;
    switch (edge) {
    case HAL_GPIO_INTR_NONE:     itype = GPIO_INTR_DISABLE;    break;
    case HAL_GPIO_INTR_RISING:   itype = GPIO_INTR_POSEDGE;    break;
    case HAL_GPIO_INTR_FALLING:  itype = GPIO_INTR_NEGEDGE;    break;
    case HAL_GPIO_INTR_ANY_EDGE: itype = GPIO_INTR_ANYEDGE;    break;
    default: return HAL_ERR_INVALID_ARG;
    }

    esp_err_t ret = gpio_set_intr_type((gpio_num_t)pin, itype);
    if (ret != ESP_OK) return HAL_ERR_IO;

    if (edge == HAL_GPIO_INTR_NONE) {
        gpio_isr_handler_remove((gpio_num_t)pin);
    } else {
        ret = gpio_isr_handler_add((gpio_num_t)pin, cb, arg);
        if (ret != ESP_OK) return HAL_ERR_IO;
    }

    return HAL_OK;
}

// hal_gpio_enable_wake removed — wake source config is in hal_sleep.h
