<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Bring-Up And Service Matrix

This file defines the minimum service access and first-board checks for the
active hub lane.

The goal is simple: if the first board enumerates badly, if the connector needs
replacement, or if the composite USB firmware regresses, the board must still
be recoverable without destructive rework.

## Required Service Access

| Access point | Why it must exist | Minimum implementation |
|--------------|-------------------|------------------------|
| `VBUS_5V` | Confirm host power reaches the board | Probeable pad or connector-visible test point |
| `VREG_3V3` | Confirm regulator and brownout margin | Probeable pad |
| `GND` | Common reference for debug and measurement | Probeable pad |
| `USB_D+` / `USB_D-` | Inspect continuity and connector damage | Reachable test pads or trace access near connector |
| `EN` | Reset and recovery sequencing | Pad and/or recessed switch |
| `BOOT_N` (`GPIO0`) | Force ROM download mode | Pad and/or recessed switch |
| `UART_TX_DBG` / `UART_RX_DBG` | Recover if native USB path is broken | Probeable pads even if no permanent header is stuffed |

## First-Hardware Checks

1. Power-on sanity
   Confirm `VBUS_5V` reaches the board, `VREG_3V3` is present, and reset is not
   held unintentionally by connector-side circuitry.
2. Native USB enumeration
   Confirm the host sees the hub as a composite HID + CDC device over the
   native S3 USB path.
3. Recovery path
   Confirm asserting `BOOT_N` low while pulsing `EN` enters the ROM download
   path without requiring enclosure disassembly.
4. UART fallback
   Confirm UART0 pads are still reachable for recovery logging or flashing if
   the native USB firmware image is broken.
5. Serviceability
   Confirm connector replacement or rework does not require destroying the
   status LED, switch, or enclosure seam.

## Capture Implications

- The first board keeps `SW1` as a pad-actuated `BOOT_N` service footprint and
  exposes `EN` separately at `TP6`; recovery still requires both points.
- If the enclosure hides the USB connector shell inside a narrow cavity, leave
  extraction space for hot-air rework and non-destructive board removal.
- If USB-A geometry makes `EN`/`BOOT_N` or UART pads unreachable, that is a real
  reason to pivot to the USB-C-plus-cable fallback.
