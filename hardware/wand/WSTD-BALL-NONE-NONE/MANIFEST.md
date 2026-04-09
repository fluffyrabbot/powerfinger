<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# WSTD-BALL-NONE-NONE Manifest

## Status

- Variant ID: `WSTD-BALL-NONE-NONE`
- Form factor: wand
- Lane: hedge lane
- Publication state: BOM-backed hardware packet, pre-CAD, pre-PCB
- BOM source: [hardware/bom/WSTD-BALL-NONE-NONE.csv](../../bom/WSTD-BALL-NONE-NONE.csv)
- BOM target: `~$14` at prototype scale

This packet defines the current intended service and assembly boundaries for the
wand without pretending the pen-angle or surface claims are already verified.

## Intended Use

- Accessibility use case: pen-grip pointing for users who need less forearm
  pronation than a flat mouse
- Tracking claim under evaluation: ball+Hall sensing at natural pen angles on
  harder cases than direct optical pens handle reliably
- Honesty boundary: angle range and surface flexibility remain to be measured

## Key Physical Assumptions

- Body format: approximately 8 mm OD by 120 mm tube or equivalent shell
- Tip assembly: removable ball socket module
- Battery bay: separate from tip service path
- Control input: thumb barrel switch, optional tip click as a secondary path

## Replaceable Subassemblies

- Rigid PCB or electronics carrier
- Ball, roller, and magnet set
- Tip module
- Barrel switch
- LiPo cell
- USB-C end-cap module
- Tube body

## Missing Artifacts

- CAD source for tube, tip, and end-cap retention
- PCB layout and connector routing
- Verified pen-angle and surface test notes
- Measured center-of-mass and hand-fatigue observations

## Required First-Hardware Evidence

- Tip module is removable without sacrificing the battery bay
- USB-C end cap is serviceable and not welded into the body
- Switch replacement does not require destroying the tube body

