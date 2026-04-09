<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Wand Hardware

This directory is reserved for wand-specific hardware deliverables.

## Current Repo Status

- No wand KiCad or CAD sources are checked in yet.
- The current wand-side publication packet is `WSTD-BALL-NONE-NONE/`, grounded in
  `hardware/bom/WSTD-BALL-NONE-NONE.csv`.
- The wand remains the hedge lane rather than the active validation lane.
- The packet documents the current intended service path without claiming the
  package geometry is already proven.

## Expected Contents Per Variant

- `kicad/` or equivalent PCB source
- `cad/` or equivalent body/tip source
- `ASSEMBLY.md`
- `DISASSEMBLY.md`
- `TEST-NOTES.md`

## Minimum Metadata

- Variant ID, sensing mechanism, and intended accessibility use case
- BOM estimate at prototype quantity
- Supported angle range as tested, not assumed
- Surface limitations stated honestly
