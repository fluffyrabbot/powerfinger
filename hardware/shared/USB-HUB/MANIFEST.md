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

This packet documents the intended repair and assembly baseline for the hub
dongle that composes multiple PowerFinger devices into one USB HID mouse.

## Intended Use

- BLE central for rings, pucks, and wand variants
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
