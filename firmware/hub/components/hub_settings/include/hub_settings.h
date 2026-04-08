// SPDX-License-Identifier: MIT
// PowerFinger Hub — Persisted hub-owned settings
//
// Stores companion-configurable hub settings that are not owned by any one
// ring, such as scan policy and host-report cadence.

#pragma once

#include "ble_central.h"
#include "hal_types.h"

#include <stdbool.h>
#include <stdint.h>

#define HUB_SETTINGS_USB_POLL_MS_DEFAULT     1U
#define HUB_SETTINGS_SCAN_POLICY_BOOT_ONLY   0U
#define HUB_SETTINGS_SCAN_POLICY_CONTINUOUS  1U
#define HUB_SETTINGS_SCAN_POLICY_EXPECTED    2U
#define HUB_SETTINGS_EXPECTED_RINGS_DEFAULT  2U

typedef struct {
    uint8_t usb_poll_ms;
    uint8_t scan_policy;
    uint8_t expected_rings;
} hub_settings_snapshot_t;

typedef enum {
    HUB_SETTINGS_PARAM_USB_POLL_MS = 0,
    HUB_SETTINGS_PARAM_SCAN_POLICY,
    HUB_SETTINGS_PARAM_EXPECTED_RINGS,
} hub_settings_param_t;

hal_status_t hub_settings_init(void);
void hub_settings_get(hub_settings_snapshot_t *snapshot_out);

uint8_t hub_settings_get_usb_poll_ms(void);
uint8_t hub_settings_get_scan_policy(void);
uint8_t hub_settings_get_expected_rings(void);

hal_status_t hub_settings_set_usb_poll_ms(uint8_t usb_poll_ms);
hal_status_t hub_settings_set_scan_policy(uint8_t scan_policy);
hal_status_t hub_settings_set_expected_rings(uint8_t expected_rings);
void hub_settings_flush_if_dirty(void);

bool hub_settings_usb_poll_ms_supported(uint8_t usb_poll_ms);
bool hub_settings_scan_policy_supported(uint8_t scan_policy);
bool hub_settings_expected_rings_supported(uint8_t expected_rings);
