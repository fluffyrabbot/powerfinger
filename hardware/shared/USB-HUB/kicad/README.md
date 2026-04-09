<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB KiCad Skeleton

This directory is the schematic/layout starting point for the active hub
accessory lane.

There is no routed KiCad project here yet. The purpose of this skeleton is to
pin down the board blocks and placement rules before a small USB dongle PCB is
captured in a way that fights serviceability or native USB.

## What Belongs Here Next

- `usb_hub.kicad_sch`
- `usb_hub.kicad_pcb`
- Optional local footprints if the chosen connector or enclosure retention needs
  a custom land pattern
- `INTERFACE-CONTRACT.md` — first-pass native USB and recovery-pin contract
- `BRINGUP-SERVICE-MATRIX.md` — mandatory recovery access and first-board checks
- `SCHEMATIC-CAPTURE.md` — sheet split, net naming, and capture order
- `BOM-BLOCK-MAP.md` — map the BOM into the first hub schematic
- `RECOMMENDED-FIRST-CAPTURE.md` — the recommended first board scope


## Capture Order

1. Native USB path from connector to `ESP32-S3-MINI-1-N8`
2. Power entry, decoupling, and any ESD/connector protection needed
3. Boot/reset and bring-up access
4. Status LED and any test pads

## Hard Constraints

- Native USB is load-bearing for the hub. Do not insert an unnecessary USB-UART
  bridge into the normal data path.
- Treat [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md) as a blocker list.
- Connector mechanical support matters as much as the schematic.
