// SPDX-License-Identifier: MIT
// PowerFinger — PowerPen dual-click driver
//
// PP3 exposes two independent GPIO-backed click sources:
//   - Primary: barrel switch (thumb, left-click default)
//   - Secondary: tip dome (press-down, right-click default)
//
// Both inputs are active-low with pull-ups. Each source maintains its own ISR
// latched raw state and debounced state so simultaneous presses are preserved.

#include "click_interface.h"
#include "ring_config.h"
#include "hal_gpio.h"
#include "hal_timer.h"

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#include "esp_log.h"
#include <stdatomic.h>
static const char *TAG = "click_pen";
#else
#include <stdatomic.h>
#ifndef IRAM_ATTR
#define IRAM_ATTR
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

typedef struct {
    hal_pin_t pin;
    atomic_bool raw_pressed;
    bool debounced_state;
    uint32_t last_change_ms;
} click_input_t;

static click_input_t s_inputs[] = {
    [CLICK_SOURCE_PRIMARY] = {
        .pin = PIN_BARREL,
    },
    [CLICK_SOURCE_SECONDARY] = {
        .pin = PIN_TIP_DOME,
    },
};

static void IRAM_ATTR pen_click_isr(void *arg)
{
    click_input_t *input = (click_input_t *)arg;
    if (!input) {
        return;
    }

    atomic_store(&input->raw_pressed, !hal_gpio_get(input->pin));
}

static click_input_t *click_input_for_source(click_source_t source)
{
    if (source < CLICK_SOURCE_PRIMARY || source > CLICK_SOURCE_SECONDARY) {
        return NULL;
    }
    return &s_inputs[source];
}

static hal_status_t click_input_init(click_input_t *input)
{
    if (!input) {
        return HAL_ERR_INVALID_ARG;
    }

    hal_status_t rc = hal_gpio_init(input->pin, HAL_GPIO_INPUT_PULLUP);
    if (rc != HAL_OK) {
        return rc;
    }

    rc = hal_gpio_set_interrupt(input->pin, HAL_GPIO_INTR_ANY_EDGE, pen_click_isr, input);
    if (rc != HAL_OK) {
        return rc;
    }

    input->debounced_state = !hal_gpio_get(input->pin);
    atomic_store(&input->raw_pressed, input->debounced_state);
    input->last_change_ms = 0;
    return HAL_OK;
}

static bool click_input_poll(click_input_t *input)
{
    if (!input) {
        return false;
    }

    bool raw = atomic_load(&input->raw_pressed);
    uint32_t now = hal_timer_get_ms();

    if (raw != input->debounced_state &&
        (now - input->last_change_ms) >= CLICK_DEBOUNCE_MS) {
        input->debounced_state = raw;
        input->last_change_ms = now;
    }

    return input->debounced_state;
}

hal_status_t click_init(void)
{
    hal_status_t rc = click_input_init(&s_inputs[CLICK_SOURCE_PRIMARY]);
    if (rc != HAL_OK) {
        return rc;
    }

    rc = click_input_init(&s_inputs[CLICK_SOURCE_SECONDARY]);
    if (rc != HAL_OK) {
        hal_gpio_set_interrupt(PIN_BARREL, HAL_GPIO_INTR_NONE, NULL, NULL);
        return rc;
    }

#ifdef ESP_PLATFORM
    ESP_LOGI(TAG,
             "pen click inputs initialized (barrel=%lu tip=%lu)",
             (unsigned long)PIN_BARREL,
             (unsigned long)PIN_TIP_DOME);
#endif

    return HAL_OK;
}

bool click_is_pressed(click_source_t source)
{
    click_input_t *input = click_input_for_source(source);
    return click_input_poll(input);
}

hal_status_t click_deinit(void)
{
    hal_status_t rc_primary =
        hal_gpio_set_interrupt(PIN_BARREL, HAL_GPIO_INTR_NONE, NULL, NULL);
    hal_status_t rc_secondary =
        hal_gpio_set_interrupt(PIN_TIP_DOME, HAL_GPIO_INTR_NONE, NULL, NULL);

    if (rc_primary != HAL_OK) {
        return rc_primary;
    }
    return rc_secondary;
}
