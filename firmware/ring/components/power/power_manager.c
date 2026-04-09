// SPDX-License-Identifier: MIT
// PowerFinger — Power management implementation
//
// Manages adaptive connection intervals, sleep entry/exit,
// battery voltage monitoring, thermal safety (NTC + charge MOSFET),
// USB VBUS detection, watchdog, and Hall sensor power gating.

#include "power_manager.h"
#include "ring_config.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "ble_gap_handler.h"

#include <math.h>
#include <limits.h>

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

// --- Kconfig-gated hardware feature pins ---

#ifdef CONFIG_POWERFINGER_HALL_POWER_PIN
#define PIN_HALL_POWER ((hal_pin_t)CONFIG_POWERFINGER_HALL_POWER_PIN)
#else
#define PIN_HALL_POWER HAL_PIN_NONE
#endif

#ifdef CONFIG_POWERFINGER_HALL_POWER_ACTIVE_LOW
#define HALL_POWER_ACTIVE_LOW true
#else
#define HALL_POWER_ACTIVE_LOW false
#endif

#ifdef CONFIG_POWERFINGER_VBAT_ADC_CHANNEL
#define VBAT_ADC_CHANNEL CONFIG_POWERFINGER_VBAT_ADC_CHANNEL
#else
#define VBAT_ADC_CHANNEL 0
#endif

#if defined(CONFIG_POWERFINGER_WAKE_GPIO_MASK) && (CONFIG_POWERFINGER_WAKE_GPIO_MASK > 0)
#define WAKE_GPIO_MASK ((uint64_t)CONFIG_POWERFINGER_WAKE_GPIO_MASK)
#elif defined(CONFIG_POWERFINGER_DOME_PIN)
#define WAKE_GPIO_MASK (1ULL << CONFIG_POWERFINGER_DOME_PIN)
#else
#define WAKE_GPIO_MASK 0ULL
#endif

// NTC temperature monitoring (Kconfig-gated: only present on boards with NTC)
#ifdef CONFIG_POWERFINGER_NTC_ADC_CHANNEL
#define NTC_ADC_CHANNEL ((hal_adc_channel_t)CONFIG_POWERFINGER_NTC_ADC_CHANNEL)
#define HAS_NTC 1
#else
#define HAS_NTC 0
#endif

// Charge enable GPIO (Kconfig-gated: P-ch MOSFET gate on VBUS line)
// GPIO high = gate high = MOSFET off = charge DISABLED (safe default via pull-up)
// GPIO low  = gate low  = MOSFET on  = charge ENABLED
#ifdef CONFIG_POWERFINGER_CHARGE_ENABLE_PIN
#define PIN_CHARGE_ENABLE ((hal_pin_t)CONFIG_POWERFINGER_CHARGE_ENABLE_PIN)
#define HAS_CHARGE_CONTROL 1
#else
#define PIN_CHARGE_ENABLE HAL_PIN_NONE
#define HAS_CHARGE_CONTROL 0
#endif

// VBUS detection GPIO (Kconfig-gated: reads USB 5V presence)
#ifdef CONFIG_POWERFINGER_VBUS_DETECT_PIN
#define PIN_VBUS_DETECT ((hal_pin_t)CONFIG_POWERFINGER_VBUS_DETECT_PIN)
#define HAS_VBUS_DETECT 1
#else
#define PIN_VBUS_DETECT HAL_PIN_NONE
#define HAS_VBUS_DETECT 0
#endif

// --- State ---

static uint32_t s_last_activity_ms = 0;
static uint32_t s_last_battery_check_ms = 0;
static uint32_t s_last_thermal_check_ms = 0;
static bool s_connected = false;
static bool s_interaction_active = false;
static bool s_active_params_requested = false;
static bool s_conn_param_rejected = false;  // don't retry if central rejected
static uint8_t s_adc_fail_count = 0;        // consecutive VBAT read failures
static uint32_t s_last_vbat_mv = 0;
static uint8_t s_last_battery_pct = 0;
static bool s_hall_power_ready = false;
static bool s_hall_power_state_known = false;
static bool s_hall_power_enabled = false;
static uint64_t s_wake_gpio_mask = 0;
static bool s_low_battery_lockout = false;

// Thermal / charge state
static int8_t s_last_cell_temp_c = INT8_MIN;  // INT8_MIN = never read / not configured
static bool s_charging_enabled = false;
static bool s_vbus_present = false;
static bool s_charge_thermal_lockout = false;  // true when temp is outside safe range

// --- Hall sensor power gating (ball variants only) ---

static hal_status_t hall_power_prepare(void)
{
    if (PIN_HALL_POWER == HAL_PIN_NONE || s_hall_power_ready) {
        return HAL_OK;
    }

    hal_status_t rc = hal_gpio_init(PIN_HALL_POWER, HAL_GPIO_OUTPUT);
    if (rc == HAL_OK) {
        s_hall_power_ready = true;
    }
    return rc;
}

static hal_status_t hall_power_set(bool on)
{
    if (PIN_HALL_POWER == HAL_PIN_NONE) {
        return HAL_OK;
    }

    hal_status_t rc = hall_power_prepare();
    if (rc != HAL_OK) {
        return rc;
    }

    if (s_hall_power_state_known && s_hall_power_enabled == on) {
        return HAL_OK;
    }

    bool gpio_level = HALL_POWER_ACTIVE_LOW ? !on : on;
    rc = hal_gpio_set(PIN_HALL_POWER, gpio_level);
    if (rc == HAL_OK) {
        s_hall_power_state_known = true;
        s_hall_power_enabled = on;
    }
    return rc;
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

// --- NTC temperature conversion ---
// Uses B-parameter equation: T = 1 / (1/T0 + (1/B) * ln(R_NTC / R0))
// R_NTC is derived from ADC voltage across a voltage divider:
//   R_NTC = R_DIVIDER * V_ADC / (VCC - V_ADC)
// Returns temperature in degrees Celsius, or INT8_MIN on error.

static int8_t ntc_adc_to_temp_c(uint32_t adc_mv)
{
    if (adc_mv == 0 || adc_mv >= NTC_VCC_MV) {
        // Short or open circuit — cannot compute
        return INT8_MIN;
    }

    float r_ntc = (float)NTC_DIVIDER_R_OHMS * (float)adc_mv /
                  (float)(NTC_VCC_MV - adc_mv);

    if (r_ntc <= 0.0f) {
        return INT8_MIN;
    }

    float ln_ratio = logf(r_ntc / (float)NTC_R0_OHMS);
    float inv_t = (1.0f / NTC_T0_K) + (1.0f / (float)NTC_BETA) * ln_ratio;

    if (inv_t <= 0.0f) {
        return INT8_MIN;
    }

    float temp_c = (1.0f / inv_t) - 273.15f;

    // Clamp to int8_t range
    if (temp_c < -127.0f) return -127;
    if (temp_c > 127.0f) return 127;
    return (int8_t)temp_c;
}

// --- Charge control ---
// Charge MOSFET: GPIO high = off (safe default), GPIO low = on.

static void charge_set_enabled(bool enable)
{
    if (!HAS_CHARGE_CONTROL) {
        return;
    }

    // GPIO low = MOSFET on = charging enabled
    // GPIO high = MOSFET off = charging disabled
    hal_gpio_set(PIN_CHARGE_ENABLE, !enable);
    s_charging_enabled = enable;
}

// Evaluate whether charging should be enabled based on current conditions.
// Requires: VBUS present, temperature in safe range, voltage not over limit.
static void charge_evaluate(void)
{
    if (!HAS_CHARGE_CONTROL) {
        return;
    }

    if (!s_vbus_present) {
        charge_set_enabled(false);
        s_charge_thermal_lockout = false;
        return;
    }

    // Check thermal lockout with hysteresis
    if (s_last_cell_temp_c != INT8_MIN) {
        if (s_last_cell_temp_c >= CHARGE_TEMP_CUTOFF_C ||
            s_last_cell_temp_c <= CHARGE_TEMP_COLD_CUTOFF_C) {
            s_charge_thermal_lockout = true;
        } else if (s_charge_thermal_lockout) {
            // Only clear lockout when temp is within resume band
            if (s_last_cell_temp_c <= CHARGE_TEMP_RESUME_C &&
                s_last_cell_temp_c >= CHARGE_TEMP_COLD_RESUME_C) {
                s_charge_thermal_lockout = false;
            }
        }
    }

    // Check overvoltage
    bool overvoltage = (s_last_vbat_mv > 0 && s_last_vbat_mv >= CHARGE_OVERVOLTAGE_MV);

    bool should_charge = !s_charge_thermal_lockout && !overvoltage;
    charge_set_enabled(should_charge);
}

static void note_activity(void)
{
    s_last_activity_ms = hal_timer_get_ms();
    hal_status_t power_rc = hall_power_set(true);
#ifdef ESP_PLATFORM
    if (power_rc != HAL_OK) {
        ESP_LOGW(TAG, "failed to enable sensor power on activity: %d", power_rc);
    }
#endif

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
    s_last_thermal_check_ms = now;
    s_connected = false;
    s_interaction_active = false;
    s_active_params_requested = false;
    s_conn_param_rejected = false;
    s_adc_fail_count = 0;
    s_last_vbat_mv = 0;
    s_last_battery_pct = 0;
    s_hall_power_ready = false;
    s_hall_power_state_known = false;
    s_hall_power_enabled = false;
    s_wake_gpio_mask = WAKE_GPIO_MASK;
    s_low_battery_lockout = false;
    s_last_cell_temp_c = INT8_MIN;
    s_charging_enabled = false;
    s_vbus_present = false;
    s_charge_thermal_lockout = false;

    // Initialize battery ADC — failure is fatal for LiPo safety
    hal_status_t adc_rc = hal_adc_init(VBAT_ADC_CHANNEL);
    if (adc_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "VBAT ADC init failed (%d) — battery monitoring unavailable", adc_rc);
#endif
        return HAL_ERR_IO;
    }

    // Initialize NTC ADC channel if configured
#if HAS_NTC
    adc_rc = hal_adc_init(NTC_ADC_CHANNEL);
    if (adc_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "NTC ADC init failed (%d) — thermal monitoring unavailable", adc_rc);
#endif
        return HAL_ERR_IO;
    }
#endif

    // Initialize charge control GPIO (default: charging disabled via pull-up)
#if HAS_CHARGE_CONTROL
    hal_status_t gpio_rc = hal_gpio_init(PIN_CHARGE_ENABLE, HAL_GPIO_OUTPUT);
    if (gpio_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "charge enable GPIO init failed (%d)", gpio_rc);
#endif
        return gpio_rc;
    }
    // Ensure charging is disabled until first safety check passes
    charge_set_enabled(false);
#endif

    // Initialize VBUS detection GPIO
#if HAS_VBUS_DETECT
    gpio_rc = hal_gpio_init(PIN_VBUS_DETECT, HAL_GPIO_INPUT_PULLDOWN);
    if (gpio_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "VBUS detect GPIO init failed (%d)", gpio_rc);
#endif
        return gpio_rc;
    }
    s_vbus_present = hal_gpio_get(PIN_VBUS_DETECT);
#endif

    // Initialize Hall power gate if configured.
    // The sensor rail starts enabled so boot-time sensor init/calibration can run.
    hal_status_t hall_rc = hall_power_set(true);
    if (hall_rc != HAL_OK) {
#ifdef ESP_PLATFORM
        ESP_LOGE(TAG, "Hall power init failed (%d)", hall_rc);
#endif
        return hall_rc;
    }

    // Prime the battery cache so the Battery Service is truthful from boot.
    uint32_t vbat_mv = 0;
    if (hal_adc_read_mv(VBAT_ADC_CHANNEL, &vbat_mv) == HAL_OK) {
        update_battery_cache(vbat_mv);
        if (vbat_mv < LOW_VOLTAGE_CUTOFF_MV && !s_vbus_present) {
            s_low_battery_lockout = true;
        }
    }

    // Prime thermal cache and evaluate initial charge state
#if HAS_NTC
    {
        uint32_t ntc_mv = 0;
        if (hal_adc_read_mv(NTC_ADC_CHANNEL, &ntc_mv) == HAL_OK) {
            s_last_cell_temp_c = ntc_adc_to_temp_c(ntc_mv);
        }
    }
#endif
    charge_evaluate();

    // Register with task watchdog
#ifdef ESP_PLATFORM
    esp_task_wdt_add(NULL);  // add current task to WDT
#endif

#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "power management initialized (VBAT ch=%d, NTC=%s, VBUS=%s, charge=%s)",
             VBAT_ADC_CHANNEL,
             HAS_NTC ? "yes" : "no",
             HAS_VBUS_DETECT ? "yes" : "no",
             HAS_CHARGE_CONTROL ? "yes" : "no");
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
    // --- Thermal safety check (fast path when charging) ---
    // When VBUS is present, check temperature every THERMAL_CHECK_INTERVAL_MS.
    // Thermal emergency always takes priority over all other events.
#if HAS_VBUS_DETECT
    s_vbus_present = hal_gpio_get(PIN_VBUS_DETECT);
#endif

    if (s_low_battery_lockout) {
        if (!s_vbus_present) {
            return POWER_EVT_LOW_BATTERY;
        }

        // USB power is present again, so normal charging / thermal handling
        // may resume on this boot.
        s_low_battery_lockout = false;
        s_last_activity_ms = now_ms;
    }

    bool thermal_check_due = false;
    if (HAS_NTC) {
        if (s_vbus_present) {
            // Fast-poll while plugged in
            thermal_check_due = (now_ms - s_last_thermal_check_ms) >= THERMAL_CHECK_INTERVAL_MS;
        } else {
            // Normal interval when on battery
            thermal_check_due = (now_ms - s_last_thermal_check_ms) >= BATTERY_CHECK_INTERVAL_MS;
        }
    }

    if (thermal_check_due) {
#if HAS_NTC
        s_last_thermal_check_ms = now_ms;

        uint32_t ntc_mv = 0;
        if (hal_adc_read_mv(NTC_ADC_CHANNEL, &ntc_mv) == HAL_OK) {
            s_last_cell_temp_c = ntc_adc_to_temp_c(ntc_mv);

            if (s_last_cell_temp_c != INT8_MIN &&
                s_last_cell_temp_c >= THERMAL_EMERGENCY_TEMP_C) {
#ifdef ESP_PLATFORM
                ESP_LOGE(TAG, "THERMAL EMERGENCY: cell temp %d°C >= %d°C — forcing shutdown",
                         s_last_cell_temp_c, THERMAL_EMERGENCY_TEMP_C);
#endif
                charge_set_enabled(false);
                return POWER_EVT_THERMAL_SHUTDOWN;
            }
        }
#endif
        // Re-evaluate charge state after thermal reading
        charge_evaluate();
    }

    // --- Active -> idle transition ---
    if (s_connected && s_interaction_active &&
        (now_ms - s_last_activity_ms) >= IDLE_TRANSITION_MS) {
        if (s_active_params_requested) {
            hal_status_t idle_rc = ble_gap_request_idle_params();
#ifdef ESP_PLATFORM
            if (idle_rc != HAL_OK) {
                ESP_LOGW(TAG, "idle conn-param request failed (%d) — link stays at active interval", idle_rc);
            }
#else
            (void)idle_rc;
#endif
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

            // Overvoltage check during charging
            if (s_vbus_present && vbat_mv >= CHARGE_OVERVOLTAGE_MV) {
#ifdef ESP_PLATFORM
                ESP_LOGW(TAG, "overvoltage during charging: %lu mV >= %d mV — disabling charge",
                         (unsigned long)vbat_mv, CHARGE_OVERVOLTAGE_MV);
#endif
                charge_set_enabled(false);
            }

            if (vbat_mv < LOW_VOLTAGE_CUTOFF_MV) {
                s_low_battery_lockout = !s_vbus_present;
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
                s_low_battery_lockout = !s_vbus_present;
#ifdef ESP_PLATFORM
                ESP_LOGE(TAG, "VBAT ADC failed %d consecutive reads — forcing shutdown (LiPo safety)",
                         s_adc_fail_count);
#endif
                return POWER_EVT_LOW_BATTERY;
            }
        }

        // Re-evaluate charge state after VBAT reading
        charge_evaluate();
    }

    // --- Sleep timeout check (idle only) ---
    if (s_connected &&
        !s_interaction_active &&
        (now_ms - s_last_activity_ms) >= SLEEP_TIMEOUT_MS) {

        // Suppress sleep timeout while USB is plugged in.
        // The device stays awake for charging + thermal monitoring.
        // Note: thermal emergency and low-battery ALWAYS override this.
        if (s_vbus_present) {
            // Reset sleep timer so we don't immediately re-trigger next tick
            s_last_activity_ms = now_ms;
            return POWER_EVT_NONE;
        }

        hal_status_t power_rc = hall_power_set(false);
#ifdef ESP_PLATFORM
        if (power_rc != HAL_OK) {
            ESP_LOGW(TAG, "failed to gate sensor power on sleep timeout: %d", power_rc);
        }
#endif
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

int8_t power_manager_get_cell_temp_c(void)
{
    return s_last_cell_temp_c;
}

bool power_manager_is_vbus_present(void)
{
    return s_vbus_present;
}

bool power_manager_is_charging_enabled(void)
{
    return s_charging_enabled;
}

void power_manager_feed_watchdog(void)
{
#ifdef ESP_PLATFORM
    esp_task_wdt_reset();
#endif
}

hal_status_t power_manager_set_sensor_power(bool enabled)
{
    return hall_power_set(enabled);
}

void power_manager_set_wake_gpio_mask(uint64_t pin_mask)
{
    s_wake_gpio_mask = pin_mask;
}

void power_manager_enter_sleep(bool deep)
{
    // Disable charging before sleep — hardware pull-up provides backup,
    // but explicitly disabling is defense-in-depth.
    charge_set_enabled(false);

    // Power gate Hall sensors
    hal_status_t power_rc = hall_power_set(false);
#ifdef ESP_PLATFORM
    if (power_rc != HAL_OK) {
        ESP_LOGW(TAG, "failed to gate sensor power before sleep: %d", power_rc);
    }
#endif

    if (deep) {
#ifdef ESP_PLATFORM
        ESP_LOGI(TAG, "entering deep sleep");
#endif
        // Configure wake sources. Both must be attempted; we track whether at
        // least one succeeded to prevent entering an unwakeable deep sleep.
        bool has_wake_source = false;
        bool low_battery_vbus_only = s_low_battery_lockout;

        if (low_battery_vbus_only) {
#if HAS_VBUS_DETECT
            hal_status_t vbus_rc = hal_sleep_configure_wake_gpio(PIN_VBUS_DETECT, true);
            if (vbus_rc == HAL_OK) {
                has_wake_source = true;
            } else {
#ifdef ESP_PLATFORM
                ESP_LOGW(TAG, "low-battery VBUS wake config failed (%d) — falling back to timer wake",
                         vbus_rc);
#endif
            }
#else
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "low-battery lockout requested but no VBUS detect pin is configured");
#endif
#endif
        }

        hal_status_t gpio_rc = HAL_ERR_NOT_SUPPORTED;
        if (!low_battery_vbus_only && s_wake_gpio_mask != 0) {
            gpio_rc = hal_sleep_configure_wake_gpio_mask(s_wake_gpio_mask, false);
        }
        if (!low_battery_vbus_only && gpio_rc == HAL_OK) {
            has_wake_source = true;
        } else if (!low_battery_vbus_only && s_wake_gpio_mask != 0) {
#ifdef ESP_PLATFORM
            ESP_LOGW(TAG, "wake GPIO mask 0x%llx config failed (%d) — timer wake only",
                     (unsigned long long)s_wake_gpio_mask, gpio_rc);
#endif
        }
        // Safety net: timer wake so device eventually wakes even if a GPIO wake
        // source is unavailable. When low-battery lockout is active we prefer a
        // VBUS-only wake, but still fall back to a timer so the device is not
        // left in an unrecoverable sleep if board support is incomplete.
        if (!has_wake_source) {
            hal_status_t timer_rc = hal_sleep_configure_wake_timer(60 * 1000 * 1000);
            if (timer_rc == HAL_OK) {
                has_wake_source = true;
            } else {
#ifdef ESP_PLATFORM
                ESP_LOGE(TAG, "wake timer config failed (%d)", timer_rc);
#endif
            }
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
        power_rc = hall_power_set(true);
#ifdef ESP_PLATFORM
        if (power_rc != HAL_OK) {
            ESP_LOGW(TAG, "failed to restore sensor power after light sleep: %d", power_rc);
        }
#endif
    }
}
