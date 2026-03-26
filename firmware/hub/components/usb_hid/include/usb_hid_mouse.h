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
