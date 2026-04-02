// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion protocol parser

#include "companion_protocol.h"

#include "ble_central.h"
#include "hub_control.h"
#include "role_engine.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define COMMAND_LINE_MAX_LEN 128

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

hal_status_t companion_protocol_handle_line(const char *line,
                                            const companion_protocol_hub_info_t *hub_info,
                                            char *response_out,
                                            size_t response_out_len)
{
    if (!line || !hub_info || !response_out || response_out_len == 0) {
        return HAL_ERR_INVALID_ARG;
    }

    char normalized[COMMAND_LINE_MAX_LEN] = {0};
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

    if (token_equals_ignore_case(command, "SET_ROLE")) {
        return handle_set_role(args, response_out, response_out_len);
    }

    if (token_equals_ignore_case(command, "SWAP_ROLES") ||
        token_equals_ignore_case(command, "ROLE_SWAP")) {
        return handle_swap_roles(args, response_out, response_out_len);
    }

    return write_protocol_error(response_out,
                                response_out_len,
                                400,
                                "unknown_command");
}
