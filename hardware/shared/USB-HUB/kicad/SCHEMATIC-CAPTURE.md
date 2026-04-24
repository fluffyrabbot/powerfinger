<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Schematic Capture Pack

This file is the capture-ready bridge between the published hub BOM and the
first real KiCad schematic for the active hub lane.

References:

- [INTERFACE-CONTRACT.md](INTERFACE-CONTRACT.md)
- [BRINGUP-SERVICE-MATRIX.md](BRINGUP-SERVICE-MATRIX.md)
- [CAPTURE-BINDINGS.md](CAPTURE-BINDINGS.md)
- [P0-COMPONENT-LOCKS.md](P0-COMPONENT-LOCKS.md)
- [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md)
- [BOM-BLOCK-MAP.md](BOM-BLOCK-MAP.md)
- [RECOMMENDED-FIRST-CAPTURE.md](RECOMMENDED-FIRST-CAPTURE.md)
- [hardware/bom/USB-HUB.csv](../../bom/USB-HUB.csv)
- [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- [docs/REGULATORY-PRESCAN.md](../../../docs/REGULATORY-PRESCAN.md)

## Recommended Sheet Split

1. `usb_and_power`
   Includes connector choice, `VBUS_5V` entry, `RT9080-33GJ5` regulation,
   native USB D+/D- path, required input/output capacitors, and any USB-side
   ESD support included during capture.
2. `mcu_radio`
   Includes `ESP32-S3-MINI-1-N8`, boot/enable support, and module decoupling.
3. `controls_and_indicators`
   Includes status LED, boot/reset control, and service/test pads.

## Named Net Groups To Use

- Power / USB:
  - `VBUS_5V`
  - `VREG_3V3`
  - `USB_D+`
  - `USB_D-`
- Control / bring-up:
  - `EN`
  - `BOOT_N`
  - `LED_STATUS`
  - `UART_TX_DBG`
  - `UART_RX_DBG`

## Capture-Specific Notes

- Native USB is load-bearing. Do not route the user-facing data path through a
  USB-UART bridge.
- Use [INTERFACE-CONTRACT.md](INTERFACE-CONTRACT.md) as the source of truth for
  the native USB, boot, reset, and UART recovery nets before assigning GPIOs.
- Use [CAPTURE-BINDINGS.md](CAPTURE-BINDINGS.md) before choosing connector,
  module, and recovery-control footprints so the first hub board does not drift
  into generic placeholders.
- Use [P0-COMPONENT-LOCKS.md](P0-COMPONENT-LOCKS.md) when placing the first real
  `usb_and_power` symbols so the chosen dongle connector and USB protector do
  not get "temporarily" swapped for library-default stand-ins.
- The hub BOM now carries both USB-side ESD protection and an explicit 3.3V
  regulator baseline. Capture them now instead of hand-waving `VREG_3V3` as
  something the board will “figure out later.”
- `usb_and_power.kicad_sch` now contains a first real placement pass for
  `J1`, `D1`, `U2`, `C2`, `C3`, and the resolved `R1A` / `R1B` native-USB
  series resistors. KiCad ERC now runs locally and passes at error severity.
- `mcu_radio.kicad_sch` now places the first-board `ESP32-S3-MINI-1-N8`
  capture subset for native USB, recovery, UART, status, power, and EPAD/GND.
- `usb_hub.kicad_pcb` carries first-board consequences that the sheet symbols
  alone cannot show: stepped board edge, connector reinforcement holes, service
  pad row, rear antenna keep-out, and no-BOM DNI USB shunt-cap placeholders.
- Preserve the service access defined in
  [BRINGUP-SERVICE-MATRIX.md](BRINGUP-SERVICE-MATRIX.md) even if the final
  enclosure is a compact dongle.

## First KiCad Checklist

- Pick the connector path intentionally, not by whichever footprint is fastest
  to drag into the sheet.
- Lock the `VBUS_5V` to `VREG_3V3` path intentionally, including the LDO and its
  required capacitors, in the same pass as the connector decision.
- Put a note block on the top sheet pointing back to
  [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md).
- Reserve the S3 antenna consequence early so connector mechanics do not crowd it.
- Add service/test pads before the board outline gets tiny.
