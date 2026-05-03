<!-- SPDX-License-Identifier: MIT -->
# Active-Lane Driver/Hardware Contract

This document records the minimum contract between the current firmware drivers
and the active hardware packets. Its purpose is narrow: keep software bring-up
honest even while CAD, board placement, and first-board routing are still moving.

The firmware architecture is mostly stable across mechanical changes. The ring
runtime reads a small sensor interface, applies calibration and click
suppression, then emits HID reports. The hub runtime accepts ring reports,
applies roles, and emits one USB HID mouse. CAD can change around that shape.
Electrical semantics, focal geometry, battery safety, and RF behavior cannot.

## Scope

- Active validation lane: `R30-OLED-NONE-NONE` ring pair plus `USB-HUB`
- Hedge lane: `WSTD-BALL-NONE-NONE` only where it clarifies deferred risk
- Deferred here: ball+Hall ring, Pro optical-on-ball ring, puck, IMU, OCR,
  direct companion mode, packaged apps, and broader gesture expansion

This file does not replace packet-level hardware contracts. The ring packet's
local net map remains
[`hardware/ring/R30-OLED-NONE-NONE/kicad/INTERFACE-CONTRACT.md`](../hardware/ring/R30-OLED-NONE-NONE/kicad/INTERFACE-CONTRACT.md).
The active board firmware binding is
[`R30-OLED-FIRMWARE-CONFIG.md`](R30-OLED-FIRMWARE-CONFIG.md). This file maps
those hardware facts back to firmware behavior and gate impact.

## Classification

| Classification | Meaning | Required action |
| --- | --- | --- |
| Driver-stable | The part may move or the shell may change while the same firmware seam still applies. | Preserve the signal type, voltage domain, pin contract, and report shape. |
| Board-contract change | The signal semantics, pin allocation, Kconfig value, voltage level, or report format changes. | Update firmware config, packet contract docs, and focused tests together. |
| Concept-risk item | Firmware can run, but physics may still make the product unusable. | Treat bench or human-loop evidence as the gate, not a green build. |
| No-go candidate | If the item fails, the active lane must pause or change architecture. | Escalate to BDFL decision with a recommended fallback. |

## Ring Contract

| Hardware surface | Firmware seam | Required hardware contract | Missing or degraded behavior | Gate impact |
| --- | --- | --- | --- | --- |
| ESP32-C3 ring MCU, flash, NVS, BLE radio | `firmware/ring/main`, `pf_hal`, `ble_hid` | ESP32-C3-compatible module with enough flash for the ring app, working NVS, BLE HID support, stable reset/boot, and antenna keep-out preserved. | No meaningful fallback. A dev board can prove firmware, but a custom ring with broken boot, NVS, or RF is not a ring proof. | Blocks Gate 1 and Gate 3. |
| PAW3204-class optical sensor IC | `CONFIG_SENSOR_PAW3204`, `sensor_paw3204.c` | `SENSOR_SCLK` and `SENSOR_SDIO` must match Kconfig, run at the sensor's logic level, and reach a real PAW3204-compatible sensor that returns the expected product ID, deltas, motion, and SQUAL-like confidence. | Runtime disables motion and can remain click-only. That is useful diagnostics, not a successful ring mouse. | Blocks Gate 1. |
| PAW3204 lens, emitter, aperture, glide/focal stack | Same PAW3204 driver, plus `SURFACE-TEST-PROTOCOL.md` evidence | Matched lens/emitter kit, matte cavity, aperture alignment, and 2.4-3.2 mm working focal distance under realistic pressure and click force. | Driver may return plausible bytes while the human loop fails through dropout, jitter, or click displacement. | Blocks Gate 1 and Gate 2. |
| Primary snap dome | `CONFIG_CLICK_SNAP_DOME`, `click_dome.c`, dead-zone logic | Dome shorts the configured GPIO to ground when pressed, supports internal pull-up, raises both edges, and is mechanically placed so pressing does not rotate the ring off target. | Firmware can disable buttons if init fails. If it works electrically but shifts the ring, software filtering cannot rescue the click loop. | Blocks Gate 1. |
| VBAT sense divider | `CONFIG_POWERFINGER_VBAT_ADC_CHANNEL`, `power_manager.c`, diagnostics | Divider must scale protected cell voltage into ADC range, stay tied to the Kconfig ADC channel, and be present on any board claiming low-voltage cutoff or battery diagnostics. | ADC init/read failure drives safety fallback toward shutdown. A board with no configured divider must not claim battery safety behavior. | Blocks Gate 3. |
| Cell NTC sense | `CONFIG_POWERFINGER_NTC_ADC_CHANNEL`, thermal policy | NTC must measure cell temperature, not convenient board ambient. Kconfig must name the populated ADC channel for any board claiming thermal charge cutoff. | If omitted or disabled, firmware cannot protect the cell from TP4054's missing NTC input. | Blocks Gate 3 for wearable charging claims. |
| Charge-enable MOSFET path | `CONFIG_POWERFINGER_CHARGE_ENABLE_PIN`, `power_manager.c` | GPIO controls a logic-safe gate path. The MCU must never see the 5 V high-side gate pull-up directly. Default hardware state must be charge disabled. | If unpopulated or Kconfig-disabled, charging may still occur electrically, but firmware charge cutoff is not present. | Blocks Gate 3 for enclosure charging. |
| VBUS detect | `CONFIG_POWERFINGER_VBUS_DETECT_PIN`, low-battery wake/charge policy | Resistor-divided VBUS presence must reach the configured GPIO and read false when unplugged. | Firmware cannot distinguish charge-present recovery from battery-only lockout. Do not claim USB-wake or charge-state behavior. | Blocks Gate 3 edge cases. |
| `CHRG_STAT` pull-up/status pad | No production consumer in the active packet | Local status pad may exist for hardware test, but no ring firmware behavior depends on it today. | Safe to omit from firmware claims. Adding a firmware consumer is a board-contract change. | Not a current gate blocker. |
| USB-C charge/service connector | Flashing, charge path, bring-up access | Connector must be mechanically retained, provide the intended charge/service access, and preserve reopenable battery service. | Firmware can be correct while the device is unserviceable or unsafe to charge. | Blocks Gate 2 and Gate 3. |
| Protected harnessed LiPo | Power manager, battery safety docs | Cell must fit the shell, have integrated PCM, have a replaceable harness path, and meet the published safety procurement requirements. | No software workaround for an unprotected, non-serviceable, or unfittable cell. | No-go candidate for the ring form factor. |
| RF/antenna keep-out | BLE HID and hub/single-host link | Module antenna keep-out and body-worn orientation must preserve a stable low-power BLE link. | Firmware may pass host tests while the physical ring drops or needs unacceptable TX power. | Blocks Gate 3. |

## Hub Contract

| Hardware surface | Firmware seam | Required hardware contract | Missing or degraded behavior | Gate impact |
| --- | --- | --- | --- | --- |
| ESP32-S3 hub MCU | `firmware/hub/main`, `ble_central`, TinyUSB | ESP32-S3-compatible module with native USB device support, BLE central support, NVS, enough flash, and preserved antenna keep-out. | No meaningful fallback for the active hub lane. | Blocks Gate 4. |
| USB power and 3.3 V rail | `usb_hid_mouse_init`, BLE central runtime | On-board regulation must provide stable 3.3 V from USB VBUS, with local decoupling and no dev-board-only power assumption. | Firmware init failures or intermittent USB/BLE behavior become expected. | Blocks Gate 4. |
| USB connector and ESD entry | TinyUSB HID report path | D+/D-, VBUS, ground, connector orientation, shell retention, and ESD protection must match the USB-HUB packet. | Host may never enumerate, or mechanical insertion may damage the board. | Blocks Gate 4. |
| Ring BLE report ingest | `hub_ring_report_t`, `ble_central`, `event_composer_feed` | Rings must present the expected HID/report characteristics and stable ring indices. Reports are `{buttons, dx, dy, wheel}` with signed 8-bit motion fields. | Composer ignores disconnected or stale slots, but a report-shape mismatch breaks composition. | Blocks Gate 4. |
| Role persistence | `role_engine.c`, `hal_storage` | NVS must persist MAC-to-role assignments without blocking the USB HID loop. | Runtime defaults may work in a session, but reconnect role stability is not proved. | Blocks Gate 4 reconnect criteria. |
| Event composition output | `event_composer.c`, `usb_hid_mouse.c` | USB host must see one standard HID mouse. Disconnect paths must always release button state and zero stale deltas. | Stuck buttons or stale role state are accessibility hazards, not acceptable rough edges. | Blocks Gate 4. |
| Companion CDC control surface | `companion_cdc`, `companion_protocol` | CDC may expose settings/diagnostics, but the active mouse workflow must not require a packaged app or cloud dependency. | CDC failure is serious for configuration, but not a valid reason to make host-side software mandatory. | Supports Gate 4; does not replace HID proof. |

## Deferred Driver Risks

| Deferred component | Firmware status | Why it is deferred |
| --- | --- | --- |
| Ball+Hall ring or wand | The ADC driver shape is clear: four Hall channels, baseline capture, differential X/Y, and Hall power gating. | The risky part is not the C driver. It is the ball, roller, magnet, friction, cleaning, and signal linearity at usable force. |
| PMW3360 optical-on-ball | SPI driver shape is clear, including SROM upload and burst reads. | The checked-in SROM blob is a placeholder; the sensor also adds rails, lens geometry, current draw, and possible level-shift or out-of-spec VDDIO questions. |
| ADNS-2080 or another PAW3204 fallback | Treat as a real substitution, not a free BOM swap. | A fallback optical sensor may preserve the product concept, but it can change protocol, timing, lens, footprint, and surface behavior. |
| Piezo plus LRA click | Click interface can support another implementation. | It changes wake behavior, haptic suppression, power spikes, and mechanical coupling into the optical sensor. |

## Bring-Up Rules

1. If a driver init fails, first classify whether the failure is a driver bug,
   board-contract mismatch, or missing physical evidence. Do not immediately
   tune filters around a mechanical or optical failure.
2. Do not claim Gate 1 from host tests alone. Gate 1 requires a real single-ring
   human control loop on the surface protocol.
3. Do not claim Gate 3 charge safety unless `VBAT_SENSE`, NTC, charge enable,
   and VBUS detect are wired, Kconfig-enabled, and observed on the actual board.
4. Do not claim `CHRG_STAT` firmware behavior until a production firmware symbol
   consumes it and tests cover it.
5. If a pin assignment changes, update the packet-level interface contract,
   Kconfig fragment or board config, and host tests in the same patch.
6. If a component is replaced by an alternate, re-check the driver protocol,
   voltage domain, package, lens/mechanical interface, and BOM ceiling before
   calling it compatible.

## Highest No-Go Watchlist

| Risk | Reason | Preferred fallback if it fails |
| --- | --- | --- |
| Protected harnessed 80-100 mAh ring LiPo | No software fix exists for a cell that cannot be safe, serviceable, and physically fit. | Promote wand or puck-style packaging while preserving the ring as research. |
| PAW3204 sensor plus matched lens/emitter kit | The active lane depends on a real optical kit, and supply is weaker than commodity passives or Espressif modules. | Validate an ADNS-2080-class optical packet as an explicit driver and mechanical substitution. |
| Optical focal and click stability | Firmware can suppress click micro-motion, but it cannot fix a ring that shifts or loses focus under use. | Rework shell, glide pads, dome placement, or temporarily test the same firmware in a puck-like cradle. |
| Enclosure charging thermal safety | TP4054 has no cell NTC input, so the external NTC and charge gate are load-bearing for body-worn charging. | Disable in-enclosure charging until the safety path is proven, or move charging to a less constrained form factor. |
| Ball+Hall mechanism | The electronics are plausible; the uncertain part is whether the small mechanical sensor produces clean control at accessible force levels. | Keep as hedge work, not as active-lane replacement until bench evidence exists. |
