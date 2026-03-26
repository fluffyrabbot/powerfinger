// SPDX-License-Identifier: MIT
// PowerFinger — Metal snap dome click driver
//
// GPIO input with pull-up, active-low. ISR sets atomic flag,
// main loop polls via click_is_pressed(). Hardware debounce via
// minimum interval between state changes.

#include "click_interface.h"
#include "ring_config.h"
#include "hal_gpio.h"
#include "hal_timer.h"

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#include "esp_log.h"
#include <stdatomic.h>
static const char *TAG = "click_dome";
#else
#include <stdatomic.h>
#endif

#ifdef CONFIG_POWERFINGER_DOME_PIN
#define PIN_DOME ((hal_pin_t)CONFIG_POWERFINGER_DOME_PIN)
#else
#define PIN_DOME 8
#endif

// ISR-safe state: set by ISR, read by main loop
static atomic_bool s_raw_pressed = false;

// Debounce state
static bool s_debounced_state = false;
static uint32_t s_last_change_ms = 0;

// ISR callback — minimal work, just record the edge
static void IRAM_ATTR dome_isr(void *arg)
{
    (void)arg;
    // Read current level: active-low (pressed = GPIO low)
    bool pressed = !hal_gpio_get(PIN_DOME);
    atomic_store(&s_raw_pressed, pressed);
}

hal_status_t click_init(void)
{
    // Configure dome pin as input with pull-up (dome shorts to GND when pressed)
    hal_status_t ret = hal_gpio_init(PIN_DOME, HAL_GPIO_INPUT_PULLUP);
    if (ret != HAL_OK) return ret;

    // Register interrupt on both edges (detect press and release)
    ret = hal_gpio_set_interrupt(PIN_DOME, HAL_GPIO_INTR_ANY_EDGE, dome_isr, NULL);
    if (ret != HAL_OK) return ret;

    // Read initial state
    s_debounced_state = !hal_gpio_get(PIN_DOME);
    s_raw_pressed = s_debounced_state;

#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "snap dome initialized on GPIO %lu", (unsigned long)PIN_DOME);
#endif

    return HAL_OK;
}

bool click_is_pressed(void)
{
    bool raw = atomic_load(&s_raw_pressed);
    uint32_t now = hal_timer_get_ms();

    // Debounce: only accept state change if enough time has elapsed
    if (raw != s_debounced_state) {
        if ((now - s_last_change_ms) >= CLICK_DEBOUNCE_MS) {
            s_debounced_state = raw;
            s_last_change_ms = now;
        }
    }

    return s_debounced_state;
}

hal_status_t click_deinit(void)
{
    hal_gpio_set_interrupt(PIN_DOME, HAL_GPIO_INTR_NONE, NULL, NULL);
    return HAL_OK;
}
