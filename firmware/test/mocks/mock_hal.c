// SPDX-License-Identifier: MIT
// PowerFinger — Mock HAL implementation with failure injection

#include "mock_hal.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_adc.h"
#include "hal_sleep.h"
#include "hal_storage.h"
#include "hal_spi.h"
#include "hal_ble.h"
#include "hal_ota.h"

static uint32_t s_time_ms = 0;
static uint32_t s_adc_mv = 3700;  // default: healthy battery
static hal_status_t s_adc_status = HAL_OK;
static hal_status_t s_ble_send_status = HAL_OK;
static int s_ble_send_fail_count = 0;
static hal_status_t s_ble_conn_param_status = HAL_OK;
static int s_ble_conn_param_request_count = 0;
static uint16_t s_ble_conn_param_min = 0;
static uint16_t s_ble_conn_param_max = 0;

void mock_hal_reset(void)
{
    s_time_ms = 0;
    s_adc_mv = 3700;
    s_adc_status = HAL_OK;
    s_ble_send_status = HAL_OK;
    s_ble_send_fail_count = 0;
    s_ble_conn_param_status = HAL_OK;
    s_ble_conn_param_request_count = 0;
    s_ble_conn_param_min = 0;
    s_ble_conn_param_max = 0;
}

void mock_hal_set_time_ms(uint32_t ms) { s_time_ms = ms; }
void mock_hal_advance_time_ms(uint32_t ms) { s_time_ms += ms; }
void mock_hal_set_adc_mv(uint32_t mv) { s_adc_mv = mv; }
void mock_hal_set_adc_status(hal_status_t status) { s_adc_status = status; }

void mock_hal_inject_ble_send_failure(hal_status_t status, int count)
{
    s_ble_send_status = status;
    s_ble_send_fail_count = count;
}

void mock_hal_set_ble_conn_param_status(hal_status_t status)
{
    s_ble_conn_param_status = status;
}

int mock_hal_get_ble_conn_param_request_count(void)
{
    return s_ble_conn_param_request_count;
}

void mock_hal_get_last_ble_conn_param_request(uint16_t *min_1_25ms,
                                              uint16_t *max_1_25ms)
{
    if (min_1_25ms) *min_1_25ms = s_ble_conn_param_min;
    if (max_1_25ms) *max_1_25ms = s_ble_conn_param_max;
}

// --- hal_timer ---
uint32_t hal_timer_get_ms(void) { return s_time_ms; }
hal_status_t hal_timer_delay_ms(uint32_t ms) { s_time_ms += ms; return HAL_OK; }
hal_status_t hal_timer_create(const char *n, uint32_t p, bool per, hal_callback_t cb, void *a, hal_timer_handle_t *h) { (void)n;(void)p;(void)per;(void)cb;(void)a;(void)h; return HAL_OK; }
hal_status_t hal_timer_start(hal_timer_handle_t h) { (void)h; return HAL_OK; }
hal_status_t hal_timer_stop(hal_timer_handle_t h) { (void)h; return HAL_OK; }
hal_status_t hal_timer_set_period(hal_timer_handle_t h, uint32_t p) { (void)h;(void)p; return HAL_OK; }
hal_status_t hal_timer_delete(hal_timer_handle_t h) { (void)h; return HAL_OK; }

// --- hal_gpio ---
hal_status_t hal_gpio_init(hal_pin_t p, hal_gpio_mode_t m) { (void)p;(void)m; return HAL_OK; }
hal_status_t hal_gpio_set(hal_pin_t p, bool l) { (void)p;(void)l; return HAL_OK; }
bool hal_gpio_get(hal_pin_t p) { (void)p; return false; }
hal_status_t hal_gpio_set_interrupt(hal_pin_t p, hal_gpio_intr_t e, hal_isr_callback_t cb, void *a) { (void)p;(void)e;(void)cb;(void)a; return HAL_OK; }

// --- hal_adc (configurable for battery tests) ---
hal_status_t hal_adc_init(hal_adc_channel_t ch) { (void)ch; return HAL_OK; }
hal_status_t hal_adc_read_mv(hal_adc_channel_t ch, uint32_t *mv) {
    (void)ch;
    if (s_adc_status != HAL_OK) return s_adc_status;
    if (mv) *mv = s_adc_mv;
    return HAL_OK;
}
hal_status_t hal_adc_deinit(hal_adc_channel_t ch) { (void)ch; return HAL_OK; }

// --- hal_sleep ---
hal_status_t hal_sleep_enter(hal_sleep_mode_t m) { (void)m; return HAL_OK; }
hal_status_t hal_sleep_configure_wake_gpio(hal_pin_t p, bool l) { (void)p;(void)l; return HAL_OK; }
hal_status_t hal_sleep_configure_wake_timer(uint32_t us) { (void)us; return HAL_OK; }

// --- hal_storage ---
hal_status_t hal_storage_init(void) { return HAL_OK; }
hal_status_t hal_storage_set(const char *k, const void *d, size_t l) { (void)k;(void)d;(void)l; return HAL_OK; }
hal_status_t hal_storage_get(const char *k, void *d, size_t *l) { (void)k;(void)d;(void)l; return HAL_ERR_NOT_FOUND; }
hal_status_t hal_storage_delete(const char *k) { (void)k; return HAL_OK; }
hal_status_t hal_storage_commit(void) { return HAL_OK; }

// --- hal_spi ---
hal_status_t hal_spi_init(const hal_spi_config_t *c, hal_spi_handle_t *h) { (void)c;(void)h; return HAL_OK; }
hal_status_t hal_spi_transfer(hal_spi_handle_t h, const uint8_t *tx, uint8_t *rx, size_t l) { (void)h;(void)tx;(void)rx;(void)l; return HAL_OK; }
hal_status_t hal_spi_write_reg(hal_spi_handle_t h, uint8_t a, uint8_t v) { (void)h;(void)a;(void)v; return HAL_OK; }
hal_status_t hal_spi_read_reg(hal_spi_handle_t h, uint8_t a, uint8_t *v) { (void)h;(void)a; if(v) *v=0; return HAL_OK; }
hal_status_t hal_spi_deinit(hal_spi_handle_t h) { (void)h; return HAL_OK; }

// --- hal_ble (with failure injection) ---
hal_status_t hal_ble_init(const char *n, hal_ble_event_cb_t cb, void *a) { (void)n;(void)cb;(void)a; return HAL_OK; }
hal_status_t hal_ble_start_advertising(uint32_t t) { (void)t; return HAL_OK; }
hal_status_t hal_ble_stop_advertising(void) { return HAL_OK; }
hal_status_t hal_ble_send_mouse_report(const hal_hid_mouse_report_t *r) {
    (void)r;
    if (s_ble_send_fail_count > 0) {
        s_ble_send_fail_count--;
        return s_ble_send_status;
    }
    return HAL_OK;
}
hal_status_t hal_ble_request_conn_params(uint16_t mn, uint16_t mx) {
    s_ble_conn_param_request_count++;
    s_ble_conn_param_min = mn;
    s_ble_conn_param_max = mx;
    return s_ble_conn_param_status;
}
hal_status_t hal_ble_delete_all_bonds(void) { return HAL_OK; }
void hal_ble_set_battery_level(uint8_t level_percent) { (void)level_percent; }
bool hal_ble_is_connected(void) { return false; }

// --- hal_ota ---
hal_status_t hal_ota_begin(hal_ota_handle_t *h) { (void)h; return HAL_OK; }
hal_status_t hal_ota_write(hal_ota_handle_t h, const void *d, size_t l) { (void)h;(void)d;(void)l; return HAL_OK; }
hal_status_t hal_ota_finish(hal_ota_handle_t h) { (void)h; return HAL_OK; }
void hal_ota_reboot(void) {}
hal_status_t hal_ota_confirm(void) { return HAL_OK; }
void hal_ota_rollback(void) {}
