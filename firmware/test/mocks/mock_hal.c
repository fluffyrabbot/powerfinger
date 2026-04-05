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
#include "ble_central.h"

#include <string.h>

#define MOCK_STORAGE_MAX_KEY_LEN   32
#define MOCK_STORAGE_MAX_BLOB_LEN  128
#define MOCK_GPIO_MAX_PINS         64

static uint32_t s_time_ms = 0;
static uint32_t s_adc_mv = 3700;  // default: healthy battery
static hal_status_t s_adc_status = HAL_OK;
static uint32_t s_adc_mv_per_channel[8] = {0};
static bool s_adc_channel_configured[8] = {false};
// SPI register mock
static uint8_t s_spi_registers[256] = {0};
static uint8_t s_spi_last_write_addr = 0;
static uint8_t s_spi_last_write_value = 0;
static int s_spi_write_count = 0;
static hal_status_t s_spi_status = HAL_OK;
static int s_spi_fail_count = 0;
static hal_status_t s_ble_send_status = HAL_OK;
static int s_ble_send_fail_count = 0;
static hal_status_t s_ble_conn_param_status = HAL_OK;
static int s_ble_conn_param_request_count = 0;
static uint16_t s_ble_conn_param_min = 0;
static uint16_t s_ble_conn_param_max = 0;
static hal_pin_t s_last_gpio_set_pin = HAL_PIN_NONE;
static bool s_last_gpio_set_level = false;
static int s_gpio_set_count = 0;
static uint64_t s_last_wake_gpio_mask = 0;
static bool s_last_wake_gpio_level = false;
static int s_wake_gpio_config_count = 0;
static hal_sleep_mode_t s_last_sleep_mode = HAL_SLEEP_LIGHT;
static int s_sleep_enter_count = 0;
static bool s_gpio_level[MOCK_GPIO_MAX_PINS] = {0};
static hal_gpio_intr_t s_gpio_intr_type[MOCK_GPIO_MAX_PINS] = {0};
static hal_isr_callback_t s_gpio_isr_callback[MOCK_GPIO_MAX_PINS] = {0};
static void *s_gpio_isr_arg[MOCK_GPIO_MAX_PINS] = {0};
static bool s_storage_committed_present = false;
static char s_storage_committed_key[MOCK_STORAGE_MAX_KEY_LEN] = {0};
static uint8_t s_storage_committed_blob[MOCK_STORAGE_MAX_BLOB_LEN] = {0};
static size_t s_storage_committed_len = 0;
static bool s_storage_pending_present = false;
static bool s_storage_pending_delete = false;
static char s_storage_pending_key[MOCK_STORAGE_MAX_KEY_LEN] = {0};
static uint8_t s_storage_pending_blob[MOCK_STORAGE_MAX_BLOB_LEN] = {0};
static size_t s_storage_pending_len = 0;
static hal_status_t s_storage_set_status = HAL_OK;
static int s_storage_set_fail_count = 0;
static hal_status_t s_storage_commit_status = HAL_OK;
static int s_storage_commit_fail_count = 0;
static bool s_mock_ring_present[HUB_MAX_RINGS] = {0};
static uint8_t s_mock_ring_macs[HUB_MAX_RINGS][6] = {{0}};
static bool s_mock_bond_present[HUB_MAX_RINGS] = {0};
static uint8_t s_mock_bond_macs[HUB_MAX_RINGS][6] = {{0}};

void mock_hal_reset(void)
{
    s_time_ms = 0;
    s_adc_mv = 3700;
    memset(s_adc_mv_per_channel, 0, sizeof(s_adc_mv_per_channel));
    memset(s_adc_channel_configured, 0, sizeof(s_adc_channel_configured));
    memset(s_spi_registers, 0, sizeof(s_spi_registers));
    s_spi_last_write_addr = 0;
    s_spi_last_write_value = 0;
    s_spi_write_count = 0;
    s_spi_status = HAL_OK;
    s_spi_fail_count = 0;
    s_adc_status = HAL_OK;
    s_ble_send_status = HAL_OK;
    s_ble_send_fail_count = 0;
    s_ble_conn_param_status = HAL_OK;
    s_ble_conn_param_request_count = 0;
    s_ble_conn_param_min = 0;
    s_ble_conn_param_max = 0;
    s_last_gpio_set_pin = HAL_PIN_NONE;
    s_last_gpio_set_level = false;
    s_gpio_set_count = 0;
    s_last_wake_gpio_mask = 0;
    s_last_wake_gpio_level = false;
    s_wake_gpio_config_count = 0;
    s_last_sleep_mode = HAL_SLEEP_LIGHT;
    s_sleep_enter_count = 0;
    memset(s_gpio_level, 0, sizeof(s_gpio_level));
    memset(s_gpio_intr_type, 0, sizeof(s_gpio_intr_type));
    memset(s_gpio_isr_callback, 0, sizeof(s_gpio_isr_callback));
    memset(s_gpio_isr_arg, 0, sizeof(s_gpio_isr_arg));
    s_storage_committed_present = false;
    memset(s_storage_committed_key, 0, sizeof(s_storage_committed_key));
    memset(s_storage_committed_blob, 0, sizeof(s_storage_committed_blob));
    s_storage_committed_len = 0;
    s_storage_pending_present = false;
    s_storage_pending_delete = false;
    memset(s_storage_pending_key, 0, sizeof(s_storage_pending_key));
    memset(s_storage_pending_blob, 0, sizeof(s_storage_pending_blob));
    s_storage_pending_len = 0;
    s_storage_set_status = HAL_OK;
    s_storage_set_fail_count = 0;
    s_storage_commit_status = HAL_OK;
    s_storage_commit_fail_count = 0;
    memset(s_mock_ring_present, 0, sizeof(s_mock_ring_present));
    memset(s_mock_ring_macs, 0, sizeof(s_mock_ring_macs));
    memset(s_mock_bond_present, 0, sizeof(s_mock_bond_present));
    memset(s_mock_bond_macs, 0, sizeof(s_mock_bond_macs));
}

void mock_hal_set_time_ms(uint32_t ms) { s_time_ms = ms; }
void mock_hal_advance_time_ms(uint32_t ms) { s_time_ms += ms; }
void mock_hal_set_adc_mv(uint32_t mv) { s_adc_mv = mv; }
void mock_hal_set_adc_status(hal_status_t status) { s_adc_status = status; }
hal_sleep_mode_t mock_hal_get_last_sleep_mode(void) { return s_last_sleep_mode; }
int mock_hal_get_sleep_enter_count(void) { return s_sleep_enter_count; }
void mock_hal_get_last_gpio_set(hal_pin_t *pin, bool *level)
{
    if (pin) *pin = s_last_gpio_set_pin;
    if (level) *level = s_last_gpio_set_level;
}
int mock_hal_get_gpio_set_count(void) { return s_gpio_set_count; }
void mock_hal_get_last_wake_gpio_mask(uint64_t *pin_mask, bool *level)
{
    if (pin_mask) *pin_mask = s_last_wake_gpio_mask;
    if (level) *level = s_last_wake_gpio_level;
}
int mock_hal_get_wake_gpio_config_count(void) { return s_wake_gpio_config_count; }
void mock_hal_set_gpio_input(hal_pin_t pin, bool level)
{
    if (pin >= MOCK_GPIO_MAX_PINS) {
        return;
    }

    bool previous = s_gpio_level[pin];
    s_gpio_level[pin] = level;

    if (!s_gpio_isr_callback[pin] || previous == level) {
        return;
    }

    bool rising = (!previous && level);
    bool falling = (previous && !level);
    bool should_fire = false;

    switch (s_gpio_intr_type[pin]) {
    case HAL_GPIO_INTR_RISING:
        should_fire = rising;
        break;
    case HAL_GPIO_INTR_FALLING:
        should_fire = falling;
        break;
    case HAL_GPIO_INTR_ANY_EDGE:
        should_fire = true;
        break;
    case HAL_GPIO_INTR_NONE:
    default:
        break;
    }

    if (should_fire) {
        s_gpio_isr_callback[pin](s_gpio_isr_arg[pin]);
    }
}

void mock_hal_inject_ble_send_failure(hal_status_t status, int count)
{
    s_ble_send_status = status;
    s_ble_send_fail_count = count;
}

void mock_hal_set_ble_conn_param_status(hal_status_t status)
{
    s_ble_conn_param_status = status;
}

void mock_hal_storage_seed(const char *key, const void *data, size_t len)
{
    if (!key || !data || len > MOCK_STORAGE_MAX_BLOB_LEN) return;
    s_storage_committed_present = true;
    memset(s_storage_committed_key, 0, sizeof(s_storage_committed_key));
    strncpy(s_storage_committed_key, key, sizeof(s_storage_committed_key) - 1);
    memcpy(s_storage_committed_blob, data, len);
    s_storage_committed_len = len;
}

void mock_hal_inject_storage_set_failure(hal_status_t status, int count)
{
    s_storage_set_status = status;
    s_storage_set_fail_count = count;
}

void mock_hal_inject_storage_commit_failure(hal_status_t status, int count)
{
    s_storage_commit_status = status;
    s_storage_commit_fail_count = count;
}

void mock_hal_set_adc_mv_channel(uint8_t channel, uint32_t mv)
{
    if (channel < 8) {
        s_adc_mv_per_channel[channel] = mv;
        s_adc_channel_configured[channel] = true;
    }
}

void mock_hal_spi_set_register(uint8_t addr, uint8_t value)
{
    s_spi_registers[addr] = value;
}

uint8_t mock_hal_spi_get_last_write_addr(void)
{
    return s_spi_last_write_addr;
}

uint8_t mock_hal_spi_get_last_write_value(void)
{
    return s_spi_last_write_value;
}

int mock_hal_spi_get_write_count(void)
{
    return s_spi_write_count;
}

void mock_hal_inject_spi_failure(hal_status_t status, int count)
{
    s_spi_status = status;
    s_spi_fail_count = count;
}

void mock_ble_central_clear_connected_rings(void)
{
    memset(s_mock_ring_present, 0, sizeof(s_mock_ring_present));
    memset(s_mock_ring_macs, 0, sizeof(s_mock_ring_macs));
}

void mock_ble_central_set_connected_ring(uint8_t ring_index, const uint8_t mac[6])
{
    if (ring_index >= HUB_MAX_RINGS || !mac) {
        return;
    }

    s_mock_ring_present[ring_index] = true;
    memcpy(s_mock_ring_macs[ring_index], mac, sizeof(s_mock_ring_macs[ring_index]));
}

void mock_ble_central_clear_bonds(void)
{
    memset(s_mock_bond_present, 0, sizeof(s_mock_bond_present));
    memset(s_mock_bond_macs, 0, sizeof(s_mock_bond_macs));
}

void mock_ble_central_seed_bond(const uint8_t mac[6])
{
    if (!mac) {
        return;
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_mock_bond_present[i] &&
            memcmp(s_mock_bond_macs[i], mac, sizeof(s_mock_bond_macs[i])) == 0) {
            return;
        }
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (!s_mock_bond_present[i]) {
            s_mock_bond_present[i] = true;
            memcpy(s_mock_bond_macs[i], mac, sizeof(s_mock_bond_macs[i]));
            return;
        }
    }
}

bool mock_ble_central_has_bond(const uint8_t mac[6])
{
    if (!mac) {
        return false;
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_mock_bond_present[i] &&
            memcmp(s_mock_bond_macs[i], mac, sizeof(s_mock_bond_macs[i])) == 0) {
            return true;
        }
    }

    return false;
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
hal_status_t hal_gpio_init(hal_pin_t p, hal_gpio_mode_t m)
{
    if (p < MOCK_GPIO_MAX_PINS) {
        if (m == HAL_GPIO_INPUT_PULLUP) {
            s_gpio_level[p] = true;
        } else if (m == HAL_GPIO_INPUT_PULLDOWN) {
            s_gpio_level[p] = false;
        }
    }
    return HAL_OK;
}
hal_status_t hal_gpio_set(hal_pin_t p, bool l)
{
    s_last_gpio_set_pin = p;
    s_last_gpio_set_level = l;
    s_gpio_set_count++;
    if (p < MOCK_GPIO_MAX_PINS) {
        s_gpio_level[p] = l;
    }
    return HAL_OK;
}
bool hal_gpio_get(hal_pin_t p)
{
    if (p >= MOCK_GPIO_MAX_PINS) {
        return false;
    }
    return s_gpio_level[p];
}
hal_status_t hal_gpio_set_interrupt(hal_pin_t p, hal_gpio_intr_t e, hal_isr_callback_t cb, void *a)
{
    if (p >= MOCK_GPIO_MAX_PINS) {
        return HAL_ERR_INVALID_ARG;
    }

    s_gpio_intr_type[p] = e;
    s_gpio_isr_callback[p] = (e == HAL_GPIO_INTR_NONE) ? NULL : cb;
    s_gpio_isr_arg[p] = (e == HAL_GPIO_INTR_NONE) ? NULL : a;
    return HAL_OK;
}

// --- hal_adc (configurable for battery and sensor tests) ---
hal_status_t hal_adc_init(hal_adc_channel_t ch) { (void)ch; return HAL_OK; }
hal_status_t hal_adc_read_mv(hal_adc_channel_t ch, uint32_t *mv) {
    if (s_adc_status != HAL_OK) return s_adc_status;
    if (mv) {
        // Use per-channel value if configured, else global default
        if (ch < 8 && s_adc_channel_configured[ch]) {
            *mv = s_adc_mv_per_channel[ch];
        } else {
            *mv = s_adc_mv;
        }
    }
    return HAL_OK;
}
hal_status_t hal_adc_deinit(hal_adc_channel_t ch) { (void)ch; return HAL_OK; }

// --- hal_sleep ---
hal_status_t hal_sleep_enter(hal_sleep_mode_t m)
{
    s_last_sleep_mode = m;
    s_sleep_enter_count++;
    return HAL_OK;
}
hal_status_t hal_sleep_configure_wake_gpio(hal_pin_t p, bool l)
{
    return hal_sleep_configure_wake_gpio_mask((1ULL << p), l);
}
hal_status_t hal_sleep_configure_wake_gpio_mask(uint64_t pin_mask, bool level)
{
    s_last_wake_gpio_mask = pin_mask;
    s_last_wake_gpio_level = level;
    s_wake_gpio_config_count++;
    return HAL_OK;
}
hal_status_t hal_sleep_configure_wake_timer(uint32_t us) { (void)us; return HAL_OK; }

// --- hal_storage ---
hal_status_t hal_storage_init(void) { return HAL_OK; }
hal_status_t hal_storage_set(const char *k, const void *d, size_t l) {
    if (!k || !d || l > MOCK_STORAGE_MAX_BLOB_LEN) return HAL_ERR_INVALID_ARG;
    if (s_storage_set_fail_count > 0) {
        s_storage_set_fail_count--;
        return s_storage_set_status;
    }
    s_storage_pending_present = true;
    s_storage_pending_delete = false;
    memset(s_storage_pending_key, 0, sizeof(s_storage_pending_key));
    strncpy(s_storage_pending_key, k, sizeof(s_storage_pending_key) - 1);
    memcpy(s_storage_pending_blob, d, l);
    s_storage_pending_len = l;
    return HAL_OK;
}
hal_status_t hal_storage_get(const char *k, void *d, size_t *l) {
    if (!k || !l) return HAL_ERR_INVALID_ARG;
    if (!s_storage_committed_present || strcmp(s_storage_committed_key, k) != 0) {
        return HAL_ERR_NOT_FOUND;
    }
    if (*l < s_storage_committed_len) {
        *l = s_storage_committed_len;
        return HAL_ERR_INVALID_ARG;
    }
    if (d && s_storage_committed_len > 0) {
        memcpy(d, s_storage_committed_blob, s_storage_committed_len);
    }
    *l = s_storage_committed_len;
    return HAL_OK;
}
hal_status_t hal_storage_delete(const char *k) {
    if (!k) return HAL_ERR_INVALID_ARG;
    s_storage_pending_present = true;
    s_storage_pending_delete = true;
    memset(s_storage_pending_key, 0, sizeof(s_storage_pending_key));
    strncpy(s_storage_pending_key, k, sizeof(s_storage_pending_key) - 1);
    s_storage_pending_len = 0;
    return HAL_OK;
}
hal_status_t hal_storage_commit(void) {
    if (s_storage_commit_fail_count > 0) {
        s_storage_commit_fail_count--;
        return s_storage_commit_status;
    }
    if (!s_storage_pending_present) {
        return HAL_OK;
    }

    if (s_storage_pending_delete) {
        if (s_storage_committed_present && strcmp(s_storage_committed_key, s_storage_pending_key) == 0) {
            s_storage_committed_present = false;
            memset(s_storage_committed_key, 0, sizeof(s_storage_committed_key));
            memset(s_storage_committed_blob, 0, sizeof(s_storage_committed_blob));
            s_storage_committed_len = 0;
        }
    } else {
        s_storage_committed_present = true;
        memset(s_storage_committed_key, 0, sizeof(s_storage_committed_key));
        strncpy(s_storage_committed_key, s_storage_pending_key, sizeof(s_storage_committed_key) - 1);
        memcpy(s_storage_committed_blob, s_storage_pending_blob, s_storage_pending_len);
        s_storage_committed_len = s_storage_pending_len;
    }

    s_storage_pending_present = false;
    s_storage_pending_delete = false;
    memset(s_storage_pending_key, 0, sizeof(s_storage_pending_key));
    memset(s_storage_pending_blob, 0, sizeof(s_storage_pending_blob));
    s_storage_pending_len = 0;
    return HAL_OK;
}

// --- hal_spi (with register file and failure injection) ---
hal_status_t hal_spi_init(const hal_spi_config_t *c, hal_spi_handle_t *h) {
    (void)c;
    if (s_spi_fail_count > 0) { s_spi_fail_count--; return s_spi_status; }
    if (h) *h = (hal_spi_handle_t)1;  // Non-null dummy handle
    return HAL_OK;
}
hal_status_t hal_spi_transfer(hal_spi_handle_t h, const uint8_t *tx, uint8_t *rx, size_t l) {
    (void)h;
    if (s_spi_fail_count > 0) { s_spi_fail_count--; return s_spi_status; }
    // For burst reads: tx[0] is address, rx[1..] filled from register file
    if (tx && rx && l > 1) {
        uint8_t addr = tx[0] & 0x7F;
        for (size_t i = 1; i < l; i++) {
            rx[i] = s_spi_registers[(addr + i - 1) & 0xFF];
        }
    }
    return HAL_OK;
}
hal_status_t hal_spi_write_reg(hal_spi_handle_t h, uint8_t a, uint8_t v) {
    (void)h;
    if (s_spi_fail_count > 0) { s_spi_fail_count--; return s_spi_status; }
    s_spi_registers[a & 0x7F] = v;
    s_spi_last_write_addr = a;
    s_spi_last_write_value = v;
    s_spi_write_count++;
    return HAL_OK;
}
hal_status_t hal_spi_read_reg(hal_spi_handle_t h, uint8_t a, uint8_t *v) {
    (void)h;
    if (s_spi_fail_count > 0) { s_spi_fail_count--; return s_spi_status; }
    if (v) *v = s_spi_registers[a & 0x7F];
    return HAL_OK;
}
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
void hal_ble_set_diagnostic_payload(const uint8_t *data, size_t len)
{
    (void)data;
    (void)len;
}
bool hal_ble_is_connected(void) { return false; }

// --- hal_ota ---
hal_status_t hal_ota_begin(hal_ota_handle_t *h) { (void)h; return HAL_OK; }
hal_status_t hal_ota_write(hal_ota_handle_t h, const void *d, size_t l) { (void)h;(void)d;(void)l; return HAL_OK; }
hal_status_t hal_ota_finish(hal_ota_handle_t h) { (void)h; return HAL_OK; }
void hal_ota_reboot(void) {}
hal_status_t hal_ota_confirm(void) { return HAL_OK; }
void hal_ota_rollback(void) {}

// --- ble_central mock helpers used by host-side hub tests ---
hal_status_t ble_central_find_ring_index_by_mac(const uint8_t mac[6],
                                                uint8_t *ring_index_out)
{
    if (!mac || !ring_index_out) {
        return HAL_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_mock_ring_present[i] &&
            memcmp(s_mock_ring_macs[i], mac, sizeof(s_mock_ring_macs[i])) == 0) {
            *ring_index_out = i;
            return HAL_OK;
        }
    }

    return HAL_ERR_NOT_FOUND;
}

hal_status_t ble_central_disconnect_ring_by_mac(const uint8_t mac[6])
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_mock_ring_present[i] &&
            memcmp(s_mock_ring_macs[i], mac, sizeof(s_mock_ring_macs[i])) == 0) {
            s_mock_ring_present[i] = false;
            memset(s_mock_ring_macs[i], 0, sizeof(s_mock_ring_macs[i]));
            return HAL_OK;
        }
    }

    return HAL_ERR_NOT_FOUND;
}

hal_status_t ble_central_delete_bond_by_mac(const uint8_t mac[6])
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_mock_bond_present[i] &&
            memcmp(s_mock_bond_macs[i], mac, sizeof(s_mock_bond_macs[i])) == 0) {
            s_mock_bond_present[i] = false;
            memset(s_mock_bond_macs[i], 0, sizeof(s_mock_bond_macs[i]));
            return HAL_OK;
        }
    }

    return HAL_OK;
}
