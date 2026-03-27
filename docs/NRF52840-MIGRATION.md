# nRF52840 Migration Spec

Migration analysis for PowerFinger firmware from ESP32-C3 (ESP-IDF + NimBLE) to
nRF52840 (Zephyr RTOS + Zephyr BLE). This document maps every HAL interface to
its Zephyr equivalent, identifies semantic mismatches, rates portability, and
recommends HAL changes that are cheaper to make now than after 9 firmware phases.

References:
- Power motivation: [POWER-BUDGET.md](POWER-BUDGET.md) (ESP32-C3 can't hit
  consumer battery targets; nRF52840 gives 3-5x improvement)
- HAL headers: `firmware/ring/components/hal/include/`
- ESP-IDF implementations: `firmware/ring/components/hal/esp_idf/`
- Firmware build order: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)

---

## Why nRF52840

| Parameter | ESP32-C3 | nRF52840 | Ratio |
|-----------|----------|----------|-------|
| BLE connected idle (15ms) | 350-2000 uA | 50-200 uA | 5-10x |
| Light sleep (no radio) | 130-350 uA | 1.5-4.5 uA | 30-90x |
| Deep sleep + RTC | 5-8 uA | 1.5-2 uA | 3-4x |
| BLE TX peak | 100-130 mA | 5-8 mA (@0 dBm) | 15-20x |
| Active CPU (80 MHz equiv) | 13-17 mA | 3-5 mA | 3-4x |

On an 80 mAh battery, the ESP32-C3 delivers 3-6 days. The nRF52840 delivers
2-4 weeks. The platform choice is the single largest determinant of battery
life -- larger than sensor choice, connection interval, or firmware optimization
combined.

**Bonus: USB 2.0 Full Speed.** The nRF52840 has a native USB 2.0 full-speed
controller. This is relevant for the hub dongle migration: an nRF52840 hub can
serve as both BLE central and USB HID device on a single chip, eliminating the
ESP32-S3 entirely. Zephyr has mature USB HID class support for nRF52840
(`include/zephyr/usb/class/usb_hid.h`).

---

## SDK Choice: nRF Connect SDK (NCS)

nRF Connect SDK is Nordic's downstream fork of Zephyr, adding:
- Nordic-specific BLE libraries including `bt_hids` (HID over GATT service)
- MCUboot integration with Nordic-specific flash layouts
- Power optimization profiles tested on nRF52840
- nRF Connect for VS Code IDE support

NCS is the recommended path over raw upstream Zephyr. It uses the same Zephyr
APIs (all `k_*`, `gpio_*`, `spi_*` calls are identical) but adds a tested
`bt_hids` library that handles HID report map registration, boot protocol
support, and notification sending -- functionality we currently build manually
on NimBLE. The SoftDevice has been removed in newer NCS versions; the BLE stack
runs as Zephyr threads, not a binary blob.

**License:** NCS and Zephyr are Apache 2.0. MCUboot is Apache 2.0. All
compatible with our MIT firmware license. No proprietary dependencies
introduced.

---

## HAL Interface Migration Map

### 1. hal_ble.h -- BLE HID Peripheral

**Portability: Rewrite needed**

This is the hardest migration point. The NimBLE and Zephyr BLE stacks have
fundamentally different programming models. NimBLE uses a single GAP event
callback with a switch statement; Zephyr uses separate callback structs
registered per-connection.

| hal_ble.h function | ESP-IDF (NimBLE) | Zephyr / NCS equivalent |
|---|---|---|
| `hal_ble_init()` | `nimble_port_init()`, `ble_gatts_add_svcs()`, `ble_svc_gap_init()`, `nimble_port_freertos_init()` | `bt_enable()`, `bt_hids_init()` (NCS), manual GATT registration via `BT_GATT_SERVICE_DEFINE()` (upstream Zephyr) |
| `hal_ble_start_advertising()` | `ble_gap_adv_start()` with `ble_gap_adv_params` | `bt_le_adv_start()` with `struct bt_le_adv_param` and `struct bt_data ad[]` |
| `hal_ble_stop_advertising()` | `ble_gap_adv_stop()` | `bt_le_adv_stop()` |
| `hal_ble_send_mouse_report()` | `ble_hs_mbuf_from_flat()` + `ble_gatts_notify_custom()` | `bt_hids_inp_rep_send()` (NCS) or `bt_gatt_notify()` (upstream Zephyr) |
| `hal_ble_request_conn_params()` | `ble_gap_update_params()` | `bt_conn_le_param_update()` with `struct bt_le_conn_param` |
| `hal_ble_delete_all_bonds()` | `ble_store_util_delete_all()` | `bt_unpair(BT_ID_DEFAULT, NULL)` |
| `hal_ble_is_connected()` | Read `atomic_bool s_connected` | Check stored `struct bt_conn *` pointer against NULL |
| GAP event callback | Single `ble_gap_event_handler` with switch on `event->type` | `struct bt_conn_cb` with `.connected`, `.disconnected`, `.le_param_updated`, `.security_changed` members |
| Bond recovery | `BLE_GAP_EVENT_ENC_CHANGE` + `ble_store_util_delete_peer()` | `security_changed` callback + `bt_unpair()` on auth failure |
| HID report map | Manual GATT service table (`struct ble_gatt_svc_def`) | `BT_HIDS_DEF()` macro + `struct bt_hids_init_param` (NCS) or manual `BT_GATT_SERVICE_DEFINE` |
| Security config | `ble_hs_cfg.sm_*` fields | `bt_conn_auth_cb_register()` + Kconfig `CONFIG_BT_SMP`, `CONFIG_BT_BONDABLE` |

**Semantic mismatches:**

1. **Notification model.** NimBLE: build an mbuf, call `ble_gatts_notify_custom`.
   Zephyr: pass a flat buffer to `bt_gatt_notify()` or use the NCS
   `bt_hids_inp_rep_send()` helper. The NCS helper is the closest match to
   our `hal_ble_send_mouse_report()` -- it takes a connection, a report index,
   and a data buffer. Upstream Zephyr requires manual attribute handle lookup.

2. **Connection handle type.** NimBLE uses `uint16_t conn_handle`. Zephyr uses
   `struct bt_conn *` (reference-counted opaque pointer). The HAL correctly
   hides this behind `hal_ble_is_connected()` -- no conn handle leaks into
   application code.

3. **Advertising data format.** NimBLE: `struct ble_hs_adv_fields` with named
   fields. Zephyr: `struct bt_data ad[]` array built with `BT_DATA_BYTES()`
   and `BT_DATA()` macros. Different format, same information.

4. **Host task model.** NimBLE runs in a dedicated FreeRTOS task
   (`nimble_port_freertos_init`). Zephyr BLE runs as kernel threads, started
   by `bt_enable()`. No equivalent of `nimble_port_run()`.

5. **Boot protocol.** Our current implementation registers Protocol Mode
   characteristic (0x2A4E) manually. NCS `bt_hids` handles boot protocol
   support automatically if `CONFIG_BT_HIDS_DEFAULT_PERM_RW_AUTHEN` is set.

6. **Advertising timeout.** NimBLE accepts timeout in 10ms units via
   `ble_gap_adv_start` duration parameter. Zephyr `bt_le_adv_start` uses
   `BT_LE_ADV_OPT_ONE_TIME` for single-cycle advertising but has no built-in
   timeout -- the HAL implementation must use a `k_timer` to stop advertising
   after the specified duration.

**HAL change recommended NOW:**

- Add `hal_ble_set_battery_level(uint8_t percent)` to the HAL. The current
  ESP-IDF implementation has a hardcoded `uint8_t level = 100` in the Battery
  Service read callback. On Zephyr/NCS, the Battery Service is a separate
  module (`CONFIG_BT_BAS`, `bt_bas_set_battery_level()`). Adding this to the
  HAL now means the power manager can report real battery levels through a
  portable interface rather than reaching into BLE internals.

- Add `hal_ble_deinit()`. NimBLE cleanup requires `nimble_port_deinit()`.
  Zephyr requires `bt_disable()` (available since Zephyr 3.4). Currently no
  way to shut down the BLE stack cleanly before deep sleep -- the ESP-IDF
  implementation relies on `esp_deep_sleep_start()` doing a hard reset.

### 2. hal_spi.h -- SPI Bus

**Portability: Minor changes**

| hal_spi.h function | ESP-IDF | Zephyr equivalent |
|---|---|---|
| `hal_spi_init()` | `spi_bus_initialize()` + `spi_bus_add_device()` | Device tree node + `spi_transceive_dt()` (no runtime init needed) |
| `hal_spi_transfer()` | `spi_device_transmit()` | `spi_transceive()` / `spi_transceive_dt()` with `struct spi_buf_set` |
| `hal_spi_write_reg()` | Calls `hal_spi_transfer()` | Same -- can reuse |
| `hal_spi_read_reg()` | Calls `hal_spi_transfer()` | Same -- can reuse |
| `hal_spi_deinit()` | `spi_bus_remove_device()` + `spi_bus_free()` | `spi_release()` (optional) |

**Semantic mismatches:**

1. **Configuration model.** ESP-IDF: runtime configuration via
   `spi_bus_config_t` and `spi_device_interface_config_t`. Zephyr: pin
   assignments and bus parameters are defined in the device tree overlay
   (`.overlay` file), not in C code. The `hal_spi_config_t` struct's pin
   fields (`clk`, `mosi`, `miso`, `cs`) become device tree properties. The
   Zephyr implementation of `hal_spi_init()` would obtain the device from
   `DEVICE_DT_GET()` and store the `struct spi_dt_spec` rather than creating
   a bus.

2. **Buffer format.** ESP-IDF: `spi_transaction_t` with `tx_buffer`,
   `rx_buffer`, `length` (in bits). Zephyr: `struct spi_buf_set` containing
   an array of `struct spi_buf` (pointer + length in bytes). The conversion is
   mechanical but the struct shapes differ.

3. **CS handling.** ESP-IDF: CS pin is part of `spi_device_interface_config_t`,
   asserted automatically. Zephyr: CS is part of `struct spi_cs_control` in
   the `spi_dt_spec`, also automatic. Functionally equivalent.

**HAL change recommended NOW:**

- Add a `hal_spi_config_t` field for `bus_id` (uint8_t). The current ESP-IDF
  implementation hardcodes `SPI2_HOST`. On nRF52840, the SPI instance is
  selected via device tree (`&spi1`, `&spi2`, etc.). A bus ID field makes the
  HAL ready for multi-SPI configurations without changing the interface.
  Alternatively, accept that `hal_spi_init` will always get its bus identity
  from a board-level config header or device tree, and document this as
  platform-specific.

### 3. hal_gpio.h -- GPIO

**Portability: Minor changes**

| hal_gpio.h function | ESP-IDF | Zephyr equivalent |
|---|---|---|
| `hal_gpio_init()` | `gpio_config()` | `gpio_pin_configure()` with `GPIO_INPUT`, `GPIO_OUTPUT`, `GPIO_PULL_UP`, `GPIO_PULL_DOWN` flags |
| `hal_gpio_set()` | `gpio_set_level()` | `gpio_pin_set()` |
| `hal_gpio_get()` | `gpio_get_level()` | `gpio_pin_get()` |
| `hal_gpio_set_interrupt()` | `gpio_install_isr_service()` + `gpio_isr_handler_add()` | `gpio_pin_interrupt_configure()` + `gpio_init_callback()` + `gpio_add_callback()` |

**Semantic mismatches:**

1. **Pin numbering.** ESP-IDF: flat GPIO number (0-21 on ESP32-C3). Zephyr:
   `struct gpio_dt_spec` from device tree, containing port device + pin number.
   The HAL's `hal_pin_t` (uint32_t) can encode both schemes, but Zephyr GPIO
   functions need the port device pointer too. The Zephyr implementation will
   need a lookup table or device tree macro to map `hal_pin_t` to
   `(port, pin)` pairs.

2. **Interrupt registration.** ESP-IDF: per-pin ISR handler via
   `gpio_isr_handler_add()`. Zephyr: callback struct (`struct gpio_callback`)
   registered per-port, with a pin mask. Multiple pins on the same port share
   one callback struct. The Zephyr implementation must maintain a callback
   array and dispatch to the correct `hal_isr_callback_t` based on which pin
   triggered.

3. **ISR service lifecycle.** ESP-IDF requires `gpio_install_isr_service()`
   once. Zephyr has no equivalent -- interrupt dispatch is built into the GPIO
   driver.

**HAL change recommended NOW:**

- Consider changing `hal_pin_t` from `uint32_t` to a struct that can hold a
  port identifier and pin number: `typedef struct { uint8_t port; uint8_t pin; } hal_pin_t;`. On ESP-IDF, `port` is always 0 (single GPIO port).
  On nRF52840, port is 0 or 1 (P0 and P1). **However**, this changes every
  call site. A less invasive approach: keep `hal_pin_t` as `uint32_t` and
  encode port in the upper bits (e.g., `port << 8 | pin`). Define macros:
  `HAL_PIN(port, pin)` and `HAL_PIN_PORT(p)` / `HAL_PIN_NUM(p)`. The ESP-IDF
  implementation ignores the port byte. The Zephyr implementation extracts it.
  **Recommendation: do this now.** It costs nothing on ESP-IDF (port is always
  0, so existing pin numbers are unchanged) and avoids a painful retrofit
  later.

### 4. hal_adc.h -- Analog-to-Digital Converter

**Portability: Minor changes**

| hal_adc.h function | ESP-IDF | Zephyr equivalent |
|---|---|---|
| `hal_adc_init()` | `adc_oneshot_new_unit()` + `adc_oneshot_config_channel()` + `adc_cali_create_scheme_curve_fitting()` | Device tree ADC channel config + `adc_channel_setup()` with `struct adc_channel_cfg` |
| `hal_adc_read_mv()` | `adc_oneshot_read()` + `adc_cali_raw_to_voltage()` | `adc_read()` with `struct adc_sequence`, then `adc_raw_to_millivolts()` (Zephyr 3.3+) |
| `hal_adc_deinit()` | No-op (ESP-IDF doesn't support per-channel deinit) | No direct equivalent -- Zephyr ADC channels don't need explicit teardown |

**Semantic mismatches:**

1. **Channel identification.** ESP-IDF: `adc_channel_t` enum (ADC_CHANNEL_0
   through ADC_CHANNEL_9). Zephyr: ADC channels defined in device tree,
   accessed via `ADC_DT_SPEC_GET()`. The HAL's `hal_adc_channel_t` (uint8_t)
   maps cleanly to both.

2. **Read model.** ESP-IDF `adc_oneshot_read()` returns a single raw value.
   Zephyr `adc_read()` fills a `struct adc_sequence` with a buffer, supporting
   multi-sample reads. For single reads, the buffer is one `int16_t`. Slightly
   more ceremony but functionally equivalent.

3. **Calibration.** ESP-IDF has curve-fitting calibration built in
   (`adc_cali_raw_to_voltage`). Zephyr provides `adc_raw_to_millivolts()`
   which uses the ADC reference voltage and gain. The nRF52840 SAADC has
   better inherent accuracy than the ESP32-C3 ADC (lower DNL/INL), so
   calibration complexity may actually decrease.

**HAL change recommended NOW:** None. The interface is clean. The Zephyr
implementation will be slightly longer (more struct setup) but the HAL
abstraction holds.

### 5. hal_timer.h -- Software Timers

**Portability: Clean**

| hal_timer.h function | ESP-IDF | Zephyr equivalent |
|---|---|---|
| `hal_timer_create()` | `esp_timer_create()` | `k_timer_init()` (static allocation) or dynamic allocation + init |
| `hal_timer_start()` | `esp_timer_start_periodic()` / `esp_timer_start_once()` | `k_timer_start()` with `K_MSEC(period)` and `K_MSEC(period)` (periodic) or `K_NO_WAIT` (one-shot) |
| `hal_timer_stop()` | `esp_timer_stop()` | `k_timer_stop()` |
| `hal_timer_set_period()` | Stop + restart with new period | `k_timer_start()` with new duration (restarts the timer) |
| `hal_timer_delete()` | `esp_timer_delete()` + `free()` | `free()` the wrapper (k_timer has no delete -- it's just memory) |
| `hal_timer_get_ms()` | `esp_timer_get_time() / 1000` | `k_uptime_get_32()` |
| `hal_timer_delay_ms()` | `vTaskDelay(pdMS_TO_TICKS(ms))` | `k_msleep(ms)` |

**Semantic mismatches:**

1. **Allocation model.** ESP-IDF `esp_timer_create()` allocates internally.
   Zephyr `k_timer` is typically stack/static allocated. Our HAL uses
   `calloc()` for the wrapper struct, which can contain an embedded
   `struct k_timer` -- this pattern works fine on Zephyr.

2. **Callback context.** ESP-IDF: `esp_timer` callback runs in a dedicated
   timer task (not ISR). Zephyr: `k_timer` expiry function runs in ISR context
   by default. **This is a significant mismatch.** The HAL documents that
   `hal_callback_t` is called from task context, not ISR. On Zephyr, the
   `k_timer` expiry handler must defer to a work queue (`k_work_submit()`) to
   maintain this contract.

3. **One-shot vs periodic.** ESP-IDF has separate start functions
   (`esp_timer_start_once` vs `esp_timer_start_periodic`). Zephyr's
   `k_timer_start(timer, duration, period)` handles both: pass `K_NO_WAIT`
   as period for one-shot, `K_MSEC(period)` for periodic.

**HAL change recommended NOW:**

- Document explicitly in `hal_timer.h` that the callback MUST execute in task
  context (not ISR). This is already stated in a comment but should be a
  contract that both implementations must honor. The Zephyr implementation
  will need a `k_work` per timer to guarantee this. The ESP-IDF implementation
  already satisfies it via `ESP_TIMER_TASK` dispatch.

### 6. hal_sleep.h -- Power Management

**Portability: Minor changes**

| hal_sleep.h function | ESP-IDF | Zephyr equivalent |
|---|---|---|
| `hal_sleep_enter(LIGHT)` | `esp_light_sleep_start()` | `pm_state_force(0, &(struct pm_state_info){PM_STATE_SUSPEND_TO_IDLE, ...})` + `k_cpu_idle()` / `k_msleep()`, or automatic via `CONFIG_PM=y` |
| `hal_sleep_enter(DEEP)` | `esp_deep_sleep_start()` (does not return) | `sys_poweroff()` (does not return -- maps to nRF52840 SYSTEM OFF) |
| `hal_sleep_configure_wake_gpio()` | `esp_deep_sleep_enable_gpio_wakeup()` | Configure GPIO SENSE via `gpio_pin_interrupt_configure()` with `GPIO_INT_LEVEL_LOW` or `GPIO_INT_LEVEL_HIGH` before `sys_poweroff()` |
| `hal_sleep_configure_wake_timer()` | `esp_sleep_enable_timer_wakeup()` | Light sleep: managed by Zephyr PM automatically. Deep sleep (SYSTEM OFF): nRF52840 has no RTC wake from SYSTEM OFF -- **GPIO only** |

**Semantic mismatches:**

1. **Light sleep model.** ESP-IDF: explicit `esp_light_sleep_start()` call,
   returns on wake. Zephyr: automatic idle-thread power management
   (`CONFIG_PM=y`). When no threads are runnable, Zephyr enters the deepest
   allowed sleep state automatically. The BLE stack's timing constraints
   naturally keep the system from sleeping too deep. **The Zephyr implementation
   of `hal_sleep_enter(LIGHT)` may be a no-op** -- just let the idle thread
   handle it. If explicit light sleep is needed, `k_msleep()` with PM enabled
   achieves the same effect.

2. **Deep sleep wake sources.** ESP-IDF supports both GPIO and timer wake from
   deep sleep. **nRF52840 SYSTEM OFF only supports GPIO wake via DETECT
   signal.** There is no timer wake from SYSTEM OFF. This means
   `hal_sleep_configure_wake_timer()` cannot be used before deep sleep on
   nRF52840 -- only before light sleep (where Zephyr's PM handles it
   automatically). The HAL should document this constraint.

3. **GPIO wake mechanism.** ESP-IDF: bitmask of pins + level. nRF52840: each
   GPIO pin has a SENSE register (high or low). Edge-triggered interrupts do
   NOT work for SYSTEM OFF wake -- must use level-triggered SENSE. The Zephyr
   implementation must use `GPIO_INT_LEVEL_LOW` or `GPIO_INT_LEVEL_HIGH`, not
   edge triggers.

4. **RAM retention.** ESP-IDF deep sleep loses all RAM. nRF52840 SYSTEM OFF
   also loses RAM, but nRF52840 has a "SYSTEM ON idle" state that retains RAM
   while drawing ~1.5 uA. This is effectively a third sleep mode between light
   and deep. The current HAL doesn't expose it.

**HAL change recommended NOW:**

- Add `HAL_SLEEP_SYSTEM_ON_IDLE` as a third sleep mode. On ESP-IDF, this maps
  to light sleep. On nRF52840, this maps to SYSTEM ON with RAM retention
  (~1.5 uA) -- dramatically better than ESP32-C3 light sleep (~350 uA) while
  retaining all state. This is the nRF52840's killer feature for BLE idle
  periods.

- Document in `hal_sleep.h` that `hal_sleep_configure_wake_timer()` is only
  valid before `HAL_SLEEP_LIGHT`, not before `HAL_SLEEP_DEEP`. On nRF52840,
  timer wake from SYSTEM OFF is not supported.

- Add `hal_sleep_get_wake_cause()` returning an enum
  (`HAL_WAKE_GPIO`, `HAL_WAKE_TIMER`, `HAL_WAKE_RESET`). ESP-IDF provides
  `esp_sleep_get_wakeup_cause()`. Zephyr provides
  `nrf_power_resetreas_get()` / `pm_state_*`. The ring state machine needs
  this to decide whether to re-advertise or recalibrate after waking.

### 7. hal_storage.h -- Non-Volatile Storage

**Portability: Minor changes**

| hal_storage.h function | ESP-IDF (NVS) | Zephyr equivalent |
|---|---|---|
| `hal_storage_init()` | `nvs_open()` | `settings_subsys_init()` (settings subsystem) or `nvs_mount()` (Zephyr NVS directly) |
| `hal_storage_set()` | `nvs_set_blob()` | `settings_save_one()` with key path (settings) or `nvs_write()` (Zephyr NVS) |
| `hal_storage_get()` | `nvs_get_blob()` | `settings_runtime_get()` or handler-based `settings_load()` (settings) or `nvs_read()` (Zephyr NVS) |
| `hal_storage_delete()` | `nvs_erase_key()` | `settings_delete()` (settings) or `nvs_delete()` (Zephyr NVS) |
| `hal_storage_commit()` | `nvs_commit()` | `settings_save()` (settings subsystem batches writes) or no-op (Zephyr NVS writes immediately) |

**Two Zephyr options:**

1. **Zephyr NVS directly** (`zephyr/storage/flash_map.h` +
   `zephyr/fs/nvs.h`). API: `nvs_mount()`, `nvs_read()`, `nvs_write()`,
   `nvs_delete()`. Uses integer IDs, not string keys. Would require a
   key-to-ID mapping table.

2. **Settings subsystem** (`zephyr/settings/settings.h`). API:
   `settings_subsys_init()`, `settings_save_one()`, `settings_load()`.
   String keys with hierarchical paths (e.g., `"pf/sensitivity"`). NVS
   backend by default on nRF52840. **This is the better match** for our
   string-keyed HAL.

**Semantic mismatches:**

1. **Read model.** ESP-IDF NVS: synchronous read into caller buffer.
   Zephyr settings: handler-based. You register a handler with
   `settings_register()`, call `settings_load()`, and the handler receives
   each key-value pair asynchronously. For direct reads,
   `settings_runtime_get()` provides synchronous access but requires the
   setting to have been loaded first. The Zephyr implementation of
   `hal_storage_get()` should use `settings_runtime_get()` with prior
   `settings_load()` during init.

2. **Commit semantics.** ESP-IDF NVS: writes are buffered until
   `nvs_commit()`. Zephyr settings: `settings_save_one()` writes immediately
   (through the NVS backend). Our `hal_storage_commit()` becomes a no-op on
   Zephyr. This is fine -- the HAL comment already says "call during idle
   periods," so callers won't depend on batching.

3. **Key length.** ESP-IDF NVS keys: max 15 characters. Zephyr settings keys:
   hierarchical, max path length depends on `CONFIG_SETTINGS_NAME_MAX`
   (default 32). No constraint tightening.

**HAL change recommended NOW:**

- Enforce max key length in the HAL header: `#define HAL_STORAGE_KEY_MAX 15`.
  ESP-IDF NVS silently truncates at 15 chars. Zephyr settings allows longer
  keys. If callers start using 20-char keys on Zephyr, they'll break when
  tested on ESP-IDF. Define the constraint now.

### 8. hal_ota.h -- Over-the-Air Update

**Portability: Rewrite needed**

| hal_ota.h function | ESP-IDF | Zephyr / MCUboot equivalent |
|---|---|---|
| `hal_ota_begin()` | `esp_ota_get_next_update_partition()` + `esp_ota_begin()` | `boot_erase_img_bank()` on secondary slot, open flash area via `flash_area_open()` |
| `hal_ota_write()` | `esp_ota_write()` | `flash_area_write()` at tracked offset |
| `hal_ota_finish()` | `esp_ota_end()` + `esp_ota_set_boot_partition()` | `boot_request_upgrade()` (marks secondary slot for swap on next reboot) |
| `hal_ota_reboot()` | `esp_restart()` | `sys_reboot(SYS_REBOOT_COLD)` |
| `hal_ota_confirm()` | `esp_ota_mark_app_valid_cancel_rollback()` | `boot_write_img_confirmed()` |
| `hal_ota_rollback()` | `esp_ota_mark_app_invalid_rollback_and_reboot()` | `sys_reboot()` without confirming -- MCUboot reverts on next boot |

**Semantic mismatches:**

1. **Partition model.** ESP-IDF: named partitions in partition table, queried
   at runtime via `esp_ota_get_next_update_partition()`. MCUboot: fixed
   primary/secondary slots defined in device tree. The concept maps 1:1 but
   the discovery mechanism differs entirely.

2. **Write model.** ESP-IDF `esp_ota_write()` handles flash page alignment
   and erase internally. MCUboot's flash area API requires the caller to
   erase first (`boot_erase_img_bank()` or `flash_area_erase()`) and write
   at aligned offsets. The HAL implementation must track the write offset.

3. **Validation.** ESP-IDF `esp_ota_end()` validates the image (checksum,
   header). MCUboot validates on boot, not on write completion.
   `hal_ota_finish()` on Zephyr calls `boot_request_upgrade()` but
   validation happens at next reboot. If the image is corrupt, MCUboot
   stays on the primary slot -- functionally equivalent but the failure
   point is different.

4. **Transfer protocol.** The current architecture uses a custom GATT service
   for ring OTA via the hub. On Zephyr/NCS, the standard approach is the
   **SMP (Simple Management Protocol)** over BLE, using the `mcumgr` library.
   This provides a well-tested transport that phone apps (nRF Connect) and
   the hub can speak. **Recommendation:** migrate to SMP for the OTA transport
   layer rather than a custom GATT service. The HAL's `hal_ota_write()` /
   `hal_ota_finish()` abstraction still works -- the transport layer sits
   above the HAL.

**HAL change recommended NOW:**

- Add `hal_ota_get_image_size(size_t *out_max_bytes)` to query available
  flash for the update image. ESP-IDF: partition size. MCUboot: secondary
  slot size. The hub needs this to validate that a firmware binary fits
  before starting transfer.

- Consider whether to expose SMP/mcumgr at the HAL level or keep it above
  the HAL. Recommendation: keep the HAL as a raw flash write interface.
  The SMP transport is a BLE-layer concern that sits between the hub's
  BLE central and the ring's `hal_ota_*` calls. No HAL change needed for
  SMP -- it wraps the same begin/write/finish sequence.

### 9. hal_types.h -- Portable Types

**Portability: Clean**

This file is already platform-independent. It uses `stdint.h`, `stdbool.h`,
`stddef.h` only. No changes needed for nRF52840.

| Type | Current | nRF52840 compatibility |
|---|---|---|
| `hal_status_t` (enum) | 9 values | Portable -- maps to any platform |
| `hal_pin_t` (uint32_t) | Flat GPIO number | Needs port encoding -- see hal_gpio.h recommendation |
| `hal_spi_handle_t` (opaque pointer) | `struct hal_spi_ctx *` | Portable -- implementation defines the struct |
| `hal_timer_handle_t` (opaque pointer) | `struct hal_timer_ctx *` | Portable -- implementation defines the struct |
| `hal_isr_callback_t` | `void (*)(void *arg)` | Portable |
| `hal_callback_t` | `void (*)(void *arg)` | Portable |
| `HAL_PIN_NONE` | `UINT32_MAX` | Portable |

**HAL change recommended NOW:**

- Add `HAL_PIN(port, pin)` / `HAL_PIN_PORT(p)` / `HAL_PIN_NUM(p)` macros as
  discussed in the hal_gpio.h section. Place them here in hal_types.h since
  `hal_pin_t` is defined here.

---

## Build System Migration

### ESP-IDF (current)

```
Framework:  ESP-IDF (CMake-based)
Build:      idf.py build
Flash:      idf.py flash
Monitor:    idf.py monitor
Config:     sdkconfig / menuconfig
Components: firmware/ring/components/*/CMakeLists.txt
```

### Zephyr / NCS (target)

```
Framework:  Zephyr RTOS via nRF Connect SDK
Build:      west build -b nrf52840dk_nrf52840
Flash:      west flash
Monitor:    west espressif monitor (N/A) -- use J-Link RTT or UART
Config:     prj.conf (Kconfig) + *.overlay (device tree)
Components: Zephyr modules or in-tree source
```

**Key differences:**

1. **Project structure.** ESP-IDF uses a `components/` directory with
   per-component `CMakeLists.txt`. Zephyr uses a flat `src/` directory with a
   single `CMakeLists.txt`, or Zephyr modules for reusable libraries. The HAL
   implementations would live in `src/hal/` with a board-specific overlay
   selecting the correct files.

2. **Pin configuration.** ESP-IDF: runtime GPIO configuration in C code.
   Zephyr: pin muxing and peripheral assignment in device tree overlays.
   A `powerfinger_ring.overlay` file replaces all the pin constants currently
   in `ring_config.h`.

3. **Kconfig.** ESP-IDF uses `sdkconfig` (menuconfig). Zephyr uses `prj.conf`.
   Both are Kconfig-based, so the concepts transfer, but all config symbol
   names change (e.g., `CONFIG_BT_NIMBLE_ENABLED` becomes `CONFIG_BT`,
   `CONFIG_BT_PERIPHERAL`, `CONFIG_BT_HID`).

4. **Build tool.** `idf.py` becomes `west`. West also manages the Zephyr
   workspace (fetching modules, updating SDK).

5. **Debugging.** ESP-IDF uses JTAG via OpenOCD or `esptool.py` for serial.
   nRF52840 uses J-Link (built into Nordic dev kits) or SWD with any probe.
   Segger RTT replaces UART logging for zero-pin debug output.

---

## Migration Scope Estimate

### Files changed

| Category | Files | Notes |
|---|---|---|
| HAL implementations (new) | 8 | One `.c` per interface in `hal/zephyr/` (hal_types.h needs no implementation) |
| HAL headers (modified) | 3-4 | hal_types.h (pin macros), hal_sleep.h (new sleep mode), hal_ble.h (battery level, deinit), hal_ota.h (image size) |
| Build system (new) | 3-5 | `CMakeLists.txt`, `prj.conf`, board overlay, partition layout |
| Application code | 0 | **Zero changes** if HAL contract is maintained |
| Sensor drivers | 0 | PAW3204 and PMW3360 call only HAL functions |
| State machine | 0 | ring_state.c calls only HAL functions |
| Click/dead zone | 0 | Calls only HAL functions |
| Power manager | 0-1 | May need update for new sleep mode enum |

### Rough LOC delta

| Category | Lines |
|---|---|
| New Zephyr HAL implementations | ~1200-1500 (hal_ble_zephyr.c is ~400 alone) |
| New build/config files | ~150-200 |
| HAL header modifications | ~30-50 |
| Device tree overlays | ~60-80 |
| **Total new/changed** | **~1450-1830** |
| ESP-IDF implementations (unchanged, not deleted) | ~700 existing |

The application-layer firmware (state machine, sensors, click, dead zone,
calibration, power policy) requires **zero changes**. This is the HAL's entire
purpose.

---

## Hardest Migration Point

**`hal_ble.h` is the hardest migration, by a wide margin.**

Reasons:
1. NimBLE and Zephyr BLE have fundamentally different programming models
   (single event callback vs. connection callback structs).
2. HID GATT service registration is completely different (NimBLE service table
   vs. Zephyr `BT_GATT_SERVICE_DEFINE` macros or NCS `bt_hids`).
3. Bond management, security configuration, and pairing flows differ in every
   detail.
4. The current ESP-IDF implementation is 576 lines -- the largest HAL file.
   The Zephyr implementation will be ~400-500 lines with NCS `bt_hids`, or
   ~600-700 lines on raw upstream Zephyr.
5. BLE HID interop testing across OSes (macOS, Windows, Linux, iOS, Android)
   must be repeated from scratch -- BLE stacks behave differently with
   different host controllers.

The other 7 HAL interfaces are straightforward mechanical ports. BLE is the
only one where the Zephyr implementation requires rethinking the internal
architecture rather than just swapping API calls.

---

## Summary of Recommended HAL Changes (Do Now)

These changes have near-zero cost on the current ESP-IDF implementation but
significantly reduce migration friction later.

| # | Interface | Change | Cost now | Cost if deferred |
|---|---|---|---|---|
| 1 | `hal_types.h` | Add `HAL_PIN(port, pin)` / `HAL_PIN_PORT()` / `HAL_PIN_NUM()` macros | Trivial -- define macros, existing code unchanged (port=0) | Every GPIO call site and pin constant must be audited |
| 2 | `hal_ble.h` | Add `hal_ble_set_battery_level(uint8_t percent)` | One function, ~10 lines in ESP impl | Battery reporting code scattered across BLE and power modules |
| 3 | `hal_ble.h` | Add `hal_ble_deinit()` | One function, ~5 lines (call `nimble_port_deinit`) | Deep sleep cleanup path must be retrofitted |
| 4 | `hal_sleep.h` | Add `HAL_SLEEP_SYSTEM_ON_IDLE` mode | Maps to `HAL_SLEEP_LIGHT` on ESP-IDF (no behavior change) | Miss nRF52840's best power state (~1.5 uA with RAM) |
| 5 | `hal_sleep.h` | Add `hal_sleep_get_wake_cause()` | Wrap `esp_sleep_get_wakeup_cause()` | State machine wake logic is platform-specific |
| 6 | `hal_sleep.h` | Document timer wake only valid for light sleep | Comment addition only | Silent failure on nRF52840 deep sleep |
| 7 | `hal_storage.h` | Define `HAL_STORAGE_KEY_MAX 15` | Compile-time assert, no runtime cost | Keys work on Zephyr, break on ESP-IDF -- discovered late |
| 8 | `hal_ota.h` | Add `hal_ota_get_image_size()` | Wrap `esp_partition_get_size()` | Hub can't validate firmware binary size before transfer |
| 9 | `hal_timer.h` | Strengthen callback-from-task-context contract in header | Comment only | Zephyr port has subtle ISR-context bugs |

**Total effort for all 9 changes: small.** Each is 0-15 lines of implementation
code plus header updates. The alternative -- discovering these gaps during
migration -- means touching application code that should be platform-independent.

---

## Portability Scorecard

| HAL Interface | Rating | Notes |
|---|---|---|
| `hal_types.h` | **Clean** | Pure C types, fully portable |
| `hal_timer.h` | **Clean** | Direct mapping, callback context needs k_work |
| `hal_adc.h` | **Minor changes** | Different read ceremony, same semantics |
| `hal_spi.h` | **Minor changes** | Device tree config model, buffer struct conversion |
| `hal_gpio.h` | **Minor changes** | Port+pin encoding, interrupt dispatch model |
| `hal_sleep.h` | **Minor changes** | Deep sleep wake constraints differ |
| `hal_storage.h` | **Minor changes** | Settings subsystem read model, commit semantics |
| `hal_ota.h` | **Rewrite needed** | MCUboot has different partition/validation model |
| `hal_ble.h` | **Rewrite needed** | Completely different BLE programming model |

7 of 9 interfaces are clean or minor changes. The HAL design is solid. The two
rewrites (BLE and OTA) are unavoidable regardless of HAL design -- the
underlying stacks are fundamentally different. The HAL's value is that these
two rewrites are the **only** files that change; the entire application layer
(state machine, sensors, click, power policy) is untouched.
