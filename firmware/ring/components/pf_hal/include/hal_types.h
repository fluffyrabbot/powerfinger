// SPDX-License-Identifier: MIT
// PowerFinger HAL — Portable type definitions
//
// No platform-specific types may appear in this file or any hal_*.h header.
// All ESP-IDF / Zephyr types are confined to the implementation .c files.

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Return status for all HAL functions
typedef enum {
    HAL_OK = 0,
    HAL_ERR_TIMEOUT,
    HAL_ERR_INVALID_ARG,
    HAL_ERR_IO,
    HAL_ERR_NO_MEM,
    HAL_ERR_NOT_FOUND,
    HAL_ERR_NOT_SUPPORTED,
    HAL_ERR_BUSY,
    HAL_ERR_REJECTED,       // e.g. BLE central rejected conn param request
} hal_status_t;

// Opaque GPIO pin number (maps to platform-specific numbering)
typedef uint32_t hal_pin_t;

// Pin value indicating "not configured" / unused
#define HAL_PIN_NONE UINT32_MAX

// Opaque handles — distinct types prevent passing a timer handle to an SPI
// function. The implementation files define the actual struct contents.
typedef struct hal_spi_ctx *hal_spi_handle_t;
typedef struct hal_timer_ctx *hal_timer_handle_t;

// ISR callback signature (must be safe to call from interrupt context)
typedef void (*hal_isr_callback_t)(void *arg);

// Generic callback (called from task context, not ISR)
typedef void (*hal_callback_t)(void *arg);
