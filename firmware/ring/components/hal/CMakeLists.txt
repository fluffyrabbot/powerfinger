# SPDX-License-Identifier: MIT
# PowerFinger — Platform Hardware Abstraction Layer

set(HAL_SRCS
    "esp_idf/hal_gpio_esp.c"
    "esp_idf/hal_spi_esp.c"
    "esp_idf/hal_adc_esp.c"
    "esp_idf/hal_timer_esp.c"
    "esp_idf/hal_sleep_esp.c"
    "esp_idf/hal_storage_esp.c"
    "esp_idf/hal_ble_esp.c"
    "esp_idf/hal_ota_esp.c"
)

idf_component_register(
    SRCS ${HAL_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES driver nvs_flash esp_timer esp_adc esp_pm bt esp_app_format
    PRIV_REQUIRES app_update
)
