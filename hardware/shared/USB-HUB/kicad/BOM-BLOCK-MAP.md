<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB BOM To Block Map

This file keeps the first hub schematic aligned with the published BOM.

| Schematic block | BOM refs | What belongs in the first capture | Notes |
|-----------------|----------|-----------------------------------|-------|
| MCU / radio | `U1` | `ESP32-S3-MINI-1-N8` module and required decoupling | Native USB and BLE central both live here |
| USB connector path | `J1`, `R1`, `D1` | USB connector, USB-side resistors, and USB ESD protection | Keep the connector decision explicit and the protection close to the connector |
| Board decoupling | `C1`, `C2` | Board-level decoupling and bulk cap placement | Keep this close to the S3 power entry |
| Status indication | `LED1`, `R2` | Optional but capture-ready status LED path | Do not let this become mechanically unserviceable |
| Bring-up control | `SW1` | Boot/reset access path | Recessed is fine, unreachable is not |
| Service / recovery | `—` | `EN`, `BOOT_N`, `UART_TX_DBG`, `UART_RX_DBG`, `3V3`, and `GND` service pads | Required capture item even though it is mostly copper and pad geometry, not BOM line items |
| Mechanics | `PCB1`, `ENCL1` | Mechanical notes and connector-support expectations | The schematic should carry enough notes that the PCB is not designed like a floating connector tongue |

## Capture Boundary

The first schematic should support the hub’s active role: BLE central plus USB
HID bridge, with minimal bring-up controls and honest connector serviceability.
