// SPDX-License-Identifier: MIT
// PowerFinger - Shared runtime entry for ring-derived firmware variants.

#pragma once

typedef struct {
    const char *log_tag;
    const char *firmware_name;
} powerfinger_app_config_t;

void powerfinger_app_main(const powerfinger_app_config_t *config);
