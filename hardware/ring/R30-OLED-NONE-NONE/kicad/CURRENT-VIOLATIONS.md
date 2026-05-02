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
| `sch erc` violations | 27 | 11 errors, 16 warnings |
| `pcb drc` violations | 381 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 41 | Net endpoints with no track to them |
| `pcb drc` schematic-parity issues | 119 | Mixed symbol/footprint/netlist parity issues, not fab-clean |

The schematic capture now has real first-pass symbols in `power_and_charge`,
`usb_and_service`, `mcu_radio`, and `sensor_and_click`. This closes most of the
previous "PCB footprint has no symbol counterpart" gap for the routed MCU,
sensor, click, decoupling, bring-up pad, battery connector, charge-status pad,
and USB CC references, but it does not make the board parity-clean. ERC will
stay red until root/sheet labels are physically wired; DRC `--schematic-parity`
will stay red until the board is driven from the full schematic instead of this
hand-routed validation pass.

## ERC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `label_dangling` | 11 | Root-sheet labels and inherited first-two-sheet labels not physically wired into the current hierarchy |
| `lib_symbol_issues` | 11 | Embedded `PowerFinger:` symbols exist in the sheets but are not backed by a project-local symbol library entry yet |
| `footprint_link_issues` | 5 | Custom `PowerFinger_Ring` footprints are embedded on the board but not backed by a local footprint library yet |

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `solder_mask_bridge` | 122 | Adjacent pads of different nets share an unbroken mask aperture |
| `net_conflict` | 92 | Schematic-parity import now sees populated sheet-local nets that do not match PCB global nets yet |
| `copper_edge_clearance` | 45 | Copper inside the 0.5 mm board-edge clearance band |
| `shorting_items` | 38 | Nets physically shorted or crossing through pads in the hand-routed pass |
| `unconnected_items` | 41 | Routed pads with no track or via reaching them |
| `track_width` | 35 | Tracks routed at 0.18 mm where the board setup minimum is 0.20 mm |
| `text_height` | 31 | Silkscreen text below the rule minimum |
| `clearance` | 27 | Copper-to-copper or pad-to-track clearance failures |
| `lib_footprint_mismatch` | 26 | Schematic footprint filters do not match assigned PCB footprints |
| `footprint_symbol_field_mismatch` | 22 | Schematic and PCB fields differ |
| `tracks_crossing` | 15 | Tracks of different nets physically crossing on the same layer |
| `silk_over_copper` | 14 | Silkscreen text crossing exposed copper |
| `footprint_symbol_mismatch` | 5 | Footprint and symbol values or pad mappings do not agree |
| `via_diameter` | 6 | Vias below the current board setup diameter rule |
| `lib_footprint_issues` | 6 | Footprint library provenance/configuration warnings |
| `drill_out_of_range` | 6 | Drill sizes outside the current board setup limits |

The shorting-class and crossing-class violations are the blocking ones for
fabrication. Mask-bridge and edge-clearance violations are easier mechanical
fixes but still block a clean release.

## What this means for downstream packets

- The `cad/r30_oled_none_none_shell_blank.scad` board-coordinate bindings
  reference the current PCB pass. If a violation fix moves `J1`, `J_BAT`,
  `SW1`, `U2`, or the antenna keep-out, the shell CAD must move with it.
- `VBAT_SENSE` and `VBUS_DETECT` now have BDFL-accepted first-board divider
  parts and MCU-side sense/testpad counterparts in the schematic and PCB, so the
  remaining blocker is DRC/ERC cleanup rather than a missing packet decision.
- `CHRG_STAT` now has the BDFL-accepted `R11` pull-up and a local status
  pad, but the packet still makes no firmware-consumed claim for that signal.
- `J_BAT`, `R2A`, `R2B`, and `TP_CHRG` now have schematic counterparts; `R2`
  and `R12` were renamed to the board's `R2A`/`R2B` split.
- The stale schematic-only `C1`/`C3` placeholders are retained as DNP/off-board
  notes for the next schematic-driven PCB update rather than missing board
  footprints.

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

1. Add durable project-local `PowerFinger` symbol and `PowerFinger_Ring`
   footprint libraries, or replace each embedded custom item with a stock KiCad
   symbol/footprint where that is honest.
2. Resolve the root hierarchy/global net story, then drive the PCB from the
   schematic via "Update PCB from Schematic" instead of the standalone routed
   pass.
3. Fix net shorts, crossing tracks, and clearance violations on a routing pass.
4. Re-run ERC + DRC and update this file's counts in the same commit.
5. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
