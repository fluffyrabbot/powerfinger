<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Shared Hardware

Use this directory for components and notes reused across ring and wand builds.

## Good Fits

- Battery selection notes
- Connector footprints and pinouts
- ESP32 module placement / antenna keep-out references
- Common charge/power subcircuits
- Shared assembly practices and repair notes
- Accessory hardware that serves multiple form factors, such as `USB-HUB/`
- Cross-variant source-alternative tables such as `SOURCE-ALTERNATIVES.md`

## Current Repo Status

- `USB-HUB/` now includes first-pass `kicad/` and `cad/` skeletons for the
  active multi-device composition lane.
- Other shared files in this directory are still documentation and sourcing
  baselines rather than finished hardware sources.

If a file is specific to one form factor, keep it under `ring/` or `wand/`
instead.
