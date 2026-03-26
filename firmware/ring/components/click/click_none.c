// SPDX-License-Identifier: MIT
// PowerFinger — Null click driver (Phase 0: no physical button)

#include "click_interface.h"

hal_status_t click_init(void)
{
    return HAL_OK;
}

bool click_is_pressed(void)
{
    return false;
}

hal_status_t click_deinit(void)
{
    return HAL_OK;
}
