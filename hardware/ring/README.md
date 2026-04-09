<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Ring Hardware

This directory is reserved for ring-specific hardware deliverables.

## Current Repo Status

- No ring KiCad or CAD sources are checked in yet.
- The current ring-side publication packets are:
  - `R30-OLED-NONE-NONE/`
  - `R30-BALL-NONE-NONE/`
- Those packets are grounded in the BOM intent files in `hardware/bom/` and define
  the minimum serviceability baseline for the first real hardware drop.
- The first hardware lane to prove itself remains the optical ring pair + hub
  validation path described in the top-level README.

## Expected Contents Per Variant

- `kicad/` or equivalent PCB source
- `cad/` or equivalent shell/mechanical source
- `ASSEMBLY.md`
- `DISASSEMBLY.md`
- `TEST-NOTES.md`

## Minimum Metadata

- Variant ID, sensing mechanism, angle, and intended use case
- BOM estimate at prototype quantity
- Finger-size parameters or sizing assumptions
- Surface limitations stated honestly
