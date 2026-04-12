<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Hardware Workspace

This tree is for hardware deliverables that become part of the defensive
publication record.

## Current Repo Status

- The committed hardware artifacts are still BOM-first, but each BOM-backed
  variant now has a publication packet with a manifest plus pre-CAD assembly and
  disassembly guidance.
- The active validation lane now also has initial source skeletons:
  - `ring/R30-OLED-NONE-NONE/{kicad,cad}/`
  - `shared/USB-HUB/{kicad,cad}/`
- Those skeletons are design-starting points, not validated schematics, routed
  PCBs, or mechanically proven CAD.

## Required Structure

- `ring/` — ring-specific CAD, PCB, assembly, and disassembly files
- `wand/` — wand-specific CAD, PCB, assembly, and disassembly files
- `shared/` — reusable modules, accessories, footprints, batteries, connectors,
  and notes
- `bom/` — pre-schematic BOM intent files and later generated BOM exports

## Publication Packets Present Today

- `ring/R30-OLED-NONE-NONE/` — active validation-lane ring packet
- `ring/R30-BALL-NONE-NONE/` — deferred research packet for ball+Hall sensing
- `wand/WSTD-BALL-NONE-NONE/` — wand hedge-lane packet
- `shared/USB-HUB/` — hub packet used by multi-device composition
- `shared/SOURCE-ALTERNATIVES.md` — cross-variant second-source baseline

Each packet includes:

- `MANIFEST.md` — what is actually published now, what is still missing, and the
  repairability boundaries
- `ASSEMBLY.md` — non-destructive assembly expectations for the first hardware drop
- `DISASSEMBLY.md` — battery-safe teardown order and replacement expectations

Active-lane packets may also include:

- `kicad/` — schematic-capture and placement inputs for KiCad work
- `cad/` — editable OpenSCAD source for first-pass mechanical envelopes

## Non-Negotiables

- Every design drop must include assembly and disassembly instructions.
- Every battery selection must satisfy `docs/BATTERY-SAFETY.md`.
- Claims in hardware notes must match `docs/GO-NO-GO-RUBRIC.md`.
- Do not mark unvalidated variants as production-ready.
- Do not start secondary hardware variants before the active ring + hub packets
  have concrete first-board outputs and linked validation evidence.
