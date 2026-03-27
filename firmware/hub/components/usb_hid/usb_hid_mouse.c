// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB HID mouse implementation
//
// ESP_PLATFORM: Uses TinyUSB via ESP-IDF component for USB HID output.
// Host (test): Packs reports into a static buffer for test inspection.
//
// Report byte layout (7 bytes, matches hid_report_descriptor.c):
//   [0]   buttons (3 bits + 5 padding)
//   [1-2] X delta (int16_t LE)
//   [3-4] Y delta (int16_t LE)
//   [5]   vertical scroll (int8_t)
//   [6]   horizontal scroll (int8_t)

#include "usb_hid_mouse.h"
#include "hid_report_descriptor.h"

#include <string.h>

// Pack a composed_report_t into the 7-byte USB HID wire format.
// This function is used by both the real TinyUSB path and the test mock.
static void pack_report(const composed_report_t *report, uint8_t out[USB_HID_REPORT_SIZE])
{
    out[0] = report->buttons & 0x07;    // 3 buttons, mask upper bits
    out[1] = (uint8_t)(report->cursor_dx & 0xFF);         // X low byte
    out[2] = (uint8_t)((report->cursor_dx >> 8) & 0xFF);  // X high byte
    out[3] = (uint8_t)(report->cursor_dy & 0xFF);         // Y low byte
    out[4] = (uint8_t)((report->cursor_dy >> 8) & 0xFF);  // Y high byte
    out[5] = (uint8_t)report->scroll_v;
    out[6] = (uint8_t)report->scroll_h;
}

#ifdef ESP_PLATFORM
// ============================================================
// ESP32-S3 TinyUSB implementation
// ============================================================

#include "esp_log.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"

static const char *TAG = "usb_hid";

// TinyUSB descriptor callbacks — required by the TinyUSB stack.

// Device descriptor
static const tusb_desc_device_t s_device_desc = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,       // USB 2.0
    .bDeviceClass       = 0x00,         // Defined at interface level
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x303A,       // Espressif VID (development only)
    .idProduct          = 0x8001,       // PowerFinger hub (development PID)
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

// String descriptors
static const char *s_string_desc[] = {
    [0] = (const char[]){0x09, 0x04},   // Language: English (US)
    [1] = "PowerFinger",                 // Manufacturer
    [2] = "PowerFinger Hub",             // Product
    [3] = "000001",                      // Serial
};

// HID report descriptor callback (called by TinyUSB stack)
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return usb_hid_report_descriptor;
}

// HID GET_REPORT callback
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen)
{
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)reqlen;
    return 0;
}

// HID SET_REPORT callback (e.g. LED indicators — not used for mouse)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)bufsize;
}

hal_status_t usb_hid_mouse_init(void)
{
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = &s_device_desc,
        .string_descriptor = s_string_desc,
        .string_descriptor_count = sizeof(s_string_desc) / sizeof(s_string_desc[0]),
        .external_phy = false,
    };

    esp_err_t ret = tinyusb_driver_install(&tusb_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "TinyUSB driver install failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    ESP_LOGI(TAG, "USB HID mouse initialized (report size=%d)", USB_HID_REPORT_SIZE);
    return HAL_OK;
}

hal_status_t usb_hid_mouse_send(const composed_report_t *report)
{
    if (!report) return HAL_ERR_INVALID_ARG;

    // Wait until device is mounted and HID interface is ready
    if (!tud_mounted() || !tud_hid_ready()) {
        return HAL_ERR_BUSY;
    }

    uint8_t buf[USB_HID_REPORT_SIZE];
    pack_report(report, buf);

    // Report ID 0 (no report ID prefix in descriptor)
    if (!tud_hid_report(0, buf, sizeof(buf))) {
        return HAL_ERR_IO;
    }

    return HAL_OK;
}

#else
// ============================================================
// Host-side mock for unit testing
// ============================================================

#include <stdbool.h>

// Test-visible state: last packed report and send count
static uint8_t  s_last_report[USB_HID_REPORT_SIZE];
static uint32_t s_send_count = 0;
static bool     s_initialized = false;
static hal_status_t s_inject_status = HAL_OK;
static int          s_inject_count = 0;

hal_status_t usb_hid_mouse_init(void)
{
    memset(s_last_report, 0, sizeof(s_last_report));
    s_send_count = 0;
    s_initialized = true;
    s_inject_status = HAL_OK;
    s_inject_count = 0;
    return HAL_OK;
}

hal_status_t usb_hid_mouse_send(const composed_report_t *report)
{
    if (!report) return HAL_ERR_INVALID_ARG;
    if (!s_initialized) return HAL_ERR_IO;

    if (s_inject_count > 0) {
        s_inject_count--;
        return s_inject_status;
    }

    pack_report(report, s_last_report);
    s_send_count++;
    return HAL_OK;
}

// --- Test inspection API (not in production header) ---

void usb_hid_mouse_test_reset(void)
{
    memset(s_last_report, 0, sizeof(s_last_report));
    s_send_count = 0;
    s_initialized = false;
    s_inject_status = HAL_OK;
    s_inject_count = 0;
}

const uint8_t *usb_hid_mouse_test_get_last_report(void)
{
    return s_last_report;
}

uint32_t usb_hid_mouse_test_get_send_count(void)
{
    return s_send_count;
}

void usb_hid_mouse_test_inject_failure(hal_status_t status, int count)
{
    s_inject_status = status;
    s_inject_count = count;
}

#endif // ESP_PLATFORM
