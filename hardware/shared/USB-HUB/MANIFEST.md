<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Manifest

## Status

- Variant ID: `USB-HUB`
- Form factor: shared accessory / hub dongle
- Lane: active validation-lane accessory
- Publication state: BOM-backed hardware packet plus footprint-backed KiCad
  first-board pass for the direct-plug native-USB hub
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
- The current direct-plug board is a stepped USB-A dongle, not the older
  `~20 x 12 mm` placeholder: a host-side USB-A nose, a wider `54 x 26 mm`
  module/service body, rear antenna keep-out, and probeable service pads along
  the reopenable seam. The nose is widened to carry the SOFNG USB-05 shell-tab
  pads, so adjacent-port clearance is an open mechanical check.

## Replaceable Subassemblies

- PCB or module assembly
- USB connector
- Status LED
- Boot/reset switch
- Enclosure shell

## Tracked First-Board Outputs

- [FIRST-BOARD-CHECKLIST.md](FIRST-BOARD-CHECKLIST.md) — active capture / routing / enclosure closure checklist
- [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md) — connector strain and serviceability evidence template
- `kicad/usb_hub.kicad_pcb` — first routed PCB source with connector, ESD,
  regulator, ESP32-S3 module, 22R USB series resistors, service pads, and
  antenna keep-out represented
- Printable enclosure exports derived from `cad/usb_hub_enclosure_blank.scad`

## Missing Artifacts

- Fit-validated enclosure CAD
- Measured host-port clearance for the stepped USB-A direct-plug body
- Measured connector strain and enclosure-retention observations

## Required First-Hardware Evidence

- Connector strain does not rely solely on solder joints
- Enclosure can be reopened without sacrificing the PCB
- Boot/reset access does not interfere with normal use
- KiCad CLI ERC and PCB DRC remain clean after local footprint/library loading
- ESP32-S3 antenna zone remains clear of copper and metal enclosure features
- Adjacent-port clearance is checked with the stepped USB-A nose and wider body,
  not assumed from the schematic
- Record direct-plug mechanical evidence in
  [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md) before
  starting secondary hardware work
