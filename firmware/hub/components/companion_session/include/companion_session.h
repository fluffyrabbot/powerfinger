// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion command session
//
// Owns line buffering for the text companion protocol so transports can feed
// arbitrary byte chunks and receive framed ASCII responses.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "companion_protocol.h"
#include "hal_types.h"

#define COMPANION_SESSION_RESPONSE_MAX_LEN 256

typedef void (*companion_session_hub_info_cb_t)(companion_protocol_hub_info_t *info_out,
                                                void *arg);
typedef hal_status_t (*companion_session_emit_cb_t)(const char *response, void *arg);

typedef struct {
    companion_session_hub_info_cb_t fill_hub_info;
    companion_session_emit_cb_t emit_response;
    void *cb_arg;
    char line_buf[COMPANION_PROTOCOL_COMMAND_LINE_MAX_LEN + 1];
    size_t line_len;
    bool dropping_overlong_line;
} companion_session_t;

hal_status_t companion_session_init(companion_session_t *session,
                                    companion_session_hub_info_cb_t fill_hub_info,
                                    companion_session_emit_cb_t emit_response,
                                    void *cb_arg);

hal_status_t companion_session_feed_bytes(companion_session_t *session,
                                          const uint8_t *bytes,
                                          size_t len);
