<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Schematic Capture Pack

This file is the capture-ready bridge between the published hub BOM and the
first real KiCad schematic for the active hub lane.

References:

- [INTERFACE-CONTRACT.md](INTERFACE-CONTRACT.md)
- [BRINGUP-SERVICE-MATRIX.md](BRINGUP-SERVICE-MATRIX.md)
- [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md)
- [BOM-BLOCK-MAP.md](BOM-BLOCK-MAP.md)
- [RECOMMENDED-FIRST-CAPTURE.md](RECOMMENDED-FIRST-CAPTURE.md)
- [hardware/bom/USB-HUB.csv](../../bom/USB-HUB.csv)
- [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- [docs/REGULATORY-PRESCAN.md](../../../docs/REGULATORY-PRESCAN.md)

## Recommended Sheet Split

1. `usb_and_power`
   Includes connector choice, native USB D+/D- path, any VBUS protection,
   decoupling, and any USB-side ESD support included during capture.
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
- The hub BOM now carries USB-side ESD protection as a baseline first-capture
  item. Capture it now instead of pretending the board is “too small” to need it.
- Preserve the service access defined in
  [BRINGUP-SERVICE-MATRIX.md](BRINGUP-SERVICE-MATRIX.md) even if the final
  enclosure is a compact dongle.

## First KiCad Checklist

- Pick the connector path intentionally, not by whichever footprint is fastest
  to drag into the sheet.
- Put a note block on the top sheet pointing back to
  [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md).
- Reserve the S3 antenna consequence early so connector mechanics do not crowd it.
- Add service/test pads before the board outline gets tiny.
