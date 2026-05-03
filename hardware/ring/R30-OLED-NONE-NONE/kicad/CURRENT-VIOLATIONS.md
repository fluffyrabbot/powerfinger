<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Current ERC / DRC Snapshot

This packet is **not** fabrication-released. The first routed PCB pass still
carries DRC violations and unconnected items that block fab and any release
claim.

This file is a snapshot of those violations so the manifest cannot quietly
drift. It is intentionally not a generated artifact: regenerate the raw reports
locally before relying on these counts (see "How to regenerate" below). When
you close violations, update the counts here in the same commit so the snapshot
stays honest.

## Snapshot

Toolchain: `kicad-cli 10.0.1` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc` violations | 0 | Project-local symbols/footprints and sheet-interface labels are now ERC-clean |
| `pcb drc` violations | 256 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 35 | Net endpoints with no track to them |
| `pcb drc` schematic-parity issues | 0 | Schematic and PCB pad/net parity is clean |

The schematic capture now has real first-pass symbols in `power_and_charge`,
`usb_and_service`, `mcu_radio`, and `sensor_and_click`, plus a project-local
`PowerFinger` symbol library and `PowerFinger_Ring.pretty` footprint library.
The root and sub-sheet interfaces are now wired enough for ERC to pass, and the
PAW3204 reset/motion nets stay local to the sensor sheet/test pads instead of
pretending to be MCU GPIOs. The PCB footprint metadata now carries the matching
schematic BOM fields for the populated power/USB parts, the C2 bulk capacitor
footprint matches the routed 0603 part, and the populated value labels are
aligned. Routed sheet nets are now passive global labels where they intentionally
match the flat PCB net names, `Q1`/`Q2` use project-local SOT-23 pin-numbered
symbols, the USB ESD labels match the routed D+/D- nets, and the USB-C shell
stakes map to the schematic `SH` pin. `J_BAT` now uses a project-local
first-board symbol whose `MP1`/`MP2` mounting pads are explicit GND-tied
shield pins, so DRC `--schematic-parity` is clean. The first routing cleanup
shrinks the local USB-C SMT contact pads to match the 1.0 mm row pitch, replaces
the overlapping snap-dome copper disk with non-overlapping center/ring pads,
and corrects the first batch of power, USB, charger-status, and ESD endpoint
coordinates that were landing on adjacent pads.
The second routing cleanup normalizes first-board traces to the 0.20 mm board
minimum, removes the erroneous `SENSOR_SDIO` tie into the `SENSOR_MOTION_N`
via, doglegs the `SENSOR_SCLK` / `VBUS_DETECT` fanout away from the worst
sensor-via shorts, and moves the long `VBUS_5V` detector feed to the board-top
edge corridor instead of crossing the middle of the active lane.

## ERC top categories

No current ERC messages. The previous `label_dangling`, `lib_symbol_issues`,
and `footprint_link_issues` buckets are closed by the project-local libraries
and sheet-interface cleanup in this snapshot.

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `solder_mask_bridge` | 87 | Adjacent pads of different nets share an unbroken mask aperture |
| `copper_edge_clearance` | 44 | Copper inside the 0.5 mm board-edge clearance band |
| `unconnected_items` | 35 | Routed pads with no track or via reaching them |
| `text_height` | 31 | Silkscreen text below the rule minimum |
| `shorting_items` | 25 | Nets physically shorted or crossing through pads in the hand-routed pass |
| `clearance` | 20 | Copper-to-copper or pad-to-track clearance failures |
| `tracks_crossing` | 15 | Tracks of different nets physically crossing on the same layer |
| `silk_over_copper` | 13 | Silkscreen text crossing exposed copper |
| `via_diameter` | 6 | Vias below the current board setup diameter rule |
| `drill_out_of_range` | 6 | Drill sizes outside the current board setup limits |

The shorting-class and crossing-class violations are the blocking ones for
fabrication. Mask-bridge and edge-clearance violations are easier mechanical
fixes but still block a clean release. `lib_footprint_mismatch` is intentionally
ignored in the project because the first-board custom footprints are now
source-controlled under `PowerFinger_Ring.pretty`; upstream-library comparison
is not useful signal for this hand-routed packet. The previous
`footprint_symbol_field_mismatch`, `footprint_symbol_mismatch`, and broad
sheet-local plus `J_BAT` mounting-pad `net_conflict` buckets are closed in this
snapshot. The previous `track_width` bucket is also closed by normalizing
first-board routes from 0.18 mm to the board setup's 0.20 mm minimum.

## What this means for downstream packets

- The `cad/r30_oled_none_none_shell_blank.scad` board-coordinate bindings
  reference the current PCB pass. If a violation fix moves `J1`, `J_BAT`,
  `SW1`, `U2`, or the antenna keep-out, the shell CAD must move with it.
- `VBAT_SENSE` and `VBUS_DETECT` now have BDFL-accepted first-board divider
  parts and MCU-side sense/testpad counterparts in the schematic and PCB, so the
  remaining blocker is PCB DRC cleanup rather than a missing packet decision, a
  schematic-parity gap, or an ERC setup problem.
- `CHRG_STAT` now has the BDFL-accepted `R11` pull-up and a local status
  pad, but the packet still makes no firmware-consumed claim for that signal.
- `J_BAT`, including its `MP1`/`MP2` mounting pads, `R2A`, `R2B`, and
  `TP_CHRG` now have schematic counterparts; `R2` and `R12` were renamed to the
  board's `R2A`/`R2B` split.
- `J1` retains its shell-bound placement for the CAD service opening, but its
  local SMT contact geometry is now narrower along the row pitch so adjacent
  USB-C contacts are not treated as intrinsic copper shorts.
- `SW1` retains its shell-bound dome-pocket coordinate, but its local footprint
  no longer models the grounded dome ring as a full copper disk overlapping the
  center click contact.
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

1. Rip up or reroute the remaining power/sensor fanout clusters that still
   report `shorting_items` and `tracks_crossing`.
2. Close the 35 unconnected items without changing the shell-bound footprints
   unless the CAD packet moves with them.
3. Re-run ERC + DRC and update this file's counts in the same commit.
4. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
