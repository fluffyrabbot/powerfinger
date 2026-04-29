<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Current ERC / DRC Violations

This packet is **not** fabrication-released. The first routed PCB pass and the
schematic scaffolds carry known violations that block fab and any
schematic-parity claim.

This file is a snapshot of those violations so the manifest cannot quietly
drift. It is intentionally not a generated artifact: regenerate the raw reports
locally before relying on these counts (see "How to regenerate" below). When
you close violations, update the counts here in the same commit so the snapshot
stays honest.

## Snapshot

Toolchain: `kicad-cli 10.0.1` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc` violations | 34 | All severity-error |
| `pcb drc` violations | 324 | All severity-error |
| `pcb drc` unconnected items | 41 | Net endpoints with no track to them |
| `pcb drc` schematic-parity issues | 27 | PCB references that have no symbol counterpart |

The schematic sheets in `sheets/` are still scaffolds: they declare
hierarchical labels but contain no symbols, no wires, and no real components.
Until those sheets are populated, ERC will stay red and DRC `--schematic-parity`
will stay red regardless of how clean the PCB routing is.

## ERC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `label_dangling` | 25 | Hierarchical labels declared on the root sheet have no matching wire on the sub-sheets |
| `endpoint_off_grid` | 9 | Wire/pin endpoints not snapped to the connection grid |

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `solder_mask_bridge` | 96 | Adjacent pads of different nets share an unbroken mask aperture |
| `unconnected_items` | 41 | Routed pads with no track or via reaching them |
| `extra_footprint` | 27 | Footprints on the PCB with no symbol counterpart in the schematic |
| `copper_edge_clearance` | 26 + 8 | Copper inside the 0.5 mm board-edge clearance band |
| `track_width` | 23 | Tracks routed at 0.18 mm where the board setup minimum is 0.20 mm |
| `silk_over_copper` | 12 | Silkscreen text crossing exposed copper |
| `text_height` | 11 | Silkscreen text at 0.6 mm where the rule minimum is 0.8 mm |
| `tracks_crossing` | 8 | Tracks of different nets physically crossing on the same layer |
| `lib_footprint_issues` | 7 | Footprint library `Resistor_SMD` not configured locally |

The shorting-class and crossing-class violations are the blocking ones for
fabrication. Mask-bridge and edge-clearance violations are easier mechanical
fixes but still block a clean release.

## What this means for downstream packets

- The `cad/r30_oled_none_none_shell_blank.scad` board-coordinate bindings
  reference the current PCB pass. If a violation fix moves `J1`, `J_BAT`,
  `SW1`, `U2`, or the antenna keep-out, the shell CAD must move with it.
- The companion firmware contract treats `VBAT_SENSE`, `VBUS_DETECT`, and
  `CHRG_STAT` as bring-up pads only until `R7`–`R11` (added in the active
  BOM CSV) are wired in the schematic and routed in the PCB.

## How to regenerate

From the repo root:

```bash
mkdir -p build-kicad/R30-OLED-NONE-NONE
kicad-cli sch erc \
  --severity-all \
  -o build-kicad/R30-OLED-NONE-NONE/erc.txt \
  hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch

kicad-cli pcb drc \
  --severity-all \
  --schematic-parity \
  -o build-kicad/R30-OLED-NONE-NONE/drc.txt \
  hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb
```

Or use the wrapper:

```bash
scripts/verify-firmware-local.sh --kicad-only
```

The wrapper writes reports under `build-kicad/<packet>/`. Outputs are not
checked in; only this summary file is.

## Closing the gap

The first-board checklist in `../FIRST-BOARD-CHECKLIST.md` lists the active
PCB items. The closure order this snapshot recommends:

1. Backfill the schematic sheets with real symbols whose references match the
   PCB (`U1`, `U2`, `U3`, `U4`, `J1`, `J_BAT`, `SW1`, `BT1`, `Q1`, `Q2`,
   `R1`–`R11`, `C1`, `C2`, `D1`, `NTC1`, `LENS1`).
2. Drive the PCB from the schematic via "Update PCB from Schematic" instead of
   the standalone routed pass — that closes the parity and `extra_footprint`
   classes by construction.
3. Fix net shorts, crossing tracks, and clearance violations on a routing pass.
4. Re-run ERC + DRC and update this file's counts in the same commit.
5. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
