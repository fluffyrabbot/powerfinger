// SPDX-License-Identifier: MIT
// PowerFinger Hub — BLE central implementation
//
// Scans for PowerFinger ring peripherals advertising the HID service UUID
// (0x1812), connects, pairs, bonds, and subscribes to HID report
// notifications. Forwards incoming reports to the event composer.

#include "ble_central.h"
#include "hal_types.h"

#ifdef ESP_PLATFORM

#include "esp_log.h"
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

static const char *TAG = "ble_central";

// Callbacks
static hub_ring_report_cb_t s_report_cb = NULL;
static hub_ring_conn_cb_t s_conn_cb = NULL;
static void *s_cb_arg = NULL;

// Per-ring connection state
typedef struct {
    uint16_t conn_handle;
    uint16_t hid_report_handle;     // GATT characteristic value handle for HID report
    uint16_t hid_cccd_handle;       // CCCD handle for enabling notifications
    uint8_t  mac[6];
    bool     connected;
    bool     subscribed;
} ring_conn_t;

static ring_conn_t s_rings[HUB_MAX_RINGS];
static uint8_t s_connected_count = 0;

// --- Helpers ---

static int find_ring_by_conn(uint16_t conn_handle)
{
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected && s_rings[i].conn_handle == conn_handle) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void)
{
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        if (!s_rings[i].connected) return i;
    }
    return -1;
}

// --- Forward declarations ---
static void start_scan(void);
static int gap_event_handler(struct ble_gap_event *event, void *arg);

// --- GATT discovery callbacks ---

// Called when we discover characteristics on a connected ring
static int on_disc_chr(uint16_t conn_handle,
                       const struct ble_gatt_error *error,
                       const struct ble_gatt_chr *chr, void *arg)
{
    int ring_idx = (int)(intptr_t)arg;
    if (ring_idx < 0 || ring_idx >= HUB_MAX_RINGS) return 0;

    if (error->status == 0 && chr != NULL) {
        // Look for HID Report characteristic (UUID 0x2A4D)
        if (ble_uuid_cmp(&chr->uuid.u, BLE_UUID16_DECLARE(0x2A4D)) == 0) {
            s_rings[ring_idx].hid_report_handle = chr->val_handle;
            // CCCD is at val_handle + 1 for standard GATT
            s_rings[ring_idx].hid_cccd_handle = chr->val_handle + 1;
            ESP_LOGI(TAG, "ring %d: found HID Report handle=%d",
                     ring_idx, chr->val_handle);
        }
    } else if (error->status == BLE_HS_EDONE) {
        // Discovery complete — subscribe to notifications
        if (s_rings[ring_idx].hid_cccd_handle != 0) {
            uint8_t cccd_val[2] = { 0x01, 0x00 };  // enable notifications
            ble_gattc_write_flat(
                s_rings[ring_idx].conn_handle,
                s_rings[ring_idx].hid_cccd_handle,
                cccd_val, sizeof(cccd_val),
                NULL, NULL
            );
            s_rings[ring_idx].subscribed = true;
            ESP_LOGI(TAG, "ring %d: subscribed to HID notifications", ring_idx);
        }
    }
    return 0;
}

// --- GAP event handler ---

static int gap_event_handler(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_DISC: {
        // Check if this is a PowerFinger device (advertises HID service 0x1812)
        struct ble_hs_adv_fields fields;
        if (ble_hs_adv_parse_fields(&fields, event->disc.data,
                                     event->disc.length_data) != 0) {
            break;
        }

        bool is_powerfinger = false;
        for (int i = 0; i < fields.num_uuids16; i++) {
            if (ble_uuid_u16(&fields.uuids16[i].u) == 0x1812) {
                is_powerfinger = true;
                break;
            }
        }

        if (!is_powerfinger) break;

        // Check if we have a free slot
        int slot = find_free_slot();
        if (slot < 0) {
            ESP_LOGW(TAG, "max rings connected, ignoring discovery");
            break;
        }

        // Stop scanning and connect
        ble_gap_disc_cancel();

        ESP_LOGI(TAG, "found PowerFinger device, connecting...");
        ble_gap_connect(
            BLE_OWN_ADDR_PUBLIC,
            &event->disc.addr,
            30000,  // 30s connection timeout
            NULL,   // default connection params
            gap_event_handler,
            NULL
        );
        break;
    }

    case BLE_GAP_EVENT_CONNECT: {
        if (event->connect.status != 0) {
            ESP_LOGW(TAG, "connection failed, status=%d", event->connect.status);
            start_scan();
            break;
        }

        int slot = find_free_slot();
        if (slot < 0) break;

        s_rings[slot].conn_handle = event->connect.conn_handle;
        s_rings[slot].connected = true;
        s_rings[slot].subscribed = false;
        s_rings[slot].hid_report_handle = 0;
        s_rings[slot].hid_cccd_handle = 0;
        s_connected_count++;

        // Get peer address
        struct ble_gap_conn_desc desc;
        ble_gap_conn_find(event->connect.conn_handle, &desc);
        memcpy(s_rings[slot].mac, desc.peer_id_addr.val, 6);

        ESP_LOGI(TAG, "ring %d connected (handle=%d), discovering services...",
                 slot, event->connect.conn_handle);

        // Initiate security (bonding)
        ble_gap_security_initiate(event->connect.conn_handle);

        // Discover HID service characteristics
        ble_gattc_disc_all_chrs(
            event->connect.conn_handle,
            1, 0xFFFF,
            on_disc_chr,
            (void *)(intptr_t)slot
        );

        if (s_conn_cb) {
            s_conn_cb((uint8_t)slot, true, s_cb_arg);
        }

        // Resume scanning for more rings
        start_scan();
        break;
    }

    case BLE_GAP_EVENT_DISCONNECT: {
        int idx = find_ring_by_conn(event->disconnect.conn.conn_handle);
        if (idx >= 0) {
            ESP_LOGI(TAG, "ring %d disconnected, reason=%d",
                     idx, event->disconnect.reason);
            s_rings[idx].connected = false;
            s_rings[idx].subscribed = false;
            s_connected_count--;

            if (s_conn_cb) {
                s_conn_cb((uint8_t)idx, false, s_cb_arg);
            }
        }

        // Resume scanning
        start_scan();
        break;
    }

    case BLE_GAP_EVENT_NOTIFY_RX: {
        // Incoming HID report notification from a ring
        int idx = find_ring_by_conn(event->notify_rx.conn_handle);
        if (idx < 0) break;

        // Parse 4-byte HID report: [buttons, dx, dy, wheel]
        if (OS_MBUF_PKTLEN(event->notify_rx.om) >= 4) {
            uint8_t buf[4];
            os_mbuf_copydata(event->notify_rx.om, 0, 4, buf);

            hub_ring_report_t report = {
                .buttons = buf[0],
                .dx = (int8_t)buf[1],
                .dy = (int8_t)buf[2],
                .wheel = (int8_t)buf[3],
            };

            if (s_report_cb) {
                s_report_cb((uint8_t)idx, &report, s_cb_arg);
            }
        }
        break;
    }

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
        struct ble_gap_conn_desc desc;
        ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
        ble_store_util_delete_peer(&desc.peer_id_addr);
        return BLE_GAP_REPEAT_PAIRING_RETRY;
    }

    case BLE_GAP_EVENT_ENC_CHANGE:
        if (event->enc_change.status == 0) {
            ESP_LOGI(TAG, "encryption enabled for handle=%d",
                     event->enc_change.conn_handle);
        } else {
            ESP_LOGW(TAG, "encryption failed for handle=%d, status=%d",
                     event->enc_change.conn_handle, event->enc_change.status);
        }
        break;

    default:
        break;
    }

    return 0;
}

// --- Scanning ---

static void start_scan(void)
{
    if (s_connected_count >= HUB_MAX_RINGS) {
        ESP_LOGI(TAG, "all slots full, not scanning");
        return;
    }

    struct ble_gap_disc_params scan_params = {
        .itvl = 0x50,          // 50ms scan interval
        .window = 0x30,        // 30ms scan window
        .filter_policy = BLE_HCI_SCAN_FILT_NO_WL,
        .limited = 0,
        .passive = 0,          // active scan to get scan response
        .filter_duplicates = 1,
    };

    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER,
                          &scan_params, gap_event_handler, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "scan start failed: %d", rc);
    }
}

// --- NimBLE host callbacks ---

static void on_sync(void)
{
    ESP_LOGI(TAG, "BLE host synced, starting scan");
    ble_hs_util_ensure_addr(0);
    start_scan();
}

static void on_reset(int reason)
{
    ESP_LOGE(TAG, "BLE host reset, reason=%d, clearing all ring state", reason);

    // NimBLE host reset means all connections are gone. Clear all ring state
    // and notify the app so the event composer releases stuck buttons.
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected) {
            s_rings[i].connected = false;
            s_rings[i].subscribed = false;
            s_connected_count--;
            if (s_conn_cb) {
                s_conn_cb((uint8_t)i, false, s_cb_arg);
            }
        }
    }
    s_connected_count = 0;
}

static void host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

#endif // ESP_PLATFORM

// --- Public API ---

hal_status_t ble_central_init(hub_ring_report_cb_t report_cb,
                              hub_ring_conn_cb_t conn_cb,
                              void *arg)
{
    s_report_cb = report_cb;
    s_conn_cb = conn_cb;
    s_cb_arg = arg;

#ifdef ESP_PLATFORM
    memset(s_rings, 0, sizeof(s_rings));
    s_connected_count = 0;

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nimble_port_init failed: %s", esp_err_to_name(ret));
        return HAL_ERR_IO;
    }

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Security: bonding, no MITM, secure connections
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_store_config_init();

    nimble_port_freertos_init(host_task);

    ESP_LOGI(TAG, "BLE central initialized, max %d rings", HUB_MAX_RINGS);
#endif

    return HAL_OK;
}

hal_status_t ble_central_get_mac(uint8_t ring_index, uint8_t mac_out[6])
{
#ifdef ESP_PLATFORM
    if (ring_index >= HUB_MAX_RINGS) return HAL_ERR_INVALID_ARG;
    if (!s_rings[ring_index].connected) return HAL_ERR_NOT_FOUND;
    memcpy(mac_out, s_rings[ring_index].mac, 6);
    return HAL_OK;
#else
    (void)ring_index;
    memset(mac_out, 0, 6);
    return HAL_ERR_NOT_FOUND;
#endif
}

uint8_t ble_central_connected_count(void)
{
#ifdef ESP_PLATFORM
    return s_connected_count;
#else
    return 0;
#endif
}
