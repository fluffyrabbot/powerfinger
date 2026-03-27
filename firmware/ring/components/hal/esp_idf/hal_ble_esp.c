// SPDX-License-Identifier: MIT
// PowerFinger HAL — BLE HID peripheral implementation for ESP-IDF (NimBLE)
//
// This file contains all NimBLE-specific code. The portable hal_ble.h
// interface exposes only HID-level operations.

#include "hal_ble.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "store/config/ble_store_config.h"

#include <string.h>

static const char *TAG = "hal_ble";

// --- Forward declarations ---
static int ble_gap_event_handler(struct ble_gap_event *event, void *arg);
static void ble_host_task(void *param);
static void ble_on_sync(void);
static void ble_on_reset(int reason);

// --- Module state ---
static hal_ble_event_cb_t s_app_cb = NULL;
static void *s_app_cb_arg = NULL;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
// atomic_bool for cross-task safety: written from NimBLE task (GAP callbacks),
// read from main loop (hal_ble_is_connected, hal_ble_send_mouse_report).
// Required for ESP32-S3 dual-core; safe-by-accident on single-core ESP32-C3.
#include <stdatomic.h>
static atomic_bool s_connected = false;
static const char *s_device_name = "PowerFinger";
static uint32_t s_adv_timeout_ms = 0;

// HID report characteristic value handle (set during GATT registration)
static uint16_t s_hid_report_handle = 0;

// --- HID Report Descriptor ---
// Standard mouse: 3 buttons + X + Y + wheel (4-byte report)
static const uint8_t s_hid_report_map[] = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x02,       // Usage (Mouse)
    0xA1, 0x01,       // Collection (Application)
    0x09, 0x01,       //   Usage (Pointer)
    0xA1, 0x00,       //   Collection (Physical)
    0x05, 0x09,       //     Usage Page (Button)
    0x19, 0x01,       //     Usage Minimum (1)
    0x29, 0x03,       //     Usage Maximum (3)
    0x15, 0x00,       //     Logical Minimum (0)
    0x25, 0x01,       //     Logical Maximum (1)
    0x95, 0x03,       //     Report Count (3)
    0x75, 0x01,       //     Report Size (1)
    0x81, 0x02,       //     Input (Data, Variable, Absolute) — 3 buttons
    0x95, 0x01,       //     Report Count (1)
    0x75, 0x05,       //     Report Size (5)
    0x81, 0x03,       //     Input (Constant) — 5-bit padding
    0x05, 0x01,       //     Usage Page (Generic Desktop)
    0x09, 0x30,       //     Usage (X)
    0x09, 0x31,       //     Usage (Y)
    0x09, 0x38,       //     Usage (Wheel)
    0x15, 0x81,       //     Logical Minimum (-127)
    0x25, 0x7F,       //     Logical Maximum (127)
    0x75, 0x08,       //     Report Size (8)
    0x95, 0x03,       //     Report Count (3)
    0x81, 0x06,       //     Input (Data, Variable, Relative) — X, Y, wheel
    0xC0,             //   End Collection
    0xC0,             // End Collection
};

// --- HID Information characteristic (bcdHID 1.11, no localization, not boot capable) ---
static const uint8_t s_hid_info[] = { 0x11, 0x01, 0x00, 0x02 };

// --- GATT service definitions ---

static int ble_hid_report_map_access(uint16_t conn_handle, uint16_t attr_handle,
                                     struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_hid_report_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_hid_info_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

// HID Report Reference descriptor: Report ID 0, Input
static const uint8_t s_report_ref_input[] = { 0x00, 0x01 };

static const struct ble_gatt_svc_def s_gatt_svcs[] = {
    {
        // HID Service (0x1812)
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x1812),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                // Report Map characteristic
                .uuid = BLE_UUID16_DECLARE(0x2A4B),
                .access_cb = ble_hid_report_map_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                // HID Report characteristic (Input — mouse data)
                .uuid = BLE_UUID16_DECLARE(0x2A4D),
                .access_cb = ble_hid_report_access,
                .val_handle = &s_hid_report_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    {
                        // Report Reference descriptor
                        .uuid = BLE_UUID16_DECLARE(0x2908),
                        .att_flags = BLE_ATT_F_READ,
                        .access_cb = ble_hid_report_access,
                    },
                    { 0 }, // terminator
                },
            },
            {
                // HID Information characteristic
                .uuid = BLE_UUID16_DECLARE(0x2A4A),
                .access_cb = ble_hid_info_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                // Protocol Mode characteristic (Report Protocol = 0x01)
                .uuid = BLE_UUID16_DECLARE(0x2A4E),
                .access_cb = ble_hid_info_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            { 0 }, // terminator
        },
    },
    {
        // Device Information Service (0x180A)
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180A),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                // Manufacturer Name
                .uuid = BLE_UUID16_DECLARE(0x2A29),
                .access_cb = ble_hid_info_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {
                // PnP ID
                .uuid = BLE_UUID16_DECLARE(0x2A50),
                .access_cb = ble_hid_info_access,
                .flags = BLE_GATT_CHR_F_READ,
            },
            { 0 },
        },
    },
    {
        // Battery Service (0x180F)
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180F),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                // Battery Level
                .uuid = BLE_UUID16_DECLARE(0x2A19),
                .access_cb = ble_hid_info_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 },
        },
    },
    { 0 }, // terminator
};

// --- GATT access callbacks ---

static int ble_hid_report_map_access(uint16_t conn_handle, uint16_t attr_handle,
                                     struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle; (void)attr_handle; (void)arg;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        int rc = os_mbuf_append(ctxt->om, s_hid_report_map, sizeof(s_hid_report_map));
        return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static int ble_hid_report_access(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle; (void)arg;

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        uint8_t empty_report[4] = { 0 };
        int rc = os_mbuf_append(ctxt->om, empty_report, sizeof(empty_report));
        return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC) {
        int rc = os_mbuf_append(ctxt->om, s_report_ref_input, sizeof(s_report_ref_input));
        return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static int ble_hid_info_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle; (void)attr_handle; (void)arg;

    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        const ble_uuid_t *uuid = ctxt->chr->uuid;
        int rc = 0;

        if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(0x2A4A)) == 0) {
            // HID Information
            rc = os_mbuf_append(ctxt->om, s_hid_info, sizeof(s_hid_info));
        } else if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(0x2A4E)) == 0) {
            // Protocol Mode: Report Protocol
            uint8_t protocol_mode = 0x01;
            rc = os_mbuf_append(ctxt->om, &protocol_mode, 1);
        } else if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(0x2A29)) == 0) {
            // Manufacturer Name
            const char *name = "PowerFinger";
            rc = os_mbuf_append(ctxt->om, name, strlen(name));
        } else if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(0x2A50)) == 0) {
            // PnP ID: Bluetooth SIG vendor source (0x02), no assigned VID/PID yet
            uint8_t pnp_id[] = { 0x02, 0xFF, 0xFF, 0x01, 0x00, 0x01, 0x00 };
            rc = os_mbuf_append(ctxt->om, pnp_id, sizeof(pnp_id));
        } else if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(0x2A19)) == 0) {
            // Battery Level: placeholder 100%
            uint8_t level = 100;
            rc = os_mbuf_append(ctxt->om, &level, 1);
        }
        return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        // Protocol Mode write (switch between Boot and Report protocol)
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

// --- GAP event handler ---

static void notify_app(hal_ble_event_t type)
{
    if (s_app_cb) {
        hal_ble_event_data_t evt = { .type = type };
        s_app_cb(&evt, s_app_cb_arg);
    }
}

static int start_advertising(void);

static int ble_gap_event_handler(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            s_conn_handle = event->connect.conn_handle;
            s_connected = true;
            ESP_LOGI(TAG, "connected, handle=%d", s_conn_handle);

            // Request encryption (triggers bonding)
            ble_gap_security_initiate(s_conn_handle);

            notify_app(HAL_BLE_EVT_CONNECTED);
        } else {
            ESP_LOGW(TAG, "connection failed, status=%d", event->connect.status);
            s_connected = false;
            // Do NOT auto-advertise here — let the state machine decide.
            // The app callback will receive DISCONNECTED and determine
            // whether to re-advertise or enter deep sleep.
            notify_app(HAL_BLE_EVT_DISCONNECTED);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnected, reason=%d", event->disconnect.reason);
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_connected = false;
        notify_app(HAL_BLE_EVT_DISCONNECTED);
        // Do NOT auto-advertise — state machine controls advertising lifecycle
        break;

    case BLE_GAP_EVENT_ENC_CHANGE:
        if (event->enc_change.status == 0) {
            ESP_LOGI(TAG, "encryption enabled (bonded)");
            notify_app(HAL_BLE_EVT_BOND_RESTORED);
        } else {
            // Bond asymmetry: host deleted the bond but ring still has it.
            // Delete stale bond and re-advertise for fresh pairing.
            ESP_LOGW(TAG, "encryption failed (status=%d), clearing stale bond",
                     event->enc_change.status);
            // Look up the actual peer address — a zeroed address would fail
            // to match and leave the stale bond in place, causing a
            // permanent reconnection failure loop.
            struct ble_gap_conn_desc enc_desc;
            if (ble_gap_conn_find(s_conn_handle, &enc_desc) == 0) {
                ble_store_util_delete_peer(&enc_desc.peer_id_addr);
            } else {
                // Stale handle — delete all bonds as fallback
                ESP_LOGW(TAG, "conn_find failed, deleting all bonds");
                ble_store_util_delete_all();
            }
            ble_gap_terminate(s_conn_handle, BLE_ERR_AUTH_FAIL);
            notify_app(HAL_BLE_EVT_BOND_FAILED);
        }
        break;

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
        // Delete old bond and allow re-pairing
        struct ble_gap_conn_desc desc;
        ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        ble_store_util_delete_peer(&desc.peer_id_addr);
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    case BLE_GAP_EVENT_CONN_UPDATE:
        if (event->conn_update.status == 0) {
            struct ble_gap_conn_desc desc;
            ble_gap_conn_find(event->conn_update.conn_handle, &desc);
            ESP_LOGI(TAG, "conn params updated: interval=%d",
                     desc.conn_itvl);
            if (s_app_cb) {
                hal_ble_event_data_t evt = {
                    .type = HAL_BLE_EVT_CONN_PARAMS_UPDATED,
                    .data.conn_params.conn_interval_1_25ms = desc.conn_itvl,
                };
                s_app_cb(&evt, s_app_cb_arg);
            }
        } else {
            ESP_LOGW(TAG, "conn param update rejected");
            notify_app(HAL_BLE_EVT_CONN_PARAMS_REJECTED);
        }
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "advertising complete");
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "subscribe event: cur_notify=%d",
                 event->subscribe.cur_notify);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU updated: %d", event->mtu.value);
        break;

    default:
        break;
    }

    return 0;
}

// --- Advertising ---

// Returns 0 on success, NimBLE error code on failure
static int start_advertising(void)
{
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        .itvl_min = 0x20,   // 20ms
        .itvl_max = 0x40,   // 40ms
    };

    struct ble_hs_adv_fields fields = { 0 };
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)s_device_name;
    fields.name_len = strlen(s_device_name);
    fields.name_is_complete = 1;

    // Advertise HID service UUID (0x1812) so hub can find us
    ble_uuid16_t hid_uuid = BLE_UUID16_INIT(0x1812);
    fields.uuids16 = &hid_uuid;
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    // Appearance: HID Mouse (0x03C2)
    struct ble_hs_adv_fields rsp_fields = { 0 };
    rsp_fields.appearance = 0x03C2;
    rsp_fields.appearance_is_present = 1;
    ble_gap_adv_rsp_set_fields(&rsp_fields);

    int32_t timeout = (s_adv_timeout_ms > 0)
        ? (int32_t)(s_adv_timeout_ms / 10)  // BLE duration units = 10ms
        : BLE_HS_FOREVER;

    int rc = ble_gap_adv_start(
        BLE_OWN_ADDR_PUBLIC, NULL, timeout,
        &adv_params, ble_gap_event_handler, NULL
    );
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "advertising start failed: %d", rc);
    }
    return (rc == BLE_HS_EALREADY) ? 0 : rc;
}

// --- NimBLE host callbacks ---

static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "BLE host synced");

    // Use best available address
    int rc = ble_hs_util_ensure_addr(0);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_hs_util_ensure_addr failed: %d", rc);
        return;
    }

    // Do NOT auto-advertise — the state machine controls advertising.
    // ble_on_sync is called both at initial boot (when the state machine
    // will already be in ADVERTISING and will call start_advertising via
    // action flags) and after a NimBLE host reset (where auto-advertising
    // would bypass the state machine's sleep/low-battery decisions).
}

static void ble_on_reset(int reason)
{
    ESP_LOGE(TAG, "BLE host reset, reason=%d", reason);

    // If we were connected, the state machine doesn't know the link is gone.
    // Deliver a synthetic disconnect so it can transition correctly.
    if (s_connected) {
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_connected = false;
        notify_app(HAL_BLE_EVT_DISCONNECTED);
    }
}

static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// --- Public API ---

hal_status_t hal_ble_init(const char *device_name, hal_ble_event_cb_t cb, void *arg)
{
    s_app_cb = cb;
    s_app_cb_arg = arg;
    if (device_name) {
        s_device_name = device_name;
    }

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    // Configure NimBLE host
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Security: bonding, no MITM (NoInputNoOutput), secure connections preferred
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;

    // Register GATT services
    ble_svc_gap_init();
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(s_gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
        return HAL_ERR_IO;
    }
    rc = ble_gatts_add_svcs(s_gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
        return HAL_ERR_IO;
    }

    // Set device name
    ble_svc_gap_device_name_set(s_device_name);

    // Set appearance: HID Mouse
    ble_svc_gap_device_appearance_set(0x03C2);

    // Initialize NimBLE bond store
    ble_store_config_init();

    // Start NimBLE host task
    nimble_port_freertos_init(ble_host_task);

    ESP_LOGI(TAG, "BLE HID initialized as '%s'", s_device_name);
    return HAL_OK;
}

hal_status_t hal_ble_start_advertising(uint32_t timeout_ms)
{
    s_adv_timeout_ms = timeout_ms;
    int rc = start_advertising();
    return (rc == 0) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_ble_stop_advertising(void)
{
    int rc = ble_gap_adv_stop();
    return (rc == 0 || rc == BLE_HS_EALREADY) ? HAL_OK : HAL_ERR_IO;
}

hal_status_t hal_ble_send_mouse_report(const hal_hid_mouse_report_t *report)
{
    if (!s_connected || s_conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        return HAL_ERR_BUSY;
    }
    if (!report) return HAL_ERR_INVALID_ARG;
    if (s_hid_report_handle == 0) return HAL_ERR_IO;

    // Build 4-byte HID report: [buttons, dx, dy, wheel]
    uint8_t buf[4] = {
        report->buttons,
        (uint8_t)report->dx,
        (uint8_t)report->dy,
        (uint8_t)report->wheel,
    };

    struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, sizeof(buf));
    if (!om) return HAL_ERR_NO_MEM;

    int rc = ble_gatts_notify_custom(s_conn_handle, s_hid_report_handle, om);
    if (rc != 0) {
        ESP_LOGW(TAG, "HID notify failed: %d", rc);
        return HAL_ERR_IO;
    }

    return HAL_OK;
}

hal_status_t hal_ble_request_conn_params(uint16_t min_1_25ms, uint16_t max_1_25ms)
{
    if (!s_connected) return HAL_ERR_BUSY;

    struct ble_gap_upd_params params = {
        .itvl_min = min_1_25ms,
        .itvl_max = max_1_25ms,
        .latency = 0,
        .supervision_timeout = 400,  // 4 seconds
        .min_ce_len = 0,
        .max_ce_len = 0,
    };

    int rc = ble_gap_update_params(s_conn_handle, &params);
    if (rc != 0) {
        ESP_LOGW(TAG, "conn param update request failed: %d", rc);
        return HAL_ERR_REJECTED;
    }
    return HAL_OK;
}

hal_status_t hal_ble_delete_all_bonds(void)
{
    int rc = ble_store_util_delete_all();
    return (rc == 0) ? HAL_OK : HAL_ERR_IO;
}

bool hal_ble_is_connected(void)
{
    return s_connected;
}
