<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Interface Contract

This file is the first-pass electrical contract between the active hub
schematic capture, enclosure packaging, and the current hub firmware path.

It exists to stop three kinds of drift:

- native USB getting replaced by a convenience bridge during capture
- boot and recovery access disappearing because the HID + CDC path "should be enough"
- connector choice changing without preserving serviceability

References:

- [SCHEMATIC-CAPTURE.md](SCHEMATIC-CAPTURE.md)
- [BOM-BLOCK-MAP.md](BOM-BLOCK-MAP.md)
- [RECOMMENDED-FIRST-CAPTURE.md](RECOMMENDED-FIRST-CAPTURE.md)
- [Espressif ESP32-S3 GPIO reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html)
- [Espressif ESP32-S3 built-in USB/JTAG reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/jtag-debugging/configure-builtin-jtag.html)
- [Espressif ESP32-S3 schematic checklist](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html)

## Contract Rules

- Keep the user-facing USB data path native to the `ESP32-S3-MINI-1-N8`.
  Do not insert a USB-UART bridge into the normal HID + CDC path.
- Treat `GPIO19` and `GPIO20` as reserved for native USB on the first hub
  board: `GPIO19 = USB_D-`, `GPIO20 = USB_D+`.
- Treat `GPIO0`, `GPIO3`, `GPIO45`, and `GPIO46` as strapping pins. Only the
  explicit boot/recovery path should touch `GPIO0`, and no indicator or
  connector-side circuit should disturb those pins at reset.
- Preserve a recoverable bring-up path even if the native USB firmware image is
  broken. That means exposing `EN`, `GPIO0`, `U0TXD`, `U0RXD`, `3V3`, and `GND`
  as service access, not just hoping the composite USB device always enumerates.
- Keep the first board aligned with the current firmware reality:
  one TinyUSB composite device that exposes both HID and CDC over the native USB
  connection.

## First-Pass Interface Plan

| Function | ESP32-S3 resource | Capture net | Status | Notes |
|----------|-------------------|-------------|--------|-------|
| Host USB D- | `GPIO19` | `USB_D-` | Fixed | Native USB full-speed path |
| Host USB D+ | `GPIO20` | `USB_D+` | Fixed | Native USB full-speed path |
| USB bus power | `5V` / connector VBUS | `VBUS_5V` | Fixed | User-facing USB power entry |
| Local regulated rail | `3V3` rail | `VREG_3V3` | Fixed | Hub logic rail |
| Chip enable / reset | `EN` | `EN` | Fixed | Required for bring-up and recovery |
| ROM download strap | `GPIO0` | `BOOT_N` | Fixed | Active-low service path only |
| UART0 TX | `GPIO43` | `UART_TX_DBG` | Recommended first board | Default ROM/log UART TX |
| UART0 RX | `GPIO44` | `UART_RX_DBG` | Recommended first board | Default ROM/log UART RX |
| Status LED | `GPIO21` | `LED_STATUS` | First-board assignment | Non-strapping, kept off native USB and recovery pins |
| Boot/reset button | `EN` and/or `GPIO0` access path | `SW_BOOT_RECOVERY` | Required service path | Single button is acceptable only if pads expose the rest of recovery |

## Notes For Capture

- The USB-A-first path from [RECOMMENDED-FIRST-CAPTURE.md](RECOMMENDED-FIRST-CAPTURE.md)
  is still the default. If capture pivots to USB-C plus a short cable, preserve
  the same native USB and recovery nets.
- The first custom board owns `VBUS_5V` to `VREG_3V3` regulation locally. Do
  not treat `VREG_3V3` as a dev-board inheritance or an off-page assumption.
- `GPIO19` and `GPIO20` are used by the S3 USB block by default. Do not hang
  LEDs, strap resistors, or unrelated debug logic on them.
- `R1A` and `R1B` implement the first-board 22R series damping decision for
  `GPIO19` / `GPIO20`; keep those parts close to the module side of the pair.
- `GPIO43` and `GPIO44` are the default UART0 pins. Even if the first board
  does not stuff a connector for them, provide probeable service pads.
- The status LED should stay on `GPIO21` unless a later layout pass proves that
  it compromises antenna clearance or service access.

## Current Firmware Alignment

- The current hub firmware enumerates as one native USB composite device with
  both HID and CDC enabled.
- The hardware revision string in firmware still says `DEVBOARD-S3`; that
  should remain true until a real custom hub board exists and has been brought
  up honestly.
