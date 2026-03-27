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
#include "esp_task_wdt.h"
#include "sdkconfig.h"
static const char *TAG = "power_mgr";
#endif

// --- State ---

static uint32_t s_last_motion_ms = 0;
static uint32_t s_last_battery_check_ms = 0;
static bool s_active_params_requested = false;
static bool s_conn_param_rejected = false;  // don't retry if central rejected

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

// --- Public API ---

hal_status_t power_manager_init(void)
{
    uint32_t now = hal_timer_get_ms();
    s_last_motion_ms = now;
    s_last_battery_check_ms = now;
    s_active_params_requested = false;
    s_conn_param_rejected = false;

    // Initialize battery ADC
    hal_adc_init(VBAT_ADC_CHANNEL);

    // Initialize Hall power gate if configured
    if (PIN_HALL_POWER != HAL_PIN_NONE) {
        hal_gpio_init(PIN_HALL_POWER, HAL_GPIO_OUTPUT);
        hall_power_set(true);  // power on by default during active use
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

void power_manager_on_motion(void)
{
    s_last_motion_ms = hal_timer_get_ms();

    // Request active (7.5ms) connection parameters if not already active
    if (!s_active_params_requested && !s_conn_param_rejected) {
        hal_status_t ret = ble_gap_request_active_params();
        if (ret == HAL_OK) {
            s_active_params_requested = true;
        } else if (ret == HAL_ERR_REJECTED) {
            // Central rejected 7.5ms — don't retry this connection
            s_conn_param_rejected = true;
        }
    }

    // Ensure Hall sensors are powered during motion
    hall_power_set(true);
}

power_event_t power_manager_tick(uint32_t now_ms)
{
    // --- Adaptive connection interval: revert to idle after 250ms ---
    if (s_active_params_requested &&
        (now_ms - s_last_motion_ms) >= IDLE_TRANSITION_MS) {
        ble_gap_request_idle_params();
        s_active_params_requested = false;
    }

    // --- Battery voltage check ---
    if ((now_ms - s_last_battery_check_ms) >= BATTERY_CHECK_INTERVAL_MS) {
        s_last_battery_check_ms = now_ms;

        uint32_t vbat_mv = 0;
        if (hal_adc_read_mv(VBAT_ADC_CHANNEL, &vbat_mv) == HAL_OK) {
            if (vbat_mv < LOW_VOLTAGE_CUTOFF_MV) {
#ifdef ESP_PLATFORM
                ESP_LOGW(TAG, "low battery: %lu mV < %d mV cutoff",
                         (unsigned long)vbat_mv, LOW_VOLTAGE_CUTOFF_MV);
#endif
                return POWER_EVT_LOW_BATTERY;
            }
        }
    }

    // --- Sleep timeout check ---
    if ((now_ms - s_last_motion_ms) >= SLEEP_TIMEOUT_MS) {
        hall_power_set(false);
        return POWER_EVT_SLEEP_TIMEOUT;
    }

    return POWER_EVT_NONE;
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
        // Configure wake sources: dome press (active-low) wakes from deep sleep.
        // Without this, the device enters permanent sleep — bricked.
#ifdef CONFIG_POWERFINGER_DOME_PIN
        hal_sleep_configure_wake_gpio((hal_pin_t)CONFIG_POWERFINGER_DOME_PIN, false);
#endif
        // Safety net: also configure a timer wake so the device eventually
        // wakes even if dome pin is not available (e.g., piezo variant).
        // 60 seconds — check for USB charging voltage, then re-sleep if none.
        hal_sleep_configure_wake_timer(60 * 1000 * 1000);  // 60s in microseconds

        hal_sleep_enter(HAL_SLEEP_DEEP);
        // Does not return — device reboots on wake
    } else {
        hal_sleep_enter(HAL_SLEEP_LIGHT);
        // Returns after wake — re-enable Hall sensors if needed
    }
}
