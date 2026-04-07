// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB CDC companion transport

#include "companion_cdc.h"

#ifdef ESP_PLATFORM

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "tinyusb_cdc_acm.h"

static const char *TAG = "companion_cdc";

#define COMPANION_CDC_PORT TINYUSB_CDC_ACM_0
#define COMPANION_CDC_QUEUE_DEPTH 8
#define COMPANION_CDC_TASK_STACK_SIZE 4096
#define COMPANION_CDC_TASK_PRIORITY 4

typedef struct {
    size_t len;
    uint8_t data[CONFIG_TINYUSB_CDC_RX_BUFSIZE];
} companion_cdc_rx_message_t;

static QueueHandle_t s_rx_queue = NULL;
static TaskHandle_t s_task_handle = NULL;
static companion_session_t s_session;
static uint8_t s_rx_scratch[CONFIG_TINYUSB_CDC_RX_BUFSIZE];
static uint32_t s_rx_drop_count = 0;

static hal_status_t companion_cdc_emit_response(const char *response, void *arg)
{
    (void)arg;

    if (!response) {
        return HAL_ERR_INVALID_ARG;
    }

    size_t response_len = strlen(response);
    if (response_len == 0) {
        return HAL_OK;
    }

    size_t queued = tinyusb_cdcacm_write_queue(COMPANION_CDC_PORT,
                                               (const uint8_t *)response,
                                               response_len);
    if (queued != response_len) {
        ESP_LOGW(TAG, "CDC TX queue accepted %u of %u bytes",
                 (unsigned int)queued,
                 (unsigned int)response_len);
        return HAL_ERR_IO;
    }

    esp_err_t flush_rc = tinyusb_cdcacm_write_flush(COMPANION_CDC_PORT, 0);
    if (flush_rc == ESP_OK || flush_rc == ESP_ERR_NOT_FINISHED) {
        return HAL_OK;
    }

    ESP_LOGW(TAG, "CDC TX flush failed: %s", esp_err_to_name(flush_rc));
    return HAL_ERR_IO;
}

static void companion_cdc_task(void *arg)
{
    (void)arg;

    companion_cdc_rx_message_t message = {0};
    while (1) {
        if (xQueueReceive(s_rx_queue, &message, portMAX_DELAY) != pdPASS) {
            continue;
        }

        hal_status_t rc = companion_session_feed_bytes(&s_session,
                                                       message.data,
                                                       message.len);
        if (rc != HAL_OK) {
            ESP_LOGW(TAG, "CDC command processing error: %d", rc);
        }
    }
}

static void companion_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    (void)event;

    if (itf != COMPANION_CDC_PORT || s_rx_queue == NULL) {
        return;
    }

    size_t rx_size = 0;
    esp_err_t read_rc = tinyusb_cdcacm_read(COMPANION_CDC_PORT,
                                            s_rx_scratch,
                                            sizeof(s_rx_scratch),
                                            &rx_size);
    if (read_rc != ESP_OK) {
        ESP_LOGW(TAG, "CDC RX read failed: %s", esp_err_to_name(read_rc));
        return;
    }
    if (rx_size == 0) {
        return;
    }

    companion_cdc_rx_message_t message = {
        .len = rx_size,
    };
    memcpy(message.data, s_rx_scratch, rx_size);

    if (xQueueSend(s_rx_queue, &message, 0) != pdPASS) {
        s_rx_drop_count++;
        if ((s_rx_drop_count % 16U) == 1U) {
            ESP_LOGW(TAG, "CDC RX queue full, dropped %lu chunk(s)",
                     (unsigned long)s_rx_drop_count);
        }
    }
}

hal_status_t companion_cdc_start(const companion_cdc_config_t *config)
{
    if (!config || !config->fill_hub_info) {
        return HAL_ERR_INVALID_ARG;
    }

    if (s_task_handle != NULL) {
        return HAL_OK;
    }

    hal_status_t session_rc = companion_session_init(&s_session,
                                                     config->fill_hub_info,
                                                     companion_cdc_emit_response,
                                                     config->cb_arg);
    if (session_rc != HAL_OK) {
        return session_rc;
    }

    s_rx_queue = xQueueCreate(COMPANION_CDC_QUEUE_DEPTH,
                              sizeof(companion_cdc_rx_message_t));
    if (s_rx_queue == NULL) {
        return HAL_ERR_NO_MEM;
    }

    tinyusb_config_cdcacm_t cdc_cfg = {
        .cdc_port = COMPANION_CDC_PORT,
        .callback_rx = companion_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL,
    };

    esp_err_t cdc_rc = tinyusb_cdcacm_init(&cdc_cfg);
    if (cdc_rc != ESP_OK) {
        vQueueDelete(s_rx_queue);
        s_rx_queue = NULL;
        ESP_LOGE(TAG, "CDC ACM init failed: %s", esp_err_to_name(cdc_rc));
        return HAL_ERR_IO;
    }

    BaseType_t task_ok = xTaskCreate(companion_cdc_task,
                                     "pf_companion_cdc",
                                     COMPANION_CDC_TASK_STACK_SIZE,
                                     NULL,
                                     COMPANION_CDC_TASK_PRIORITY,
                                     &s_task_handle);
    if (task_ok != pdPASS) {
        ESP_LOGE(TAG, "CDC task creation failed");
        tinyusb_cdcacm_deinit(COMPANION_CDC_PORT);
        vQueueDelete(s_rx_queue);
        s_rx_queue = NULL;
        return HAL_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "USB CDC companion transport ready");
    return HAL_OK;
}

#else

hal_status_t companion_cdc_start(const companion_cdc_config_t *config)
{
    (void)config;
    return HAL_ERR_NOT_SUPPORTED;
}

#endif
