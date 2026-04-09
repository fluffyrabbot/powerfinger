<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Schematic Capture Pack

This file is the capture-ready bridge between the published BOM and the first
real KiCad schematic for the active optical ring lane.

References:

- [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md)
- [BOM-BLOCK-MAP.md](BOM-BLOCK-MAP.md)
- [RECOMMENDED-FIRST-CAPTURE.md](RECOMMENDED-FIRST-CAPTURE.md)
- [hardware/bom/R30-OLED-NONE-NONE.csv](../../bom/R30-OLED-NONE-NONE.csv)
- [docs/BATTERY-SAFETY.md](../../../docs/BATTERY-SAFETY.md)
- [docs/DUAL-FOOTPRINT.md](../../../docs/DUAL-FOOTPRINT.md)
- [docs/REGULATORY-PRESCAN.md](../../../docs/REGULATORY-PRESCAN.md)

## Recommended Sheet Split

1. `power_and_charge`
   Includes battery connector, TP4054 path, charge-enable MOSFET, NTC divider,
   LDO, VBAT sensing, and any charge-status wiring.
2. `mcu_radio`
   Includes `ESP32-C3-MINI-1-N4`, boot/enable support, programming/debug pads,
   and module decoupling.
3. `sensor_and_click`
   Includes the PAW3204-class sensor, lens-aligned support passives, motion
   output if available, and the primary dome click path.
4. `usb_and_service`
   Includes the USB-C receptacle, sink-attach support, and any service/test
   pads that must be reachable without reworking the battery.

## Named Net Groups To Use

Use stable functional names rather than early GPIO-number names wherever
possible. That keeps the P0 ring schematic readable and makes later variants
easier to branch.

- Power:
  - `VBUS_5V`
  - `VBAT_PROTECTED`
  - `VBAT_SENSE`
  - `VREG_3V3`
  - `CHARGE_EN`
  - `CHRG_STAT`
  - `NTC_SENSE`
- USB / service:
  - `USB_D+`
  - `USB_D-`
  - `USB_CC1_RD`
  - `USB_CC2_RD`
- Sensor / click:
  - `SENSOR_SCLK`
  - `SENSOR_SDIO`
  - `SENSOR_MOTION_N`
  - `SENSOR_NRESET`
  - `CLICK_PRIMARY_N`

## Capture-Specific Notes

- The current BOM now includes the USB-C sink-attach pull-down resistors, the
  USB ESD array, and the SPI clock damping resistor. Treat them as baseline P0
  capture items rather than optional cleanup.
- Keep the PAW3204 capture P0-specific. Do not fold PMW3360 rails or haptic
  circuitry into this first schematic.
- The NTC and charge-enable path are not optional. The battery safety docs have
  already promoted them from “nice to have” to baseline hardware.
- Leave the antenna keep-out consequence visible in the schematic notes so the
  later board layout does not quietly compromise it.

## First KiCad Checklist

- Create a fresh project and name the first top sheet for the variant, not a
  generic “ring.”
- Place a note block on the top sheet pointing back to
  [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md).
- Capture the power and charge tree first, before assigning MCU GPIOs.
- Add explicit notes for every BOM line that is still a category placeholder
  rather than a single fully locked MPN.
- Mark every service-facing electrical interface that must remain accessible for
  bring-up or repair.
