// SPDX-License-Identifier: MIT
// PowerFinger Hub — BLE central implementation
//
// Scans for PowerFinger ring peripherals advertising the HID service UUID
// (0x1812) plus a PowerFinger-specific service-data marker, connects, pairs,
// bonds, and subscribes to HID report notifications. Forwards incoming reports
// to the event composer.

#include "ble_central.h"
#include "hal_types.h"
#include "hal_timer.h"

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
#include "freertos/FreeRTOS.h"

#include <string.h>

static const char *TAG = "ble_central";
// BLE timing constants
// 30s gives slow hosts time to complete the connection procedure.
#define BLE_CONNECT_TIMEOUT_MS    30000

// Scan interval and window in BLE units (0.625ms each).
// 50ms interval / 30ms window = 60% duty cycle.
// Aggressive enough to find rings quickly; conservative enough not to
// starve BLE connections already in flight.
#define BLE_SCAN_INTERVAL         0x50   // 50ms
#define BLE_SCAN_WINDOW           0x30   // 30ms

// Descriptor discovery range for HID Report characteristic.
// Covers CCCD (0x2902) + Report Reference (0x2908) + margin.
// PowerFinger rings have exactly 2 descriptors; 8 handles is generous.
#define HID_DSC_SEARCH_RANGE      8

// Spinlock protecting s_rings[] and s_connected_count from concurrent
// reads on the app core (ble_central_get_mac, ble_central_connected_count)
// while the NimBLE host task on the other core writes during GAP events.
static portMUX_TYPE s_rings_lock = portMUX_INITIALIZER_UNLOCKED;
#define RINGS_LOCK()   portENTER_CRITICAL(&s_rings_lock)
#define RINGS_UNLOCK() portEXIT_CRITICAL(&s_rings_lock)

// Callbacks
static hub_ring_report_cb_t s_report_cb = NULL;
static hub_ring_conn_cb_t s_conn_cb = NULL;
static void *s_cb_arg = NULL;

// H8: timeout for GATT discovery. If a ring is connected but not subscribed
// within this window, disconnect it and let rescan/reconnect retry.
// 10s is generous: typical discovery + CCCD write completes in <2s.
#define GATT_DISCOVERY_TIMEOUT_MS 10000

// PowerFinger discovery marker carried in the HID Service Data AD field.
// Layout: [0x12, 0x18, 'P', 'F', 'R', 0x01]
//         ^ HID service UUID LE ^ magic/version payload
// This lets the hub reject generic BLE HID mice without depending on a
// spoofable local-name match or inventing a fake manufacturer company ID.
static const uint8_t POWERFINGER_SERVICE_DATA[] = {
    0x12, 0x18, 'P', 'F', 'R', 0x01,
};

// Per-ring connection state
typedef struct {
    uint16_t conn_handle;
    uint16_t hid_report_handle;     // GATT characteristic value handle for HID report
    uint16_t hid_cccd_handle;       // CCCD handle for enabling notifications
    uint8_t  mac[6];
    bool     connected;
    bool     subscribed;
    uint32_t connect_time_ms;       // H8: timestamp for discovery timeout
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

static bool adv_has_powerfinger_identity(const struct ble_hs_adv_fields *fields)
{
    return fields->svc_data_uuid16 != NULL &&
           fields->svc_data_uuid16_len == sizeof(POWERFINGER_SERVICE_DATA) &&
           memcmp(fields->svc_data_uuid16,
                  POWERFINGER_SERVICE_DATA,
                  sizeof(POWERFINGER_SERVICE_DATA)) == 0;
}

// --- Forward declarations ---
static void start_scan(void);
static int gap_event_handler(struct ble_gap_event *event, void *arg);
static int on_disc_dsc(uint16_t conn_handle, const struct ble_gatt_error *error,
                       uint16_t chr_val_handle, const struct ble_gatt_dsc *dsc,
                       void *arg);

// --- GATT discovery callbacks ---

// Called when descriptor discovery for the HID Report characteristic completes.
// Looks for CCCD (0x2902) and writes it to enable notifications.
static int on_disc_dsc(uint16_t conn_handle,
                       const struct ble_gatt_error *error,
                       uint16_t chr_val_handle,
                       const struct ble_gatt_dsc *dsc,
                       void *arg)
{
    (void)chr_val_handle;
    int ring_idx = (int)(intptr_t)arg;
    if (ring_idx < 0 || ring_idx >= HUB_MAX_RINGS) return 0;

    // C2 fix: verify conn_handle still matches this ring's slot.
    // If the ring disconnected and another ring claimed this slot before
    // the callback fired, the conn_handle will differ — abort silently.
    RINGS_LOCK();
    bool slot_valid = s_rings[ring_idx].connected &&
                      s_rings[ring_idx].conn_handle == conn_handle;
    RINGS_UNLOCK();
    if (!slot_valid) return 0;

    if (error->status == 0 && dsc != NULL) {
        if (ble_uuid_cmp(&dsc->uuid.u, BLE_UUID16_DECLARE(0x2902)) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].hid_cccd_handle = dsc->handle;
            RINGS_UNLOCK();
            ESP_LOGI(TAG, "ring %d: found CCCD at handle=%d", ring_idx, dsc->handle);
        }
    } else if (error->status == BLE_HS_EDONE) {
        RINGS_LOCK();
        uint16_t cccd_handle      = s_rings[ring_idx].hid_cccd_handle;
        uint16_t ring_conn_handle = s_rings[ring_idx].conn_handle;
        RINGS_UNLOCK();

        if (cccd_handle != 0) {
            // C1 fix: check CCCD write result. Only mark subscribed on
            // success. A failed write means no HID notifications — the
            // ring would appear connected but produce no input.
            uint8_t cccd_val[2] = { 0x01, 0x00 };
            int rc = ble_gattc_write_flat(ring_conn_handle, cccd_handle,
                                          cccd_val, sizeof(cccd_val), NULL, NULL);
            if (rc == 0) {
                RINGS_LOCK();
                s_rings[ring_idx].subscribed = true;
                RINGS_UNLOCK();
                ESP_LOGI(TAG, "ring %d: subscribed to HID notifications", ring_idx);
            } else {
                ESP_LOGE(TAG, "ring %d: CCCD write failed (rc=%d) — "
                         "HID notifications will not arrive, disconnecting",
                         ring_idx, rc);
                ble_gap_terminate(ring_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
            }
        } else {
            ESP_LOGW(TAG, "ring %d: CCCD not found in descriptor search range — "
                     "HID notifications disabled, disconnecting", ring_idx);
            RINGS_LOCK();
            uint16_t ring_conn_handle2 = s_rings[ring_idx].conn_handle;
            RINGS_UNLOCK();
            ble_gap_terminate(ring_conn_handle2, BLE_ERR_REM_USER_CONN_TERM);
        }
    }
    return 0;
}

// Called for each characteristic discovered on a connected ring.
static int on_disc_chr(uint16_t conn_handle,
                       const struct ble_gatt_error *error,
                       const struct ble_gatt_chr *chr, void *arg)
{
    int ring_idx = (int)(intptr_t)arg;
    if (ring_idx < 0 || ring_idx >= HUB_MAX_RINGS) return 0;

    // C2 fix: verify conn_handle still matches this ring's slot.
    RINGS_LOCK();
    bool slot_valid = s_rings[ring_idx].connected &&
                      s_rings[ring_idx].conn_handle == conn_handle;
    RINGS_UNLOCK();
    if (!slot_valid) return 0;

    if (error->status == 0 && chr != NULL) {
        // Look for HID Report characteristic (UUID 0x2A4D)
        if (ble_uuid_cmp(&chr->uuid.u, BLE_UUID16_DECLARE(0x2A4D)) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].hid_report_handle = chr->val_handle;
            RINGS_UNLOCK();
            ESP_LOGI(TAG, "ring %d: found HID Report handle=%d",
                     ring_idx, chr->val_handle);
        }
    } else if (error->status == BLE_HS_EDONE) {
        // Characteristic discovery complete. Initiate descriptor discovery to
        // find the CCCD handle for the HID Report characteristic.
        // Searching [val_handle, val_handle+HID_DSC_SEARCH_RANGE] covers all
        // practical HID descriptor configurations without needing the next
        // characteristic's handle. val_handle+1 assumption is not used.
        RINGS_LOCK();
        uint16_t report_handle = s_rings[ring_idx].hid_report_handle;
        RINGS_UNLOCK();

        if (report_handle != 0) {
            // M1: check return; on failure the ring will hit the H8
            // discovery timeout and be disconnected.
            int dsc_rc = ble_gattc_disc_all_dscs(
                conn_handle,
                report_handle,
                report_handle + HID_DSC_SEARCH_RANGE,
                on_disc_dsc,
                (void *)(intptr_t)ring_idx
            );
            if (dsc_rc != 0) {
                ESP_LOGE(TAG, "ring %d: descriptor discovery failed (rc=%d)",
                         ring_idx, dsc_rc);
            }
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

        bool has_hid_service = false;
        for (int i = 0; i < fields.num_uuids16; i++) {
            if (ble_uuid_u16(&fields.uuids16[i].u) == 0x1812) {
                has_hid_service = true;
                break;
            }
        }

        if (!has_hid_service || !adv_has_powerfinger_identity(&fields)) {
            break;
        }

        // Check if we have a free slot
        int slot = find_free_slot();
        if (slot < 0) {
            ESP_LOGW(TAG, "max rings connected, ignoring discovery");
            break;
        }

        // Stop scanning and connect
        ble_gap_disc_cancel();

        ESP_LOGI(TAG, "found PowerFinger device, connecting...");
        int rc = ble_gap_connect(
            BLE_OWN_ADDR_PUBLIC,
            &event->disc.addr,
            BLE_CONNECT_TIMEOUT_MS,
            NULL,   // default connection params
            gap_event_handler,
            NULL
        );
        if (rc != 0) {
            ESP_LOGW(TAG, "ble_gap_connect failed: %d (will retry on next scan)", rc);
            start_scan();
        }
        break;
    }

    case BLE_GAP_EVENT_CONNECT: {
        if (event->connect.status != 0) {
            ESP_LOGW(TAG, "connection failed, status=%d", event->connect.status);
            start_scan();
            break;
        }

        // Resolve peer MAC before acquiring lock (ble_gap_conn_find may block)
        struct ble_gap_conn_desc desc;
        bool got_desc = (ble_gap_conn_find(event->connect.conn_handle, &desc) == 0);
        if (!got_desc) {
            ESP_LOGW(TAG, "conn_find failed for handle=%d", event->connect.conn_handle);
        }

        // Atomically claim a slot and record state
        RINGS_LOCK();
        int slot = find_free_slot();
        if (slot >= 0) {
            s_rings[slot].conn_handle = event->connect.conn_handle;
            s_rings[slot].connected = true;
            s_rings[slot].subscribed = false;
            s_rings[slot].hid_report_handle = 0;
            s_rings[slot].hid_cccd_handle = 0;
            s_rings[slot].connect_time_ms = hal_timer_get_ms();
            if (got_desc) {
                memcpy(s_rings[slot].mac, desc.peer_id_addr.val, 6);
            } else {
                memset(s_rings[slot].mac, 0, 6);
            }
            s_connected_count++;
        }
        RINGS_UNLOCK();

        if (slot < 0) break;

        ESP_LOGI(TAG, "ring %d connected (handle=%d), discovering services...",
                 slot, event->connect.conn_handle);

        // Initiate security (bonding) — outside lock, may yield.
        // H3: log on failure; connection can proceed without encryption
        // but some centrals reject unencrypted HID.
        {
            int sec_rc = ble_gap_security_initiate(event->connect.conn_handle);
            if (sec_rc != 0) {
                ESP_LOGW(TAG, "ring %d: security initiate failed (rc=%d), "
                         "proceeding without encryption", slot, sec_rc);
            }
        }

        // Discover HID service characteristics — outside lock.
        // M2: check return; on failure the ring will hit the H8 discovery
        // timeout and be disconnected.
        {
            int chr_rc = ble_gattc_disc_all_chrs(
                event->connect.conn_handle,
                1, 0xFFFF,
                on_disc_chr,
                (void *)(intptr_t)slot
            );
            if (chr_rc != 0) {
                ESP_LOGE(TAG, "ring %d: characteristic discovery failed (rc=%d)",
                         slot, chr_rc);
            }
        }

        if (s_conn_cb) {
            s_conn_cb((uint8_t)slot, true, s_cb_arg);
        }

        // Resume scanning for more rings
        start_scan();
        break;
    }

    case BLE_GAP_EVENT_DISCONNECT: {
        RINGS_LOCK();
        int idx = find_ring_by_conn(event->disconnect.conn.conn_handle);
        if (idx >= 0) {
            s_rings[idx].connected = false;
            s_rings[idx].subscribed = false;
            s_connected_count--;
        }
        RINGS_UNLOCK();

        if (idx >= 0) {
            ESP_LOGI(TAG, "ring %d disconnected, reason=%d",
                     idx, event->disconnect.reason);
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
        RINGS_LOCK();
        int idx = find_ring_by_conn(event->notify_rx.conn_handle);
        RINGS_UNLOCK();
        if (idx < 0) break;

        // Parse 4-byte HID report: [buttons, dx, dy, wheel]
        if (OS_MBUF_PKTLEN(event->notify_rx.om) < 4) {
            ESP_LOGW(TAG, "ring %d: short HID report (%d bytes, expected 4)",
                     idx, OS_MBUF_PKTLEN(event->notify_rx.om));
            break;
        }

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
        break;
    }

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
        struct ble_gap_conn_desc desc;
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0) {
            ble_store_util_delete_peer(&desc.peer_id_addr);
        } else {
            ESP_LOGW(TAG, "repeat pairing: conn_find failed for handle=%d",
                     event->repeat_pairing.conn_handle);
        }
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
    // M3: read s_connected_count under lock for dual-core consistency
    RINGS_LOCK();
    uint8_t count = s_connected_count;
    RINGS_UNLOCK();
    if (count >= HUB_MAX_RINGS) {
        ESP_LOGI(TAG, "all slots full, not scanning");
        return;
    }

    struct ble_gap_disc_params scan_params = {
        .itvl = BLE_SCAN_INTERVAL,
        .window = BLE_SCAN_WINDOW,
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

    // NimBLE host reset means all connections are gone. Snapshot which rings
    // were connected under the lock, then fire callbacks outside it.
    // on_sync() will restart scanning after the host reinitializes.
    bool was_connected[HUB_MAX_RINGS];

    RINGS_LOCK();
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        was_connected[i] = s_rings[i].connected;
        s_rings[i].connected = false;
        s_rings[i].subscribed = false;
    }
    // H6: recompute count from array state rather than unconditionally
    // setting to 0. Prevents underflow if a concurrent disconnect handler
    // on the other core already decremented count for a ring we just cleared.
    s_connected_count = 0;
    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected) s_connected_count++;
    }
    RINGS_UNLOCK();

    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        if (was_connected[i] && s_conn_cb) {
            s_conn_cb((uint8_t)i, false, s_cb_arg);
        }
    }
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
    RINGS_LOCK();
    if (!s_rings[ring_index].connected) {
        RINGS_UNLOCK();
        return HAL_ERR_NOT_FOUND;
    }
    memcpy(mac_out, s_rings[ring_index].mac, 6);
    RINGS_UNLOCK();
    return HAL_OK;
#else
    (void)ring_index;
    memset(mac_out, 0, 6);
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_find_ring_index_by_mac(const uint8_t mac[6],
                                                uint8_t *ring_index_out)
{
    if (!mac || !ring_index_out) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    RINGS_LOCK();
    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected && memcmp(s_rings[i].mac, mac, 6) == 0) {
            *ring_index_out = i;
            RINGS_UNLOCK();
            return HAL_OK;
        }
    }
    RINGS_UNLOCK();
#else
    (void)mac;
#endif

    return HAL_ERR_NOT_FOUND;
}

hal_status_t ble_central_disconnect_ring_by_mac(const uint8_t mac[6])
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    uint16_t conn_handle = 0;
    bool found = false;

    RINGS_LOCK();
    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected && memcmp(s_rings[i].mac, mac, 6) == 0) {
            conn_handle = s_rings[i].conn_handle;
            found = true;
            break;
        }
    }
    RINGS_UNLOCK();

    if (!found) {
        return HAL_ERR_NOT_FOUND;
    }

    int rc = ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    if (rc == 0 || rc == BLE_HS_EALREADY) {
        return HAL_OK;
    }

    ESP_LOGW(TAG, "disconnect by MAC failed: rc=%d", rc);
    return HAL_ERR_IO;
#else
    (void)mac;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_delete_bond_by_mac(const uint8_t mac[6])
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    // The current ring firmware advertises in public-address mode, so the MAC
    // from the role engine is sufficient to target the bond entry pre-hardware.
    // Revisit this path if privacy/random addresses are introduced.
    ble_addr_t peer_addr = {0};
    peer_addr.type = BLE_ADDR_PUBLIC;
    memcpy(peer_addr.val, mac, 6);

    int rc = ble_store_util_delete_peer(&peer_addr);
    if (rc != 0) {
        ESP_LOGW(TAG, "delete bond by MAC failed (public-address assumption): rc=%d", rc);
    }
#else
    (void)mac;
#endif

    return HAL_OK;
}

uint8_t ble_central_connected_count(void)
{
#ifdef ESP_PLATFORM
    RINGS_LOCK();
    uint8_t count = s_connected_count;
    RINGS_UNLOCK();
    return count;
#else
    return 0;
#endif
}

void ble_central_check_discovery_timeout(void)
{
#ifdef ESP_PLATFORM
    uint32_t now = hal_timer_get_ms();

    for (int i = 0; i < HUB_MAX_RINGS; i++) {
        uint16_t ch = 0;
        bool needs_disconnect = false;

        RINGS_LOCK();
        if (s_rings[i].connected && !s_rings[i].subscribed &&
            (now - s_rings[i].connect_time_ms) >= GATT_DISCOVERY_TIMEOUT_MS) {
            ch = s_rings[i].conn_handle;
            needs_disconnect = true;
        }
        RINGS_UNLOCK();

        if (needs_disconnect) {
            ESP_LOGW(TAG, "ring %d: GATT discovery timed out after %dms — disconnecting",
                     i, GATT_DISCOVERY_TIMEOUT_MS);
            ble_gap_terminate(ch, BLE_ERR_REM_USER_CONN_TERM);
        }
    }
#endif
}
