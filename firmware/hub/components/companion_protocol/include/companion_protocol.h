// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion protocol parser
//
// This transport-agnostic parser handles the text protocol used by the future
// USB CDC task and other hub-side control surfaces. Pre-hardware it supports
// the read-only GET_HUB_INFO and GET_ROLES commands so companion tooling can
// be developed against a real command contract before serial transport exists.

#pragma once

#include "hal_types.h"

typedef struct {
    const char *firmware_revision;
    const char *hardware_revision;
    uint8_t connected_rings;
    uint8_t max_rings;
    uint8_t usb_poll_ms;
    uint8_t scan_policy;
} companion_protocol_hub_info_t;

// Parses one command line and writes the protocol response into response_out.
// Protocol-level errors (unknown command, unexpected args) are returned as
// ERR lines in the response buffer with HAL_OK status. Non-OK return values
// indicate a caller bug or an output buffer that is too small.
hal_status_t companion_protocol_handle_line(const char *line,
                                            const companion_protocol_hub_info_t *hub_info,
                                            char *response_out,
                                            size_t response_out_len);
