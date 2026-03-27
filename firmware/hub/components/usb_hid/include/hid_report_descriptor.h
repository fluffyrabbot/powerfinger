// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB HID report descriptor definition
//
// Exposes the USB HID report descriptor byte array for TinyUSB registration.

#pragma once

#include <stdint.h>
#include <stddef.h>

// USB HID report byte layout (7 bytes):
//   [0]   buttons (3 bits + 5 padding)
//   [1-2] X delta (int16_t LE)
//   [3-4] Y delta (int16_t LE)
//   [5]   vertical scroll (int8_t)
//   [6]   horizontal scroll (int8_t)
#define USB_HID_REPORT_SIZE 7

extern const uint8_t usb_hid_report_descriptor[];
extern const size_t  usb_hid_report_descriptor_len;
