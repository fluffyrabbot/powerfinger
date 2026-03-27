// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB HID report descriptor
//
// Superset of the ring's BLE HID descriptor:
//   BLE:  3 buttons, int8  X/Y, int8  wheel
//   USB:  3 buttons, int16 X/Y, int8  wheel, int8 horizontal scroll (AC Pan)
//
// 16-bit X/Y handles multi-ring delta accumulation (two rings at 127 each
// exceeds int8_t range). AC Pan enables scroll-role ring X deltas as
// horizontal scroll — not available over the ring's BLE descriptor.
//
// Report byte layout (7 bytes total):
//   Byte 0:    buttons (3 bits + 5 padding)
//   Bytes 1-2: X delta (int16_t LE)
//   Bytes 3-4: Y delta (int16_t LE)
//   Byte 5:    vertical scroll wheel (int8_t)
//   Byte 6:    horizontal scroll / AC Pan (int8_t)

#include "hid_report_descriptor.h"

const uint8_t usb_hid_report_descriptor[] = {
    0x05, 0x01,             // Usage Page (Generic Desktop)
    0x09, 0x02,             // Usage (Mouse)
    0xA1, 0x01,             // Collection (Application)
    0x09, 0x01,             //   Usage (Pointer)
    0xA1, 0x00,             //   Collection (Physical)

    // --- 3 buttons ---
    0x05, 0x09,             //     Usage Page (Button)
    0x19, 0x01,             //     Usage Minimum (1)
    0x29, 0x03,             //     Usage Maximum (3)
    0x15, 0x00,             //     Logical Minimum (0)
    0x25, 0x01,             //     Logical Maximum (1)
    0x95, 0x03,             //     Report Count (3)
    0x75, 0x01,             //     Report Size (1)
    0x81, 0x02,             //     Input (Data, Variable, Absolute) — 3 button bits

    // --- 5-bit padding ---
    0x95, 0x01,             //     Report Count (1)
    0x75, 0x05,             //     Report Size (5)
    0x81, 0x03,             //     Input (Constant) — padding to byte boundary

    // --- X, Y (16-bit signed) ---
    0x05, 0x01,             //     Usage Page (Generic Desktop)
    0x09, 0x30,             //     Usage (X)
    0x09, 0x31,             //     Usage (Y)
    0x16, 0x01, 0x80,       //     Logical Minimum (-32767)
    0x26, 0xFF, 0x7F,       //     Logical Maximum (32767)
    0x75, 0x10,             //     Report Size (16)
    0x95, 0x02,             //     Report Count (2)
    0x81, 0x06,             //     Input (Data, Variable, Relative) — X, Y

    // --- Vertical scroll wheel (8-bit signed) ---
    0x09, 0x38,             //     Usage (Wheel)
    0x15, 0x81,             //     Logical Minimum (-127)
    0x25, 0x7F,             //     Logical Maximum (127)
    0x75, 0x08,             //     Report Size (8)
    0x95, 0x01,             //     Report Count (1)
    0x81, 0x06,             //     Input (Data, Variable, Relative) — wheel

    0xC0,                   //   End Collection (Physical)

    // --- Horizontal scroll / AC Pan (Consumer Page) ---
    0xA1, 0x02,             //   Collection (Logical)
    0x05, 0x0C,             //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,       //     Usage (AC Pan)
    0x15, 0x81,             //     Logical Minimum (-127)
    0x25, 0x7F,             //     Logical Maximum (127)
    0x75, 0x08,             //     Report Size (8)
    0x95, 0x01,             //     Report Count (1)
    0x81, 0x06,             //     Input (Data, Variable, Relative) — h-scroll
    0xC0,                   //   End Collection (Logical)

    0xC0,                   // End Collection (Application)
};

const size_t usb_hid_report_descriptor_len = sizeof(usb_hid_report_descriptor);
