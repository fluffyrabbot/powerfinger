<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Manifest

## Status

- Variant ID: `USB-HUB`
- Form factor: shared accessory / hub dongle
- Lane: active validation-lane accessory
- Publication state: BOM-backed hardware packet plus source skeletons, top-sheet schematic sources, locked first-board USB/power components, and first `usb_and_power` symbol placement
- BOM source: [hardware/bom/USB-HUB.csv](../../bom/USB-HUB.csv)
- BOM target: `~$5-6` at prototype scale
- Source skeletons:
  - `kicad/` — KiCad-oriented schematic/layout inputs, hierarchy scaffolds, and placement notes
  - `cad/` — OpenSCAD enclosure blank for the first serviceable hub pass
  - `FIRST-BOARD-CHECKLIST.md` — concrete first-board outputs required before secondary variants
  - `CONNECTOR-RETENTION-VERIFY.md` — mechanical evidence template for direct-plug safety and serviceability

This packet documents the intended repair and assembly baseline for the hub
dongle that composes multiple PowerFinger devices into one USB HID mouse.

## Intended Use

- BLE central for the active optical ring pair, with future-safe room for other
  PowerFinger device classes later
- USB HID mouse bridge to the host OS
- Optional local companion control port over the same USB connection

## Key Physical Assumptions

- Native USB comes from the ESP32-S3 module, not an external USB bridge
- The enclosure should be reopenable for button and connector service
- The USB connector must be mechanically supported by the PCB and enclosure

## Replaceable Subassemblies

- PCB or module assembly
- USB connector
- Status LED
- Boot/reset switch
- Enclosure shell

## Tracked First-Board Outputs

- [FIRST-BOARD-CHECKLIST.md](FIRST-BOARD-CHECKLIST.md) — active capture / routing / enclosure closure checklist
- [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md) — connector strain and serviceability evidence template
- `kicad/usb_hub.kicad_pcb` — first routed PCB source (not checked in yet)
- Printable enclosure exports derived from `cad/usb_hub_enclosure_blank.scad`

## Missing Artifacts

- Completed footprint files and KiCad validation for the locked first-board parts
- Routed PCB with USB connector reinforcement details
- Fit-validated enclosure CAD
- Fit notes for USB-A versus USB-C physical packaging
- Measured connector strain and enclosure-retention observations

## Required First-Hardware Evidence

- Connector strain does not rely solely on solder joints
- Enclosure can be reopened without sacrificing the PCB
- Boot/reset access does not interfere with normal use
- Record direct-plug mechanical evidence in
  [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md) before
  starting secondary hardware work
