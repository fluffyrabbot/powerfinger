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
#include "freertos/semphr.h"

#include <string.h>

static const char *TAG = "ble_central";

// ESP-IDF ships the implementation but does not expose a public prototype.
void ble_store_config_init(void);

// BLE timing constants
// 30s gives slow hosts time to complete the connection procedure.
#define BLE_CONNECT_TIMEOUT_MS    30000

// Scan interval and window in BLE units (0.625ms each).
// 50ms interval / 30ms window = 60% duty cycle.
// Aggressive enough to find rings quickly; conservative enough not to
// starve BLE connections already in flight.
#define BLE_SCAN_INTERVAL         0x50   // 50ms
#define BLE_SCAN_WINDOW           0x30   // 30ms
// Boot-only scan policy gets one bounded discovery window instead of a
// forever-running background scan.
#define BLE_BOOT_SCAN_DURATION_MS 30000

// Descriptor discovery range for HID Report characteristic.
// Covers CCCD (0x2902) + Report Reference (0x2908) + margin.
// PowerFinger rings have exactly 2 descriptors; 8 handles is generous.
#define HID_DSC_SEARCH_RANGE      8

// Setting relay operations run on the companion-command path and wait for the
// NimBLE callback to complete. Bound the wait so a stalled peripheral does not
// wedge the USB CDC task indefinitely.
#define GATT_RELAY_TIMEOUT_MS     3000
#define GATT_RELAY_READ_MAX_LEN   16

// PowerFinger config UUIDs must match the ring firmware. NimBLE expects
// BLE_UUID128_DECLARE bytes in little-endian order.
#define PF_UUID128_DPI \
    BLE_UUID128_DECLARE(0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0, \
                        0x67, 0x6E, 0x69, 0x72, 0x01, 0x01, 0x46, 0x50)
#define PF_UUID128_DEAD_ZONE_TIME \
    BLE_UUID128_DECLARE(0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0, \
                        0x67, 0x6E, 0x69, 0x72, 0x02, 0x01, 0x46, 0x50)
#define PF_UUID128_DEAD_ZONE_DISTANCE \
    BLE_UUID128_DECLARE(0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0, \
                        0x67, 0x6E, 0x69, 0x72, 0x03, 0x01, 0x46, 0x50)
#define PF_UUID128_FIRMWARE_VERSION \
    BLE_UUID128_DECLARE(0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0, \
                        0x67, 0x6E, 0x69, 0x72, 0x01, 0x02, 0x46, 0x50)
#define PF_UUID128_DIAGNOSTICS \
    BLE_UUID128_DECLARE(0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0, \
                        0x67, 0x6E, 0x69, 0x72, 0x01, 0x04, 0x46, 0x50)

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

// Matches the companion-surface scan-policy enum. Keep duplicated here so the
// BLE central can apply policy without depending on the hub-settings module.
#define BLE_CENTRAL_SCAN_POLICY_BOOT_ONLY  0U
#define BLE_CENTRAL_SCAN_POLICY_CONTINUOUS 1U
#define BLE_CENTRAL_SCAN_POLICY_EXPECTED   2U

typedef enum {
    SCAN_TRIGGER_BOOT = 0,
    SCAN_TRIGGER_CAPACITY_CHANGE,
    SCAN_TRIGGER_SETTINGS_CHANGE,
} scan_trigger_t;

static uint8_t s_scan_policy = BLE_CENTRAL_SCAN_POLICY_CONTINUOUS;
static uint8_t s_expected_rings = 2U;

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
    uint16_t dpi_handle;
    uint16_t dead_zone_time_handle;
    uint16_t dead_zone_distance_handle;
    uint16_t firmware_version_handle;
    uint16_t battery_level_handle;
    uint16_t diagnostics_handle;
    uint8_t  mac[6];
    bool     connected;
    bool     subscribed;
    uint32_t connect_time_ms;       // H8: timestamp for discovery timeout
} ring_conn_t;

typedef struct {
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t done;
    uint32_t token_counter;
    uint32_t active_token;
    uint16_t conn_handle;
    uint16_t attr_handle;
    hal_status_t status;
    uint8_t read_buf[GATT_RELAY_READ_MAX_LEN];
    size_t read_len;
} gatt_relay_state_t;

typedef struct {
    uint16_t conn_handle;
    uint16_t dpi_handle;
    uint16_t dead_zone_time_handle;
    uint16_t dead_zone_distance_handle;
    uint16_t firmware_version_handle;
    uint16_t battery_level_handle;
    uint16_t diagnostics_handle;
    bool subscribed;
} ring_relay_target_t;

static ring_conn_t s_rings[HUB_MAX_RINGS];
static uint8_t s_connected_count = 0;
static gatt_relay_state_t s_gatt_relay = {0};

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

static void clear_ring_slot(ring_conn_t *ring)
{
    if (!ring) {
        return;
    }

    memset(ring, 0, sizeof(*ring));
}

static hal_status_t map_gatt_status(int status)
{
    if (status == 0) {
        return HAL_OK;
    }
    if (status == BLE_HS_ENOTCONN) {
        return HAL_ERR_NOT_FOUND;
    }
    if (status == BLE_HS_EBUSY || status == BLE_HS_EALREADY) {
        return HAL_ERR_BUSY;
    }
    if (status == BLE_HS_ETIMEOUT || status == BLE_HS_ETIMEOUT_HCI) {
        return HAL_ERR_TIMEOUT;
    }
    if (status == BLE_HS_ENOTSUP ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_ATTR_NOT_FOUND) ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_READ_NOT_PERMITTED) ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_WRITE_NOT_PERMITTED) ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_REQ_NOT_SUPPORTED)) {
        return HAL_ERR_NOT_SUPPORTED;
    }
    if (status == BLE_HS_ATT_ERR(BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN) ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_VALUE_NOT_ALLOWED)) {
        return HAL_ERR_INVALID_ARG;
    }
    if (status == BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_AUTHEN) ||
        status == BLE_HS_ATT_ERR(BLE_ATT_ERR_INSUFFICIENT_ENC)) {
        return HAL_ERR_REJECTED;
    }
    return HAL_ERR_IO;
}

static hal_status_t copy_ring_relay_target_by_mac(const uint8_t mac[6],
                                                  ring_relay_target_t *target_out)
{
    if (!mac || !target_out) {
        return HAL_ERR_INVALID_ARG;
    }

    RINGS_LOCK();
    for (uint8_t i = 0; i < HUB_MAX_RINGS; i++) {
        if (s_rings[i].connected && memcmp(s_rings[i].mac, mac, 6) == 0) {
            target_out->conn_handle = s_rings[i].conn_handle;
            target_out->dpi_handle = s_rings[i].dpi_handle;
            target_out->dead_zone_time_handle = s_rings[i].dead_zone_time_handle;
            target_out->dead_zone_distance_handle = s_rings[i].dead_zone_distance_handle;
            target_out->firmware_version_handle = s_rings[i].firmware_version_handle;
            target_out->battery_level_handle = s_rings[i].battery_level_handle;
            target_out->diagnostics_handle = s_rings[i].diagnostics_handle;
            target_out->subscribed = s_rings[i].subscribed;
            RINGS_UNLOCK();
            return HAL_OK;
        }
    }
    RINGS_UNLOCK();

    return HAL_ERR_NOT_FOUND;
}

static hal_status_t decode_ring_diagnostics_payload(const uint8_t *payload,
                                                    size_t payload_len,
                                                    uint8_t battery_pct_fallback,
                                                    bool has_battery_pct_fallback,
                                                    hub_ring_diagnostics_t *diagnostics_out)
{
    if (!payload || !diagnostics_out) {
        return HAL_ERR_INVALID_ARG;
    }
    if (payload_len < 10U) {
        return HAL_ERR_IO;
    }
    if (payload[0] != 1U) {
        return HAL_ERR_NOT_SUPPORTED;
    }

    memset(diagnostics_out, 0, sizeof(*diagnostics_out));
    diagnostics_out->diagnostics_version = payload[0];
    diagnostics_out->ring_state_code = payload[1];
    diagnostics_out->sensor_state = (hub_ring_sensor_state_t)payload[2];
    diagnostics_out->bond_state = (hub_ring_bond_state_t)payload[3];
    diagnostics_out->connected = (payload[4] & 0x01U) != 0;
    diagnostics_out->calibration_valid = (payload[4] & 0x02U) != 0;
    diagnostics_out->conn_param_rejected = (payload[4] & 0x04U) != 0;
    diagnostics_out->battery_pct = has_battery_pct_fallback ? battery_pct_fallback : payload[5];
    diagnostics_out->battery_mv = (uint32_t)payload[6] |
                                  ((uint32_t)payload[7] << 8);
    diagnostics_out->conn_interval_1_25ms = (uint16_t)payload[8] |
                                            ((uint16_t)payload[9] << 8);
    return HAL_OK;
}

static bool adv_has_powerfinger_identity(const struct ble_hs_adv_fields *fields)
{
    return fields->svc_data_uuid16 != NULL &&
           fields->svc_data_uuid16_len == sizeof(POWERFINGER_SERVICE_DATA) &&
           memcmp(fields->svc_data_uuid16,
                  POWERFINGER_SERVICE_DATA,
                  sizeof(POWERFINGER_SERVICE_DATA)) == 0;
}

static bool should_scan_locked(scan_trigger_t trigger)
{
    if (s_connected_count >= HUB_MAX_RINGS) {
        return false;
    }

    switch (s_scan_policy) {
    case BLE_CENTRAL_SCAN_POLICY_BOOT_ONLY:
        return trigger == SCAN_TRIGGER_BOOT;
    case BLE_CENTRAL_SCAN_POLICY_CONTINUOUS:
        return true;
    case BLE_CENTRAL_SCAN_POLICY_EXPECTED:
        return s_connected_count < s_expected_rings;
    default:
        return true;
    }
}

// --- Forward declarations ---
static void start_scan(scan_trigger_t trigger);
static int gap_event_handler(struct ble_gap_event *event, void *arg);
static int on_disc_dsc(uint16_t conn_handle, const struct ble_gatt_error *error,
                       uint16_t chr_val_handle, const struct ble_gatt_dsc *dsc,
                       void *arg);
static int on_subscribe_cccd_write(uint16_t conn_handle,
                                   const struct ble_gatt_error *error,
                                   struct ble_gatt_attr *attr,
                                   void *arg);
static int on_gatt_relay_read(uint16_t conn_handle,
                              const struct ble_gatt_error *error,
                              struct ble_gatt_attr *attr,
                              void *arg);
static int on_gatt_relay_write(uint16_t conn_handle,
                               const struct ble_gatt_error *error,
                               struct ble_gatt_attr *attr,
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
            // Queue the ATT write and only mark the ring subscribed from the
            // completion callback once the procedure actually succeeds.
            uint8_t cccd_val[2] = { 0x01, 0x00 };
            int rc = ble_gattc_write_flat(ring_conn_handle, cccd_handle,
                                          cccd_val, sizeof(cccd_val),
                                          on_subscribe_cccd_write,
                                          (void *)(intptr_t)ring_idx);
            if (rc != 0) {
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

static int on_subscribe_cccd_write(uint16_t conn_handle,
                                   const struct ble_gatt_error *error,
                                   struct ble_gatt_attr *attr,
                                   void *arg)
{
    (void)attr;

    int ring_idx = (int)(intptr_t)arg;
    if (ring_idx < 0 || ring_idx >= HUB_MAX_RINGS) {
        return 0;
    }

    RINGS_LOCK();
    bool slot_valid = s_rings[ring_idx].connected &&
                      s_rings[ring_idx].conn_handle == conn_handle;
    RINGS_UNLOCK();
    if (!slot_valid) {
        return 0;
    }

    if (error->status == 0) {
        RINGS_LOCK();
        s_rings[ring_idx].subscribed = true;
        RINGS_UNLOCK();
        ESP_LOGI(TAG, "ring %d: subscribed to HID notifications", ring_idx);
        return 0;
    }

    ESP_LOGE(TAG, "ring %d: CCCD write completed with error=%d — disconnecting",
             ring_idx, error->status);
    ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
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
        } else if (ble_uuid_cmp(&chr->uuid.u, BLE_UUID16_DECLARE(0x2A19)) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].battery_level_handle = chr->val_handle;
            RINGS_UNLOCK();
        } else if (ble_uuid_cmp(&chr->uuid.u, PF_UUID128_DPI) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].dpi_handle = chr->val_handle;
            RINGS_UNLOCK();
        } else if (ble_uuid_cmp(&chr->uuid.u, PF_UUID128_DEAD_ZONE_TIME) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].dead_zone_time_handle = chr->val_handle;
            RINGS_UNLOCK();
        } else if (ble_uuid_cmp(&chr->uuid.u, PF_UUID128_DEAD_ZONE_DISTANCE) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].dead_zone_distance_handle = chr->val_handle;
            RINGS_UNLOCK();
        } else if (ble_uuid_cmp(&chr->uuid.u, PF_UUID128_FIRMWARE_VERSION) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].firmware_version_handle = chr->val_handle;
            RINGS_UNLOCK();
        } else if (ble_uuid_cmp(&chr->uuid.u, PF_UUID128_DIAGNOSTICS) == 0) {
            RINGS_LOCK();
            s_rings[ring_idx].diagnostics_handle = chr->val_handle;
            RINGS_UNLOCK();
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

static int on_gatt_relay_read(uint16_t conn_handle,
                              const struct ble_gatt_error *error,
                              struct ble_gatt_attr *attr,
                              void *arg)
{
    uint32_t token = (uint32_t)(uintptr_t)arg;
    if (token != s_gatt_relay.active_token ||
        conn_handle != s_gatt_relay.conn_handle ||
        !s_gatt_relay.done) {
        return 0;
    }

    if (error->status != 0) {
        s_gatt_relay.status = map_gatt_status(error->status);
        xSemaphoreGive(s_gatt_relay.done);
        return 0;
    }

    if (!attr || attr->handle != s_gatt_relay.attr_handle || !attr->om) {
        s_gatt_relay.status = HAL_ERR_IO;
        xSemaphoreGive(s_gatt_relay.done);
        return 0;
    }

    size_t read_len = OS_MBUF_PKTLEN(attr->om);
    if (read_len > sizeof(s_gatt_relay.read_buf)) {
        s_gatt_relay.status = HAL_ERR_NO_MEM;
        xSemaphoreGive(s_gatt_relay.done);
        return 0;
    }

    if (os_mbuf_copydata(attr->om, 0, read_len, s_gatt_relay.read_buf) != 0) {
        s_gatt_relay.status = HAL_ERR_IO;
        xSemaphoreGive(s_gatt_relay.done);
        return 0;
    }

    s_gatt_relay.read_len = read_len;
    s_gatt_relay.status = HAL_OK;
    xSemaphoreGive(s_gatt_relay.done);
    return 0;
}

static int on_gatt_relay_write(uint16_t conn_handle,
                               const struct ble_gatt_error *error,
                               struct ble_gatt_attr *attr,
                               void *arg)
{
    uint32_t token = (uint32_t)(uintptr_t)arg;
    (void)attr;

    if (token != s_gatt_relay.active_token ||
        conn_handle != s_gatt_relay.conn_handle ||
        !s_gatt_relay.done) {
        return 0;
    }

    s_gatt_relay.status = map_gatt_status(error->status);
    xSemaphoreGive(s_gatt_relay.done);
    return 0;
}

static hal_status_t ble_central_read_attr(uint16_t conn_handle,
                                          uint16_t attr_handle,
                                          uint8_t *buf_out,
                                          size_t buf_out_len,
                                          size_t *read_len_out)
{
    if (!buf_out || !read_len_out || buf_out_len == 0 || attr_handle == 0) {
        return HAL_ERR_INVALID_ARG;
    }
    if (!s_gatt_relay.mutex || !s_gatt_relay.done) {
        return HAL_ERR_BUSY;
    }

    if (xSemaphoreTake(s_gatt_relay.mutex, pdMS_TO_TICKS(GATT_RELAY_TIMEOUT_MS)) != pdTRUE) {
        return HAL_ERR_BUSY;
    }

    while (xSemaphoreTake(s_gatt_relay.done, 0) == pdTRUE) {
    }

    s_gatt_relay.token_counter++;
    s_gatt_relay.active_token = s_gatt_relay.token_counter;
    s_gatt_relay.conn_handle = conn_handle;
    s_gatt_relay.attr_handle = attr_handle;
    s_gatt_relay.status = HAL_ERR_BUSY;
    s_gatt_relay.read_len = 0;

    int rc = ble_gattc_read(conn_handle,
                            attr_handle,
                            on_gatt_relay_read,
                            (void *)(uintptr_t)s_gatt_relay.active_token);
    if (rc != 0) {
        s_gatt_relay.active_token = 0;
        xSemaphoreGive(s_gatt_relay.mutex);
        return map_gatt_status(rc);
    }

    if (xSemaphoreTake(s_gatt_relay.done, pdMS_TO_TICKS(GATT_RELAY_TIMEOUT_MS)) != pdTRUE) {
        s_gatt_relay.active_token = 0;
        xSemaphoreGive(s_gatt_relay.mutex);
        return HAL_ERR_TIMEOUT;
    }

    hal_status_t status = s_gatt_relay.status;
    if (status == HAL_OK) {
        if (s_gatt_relay.read_len > buf_out_len) {
            status = HAL_ERR_NO_MEM;
        } else {
            memcpy(buf_out, s_gatt_relay.read_buf, s_gatt_relay.read_len);
            *read_len_out = s_gatt_relay.read_len;
        }
    }

    s_gatt_relay.active_token = 0;
    xSemaphoreGive(s_gatt_relay.mutex);
    return status;
}

static hal_status_t ble_central_write_attr(uint16_t conn_handle,
                                           uint16_t attr_handle,
                                           const void *data,
                                           size_t data_len)
{
    if (!data || data_len == 0 || attr_handle == 0) {
        return HAL_ERR_INVALID_ARG;
    }
    if (!s_gatt_relay.mutex || !s_gatt_relay.done) {
        return HAL_ERR_BUSY;
    }

    if (xSemaphoreTake(s_gatt_relay.mutex, pdMS_TO_TICKS(GATT_RELAY_TIMEOUT_MS)) != pdTRUE) {
        return HAL_ERR_BUSY;
    }

    while (xSemaphoreTake(s_gatt_relay.done, 0) == pdTRUE) {
    }

    s_gatt_relay.token_counter++;
    s_gatt_relay.active_token = s_gatt_relay.token_counter;
    s_gatt_relay.conn_handle = conn_handle;
    s_gatt_relay.attr_handle = attr_handle;
    s_gatt_relay.status = HAL_ERR_BUSY;
    s_gatt_relay.read_len = 0;

    int rc = ble_gattc_write_flat(conn_handle,
                                  attr_handle,
                                  data,
                                  data_len,
                                  on_gatt_relay_write,
                                  (void *)(uintptr_t)s_gatt_relay.active_token);
    if (rc != 0) {
        s_gatt_relay.active_token = 0;
        xSemaphoreGive(s_gatt_relay.mutex);
        return map_gatt_status(rc);
    }

    if (xSemaphoreTake(s_gatt_relay.done, pdMS_TO_TICKS(GATT_RELAY_TIMEOUT_MS)) != pdTRUE) {
        s_gatt_relay.active_token = 0;
        xSemaphoreGive(s_gatt_relay.mutex);
        return HAL_ERR_TIMEOUT;
    }

    hal_status_t status = s_gatt_relay.status;
    s_gatt_relay.active_token = 0;
    xSemaphoreGive(s_gatt_relay.mutex);
    return status;
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
            start_scan(SCAN_TRIGGER_CAPACITY_CHANGE);
        }
        break;
    }

    case BLE_GAP_EVENT_CONNECT: {
        if (event->connect.status != 0) {
            ESP_LOGW(TAG, "connection failed, status=%d", event->connect.status);
            start_scan(SCAN_TRIGGER_CAPACITY_CHANGE);
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
            s_rings[slot].dpi_handle = 0;
            s_rings[slot].dead_zone_time_handle = 0;
            s_rings[slot].dead_zone_distance_handle = 0;
            s_rings[slot].firmware_version_handle = 0;
            s_rings[slot].battery_level_handle = 0;
            s_rings[slot].diagnostics_handle = 0;
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
        start_scan(SCAN_TRIGGER_CAPACITY_CHANGE);
        break;
    }

    case BLE_GAP_EVENT_DISCONNECT: {
        RINGS_LOCK();
        int idx = find_ring_by_conn(event->disconnect.conn.conn_handle);
        if (idx >= 0) {
            clear_ring_slot(&s_rings[idx]);
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
        start_scan(SCAN_TRIGGER_CAPACITY_CHANGE);
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

static void start_scan(scan_trigger_t trigger)
{
    bool should_scan = false;
    uint8_t count = 0;
    uint8_t scan_policy = BLE_CENTRAL_SCAN_POLICY_CONTINUOUS;

    RINGS_LOCK();
    count = s_connected_count;
    scan_policy = s_scan_policy;
    should_scan = should_scan_locked(trigger);
    RINGS_UNLOCK();

    if (!should_scan) {
        if (count >= HUB_MAX_RINGS) {
            ESP_LOGI(TAG, "all slots full, not scanning");
        }
        return;
    }

    if (ble_gap_disc_active()) {
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

    int32_t duration_ms = (scan_policy == BLE_CENTRAL_SCAN_POLICY_BOOT_ONLY)
        ? BLE_BOOT_SCAN_DURATION_MS
        : BLE_HS_FOREVER;

    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, duration_ms,
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
    start_scan(SCAN_TRIGGER_BOOT);
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
        clear_ring_slot(&s_rings[i]);
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
    s_scan_policy = BLE_CENTRAL_SCAN_POLICY_CONTINUOUS;
    s_expected_rings = 2U;
    memset(&s_gatt_relay, 0, sizeof(s_gatt_relay));

    s_gatt_relay.mutex = xSemaphoreCreateMutex();
    s_gatt_relay.done = xSemaphoreCreateBinary();
    if (!s_gatt_relay.mutex || !s_gatt_relay.done) {
        if (s_gatt_relay.mutex) {
            vSemaphoreDelete(s_gatt_relay.mutex);
            s_gatt_relay.mutex = NULL;
        }
        if (s_gatt_relay.done) {
            vSemaphoreDelete(s_gatt_relay.done);
            s_gatt_relay.done = NULL;
        }
        ESP_LOGE(TAG, "failed to allocate GATT relay synchronization primitives");
        return HAL_ERR_NO_MEM;
    }

    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        vSemaphoreDelete(s_gatt_relay.mutex);
        vSemaphoreDelete(s_gatt_relay.done);
        s_gatt_relay.mutex = NULL;
        s_gatt_relay.done = NULL;
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
    if (rc == 0 || rc == BLE_HS_ENOENT) {
        return HAL_OK;
    }

    if (rc != 0) {
        ESP_LOGW(TAG, "delete bond by MAC failed (public-address assumption): rc=%d", rc);
        return HAL_ERR_IO;
    }
#else
    (void)mac;
#endif

    return HAL_OK;
}

hal_status_t ble_central_get_ring_settings_by_mac(const uint8_t mac[6],
                                                  hub_ring_settings_t *settings_out)
{
    if (!mac || !settings_out) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }

    if (target.dpi_handle == 0 ||
        target.dead_zone_time_handle == 0 ||
        target.dead_zone_distance_handle == 0 ||
        target.firmware_version_handle == 0) {
        return target.subscribed ? HAL_ERR_NOT_SUPPORTED : HAL_ERR_BUSY;
    }

    uint8_t read_buf[GATT_RELAY_READ_MAX_LEN] = {0};
    size_t read_len = 0;
    hal_status_t rc = ble_central_read_attr(target.conn_handle,
                                            target.dpi_handle,
                                            read_buf,
                                            sizeof(read_buf),
                                            &read_len);
    if (rc != HAL_OK) {
        return rc;
    }
    if (read_len != 1) {
        return HAL_ERR_IO;
    }
    settings_out->dpi_multiplier = read_buf[0];

    rc = ble_central_read_attr(target.conn_handle,
                               target.dead_zone_time_handle,
                               read_buf,
                               sizeof(read_buf),
                               &read_len);
    if (rc != HAL_OK) {
        return rc;
    }
    if (read_len != 2) {
        return HAL_ERR_IO;
    }
    settings_out->dead_zone_time_ms = (uint16_t)read_buf[0] |
                                      ((uint16_t)read_buf[1] << 8);

    rc = ble_central_read_attr(target.conn_handle,
                               target.dead_zone_distance_handle,
                               read_buf,
                               sizeof(read_buf),
                               &read_len);
    if (rc != HAL_OK) {
        return rc;
    }
    if (read_len != 1) {
        return HAL_ERR_IO;
    }
    settings_out->dead_zone_distance = read_buf[0];

    rc = ble_central_read_attr(target.conn_handle,
                               target.firmware_version_handle,
                               read_buf,
                               sizeof(read_buf),
                               &read_len);
    if (rc != HAL_OK) {
        return rc;
    }
    if (read_len != sizeof(settings_out->firmware_version)) {
        return HAL_ERR_IO;
    }
    memcpy(settings_out->firmware_version,
           read_buf,
           sizeof(settings_out->firmware_version));
    return HAL_OK;
#else
    (void)mac;
    (void)settings_out;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_get_ring_diagnostics_by_mac(const uint8_t mac[6],
                                                     hub_ring_diagnostics_t *diagnostics_out)
{
    if (!mac || !diagnostics_out) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }
    if (target.diagnostics_handle == 0) {
        return target.subscribed ? HAL_ERR_NOT_SUPPORTED : HAL_ERR_BUSY;
    }

    uint8_t diagnostics_buf[GATT_RELAY_READ_MAX_LEN] = {0};
    size_t diagnostics_len = 0;
    hal_status_t rc = ble_central_read_attr(target.conn_handle,
                                            target.diagnostics_handle,
                                            diagnostics_buf,
                                            sizeof(diagnostics_buf),
                                            &diagnostics_len);
    if (rc != HAL_OK) {
        return rc;
    }

    uint8_t battery_pct = 0;
    bool has_battery_pct = false;
    if (target.battery_level_handle != 0) {
        uint8_t battery_buf[GATT_RELAY_READ_MAX_LEN] = {0};
        size_t battery_len = 0;
        rc = ble_central_read_attr(target.conn_handle,
                                   target.battery_level_handle,
                                   battery_buf,
                                   sizeof(battery_buf),
                                   &battery_len);
        if (rc != HAL_OK) {
            return rc;
        }
        if (battery_len != 1U) {
            return HAL_ERR_IO;
        }
        battery_pct = battery_buf[0];
        has_battery_pct = true;
    }

    return decode_ring_diagnostics_payload(diagnostics_buf,
                                           diagnostics_len,
                                           battery_pct,
                                           has_battery_pct,
                                           diagnostics_out);
#else
    (void)mac;
    (void)diagnostics_out;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_get_ring_rssi_by_mac(const uint8_t mac[6],
                                              int8_t *rssi_dbm_out)
{
    if (!mac || !rssi_dbm_out) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }

    int8_t rssi_dbm = 127;
    int rc = ble_gap_conn_rssi(target.conn_handle, &rssi_dbm);
    if (rc != 0) {
        return map_gatt_status(rc);
    }
    if (rssi_dbm == 127) {
        return HAL_ERR_NOT_SUPPORTED;
    }

    *rssi_dbm_out = rssi_dbm;
    return HAL_OK;
#else
    (void)mac;
    (void)rssi_dbm_out;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_set_ring_dpi_by_mac(const uint8_t mac[6],
                                             uint8_t dpi_multiplier)
{
    if (!mac || dpi_multiplier == 0) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }
    if (target.dpi_handle == 0) {
        return target.subscribed ? HAL_ERR_NOT_SUPPORTED : HAL_ERR_BUSY;
    }

    return ble_central_write_attr(target.conn_handle,
                                  target.dpi_handle,
                                  &dpi_multiplier,
                                  sizeof(dpi_multiplier));
#else
    (void)mac;
    (void)dpi_multiplier;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_set_ring_dead_zone_time_by_mac(const uint8_t mac[6],
                                                        uint16_t dead_zone_time_ms)
{
    if (!mac || dead_zone_time_ms > 2000U) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }
    if (target.dead_zone_time_handle == 0) {
        return target.subscribed ? HAL_ERR_NOT_SUPPORTED : HAL_ERR_BUSY;
    }

    uint8_t payload[2] = {
        (uint8_t)(dead_zone_time_ms & 0xFFU),
        (uint8_t)((dead_zone_time_ms >> 8) & 0xFFU),
    };
    return ble_central_write_attr(target.conn_handle,
                                  target.dead_zone_time_handle,
                                  payload,
                                  sizeof(payload));
#else
    (void)mac;
    (void)dead_zone_time_ms;
    return HAL_ERR_NOT_FOUND;
#endif
}

hal_status_t ble_central_set_ring_dead_zone_distance_by_mac(const uint8_t mac[6],
                                                            uint8_t dead_zone_distance)
{
    if (!mac) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    ring_relay_target_t target = {0};
    hal_status_t target_rc = copy_ring_relay_target_by_mac(mac, &target);
    if (target_rc != HAL_OK) {
        return target_rc;
    }
    if (target.dead_zone_distance_handle == 0) {
        return target.subscribed ? HAL_ERR_NOT_SUPPORTED : HAL_ERR_BUSY;
    }

    return ble_central_write_attr(target.conn_handle,
                                  target.dead_zone_distance_handle,
                                  &dead_zone_distance,
                                  sizeof(dead_zone_distance));
#else
    (void)mac;
    (void)dead_zone_distance;
    return HAL_ERR_NOT_FOUND;
#endif
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

hal_status_t ble_central_set_scan_policy(uint8_t scan_policy,
                                         uint8_t expected_rings)
{
    if (expected_rings == 0U || expected_rings > HUB_MAX_RINGS) {
        return HAL_ERR_INVALID_ARG;
    }
    if (scan_policy != BLE_CENTRAL_SCAN_POLICY_BOOT_ONLY &&
        scan_policy != BLE_CENTRAL_SCAN_POLICY_CONTINUOUS &&
        scan_policy != BLE_CENTRAL_SCAN_POLICY_EXPECTED) {
        return HAL_ERR_INVALID_ARG;
    }

#ifdef ESP_PLATFORM
    bool should_scan = false;

    RINGS_LOCK();
    s_scan_policy = scan_policy;
    s_expected_rings = expected_rings;
    should_scan = should_scan_locked(SCAN_TRIGGER_SETTINGS_CHANGE);
    RINGS_UNLOCK();

    if (!should_scan && ble_gap_disc_active()) {
        int cancel_rc = ble_gap_disc_cancel();
        if (cancel_rc != 0 && cancel_rc != BLE_HS_EALREADY) {
            return map_gatt_status(cancel_rc);
        }
        return HAL_OK;
    }

    if (should_scan) {
        start_scan(SCAN_TRIGGER_SETTINGS_CHANGE);
    }
#else
    (void)scan_policy;
    (void)expected_rings;
#endif

    return HAL_OK;
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
