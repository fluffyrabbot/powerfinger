// SPDX-License-Identifier: MIT
// PowerFinger — Power management implementation
//
// Manages adaptive connection intervals, sleep entry/exit,
// battery voltage monitoring, watchdog, and Hall sensor power gating.

#include "power_manager.h"
#include "ring_config.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "ble_gap_handler.h"

#ifdef ESP_PLATFORM
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "sdkconfig.h"
static const char *TAG = "power_mgr";
#endif

// After this many consecutive ADC read failures, force a low-battery shutdown
// rather than silently skipping the safety check. 5 failures = 5 minutes at
// the 60s battery check interval — long enough to avoid false trips on transient
// ADC errors, short enough to protect against a permanently broken ADC.
#define ADC_FAIL_THRESHOLD 5

// Approximate loaded-voltage state-of-charge mapping for a single-cell LiPo.
// This is intentionally conservative: the device cuts off at 3.2V for cell
// protection, so values near that threshold are reported as effectively empty.
#define BATTERY_EMPTY_MV 3200
#define BATTERY_FULL_MV  4200

// --- State ---

static uint32_t s_last_activity_ms = 0;
static uint32_t s_last_battery_check_ms = 0;
static bool s_connected = false;
static bool s_interaction_active = false;
static bool s_active_params_requested = false;
static bool s_conn_param_rejected = false;  // don't retry if central rejected
static uint8_t s_adc_fail_count = 0;        // consecutive VBAT read failures
static uint32_t s_last_vbat_mv = 0;
static uint8_t s_last_battery_pct = 0;

#ifdef CONFIG_POWERFINGER_HALL_POWER_PIN
#define PIN_HALL_POWER ((hal_pin_t)CONFIG_POWERFINGER_HALL_POWER_PIN)
#else
#define PIN_HALL_POWER HAL_PIN_NONE
#endif

#ifdef CONFIG_POWERFINGER_VBAT_ADC_CHANNEL
#define VBAT_ADC_CHANNEL CONFIG_POWERFINGER_VBAT_ADC_CHANNEL
#else
#define VBAT_ADC_CHANNEL 0
#endif

// --- Hall sensor power gating (ball variants only) ---

static void hall_power_set(bool on)
{
    if (PIN_HALL_POWER == HAL_PIN_NONE) return;
    hal_gpio_set(PIN_HALL_POWER, on);
}

static uint8_t battery_percent_from_mv(uint32_t vbat_mv)
{
    if (vbat_mv <= BATTERY_EMPTY_MV) return 0;
    if (vbat_mv >= BATTERY_FULL_MV) return 100;
    return (uint8_t)(((vbat_mv - BATTERY_EMPTY_MV) * 100U) /
                     (BATTERY_FULL_MV - BATTERY_EMPTY_MV));
}

static void update_battery_cache(uint32_t vbat_mv)
{
    s_last_vbat_mv = vbat_mv;
    s_last_battery_pct = battery_percent_from_mv(vbat_mv);
}

static void note_activity(void)
{
    s_last_activity_ms = hal_timer_get_ms();
    hall_power_set(true);

    if (!s_connected) {
        return;
    }

    s_interaction_active = true;

    // Request active (7.5ms) connection parameters if not already active.
    if (!s_active_params_requested && !s_conn_param_rejected) {
        hal_status_t ret = ble_gap_request_active_params();
        if (ret == HAL_OK) {
            s_active_params_requested = true;
        } else if (ret == HAL_ERR_REJECTED) {
            // Central rejected 7.5ms — don't retry this connection
            s_conn_param_rejected = true;
        }
    }
}

// --- Public API ---

hal_status_t power_manager_init(void)
{
    uint32_t now = hal_timer_get_ms();
    s_last_activity_ms = now;
    s_last_battery_check_ms = now;
    s_connected = false;
    s_interaction_active = false;
    s_active_params_requested = false;
    s_conn_param_rejected = false;
    s_adc_fail_count = 0;
    s_last_vbat_mv = 0;
    s_last_battery_pct = 0;

    // Initialize battery ADC — failure is fatal for LiPo safety
    hal_status_t adc_rc = hal_adc_init(VBAT_ADC_CHANNEL);
    if (adc_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "VBAT ADC init failed (%d) — battery monitoring unavailable", adc_rc);
#endif
        return HAL_ERR_IO;
    }

    // Initialize Hall power gate if configured
    if (PIN_HALL_POWER != HAL_PIN_NONE) {
        hal_gpio_init(PIN_HALL_POWER, HAL_GPIO_OUTPUT);
        hall_power_set(true);  // power on by default during active use
    }

    // Prime the battery cache so the Battery Service is truthful from boot.
    uint32_t vbat_mv = 0;
    if (hal_adc_read_mv(VBAT_ADC_CHANNEL, &vbat_mv) == HAL_OK) {
        update_battery_cache(vbat_mv);
    }

    // Register with task watchdog
#ifdef ESP_PLATFORM
    esp_task_wdt_add(NULL);  // add current task to WDT
#endif

#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "power management initialized (VBAT ADC ch=%d)", VBAT_ADC_CHANNEL);
#endif

    return HAL_OK;
}

void power_manager_on_connect(void)
{
    s_connected = true;
    s_interaction_active = false;
    s_last_activity_ms = hal_timer_get_ms();
    // Reset connection parameter rejection state so the new central gets
    // asked for 7.5ms active params. A previous central may have rejected
    // them, but a new one may accept.
    s_conn_param_rejected = false;
    s_active_params_requested = false;
}

void power_manager_on_disconnect(void)
{
    s_connected = false;
    s_interaction_active = false;
    s_active_params_requested = false;
    s_conn_param_rejected = false;
}

void power_manager_on_motion(void)
{
    note_activity();
}

void power_manager_on_click(void)
{
    note_activity();
}

power_event_t power_manager_tick(uint32_t now_ms)
{
    // --- Active -> idle transition ---
    if (s_connected && s_interaction_active &&
        (now_ms - s_last_activity_ms) >= IDLE_TRANSITION_MS) {
        if (s_active_params_requested) {
            ble_gap_request_idle_params();
        }
        s_interaction_active = false;
        s_active_params_requested = false;
        return POWER_EVT_IDLE_TIMEOUT;
    }

    // --- Battery voltage check ---
    if ((now_ms - s_last_battery_check_ms) >= BATTERY_CHECK_INTERVAL_MS) {
        s_last_battery_check_ms = now_ms;

        uint32_t vbat_mv = 0;
        if (hal_adc_read_mv(VBAT_ADC_CHANNEL, &vbat_mv) == HAL_OK) {
            s_adc_fail_count = 0;
            update_battery_cache(vbat_mv);
            if (vbat_mv < LOW_VOLTAGE_CUTOFF_MV) {
#ifdef ESP_PLATFORM
                ESP_LOGW(TAG, "low battery: %lu mV < %d mV cutoff",
                         (unsigned long)vbat_mv, LOW_VOLTAGE_CUTOFF_MV);
#endif
                return POWER_EVT_LOW_BATTERY;
            }
        } else {
            // ADC read failure — safety check skipped this cycle.
            // After ADC_FAIL_THRESHOLD consecutive failures, force shutdown
            // rather than running indefinitely without LiPo protection.
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "VBAT ADC read failed (%d/%d consecutive)",
                     ++s_adc_fail_count, ADC_FAIL_THRESHOLD);
#else
            s_adc_fail_count++;
#endif
            if (s_adc_fail_count >= ADC_FAIL_THRESHOLD) {
#ifdef ESP_PLATFORM
                ESP_LOGE(TAG, "VBAT ADC failed %d consecutive reads — forcing shutdown (LiPo safety)",
                         s_adc_fail_count);
#endif
                return POWER_EVT_LOW_BATTERY;
            }
        }
    }

    // --- Sleep timeout check (idle only) ---
    if (s_connected &&
        !s_interaction_active &&
        (now_ms - s_last_activity_ms) >= SLEEP_TIMEOUT_MS) {
        hall_power_set(false);
        return POWER_EVT_SLEEP_TIMEOUT;
    }

    return POWER_EVT_NONE;
}

uint8_t power_manager_get_battery_level(void)
{
    return s_last_battery_pct;
}

uint32_t power_manager_get_last_battery_mv(void)
{
    return s_last_vbat_mv;
}

void power_manager_feed_watchdog(void)
{
#ifdef ESP_PLATFORM
    esp_task_wdt_reset();
#endif
}

void power_manager_enter_sleep(bool deep)
{
    // Power gate Hall sensors
    hall_power_set(false);

    if (deep) {
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "entering deep sleep");
#endif
        // Configure wake sources. Both must be attempted; we track whether at
        // least one succeeded to prevent entering an unwakeable deep sleep.
        bool has_wake_source = false;

#ifdef CONFIG_POWERFINGER_DOME_PIN
        hal_status_t gpio_rc = hal_sleep_configure_wake_gpio(
            (hal_pin_t)CONFIG_POWERFINGER_DOME_PIN, false);
        if (gpio_rc == HAL_OK) {
            has_wake_source = true;
        } else {
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "wake GPIO config failed (%d) — timer wake only", gpio_rc);
#endif
        }
#endif
        // Safety net: timer wake so device eventually wakes even if dome pin
        // is unavailable (e.g. piezo variant). 60s — check USB charging voltage.
        hal_status_t timer_rc = hal_sleep_configure_wake_timer(60 * 1000 * 1000);
        if (timer_rc == HAL_OK) {
            has_wake_source = true;
        } else {
#ifdef ESP_PLATFORM
            ESP_LOGE(TAG, "wake timer config failed (%d)", timer_rc);
#endif
        }

        // Guard: if no wake source is available, deep sleep is permanent.
        // Restart instead so the device remains recoverable.
        if (!has_wake_source) {
#ifdef ESP_PLATFORM
            ESP_LOGE(TAG, "no wake source available — restarting instead of deep sleep");
            esp_restart();
#endif
        }

        hal_sleep_enter(HAL_SLEEP_DEEP);
        // Does not return — device reboots on wake
    } else {
        hal_sleep_enter(HAL_SLEEP_LIGHT);
        // Returns after wake — restore Hall sensor power for motion detection
        hall_power_set(true);
    }
}
