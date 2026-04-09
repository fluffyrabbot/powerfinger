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
  ESP32-C3, so the new VBUS-detect and charge-enable choices avoid them.
- Keep the pre-hardware dev-board lane honest: firmware Kconfig defaults for
  the NTC, charge-enable gate, and VBUS detect stay disabled (`-1`) until the
  real ring hardware populates them.

## First-Pass Pin Plan

| Function | ESP32-C3 resource | Firmware symbol | Status | Notes |
|----------|-------------------|-----------------|--------|-------|
| VBAT sense | `ADC1_CH0` / `GPIO0` | `CONFIG_POWERFINGER_VBAT_ADC_CHANNEL` | Active default | Existing battery monitor path |
| Cell NTC sense | `ADC1_CH1` / `GPIO1` | `CONFIG_POWERFINGER_NTC_ADC_CHANNEL` | Recommended first board | Matches the published battery-safety requirement without colliding with the optical lane |
| USB VBUS detect | `GPIO3` | `CONFIG_POWERFINGER_VBUS_DETECT_PIN` | Recommended first board | Input only from a resistor-divided 5V-detect net |
| Sensor clock | `GPIO4` | `CONFIG_POWERFINGER_SENSOR_SCLK_PIN` | Active default | Shared PAW3204 / PMW3360 clock path |
| Sensor data out | `GPIO5` | `CONFIG_POWERFINGER_SENSOR_SDIO_PIN` | Active default | PAW3204 bidirectional SDIO, PMW3360 MOSI |
| Sensor chip select | `GPIO6` | `CONFIG_POWERFINGER_SENSOR_NCS_PIN` | Active default | PMW3360-only; leave unconnected on PAW3204 capture |
| Sensor data in | `GPIO7` | `CONFIG_POWERFINGER_SENSOR_MISO_PIN` | Active default | PMW3360-only; leave unconnected on PAW3204 capture |
| Primary click | `GPIO8` | `CONFIG_POWERFINGER_DOME_PIN` | Active default | Existing wake-capable dome path |
| Ball-variant Hall gate | `GPIO9` | `CONFIG_POWERFINGER_HALL_POWER_PIN` | Cross-variant reserve | Not used on the optical capture, retained for ring-family consistency |
| Charge enable gate | `GPIO10` | `CONFIG_POWERFINGER_CHARGE_ENABLE_PIN` | Recommended first board | Non-strapping output for the default-off P-channel MOSFET gate |
| Native USB D- | `GPIO18` | Reserved | Recommended first board | Service/debug path if the ring keeps onboard USB data |
| Native USB D+ | `GPIO19` | Reserved | Recommended first board | Pair with `GPIO18` for native USB service/debug |

## Notes For Capture And Firmware

- The active optical ring should not route VBUS detect to `GPIO4`; that
  collides with the current PAW3204/PMW3360 shared clock path.
- `GPIO10` is intentionally reserved for the charge-enable gate so the safe-
  default pull-up on the MOSFET does not become a new boot-strap dependency.
- If a later board revision moves the dome off `GPIO8`, update both this file
  and the firmware Kconfig defaults in the same commit.
- The first real board should either keep the native USB pair (`GPIO18`/`GPIO19`)
  reachable or explicitly document the replacement service path in the variant
  manifest and assembly/disassembly docs.

## Current Firmware Alignment

- Host-side power-manager tests now use the same first-pass safety path:
  `NTC = ADC1_CH1`, `VBUS detect = GPIO3`, `charge enable = GPIO10`.
- Production ring firmware still treats these three nets as unpopulated by
  default until the actual optical-ring hardware capture lands.
