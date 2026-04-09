<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Hardware Workspace

This tree is for hardware deliverables that become part of the defensive
publication record.

## Current Repo Status

- The committed hardware artifacts today are still BOM-first, but each BOM-backed
  variant now has a publication packet with a manifest plus pre-CAD assembly and
  disassembly guidance.
- No KiCad projects or CAD models are checked in yet.
- The new packets document intended serviceability and replacement boundaries;
  they are not build-validated proof that the mechanical design already works.

## Required Structure

- `ring/` — ring-specific CAD, PCB, assembly, and disassembly files
- `wand/` — wand-specific CAD, PCB, assembly, and disassembly files
- `shared/` — reusable modules, accessories, footprints, batteries, connectors,
  and notes
- `bom/` — pre-schematic BOM intent files and later generated BOM exports

## Publication Packets Present Today

- `ring/R30-OLED-NONE-NONE/` — active validation-lane ring packet
- `ring/R30-BALL-NONE-NONE/` — hedge ring packet for ball+Hall sensing
- `wand/WSTD-BALL-NONE-NONE/` — wand hedge-lane packet
- `shared/USB-HUB/` — hub packet used by multi-device composition
- `shared/SOURCE-ALTERNATIVES.md` — cross-variant second-source baseline

Each packet includes:

- `MANIFEST.md` — what is actually published now, what is still missing, and the
  repairability boundaries
- `ASSEMBLY.md` — non-destructive assembly expectations for the first hardware drop
- `DISASSEMBLY.md` — battery-safe teardown order and replacement expectations

## Non-Negotiables

- Every design drop must include assembly and disassembly instructions.
- Every battery selection must satisfy `docs/BATTERY-SAFETY.md`.
- Claims in hardware notes must match `docs/GO-NO-GO-RUBRIC.md`.
- Do not mark unvalidated variants as production-ready.
