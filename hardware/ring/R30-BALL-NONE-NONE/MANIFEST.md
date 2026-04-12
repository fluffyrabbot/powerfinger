<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-BALL-NONE-NONE Manifest

## Status

- Variant ID: `R30-BALL-NONE-NONE`
- Form factor: ring
- Lane: deferred research ring, not the active validation lane or hedge lane
- Publication state: BOM-backed hardware packet, pre-CAD, pre-PCB
- BOM source: [hardware/bom/R30-BALL-NONE-NONE.csv](../../bom/R30-BALL-NONE-NONE.csv)
- BOM target: `~$11` at prototype scale

This packet captures the intended mechanical and service boundaries for the
ball+Hall ring. It does not prove the ball socket geometry, roller friction,
or Hall delta quality yet.

## Intended Use

- Accessibility use case: a ring that may survive surfaces where optical
  tracking struggles, including glass and soft materials
- Current honesty boundary: those surface claims remain hypotheses until the
  surface protocol and control-loop gates are passed

## Key Physical Assumptions

- Sensor angle: `30 deg`
- Shell sizing: parametric for finger circumference
- Tracking mechanism: 5 mm ball with four roller/magnet/Hall channels
- Battery and charging safety constraints are the same as the optical ring

## Replaceable Subassemblies

- PCB or module assembly
- Ball
- Roller and magnet set
- Hall sensor board area or daughter assembly
- LiPo cell
- USB-C connector
- Shell body and socket geometry
- Dome click element

## Missing Artifacts

- CAD proving the ball socket and roller retention geometry
- PCB layout proving four analog Hall channels fit cleanly
- Tolerance notes for ball clearance, magnet spacing, and shell flex
- Test notes for glass, fabric, skin, and rigid surfaces

## Required First-Hardware Evidence

- Ball can be removed for cleaning or replacement without shell destruction
- Roller and magnet alignment is serviceable and repeatable
- Hall power gating remains compatible with the published BOM and power budget
