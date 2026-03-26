// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB HID mouse implementation (stub)
//
// Full implementation in Step 9 (Phase 4).

#include "usb_hid_mouse.h"

hal_status_t usb_hid_mouse_init(void)
{
    // TODO: Phase 4 — TinyUSB HID init, register report descriptor
    return HAL_OK;
}

hal_status_t usb_hid_mouse_send(const composed_report_t *report)
{
    (void)report;
    // TODO: Phase 4 — tud_hid_mouse_report()
    return HAL_OK;
}
