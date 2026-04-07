# Ring Hardware

This directory is reserved for ring-specific hardware deliverables.

## Current Repo Status

- No ring KiCad or CAD sources are checked in yet.
- The current ring-side hardware artifacts are the BOM intent files in
  `hardware/bom/R30-OLED-NONE-NONE.csv` and `hardware/bom/R30-BALL-NONE-NONE.csv`.
- The first hardware lane to land here should support the optical ring pair +
  hub validation path described in the top-level README.

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
