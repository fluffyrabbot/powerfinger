// SPDX-License-Identifier: MIT
// PowerFinger — PowerPen DRV5032 wake debounce
//
// Tracks spurious DRV5032 deep-sleep wakes in retained memory so repeated
// false wakes can temporarily disable GPIO8 as a wake source.

#include "pen_wake_debounce.h"

#include "hal_gpio.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "power_manager.h"

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#include "esp_attr.h"
#include "esp_log.h"
#include <sys/time.h>
#define PEN_WAKE_RTC_DATA_ATTR RTC_DATA_ATTR
static const char *TAG = "pen_wake";
#else
#ifndef PEN_WAKE_RTC_DATA_ATTR
#define PEN_WAKE_RTC_DATA_ATTR
#endif
#endif

#ifdef CONFIG_POWERFINGER_BARREL_PIN
#define PIN_BARREL ((hal_pin_t)CONFIG_POWERFINGER_BARREL_PIN)
#else
#define PIN_BARREL 4
#endif

#ifdef CONFIG_POWERFINGER_TIP_DOME_PIN
#define PIN_TIP_DOME ((hal_pin_t)CONFIG_POWERFINGER_TIP_DOME_PIN)
#else
#define PIN_TIP_DOME 5
#endif

#ifdef CONFIG_POWERFINGER_WAKE_SENSOR_PIN
#define PIN_WAKE_SENSOR ((hal_pin_t)CONFIG_POWERFINGER_WAKE_SENSOR_PIN)
#else
#define PIN_WAKE_SENSOR 8
#endif

#ifdef CONFIG_POWERFINGER_WAKE_GPIO_MASK
#define WAKE_GPIO_MASK ((uint64_t)CONFIG_POWERFINGER_WAKE_GPIO_MASK)
#else
#define WAKE_GPIO_MASK 0ULL
#endif

#ifdef CONFIG_POWERFINGER_WAKE_VALIDATION_MS
#define WAKE_VALIDATION_MS ((uint32_t)CONFIG_POWERFINGER_WAKE_VALIDATION_MS)
#else
#define WAKE_VALIDATION_MS 200U
#endif

#ifdef CONFIG_POWERFINGER_SPURIOUS_WAKE_LIMIT
#define SPURIOUS_WAKE_LIMIT ((uint8_t)CONFIG_POWERFINGER_SPURIOUS_WAKE_LIMIT)
#else
#define SPURIOUS_WAKE_LIMIT 3U
#endif

#ifdef CONFIG_POWERFINGER_SPURIOUS_WAKE_WINDOW_MS
#define SPURIOUS_WAKE_WINDOW_MS ((uint64_t)CONFIG_POWERFINGER_SPURIOUS_WAKE_WINDOW_MS)
#else
#define SPURIOUS_WAKE_WINDOW_MS 30000ULL
#endif

#define PEN_WAKE_RTC_MAGIC 0x5046574BU

typedef struct {
    uint32_t magic;
    uint64_t spurious_window_start_rtc_ms;
    uint8_t spurious_count;
    bool drv5032_enabled;
} pen_wake_rtc_state_t;

static PEN_WAKE_RTC_DATA_ATTR pen_wake_rtc_state_t s_rtc_state = {
    .magic = 0,
    .spurious_window_start_rtc_ms = 0,
    .spurious_count = 0,
    .drv5032_enabled = true,
};

static bool s_validation_pending = false;
static uint32_t s_validation_deadline_ms = 0;

static uint64_t pen_wake_now_rtc_ms(void)
{
#ifdef ESP_PLATFORM
    // RTC-backed system time survives deep sleep on ESP-IDF's default time
    // source configuration, so it can bracket spurious wake windows.
    struct timeval now = {0};
    gettimeofday(&now, NULL);
    return ((uint64_t)now.tv_sec * 1000ULL) + ((uint64_t)now.tv_usec / 1000ULL);
#else
    return (uint64_t)hal_timer_get_ms();
#endif
}

static bool pen_wake_deadline_elapsed(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

static void pen_wake_reset_rtc_state(void)
{
    s_rtc_state.magic = PEN_WAKE_RTC_MAGIC;
    s_rtc_state.spurious_window_start_rtc_ms = 0;
    s_rtc_state.spurious_count = 0;
    s_rtc_state.drv5032_enabled = true;
}

static void pen_wake_apply_mask(void)
{
    uint64_t mask = WAKE_GPIO_MASK;

    if (!s_rtc_state.drv5032_enabled) {
        mask &= ~(1ULL << PIN_WAKE_SENSOR);
    }

    power_manager_set_wake_gpio_mask(mask);
}

static void pen_wake_log_status(const char *reason)
{
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG,
             "%s: drv5032_wake=%s spurious=%u",
             reason,
             s_rtc_state.drv5032_enabled ? "enabled" : "disabled",
             s_rtc_state.spurious_count);
#else
    (void)reason;
#endif
}

static void pen_wake_clear_validation(void)
{
    s_validation_pending = false;
    s_validation_deadline_ms = 0;
}

static void pen_wake_reset_counter(void)
{
    bool changed = (s_rtc_state.spurious_count != 0) || !s_rtc_state.drv5032_enabled;

    s_rtc_state.spurious_count = 0;
    s_rtc_state.spurious_window_start_rtc_ms = 0;
    s_rtc_state.drv5032_enabled = true;
    pen_wake_clear_validation();
    pen_wake_apply_mask();

    if (changed) {
        pen_wake_log_status("drv5032 wake reset");
    }
}

static void pen_wake_note_spurious(void)
{
    uint64_t now_rtc_ms = pen_wake_now_rtc_ms();
    bool within_window = false;

    if (s_rtc_state.spurious_window_start_rtc_ms != 0 &&
        now_rtc_ms >= s_rtc_state.spurious_window_start_rtc_ms) {
        within_window =
            (now_rtc_ms - s_rtc_state.spurious_window_start_rtc_ms) <=
            SPURIOUS_WAKE_WINDOW_MS;
    }

    if (!within_window) {
        s_rtc_state.spurious_count = 0;
        s_rtc_state.spurious_window_start_rtc_ms = now_rtc_ms;
    }

    if (s_rtc_state.spurious_count < UINT8_MAX) {
        s_rtc_state.spurious_count++;
    }

    if (s_rtc_state.spurious_count >= SPURIOUS_WAKE_LIMIT) {
        s_rtc_state.drv5032_enabled = false;
    }

    pen_wake_clear_validation();
    pen_wake_apply_mask();

    pen_wake_log_status(s_rtc_state.drv5032_enabled
                            ? "spurious drv5032 wake"
                            : "drv5032 wake disabled");
}

hal_status_t pen_wake_debounce_init(void)
{
    hal_status_t rc = hal_gpio_init(PIN_BARREL, HAL_GPIO_INPUT_PULLUP);
    if (rc != HAL_OK) {
        return rc;
    }

    rc = hal_gpio_init(PIN_TIP_DOME, HAL_GPIO_INPUT_PULLUP);
    if (rc != HAL_OK) {
        return rc;
    }

    rc = hal_gpio_init(PIN_WAKE_SENSOR, HAL_GPIO_INPUT_PULLUP);
    if (rc != HAL_OK) {
        return rc;
    }

    hal_wake_cause_t wake_cause = hal_sleep_get_wake_cause();
    if (s_rtc_state.magic != PEN_WAKE_RTC_MAGIC ||
        wake_cause == HAL_WAKE_CAUSE_COLD_BOOT) {
        pen_wake_reset_rtc_state();
    } else if (s_rtc_state.drv5032_enabled &&
               s_rtc_state.spurious_window_start_rtc_ms != 0) {
        uint64_t now_rtc_ms = pen_wake_now_rtc_ms();
        if (now_rtc_ms < s_rtc_state.spurious_window_start_rtc_ms ||
            (now_rtc_ms - s_rtc_state.spurious_window_start_rtc_ms) >
                SPURIOUS_WAKE_WINDOW_MS) {
            s_rtc_state.spurious_count = 0;
            s_rtc_state.spurious_window_start_rtc_ms = 0;
        }
    }

    bool barrel_pressed = !hal_gpio_get(PIN_BARREL);
    bool tip_pressed = !hal_gpio_get(PIN_TIP_DOME);
    bool wake_sensor_asserted = !hal_gpio_get(PIN_WAKE_SENSOR);

    pen_wake_apply_mask();
    pen_wake_clear_validation();

    if (barrel_pressed) {
        pen_wake_reset_counter();
        return HAL_OK;
    }

    if (wake_cause == HAL_WAKE_CAUSE_GPIO &&
        s_rtc_state.drv5032_enabled &&
        wake_sensor_asserted &&
        !tip_pressed) {
        s_validation_pending = true;
        s_validation_deadline_ms = hal_timer_get_ms() + WAKE_VALIDATION_MS;
        pen_wake_log_status("drv5032 wake pending validation");
    }

    return HAL_OK;
}

void pen_wake_debounce_tick(uint32_t now_ms)
{
    if (!s_validation_pending) {
        return;
    }

    if (pen_wake_deadline_elapsed(now_ms, s_validation_deadline_ms)) {
        pen_wake_note_spurious();
    }
}

void pen_wake_debounce_note_motion(void)
{
    pen_wake_reset_counter();
}

void pen_wake_debounce_note_barrel_press(void)
{
    pen_wake_reset_counter();
}

void pen_wake_debounce_note_validation_failure(void)
{
    if (!s_validation_pending) {
        return;
    }
    pen_wake_note_spurious();
}

bool pen_wake_debounce_drv5032_enabled(void)
{
    return s_rtc_state.drv5032_enabled;
}

uint8_t pen_wake_debounce_spurious_wake_count(void)
{
    return s_rtc_state.spurious_count;
}

bool pen_wake_debounce_validation_pending(void)
{
    return s_validation_pending;
}
