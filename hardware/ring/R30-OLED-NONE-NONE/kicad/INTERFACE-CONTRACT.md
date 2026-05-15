<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Interface Contract

This file is the first-pass electrical contract between the active optical ring
hardware capture and the ring firmware defaults.

It exists to stop two kinds of drift:

- KiCad capture inventing a pin plan that the firmware never names
- firmware host tests quietly assuming safety-path pins that collide with the
  active optical sensor lane

## Contract Rules

- Preserve the current optical-sensor shared path from
  [docs/DUAL-FOOTPRINT.md](../../../docs/DUAL-FOOTPRINT.md):
  `GPIO4` = clock, `GPIO5` = shared PAW3204 SDIO / PMW3360 MOSI,
  `GPIO6` = PMW3360 `NCS`, `GPIO7` = PMW3360 `MISO`.
- Keep charge-safety nets off new strapping-pin dependencies where possible.
  Espressif documents `GPIO2`, `GPIO8`, and `GPIO9` as strapping pins on
  ESP32-C3, so the VBUS-detect choice avoids them.
- Keep the pre-hardware dev-board lane honest: firmware Kconfig defaults for
  the NTC and VBUS detect stay disabled (`-1`) until the real ring hardware
  populates them. This P0 does not allocate an MCU charge-enable gate.

## First-Pass Pin Plan

| Function | ESP32-C3 resource | Firmware symbol | Status | Notes |
|----------|-------------------|-----------------|--------|-------|
| VBAT sense | `ADC1_CH0` / `GPIO0` | `CONFIG_POWERFINGER_VBAT_ADC_CHANNEL` | Active default | Existing battery monitor path |
| Cell NTC sense | `ADC1_CH1` / `GPIO1` | `CONFIG_POWERFINGER_NTC_ADC_CHANNEL` | Recommended first board | Matches the published battery-safety requirement without colliding with the optical lane |
| USB VBUS detect | `GPIO3` | `CONFIG_POWERFINGER_VBUS_DETECT_PIN` | Recommended first board | Input only from a resistor-divided 5V-detect net |
| Charger status | none allocated | none | Fixture hardware test only | `CHRG_STAT` is exposed on a local pad without an onboard pull-up; a test fixture must provide a pull-up if status is measured |
| Sensor clock | `GPIO4` | `CONFIG_POWERFINGER_SENSOR_SCLK_PIN` | Active default | Shared PAW3204 / PMW3360 clock path |
| Sensor data out | `GPIO5` | `CONFIG_POWERFINGER_SENSOR_SDIO_PIN` | Active default | PAW3204 bidirectional SDIO, PMW3360 MOSI |
| Sensor chip select | `GPIO6` | `CONFIG_POWERFINGER_SENSOR_NCS_PIN` | Active default | PMW3360-only; leave unconnected on PAW3204 capture |
| Sensor data in | `GPIO7` | `CONFIG_POWERFINGER_SENSOR_MISO_PIN` | Active default | PMW3360-only; leave unconnected on PAW3204 capture |
| PAW3204 reset / power-down | none allocated | none | Local hardware test only | `SENSOR_NRESET` stays on a pad for bring-up; the first optical board does not spend an MCU GPIO on reset |
| PAW3204 motion wake | none allocated | none | Local hardware test only | `SENSOR_MOTION_N` stays on a pad for scope/logic-analyzer checks; the first optical board wakes from the dome only |
| Primary click | `GPIO8` | `CONFIG_POWERFINGER_DOME_PIN` | Active default | Existing wake-capable dome path |
| Ball-variant Hall gate | `GPIO9` | `CONFIG_POWERFINGER_HALL_POWER_PIN` | Cross-variant reserve | Not used on the optical capture, retained for ring-family consistency |
| Charge service enable | none | none | Fixture hardware only | Fixture VBUS presence feeds TP4054 `VCC` through the non-BOM `Q1` service jumper; ESP32-C3 `GPIO10` is no-connect in this P0 |
| Native USB D- | `GPIO18` | Reserved | Recommended first board | Service/debug path if the ring keeps onboard USB data |
| Native USB D+ | `GPIO19` | Reserved | Recommended first board | Pair with `GPIO18` for native USB service/debug |

## Notes For Capture And Firmware

- The active optical ring should not route VBUS detect to `GPIO4`; that
  collides with the current PAW3204/PMW3360 shared clock path.
- `GPIO10` is intentionally no-connect on this P0 after cutting the active
  charge-gate island. Reintroduce charge-enable firmware only with a matching
  schematic, PCB, BOM, and firmware contract update.
- The first optical board intentionally does not allocate MCU GPIOs for PAW3204
  `RST/QB/PD` or `MOTSWK`. Reset control is not needed for the polling-based
  PAW3204 bring-up path, and motion wake would add firmware sleep semantics
  before the optical stack, click stability, and charge-safety path are proven.
  Keep both nets as local pads so bring-up can still force reset/power-down and
  observe motion without making either signal part of the production contract.
- If a later board revision moves the dome off `GPIO8`, update both this file
  and the firmware Kconfig defaults in the same commit.
- The first real board should either keep the native USB pair (`GPIO18`/`GPIO19`)
  reachable or explicitly document the replacement service path in the variant
  manifest and assembly/disassembly docs.

## Current Firmware Alignment

- Host-side power-manager tests keep generic charge-enable coverage, but this
  R30 P0 board contract uses only `NTC = ADC1_CH1` and `VBUS detect = GPIO3`
  as populated MCU safety/sense paths.
- Production ring firmware still leaves NTC and VBUS-detect hardware disabled
  by default for pre-hardware dev boards; enable them only in a real R30 board
  config that matches this capture. Do not enable
  `CONFIG_POWERFINGER_CHARGE_ENABLE_PIN` for this P0.
- No production firmware symbol currently consumes `CHRG_STAT`. Keep it as a
  fixture-observed local status/test point until a spare MCU GPIO and firmware
  config are deliberately allocated.

## Current PCB Alignment

- `GPIO10` is no-connect. `Q1` is a non-BOM VBUS service jumper and TP4054
  `VCC` is fed directly from fixture `VBUS_5V`; there is no routed
  `CHARGE_EN`, `CHARGE_GATE`, or `VBUS_CHG_SW` net in this P0.
- `NTC_SENSE` is routed as the active NTC divider sense net near the battery
  connector.
- `VBAT_SENSE` now routes through `R7`/`R8` to ESP32-C3 `GPIO0`.
- `VBUS_DETECT` now routes through `R9`/`R10` to ESP32-C3 `GPIO3`.
- `CHRG_STAT` now has a local fixture status pad but no onboard pull-up; it
  does not route to the MCU in this packet.
- PAW3204 `RST/QB/PD` and `MOTSWK` are exposed on local pads in the PCB pass
  instead of consuming new MCU GPIOs. This is the explicit first-board decision:
  local bench access only, no firmware consumer, no wake-mask bit, and no reset
  GPIO until hardware evidence shows the active optical board needs one.
