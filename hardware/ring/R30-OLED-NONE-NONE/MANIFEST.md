<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Manifest

## Status

- Variant ID: `R30-OLED-NONE-NONE`
- Form factor: ring
- Lane: active validation lane
- Publication state: BOM-backed hardware packet plus source skeletons and top-sheet schematic sources
- BOM source: [hardware/bom/R30-OLED-NONE-NONE.csv](../../bom/R30-OLED-NONE-NONE.csv)
- BOM target: `~$9` at prototype scale
- Source skeletons:
  - `kicad/` — KiCad-oriented schematic/layout inputs, hierarchy scaffolds, and placement notes
  - `cad/` — OpenSCAD shell blank for the first parametric ring pass
  - `FIRST-BOARD-CHECKLIST.md` — concrete first-board outputs required before secondary variants
  - `STACKUP-VERIFY.md` — measured package-closure template for height, focal distance, battery, and RF evidence

This packet documents the minimum repairability and assembly expectations for
the first optical ring hardware drop. It does not claim the package geometry,
focal distance, RF behavior, or click ergonomics are already proven in hardware.

## Intended Use

- Primary use case: one of the two identical rings in the default cursor +
  scroll pair
- Accessibility claim under evaluation: a user with limited wrist mobility can
  point and click on common opaque surfaces without host-side remapping
- Surface claim today: best-case on opaque rigid surfaces only; not glass

## Key Physical Assumptions

- Sensor angle: `30 deg`
- Shell sizing: parametric for finger circumference, not one fixed ring size
- Focal-distance strategy: raised rim and glide pads maintain the optical gap
- Shell closure must remain non-hermetic and reopenable for battery replacement

## Replaceable Subassemblies

- PCB or module assembly
- Optical sensor plus matched lens stack
- LiPo cell
- USB-C charging connector
- Shell body and structural rim
- UHMWPE glide pads
- Dome click element

## Tracked First-Board Outputs

- [FIRST-BOARD-CHECKLIST.md](FIRST-BOARD-CHECKLIST.md) — active capture / routing / CAD closure checklist
- [STACKUP-VERIFY.md](STACKUP-VERIFY.md) — package-closure evidence template
- `kicad/r30_oled_none_none.kicad_pcb` — first routed PCB source (not checked in yet)
- Printable shell exports derived from `cad/r30_oled_none_none_shell_blank.scad`

## Missing Artifacts

- Completed component capture and symbol/footprint binding
- Routed PCB with antenna keep-out and charge-path validation
- Fit-validated shell CAD and printable/exported models
- Measured ring stackup with focal-distance verification
- Assembly photos, torque values, and validated fit tolerances
- Test notes tied to [docs/GO-NO-GO-RUBRIC.md](../../../docs/GO-NO-GO-RUBRIC.md)

## Required First-Hardware Evidence

- Real stackup fits the height budget from
  [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- Battery selection still satisfies
  [docs/BATTERY-SAFETY.md](../../../docs/BATTERY-SAFETY.md)
- Shell can be opened and reclosed without destroying the battery bay, sensor
  mount, or antenna keep-out
- Record package-closure evidence in [STACKUP-VERIFY.md](STACKUP-VERIFY.md)
  before starting secondary ring or puck hardware
