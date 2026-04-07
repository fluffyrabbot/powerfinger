// SPDX-License-Identifier: MIT
// PowerFinger Hub — Companion protocol parser
//
// This transport-agnostic parser handles the text protocol used by the hub's
// USB CDC task and other hub-side control surfaces. Today it supports
// GET_HUB_INFO, GET_ROLES, GET_RINGS, GET_RING_INFO, GET_RING_SETTINGS,
// GET_RING_DIAGNOSTICS, GET_GESTURES, and the current mutating hub commands
// (SET_RING_*, SET_GESTURE, SET_ROLE, SWAP_ROLES, and FORGET_RING) so
// companion tooling can exercise both role and live ring inspection flows
// over the real transport.

#pragma once

#include "hal_types.h"

// Maximum command payload length in ASCII characters, excluding the trailing
// newline and the internal NUL terminator.
#define COMPANION_PROTOCOL_COMMAND_LINE_MAX_LEN 128

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
