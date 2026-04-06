// SPDX-License-Identifier: MIT
// PowerFinger HAL — ADC implementation for ESP-IDF

#include "hal_adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

static const char *TAG = "hal_adc";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static bool s_initialized = false;

hal_status_t hal_adc_init(uint8_t channel)
{
    if (!s_initialized) {
        adc_oneshot_unit_init_cfg_t unit_cfg = {
            .unit_id = ADC_UNIT_1,
        };
        esp_err_t ret = adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "adc_oneshot_new_unit failed: %s", esp_err_to_name(ret));
            return HAL_ERR_IO;
        }
        s_initialized = true;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_oneshot_config_channel(s_adc_handle, (adc_channel_t)channel, &chan_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "adc channel config failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    // Set up calibration if not already done
    if (!s_cali_handle) {
        adc_cali_curve_fitting_config_t cali_cfg = {
            .unit_id = ADC_UNIT_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali_handle);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "ADC calibration not available, raw values will be used");
            s_cali_handle = NULL;
        }
    }

    return HAL_OK;
}

hal_status_t hal_adc_read_mv(uint8_t channel, uint32_t *out_mv)
{
    if (!s_initialized || !out_mv) return HAL_ERR_INVALID_ARG;

    int raw = 0;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, (adc_channel_t)channel, &raw);
    if (ret != ESP_OK) return HAL_ERR_IO;

    if (s_cali_handle) {
        int voltage = 0;
        ret = adc_cali_raw_to_voltage(s_cali_handle, raw, &voltage);
        if (ret == ESP_OK) {
            *out_mv = (uint32_t)voltage;
            return HAL_OK;
        }
    }

    // Fallback: approximate mV from raw (12-bit, ~0-3300mV at 12dB atten)
    *out_mv = (uint32_t)((raw * 3300) / 4095);
    return HAL_OK;
}

hal_status_t hal_adc_deinit(uint8_t channel)
{
    // ESP-IDF ADC oneshot doesn't support per-channel deinit;
    // full cleanup happens when the unit is deleted
    (void)channel;
    return HAL_OK;
}
