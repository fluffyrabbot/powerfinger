// SPDX-License-Identifier: MIT
// PowerFinger Hub — USB CDC companion transport
//
// Exposes the text companion protocol over a TinyUSB CDC ACM port.

#pragma once

#include "companion_session.h"
#include "hal_types.h"

typedef struct {
    companion_session_hub_info_cb_t fill_hub_info;
    void *cb_arg;
} companion_cdc_config_t;

hal_status_t companion_cdc_start(const companion_cdc_config_t *config);
