// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB HID mouse output interface
//
// Presents the hub as a standard USB HID mouse to the host OS.

#pragma once

#include "hal_types.h"
#include "event_composer.h"

// Initialize USB HID device (TinyUSB on ESP32-S3 native USB OTG)
hal_status_t usb_hid_mouse_init(void);

// Send a composed report to the host via USB HID
hal_status_t usb_hid_mouse_send(const composed_report_t *report);

#ifndef ESP_PLATFORM
// --- Test inspection API (host builds only) ---

// Reset all mock state (call between tests)
void usb_hid_mouse_test_reset(void);

// Get pointer to the last packed 7-byte USB HID report
const uint8_t *usb_hid_mouse_test_get_last_report(void);

// Get total number of successfully sent reports since init/reset
uint32_t usb_hid_mouse_test_get_send_count(void);

// Inject send failures for the next N calls
void usb_hid_mouse_test_inject_failure(hal_status_t status, int count);
#endif
