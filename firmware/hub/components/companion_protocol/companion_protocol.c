// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion protocol parser

#include "companion_protocol.h"

#include "ble_central.h"
#include "hub_control.h"
#include "role_engine.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPANION_RING_DPI_MIN                    1UL
#define COMPANION_RING_DPI_MAX                    255UL
#define COMPANION_RING_DEAD_ZONE_TIME_MS_MAX      2000UL
#define COMPANION_RING_DEAD_ZONE_DISTANCE_MAX     255UL

typedef struct {
    char *buf;
    size_t len;
    size_t used;
} response_builder_t;

static hal_status_t append_format(response_builder_t *builder,
                                  const char *fmt,
                                  ...)
{
    if (!builder || !builder->buf || builder->used >= builder->len) {
        return HAL_ERR_INVALID_ARG;
    }

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(builder->buf + builder->used,
                            builder->len - builder->used,
                            fmt,
                            args);
    va_end(args);

    if (written < 0) {
        return HAL_ERR_IO;
    }
    if ((size_t)written >= (builder->len - builder->used)) {
        return HAL_ERR_NO_MEM;
    }

    builder->used += (size_t)written;
    return HAL_OK;
}

static hal_status_t write_protocol_error(char *response_out,
                                         size_t response_out_len,
                                         int code,
                                         const char *message)
{
    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder, "ERR %d %s\n", code, message);
}

static hal_status_t normalize_line(const char *line,
                                   char *normalized_out,
                                   size_t normalized_out_len)
{
    if (!line || !normalized_out || normalized_out_len == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    while (*line != '\0' && isspace((unsigned char)*line)) {
        line++;
    }

    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
        len--;
    }

    if ((len + 1) > normalized_out_len) {
        return HAL_ERR_NO_MEM;
    }

    memcpy(normalized_out, line, len);
    normalized_out[len] = '\0';
    return HAL_OK;
}

static bool token_equals_ignore_case(const char *a, const char *b)
{
    if (!a || !b) {
        return false;
    }

    while (*a != '\0' && *b != '\0') {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return false;
        }
        a++;
        b++;
    }

    return (*a == '\0') && (*b == '\0');
}

static char *skip_spaces(char *cursor)
{
    while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return cursor;
}

static char *next_token(char **cursor)
{
    if (!cursor || !*cursor) {
        return NULL;
    }

    char *token = skip_spaces(*cursor);
    if (*token == '\0') {
        *cursor = token;
        return NULL;
    }

    char *end = token;
    while (*end != '\0' && !isspace((unsigned char)*end)) {
        end++;
    }

    if (*end != '\0') {
        *end++ = '\0';
    }
    *cursor = end;
    return token;
}

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

static hal_status_t format_mac(const uint8_t mac[6],
                               char *mac_out,
                               size_t mac_out_len)
{
    if (!mac || !mac_out || mac_out_len < 18) {
        return HAL_ERR_INVALID_ARG;
    }

    int written = snprintf(mac_out,
                           mac_out_len,
                           "%02X:%02X:%02X:%02X:%02X:%02X",
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    if (written != 17) {
        return HAL_ERR_IO;
    }

    return HAL_OK;
}

static hal_status_t parse_mac_token(const char *token, uint8_t mac_out[6])
{
    if (!token || !mac_out) {
        return HAL_ERR_INVALID_ARG;
    }

    if (strlen(token) != 17) {
        return HAL_ERR_INVALID_ARG;
    }

    for (int i = 0; i < 6; i++) {
        int hi = hex_nibble(token[i * 3]);
        int lo = hex_nibble(token[i * 3 + 1]);
        if (hi < 0 || lo < 0) {
            return HAL_ERR_INVALID_ARG;
        }

        mac_out[i] = (uint8_t)((hi << 4) | lo);

        if (i < 5 && token[i * 3 + 2] != ':') {
            return HAL_ERR_INVALID_ARG;
        }
    }

    return HAL_OK;
}

static hal_status_t parse_role_token(const char *token, ring_role_t *role_out)
{
    if (!token || !role_out) {
        return HAL_ERR_INVALID_ARG;
    }

    if (token_equals_ignore_case(token, "CURSOR")) {
        *role_out = ROLE_CURSOR;
        return HAL_OK;
    }
    if (token_equals_ignore_case(token, "SCROLL")) {
        *role_out = ROLE_SCROLL;
        return HAL_OK;
    }
    if (token_equals_ignore_case(token, "MODIFIER")) {
        *role_out = ROLE_MODIFIER;
        return HAL_OK;
    }

    return HAL_ERR_INVALID_ARG;
}

static hal_status_t parse_unsigned_long_token(const char *token,
                                              unsigned long min_value,
                                              unsigned long max_value,
                                              unsigned long *value_out)
{
    if (!token || !value_out || min_value > max_value) {
        return HAL_ERR_INVALID_ARG;
    }

    errno = 0;
    char *end = NULL;
    unsigned long value = strtoul(token, &end, 10);
    if (errno != 0 || !end || *end != '\0') {
        return HAL_ERR_INVALID_ARG;
    }
    if (value < min_value || value > max_value) {
        return HAL_ERR_INVALID_ARG;
    }

    *value_out = value;
    return HAL_OK;
}

static hal_status_t lookup_known_ring(const uint8_t mac[6], ring_role_t *role_out)
{
    if (!mac || !role_out) {
        return HAL_ERR_INVALID_ARG;
    }

    role_engine_entry_t entries[HUB_MAX_RINGS] = {0};
    size_t entry_count = 0;
    hal_status_t rc = role_engine_get_all(entries, HUB_MAX_RINGS, &entry_count);
    if (rc != HAL_OK) {
        return rc;
    }

    for (size_t i = 0; i < entry_count; i++) {
        if (memcmp(entries[i].mac, mac, 6) == 0) {
            *role_out = entries[i].role;
            return HAL_OK;
        }
    }

    return HAL_ERR_NOT_FOUND;
}

static hal_status_t format_firmware_version(const uint8_t version[3],
                                            char *version_out,
                                            size_t version_out_len)
{
    if (!version || !version_out || version_out_len == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    int written = snprintf(version_out,
                           version_out_len,
                           "%u.%u.%u",
                           version[0],
                           version[1],
                           version[2]);
    if (written <= 0 || (size_t)written >= version_out_len) {
        return HAL_ERR_NO_MEM;
    }

    return HAL_OK;
}

static hal_status_t write_ring_relay_error(char *response_out,
                                           size_t response_out_len,
                                           hal_status_t relay_status,
                                           const char *unsupported_message,
                                           const char *io_message)
{
    if (relay_status == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    409,
                                    "ring_not_connected");
    }
    if (relay_status == HAL_ERR_BUSY ||
        relay_status == HAL_ERR_TIMEOUT ||
        relay_status == HAL_ERR_REJECTED) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    503,
                                    "ring_not_ready");
    }
    if (relay_status == HAL_ERR_NOT_SUPPORTED) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    501,
                                    unsupported_message);
    }
    if (relay_status == HAL_ERR_INVALID_ARG) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_value");
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                500,
                                io_message);
}

static const char *ring_state_code_name(uint8_t ring_state_code)
{
    switch (ring_state_code) {
    case 0:
        return "DEEP_SLEEP";
    case 1:
        return "BOOTING";
    case 2:
        return "ADVERTISING";
    case 3:
        return "CONNECTED_ACTIVE";
    case 4:
        return "CONNECTED_IDLE";
    default:
        return "UNKNOWN";
    }
}

static const char *ring_sensor_state_name(hub_ring_sensor_state_t sensor_state)
{
    switch (sensor_state) {
    case HUB_RING_SENSOR_UNAVAILABLE:
        return "UNAVAILABLE";
    case HUB_RING_SENSOR_CALIBRATION_PENDING:
        return "CALIBRATION_PENDING";
    case HUB_RING_SENSOR_READY:
        return "READY";
    default:
        return "UNKNOWN";
    }
}

static const char *ring_bond_state_name(hub_ring_bond_state_t bond_state)
{
    switch (bond_state) {
    case HUB_RING_BOND_UNKNOWN:
        return "UNKNOWN";
    case HUB_RING_BOND_RESTORED:
        return "RESTORED";
    case HUB_RING_BOND_FAILED:
        return "FAILED";
    default:
        return "UNKNOWN";
    }
}

static hal_status_t handle_get_hub_info(const companion_protocol_hub_info_t *hub_info,
                                        char *response_out,
                                        size_t response_out_len)
{
    if (!hub_info || !hub_info->firmware_revision || !hub_info->hardware_revision) {
        return HAL_ERR_INVALID_ARG;
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };

    hal_status_t rc = append_format(&builder,
                                    "+ fw=%s\n"
                                    "+ hw=%s\n"
                                    "+ rings=%u\n"
                                    "+ max_rings=%u\n"
                                    "+ usb_poll_ms=%u\n"
                                    "+ scan_policy=%u\n"
                                    "OK\n",
                                    hub_info->firmware_revision,
                                    hub_info->hardware_revision,
                                    hub_info->connected_rings,
                                    hub_info->max_rings,
                                    hub_info->usb_poll_ms,
                                    hub_info->scan_policy);
    return rc;
}

static hal_status_t handle_get_roles(char *response_out, size_t response_out_len)
{
    role_engine_entry_t entries[HUB_MAX_RINGS] = {0};
    size_t entry_count = 0;
    hal_status_t rc = role_engine_get_all(entries,
                                          HUB_MAX_RINGS,
                                          &entry_count);
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };

    for (size_t i = 0; i < entry_count; i++) {
        char mac[18] = {0};
        rc = format_mac(entries[i].mac, mac, sizeof(mac));
        if (rc != HAL_OK) {
            return rc;
        }

        rc = append_format(&builder,
                           "+ %s %s\n",
                           mac,
                           role_engine_role_name(entries[i].role));
        if (rc != HAL_OK) {
            return rc;
        }
    }

    return append_format(&builder, "OK\n");
}

static hal_status_t handle_get_rings(char *response_out, size_t response_out_len)
{
    role_engine_entry_t entries[HUB_MAX_RINGS] = {0};
    size_t entry_count = 0;
    hal_status_t rc = role_engine_get_all(entries,
                                          HUB_MAX_RINGS,
                                          &entry_count);
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };

    for (size_t i = 0; i < entry_count; i++) {
        char mac[18] = {0};
        rc = format_mac(entries[i].mac, mac, sizeof(mac));
        if (rc != HAL_OK) {
            return rc;
        }

        uint8_t ring_index = 0;
        bool connected = (ble_central_find_ring_index_by_mac(entries[i].mac, &ring_index) == HAL_OK);
        rc = append_format(&builder,
                           "+ %s %s %s\n",
                           mac,
                           role_engine_role_name(entries[i].role),
                           connected ? "connected" : "disconnected");
        if (rc != HAL_OK) {
            return rc;
        }
    }

    return append_format(&builder, "OK\n");
}

static hal_status_t handle_get_ring_info(char *args,
                                         char *response_out,
                                         size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);

    if (!mac_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    ring_role_t role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &role);
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    uint8_t ring_index = 0;
    bool connected = (ble_central_find_ring_index_by_mac(mac, &ring_index) == HAL_OK);
    char formatted_mac[18] = {0};
    rc = format_mac(mac, formatted_mac, sizeof(formatted_mac));
    if (rc != HAL_OK) {
        return rc;
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };

    return append_format(&builder,
                         "+ mac=%s\n"
                         "+ role=%s\n"
                         "+ connected=%u\n"
                         "OK\n",
                         formatted_mac,
                         role_engine_role_name(role),
                         connected ? 1U : 0U);
}

static hal_status_t handle_get_ring_settings(char *args,
                                             char *response_out,
                                             size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);

    if (!mac_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    ring_role_t ignored_role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &ignored_role);
    (void)ignored_role;
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    hub_ring_settings_t settings = {0};
    rc = ble_central_get_ring_settings_by_mac(mac, &settings);
    if (rc != HAL_OK) {
        return write_ring_relay_error(response_out,
                                      response_out_len,
                                      rc,
                                      "ring_settings_unavailable",
                                      "ring_settings_read_failed");
    }

    char formatted_mac[18] = {0};
    char firmware_version[16] = {0};
    rc = format_mac(mac, formatted_mac, sizeof(formatted_mac));
    if (rc != HAL_OK) {
        return rc;
    }
    rc = format_firmware_version(settings.firmware_version,
                                 firmware_version,
                                 sizeof(firmware_version));
    if (rc != HAL_OK) {
        return rc;
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder,
                         "+ mac=%s\n"
                         "+ dpi_multiplier=%u\n"
                         "+ dead_zone_time_ms=%u\n"
                         "+ dead_zone_distance=%u\n"
                         "+ firmware_version=%s\n"
                         "OK\n",
                         formatted_mac,
                         settings.dpi_multiplier,
                         settings.dead_zone_time_ms,
                         settings.dead_zone_distance,
                         firmware_version);
}

static hal_status_t handle_get_ring_diagnostics(char *args,
                                                char *response_out,
                                                size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);

    if (!mac_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    ring_role_t ignored_role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &ignored_role);
    (void)ignored_role;
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    hub_ring_diagnostics_t diagnostics = {0};
    rc = ble_central_get_ring_diagnostics_by_mac(mac, &diagnostics);
    if (rc != HAL_OK) {
        return write_ring_relay_error(response_out,
                                      response_out_len,
                                      rc,
                                      "ring_diagnostics_unavailable",
                                      "ring_diagnostics_read_failed");
    }

    char formatted_mac[18] = {0};
    rc = format_mac(mac, formatted_mac, sizeof(formatted_mac));
    if (rc != HAL_OK) {
        return rc;
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder,
                         "+ mac=%s\n"
                         "+ battery_pct=%u\n"
                         "+ battery_mv=%u\n"
                         "+ ring_state=%s\n"
                         "+ sensor_state=%s\n"
                         "+ bond_state=%s\n"
                         "+ connected=%u\n"
                         "+ calibration_valid=%u\n"
                         "+ conn_param_rejected=%u\n"
                         "+ conn_interval_1_25ms=%u\n"
                         "+ diagnostics_version=%u\n"
                         "OK\n",
                         formatted_mac,
                         diagnostics.battery_pct,
                         (unsigned int)diagnostics.battery_mv,
                         ring_state_code_name(diagnostics.ring_state_code),
                         ring_sensor_state_name(diagnostics.sensor_state),
                         ring_bond_state_name(diagnostics.bond_state),
                         diagnostics.connected ? 1U : 0U,
                         diagnostics.calibration_valid ? 1U : 0U,
                         diagnostics.conn_param_rejected ? 1U : 0U,
                         diagnostics.conn_interval_1_25ms,
                         diagnostics.diagnostics_version);
}

static hal_status_t handle_set_ring_dpi(char *args,
                                        char *response_out,
                                        size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);
    char *value_token = next_token(&cursor);

    if (!mac_token || !value_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    unsigned long dpi_value = 0;
    if (parse_unsigned_long_token(value_token,
                                  COMPANION_RING_DPI_MIN,
                                  COMPANION_RING_DPI_MAX,
                                  &dpi_value) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_value");
    }

    ring_role_t ignored_role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &ignored_role);
    (void)ignored_role;
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    rc = ble_central_set_ring_dpi_by_mac(mac, (uint8_t)dpi_value);
    if (rc != HAL_OK) {
        return write_ring_relay_error(response_out,
                                      response_out_len,
                                      rc,
                                      "ring_settings_unavailable",
                                      "ring_settings_write_failed");
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder, "OK\n");
}

static hal_status_t handle_set_ring_dead_zone_time(char *args,
                                                   char *response_out,
                                                   size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);
    char *value_token = next_token(&cursor);

    if (!mac_token || !value_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    unsigned long dead_zone_time_ms = 0;
    if (parse_unsigned_long_token(value_token,
                                  0UL,
                                  COMPANION_RING_DEAD_ZONE_TIME_MS_MAX,
                                  &dead_zone_time_ms) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_value");
    }

    ring_role_t ignored_role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &ignored_role);
    (void)ignored_role;
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    rc = ble_central_set_ring_dead_zone_time_by_mac(mac, (uint16_t)dead_zone_time_ms);
    if (rc != HAL_OK) {
        return write_ring_relay_error(response_out,
                                      response_out_len,
                                      rc,
                                      "ring_settings_unavailable",
                                      "ring_settings_write_failed");
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder, "OK\n");
}

static hal_status_t handle_set_ring_dead_zone_distance(char *args,
                                                       char *response_out,
                                                       size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);
    char *value_token = next_token(&cursor);

    if (!mac_token || !value_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    unsigned long dead_zone_distance = 0;
    if (parse_unsigned_long_token(value_token,
                                  0UL,
                                  COMPANION_RING_DEAD_ZONE_DISTANCE_MAX,
                                  &dead_zone_distance) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_value");
    }

    ring_role_t ignored_role = ROLE_CURSOR;
    hal_status_t rc = lookup_known_ring(mac, &ignored_role);
    (void)ignored_role;
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }
    if (rc != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    500,
                                    "role_read_failed");
    }

    rc = ble_central_set_ring_dead_zone_distance_by_mac(mac, (uint8_t)dead_zone_distance);
    if (rc != HAL_OK) {
        return write_ring_relay_error(response_out,
                                      response_out_len,
                                      rc,
                                      "ring_settings_unavailable",
                                      "ring_settings_write_failed");
    }

    response_builder_t builder = {
        .buf = response_out,
        .len = response_out_len,
        .used = 0,
    };
    return append_format(&builder, "OK\n");
}

static hal_status_t handle_set_role(char *args,
                                    char *response_out,
                                    size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);
    char *role_token = next_token(&cursor);

    if (!mac_token || !role_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    ring_role_t role = ROLE_CURSOR;
    if (parse_role_token(role_token, &role) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_role");
    }

    hal_status_t rc = hub_control_set_role(mac, role);
    if (rc == HAL_OK) {
        response_builder_t builder = {
            .buf = response_out,
            .len = response_out_len,
            .used = 0,
        };
        return append_format(&builder, "OK\n");
    }
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                500,
                                "role_write_failed");
}

static hal_status_t handle_swap_roles(char *args,
                                      char *response_out,
                                      size_t response_out_len)
{
    char *cursor = args;
    char *mac_a_token = next_token(&cursor);
    char *mac_b_token = next_token(&cursor);

    if (!mac_a_token || !mac_b_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac_a[6] = {0};
    uint8_t mac_b[6] = {0};
    if (parse_mac_token(mac_a_token, mac_a) != HAL_OK ||
        parse_mac_token(mac_b_token, mac_b) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }
    if (memcmp(mac_a, mac_b, 6) == 0) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "identical_macs");
    }

    hal_status_t rc = hub_control_swap_roles(mac_a, mac_b);
    if (rc == HAL_OK) {
        response_builder_t builder = {
            .buf = response_out,
            .len = response_out_len,
            .used = 0,
        };
        return append_format(&builder, "OK\n");
    }
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                500,
                                "role_swap_failed");
}

static hal_status_t handle_forget_ring(char *args,
                                       char *response_out,
                                       size_t response_out_len)
{
    char *cursor = args;
    char *mac_token = next_token(&cursor);

    if (!mac_token) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_args");
    }
    if (next_token(&cursor) != NULL) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "unexpected_args");
    }

    uint8_t mac[6] = {0};
    if (parse_mac_token(mac_token, mac) != HAL_OK) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "invalid_mac");
    }

    hal_status_t rc = hub_control_forget_ring(mac);
    if (rc == HAL_OK) {
        response_builder_t builder = {
            .buf = response_out,
            .len = response_out_len,
            .used = 0,
        };
        return append_format(&builder, "OK\n");
    }
    if (rc == HAL_ERR_NOT_FOUND) {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    404,
                                    "unknown_mac");
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                500,
                                "forget_failed");
}

hal_status_t companion_protocol_handle_line(const char *line,
                                            const companion_protocol_hub_info_t *hub_info,
                                            char *response_out,
                                            size_t response_out_len)
{
    if (!line || !hub_info || !response_out || response_out_len == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    char normalized[COMPANION_PROTOCOL_COMMAND_LINE_MAX_LEN + 1] = {0};
    hal_status_t rc = normalize_line(line, normalized, sizeof(normalized));
    if (rc != HAL_OK) {
        return rc;
    }

    if (normalized[0] == '\0') {
        return write_protocol_error(response_out,
                                    response_out_len,
                                    400,
                                    "empty_command");
    }

    char *command = normalized;
    char *args = normalized;
    while (*args != '\0' && !isspace((unsigned char)*args)) {
        args++;
    }
    if (*args != '\0') {
        *args++ = '\0';
        args = skip_spaces(args);
    }

    if (token_equals_ignore_case(command, "GET_HUB_INFO")) {
        if (*args != '\0') {
            return write_protocol_error(response_out,
                                        response_out_len,
                                        400,
                                        "unexpected_args");
        }
        return handle_get_hub_info(hub_info, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "GET_ROLES")) {
        if (*args != '\0') {
            return write_protocol_error(response_out,
                                        response_out_len,
                                        400,
                                        "unexpected_args");
        }
        return handle_get_roles(response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "GET_RINGS")) {
        if (*args != '\0') {
            return write_protocol_error(response_out,
                                        response_out_len,
                                        400,
                                        "unexpected_args");
        }
        return handle_get_rings(response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "GET_RING_INFO")) {
        return handle_get_ring_info(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "GET_RING_SETTINGS")) {
        return handle_get_ring_settings(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "GET_RING_DIAGNOSTICS")) {
        return handle_get_ring_diagnostics(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SET_RING_DPI")) {
        return handle_set_ring_dpi(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SET_RING_DEAD_ZONE_TIME")) {
        return handle_set_ring_dead_zone_time(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SET_RING_DEAD_ZONE_DISTANCE")) {
        return handle_set_ring_dead_zone_distance(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SET_ROLE")) {
        return handle_set_role(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SWAP_ROLES") ||
        token_equals_ignore_case(command, "ROLE_SWAP")) {
        return handle_swap_roles(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "FORGET_RING")) {
        return handle_forget_ring(args, response_out, response_out_len);
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                400,
                                "unknown_command");
}
