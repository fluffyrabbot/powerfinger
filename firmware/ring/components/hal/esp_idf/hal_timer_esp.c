// SPDX-License-Identifier: MIT
// PowerFinger HAL — Timer implementation for ESP-IDF

#include "hal_timer.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <stdlib.h>

static const char *TAG = "hal_timer";

struct hal_timer_ctx {
    esp_timer_handle_t handle;
    uint32_t period_ms;
    bool periodic;
};

hal_status_t hal_timer_create(const char *name, uint32_t period_ms, bool periodic,
                              hal_callback_t callback, void *arg,
                              hal_timer_handle_t *out_handle)
{
    if (!callback || !out_handle) return HAL_ERR_INVALID_ARG;

    struct hal_timer_ctx *ctx = calloc(1, sizeof(struct hal_timer_ctx));
    if (!ctx) return HAL_ERR_NO_MEM;

    ctx->period_ms = period_ms;
    ctx->periodic = periodic;

    esp_timer_create_args_t timer_args = {
        .callback = callback,
        .arg = arg,
        .dispatch_method = ESP_TIMER_TASK,
        .name = name,
    };

    esp_err_t ret = esp_timer_create(&timer_args, &ctx->handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed: %s", esp_err_to_name(ret));
        free(ctx);
        return HAL_ERR_IO;
    }

    *out_handle = (hal_timer_handle_t)ctx;
    return HAL_OK;
}

hal_status_t hal_timer_start(hal_timer_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;
    struct hal_timer_ctx *ctx = (struct hal_timer_ctx *)handle;

    uint64_t period_us = (uint64_t)ctx->period_ms * 1000;
    esp_err_t ret;

    if (ctx->periodic) {
        ret = esp_timer_start_periodic(ctx->handle, period_us);
    } else {
        ret = esp_timer_start_once(ctx->handle, period_us);
    }

    return (ret == ESP_OK) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_timer_stop(hal_timer_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;
    struct hal_timer_ctx *ctx = (struct hal_timer_ctx *)handle;

    esp_err_t ret = esp_timer_stop(ctx->handle);
    // ESP_ERR_INVALID_STATE means timer was already stopped — that's fine
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return HAL_ERR_IO;
    return HAL_OK;
}

hal_status_t hal_timer_set_period(hal_timer_handle_t handle, uint32_t period_ms)
{
    if (!handle) return HAL_ERR_INVALID_ARG;
    struct hal_timer_ctx *ctx = (struct hal_timer_ctx *)handle;
    ctx->period_ms = period_ms;

    // If timer is running, restart with new period
    if (esp_timer_is_active(ctx->handle)) {
        hal_timer_stop(handle);
        return hal_timer_start(handle);
    }
    return HAL_OK;
}

hal_status_t hal_timer_delete(hal_timer_handle_t handle)
{
    if (!handle) return HAL_ERR_INVALID_ARG;
    struct hal_timer_ctx *ctx = (struct hal_timer_ctx *)handle;

    esp_timer_stop(ctx->handle);
    esp_timer_delete(ctx->handle);
    free(ctx);
    return HAL_OK;
}

uint32_t hal_timer_get_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

hal_status_t hal_timer_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
    return HAL_OK;
}
