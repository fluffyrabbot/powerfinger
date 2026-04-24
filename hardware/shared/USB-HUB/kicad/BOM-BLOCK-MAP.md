<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB BOM To Block Map

This file keeps the first hub schematic aligned with the published BOM.
Read it together with [CAPTURE-BINDINGS.md](CAPTURE-BINDINGS.md) and
[P0-COMPONENT-LOCKS.md](P0-COMPONENT-LOCKS.md) so the block map and the actual
symbol/footprint choices stay aligned.

| Schematic block | BOM refs | What belongs in the first capture | Notes |
|-----------------|----------|-----------------------------------|-------|
| MCU / radio | `U1`, `C1` | `ESP32-S3-MINI-1-N8` module and local decoupling | Native USB and BLE central both live here |
| USB connector path | `J1`, `R1`, `D1`, `C2` | `SOFNG USB-05`, `R1A` / `R1B` as 22R USB-side resistors, `USBLC6-2SC6`, and VBUS-side bulk/input capacitance | Keep the connector decision explicit, protection close to the connector, and series resistors close to the S3 |
| Regulation | `U2`, `C1`, `C3` | `RT9080-33GJ5` 5V-to-3.3V rail and required output capacitance | `VREG_3V3` belongs to the `usb_and_power` sheet, not to an implied dev-board rail |
| Status indication | `LED1`, `R2` | Optional but capture-ready status LED path | Do not let this become mechanically unserviceable |
| Bring-up control | `SW1` | Boot/reset access path | Recessed is fine, unreachable is not |
| Service / recovery | `—` | `EN`, `BOOT_N`, `UART_TX_DBG`, `UART_RX_DBG`, `3V3`, and `GND` service pads | Required capture item even though it is mostly copper and pad geometry, not BOM line items |
| Mechanics | `PCB1`, `ENCL1` | Mechanical notes and connector-support expectations | The schematic should carry enough notes that the PCB is not designed like a floating connector tongue |

## First-Board Support Parts

`usb_hub.kicad_pcb` also includes `R3` / `C4` as an EN RC support pair and
`C5` / `C6` as DNI USB shunt-cap footprints. These are board-reality support
items rather than a new product feature; they keep the native-USB board
recoverable and still fit the current low-cost hub intent.

## Capture Boundary

The first schematic should support the hub’s active role: BLE central plus USB
HID bridge, with minimal bring-up controls and honest connector serviceability.
