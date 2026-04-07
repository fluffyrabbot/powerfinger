# Hardware Workspace

This tree is for hardware deliverables that become part of the defensive
publication record.

## Current Repo Status

- The committed hardware artifacts today are the BOM intent files under
  `hardware/bom/`.
- No KiCad projects, CAD models, assembly guides, or disassembly guides are
  checked in yet.
- `ring/`, `wand/`, and `shared/` are workspace directories for the first real
  design drops, not completed hardware packages.

## Required Structure

- `ring/` — ring-specific CAD, PCB, assembly, and disassembly files
- `wand/` — wand-specific CAD, PCB, assembly, and disassembly files
- `shared/` — reusable modules, footprints, batteries, connectors, and notes
- `bom/` — pre-schematic BOM intent files and later generated BOM exports

## Non-Negotiables

- Every design drop must include assembly and disassembly instructions.
- Every battery selection must satisfy `docs/BATTERY-SAFETY.md`.
- Claims in hardware notes must match `docs/GO-NO-GO-RUBRIC.md`.
- Do not mark unvalidated variants as production-ready.
