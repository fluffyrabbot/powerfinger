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
| `pcb drc` violations | 209 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 29 | Net endpoints with no track to them |
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
This pass moves `SW1` left of the MCU pad column and syncs the shell dome
pocket to that new board coordinate, which clears one unconnected item and the
worst `U1`/dome overlap without claiming a fab-clean board.
This pass also moves the `CHARGE_GATE` route start from Q1's VBUS source pad to
Q1's gate pad, and moves the `PROG_R` route onto U3's PROG pin and R1's PROG
pad. That reduces the unconnected count while leaving the dense USB-edge
placement/routing cluster red.
This pass moves `R4` inward from the J1 CC1 row, adds an explicit `VBUS_5V`
feed from J1 `A4` to the relocated R4 pad, and moves `VBUS_CHG_SW` onto U3's
VCC pad. At that point Q1 itself still sat too close to J1 `A7`, so a wider
charger-cluster move remained.
This pass rotates `Q1` horizontally above the charger, moves `R4` into the
same charger cluster, and reroutes raw `VBUS_5V`, `VBUS_CHG_SW`, and
`CHARGE_GATE` into separate lanes. That removes Q1's source pad from J1 `A7`
and cuts the local USB-edge shorting bucket, but leaves J1 `A9` and the D1
VBUS branch disconnected until the broader USB/D1 fanout is ripped up.
This pass moves `D1` slightly upward, doglegs the J1 `A6`/`A7` USB data
escapes before they drop toward D1, and routes `USB_D+`/`USB_D-` to U1 on
separate lanes. That removes the long `USB_D-` run from D1's `VBUS_5V` pad row
and reduces the crossing/mask-bridge bucket without reconnecting the still-red
J1 `A9`/B-side VBUS branch.
This pass extends the left service-edge outline from `x=100.000` to `x=99.000`
so the fixed `J1` B-row contacts are no longer hanging outside the PCB
clearance band, then ties the J1 `A4`/`A9`/`B4`/`B9` VBUS contacts together
with a connector-side fanout. `D1`'s VBUS pad and the B-side USB data pads
remain disconnected because verified direct ties either shorted `USB_D-` or
increased total DRC debt.

## ERC top categories

No current ERC messages. The previous `label_dangling`, `lib_symbol_issues`,
and `footprint_link_issues` buckets are closed by the project-local libraries
and sheet-interface cleanup in this snapshot.

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `solder_mask_bridge` | 59 | Adjacent pads of different nets share an unbroken mask aperture |
| `copper_edge_clearance` | 33 | Copper inside the 0.5 mm board-edge clearance band |
| `text_height` | 31 | Silkscreen text below the rule minimum |
| `unconnected_items` | 29 | Routed pads with no track or via reaching them |
| `tracks_crossing` | 19 | Tracks of different nets physically crossing on the same layer |
| `shorting_items` | 17 | Nets physically shorted or crossing through pads in the hand-routed pass |
| `clearance` | 15 | Copper-to-copper or pad-to-track clearance failures |
| `silk_over_copper` | 14 | Silkscreen text crossing exposed copper |
| `drill_out_of_range` | 6 | Drill sizes outside the current board setup limits |
| `via_diameter` | 6 | Vias below the current board setup diameter rule |

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
  USB-C contacts are not treated as intrinsic copper shorts. The PCB service
  edge now extends left of the fixed B-row pads, and J1 `A4`/`A9`/`B4`/`B9`
  are tied as one VBUS contact group.
- `Q1`/`R4` `CHARGE_GATE` and `U3`/`R1` `PROG_R` now land on their intended
  pads, so the remaining service-edge DRC rows are routing/placement debt, not
  wrong endpoint names.
- `R4` moved inward from the J1 `A5`/CC1 row and now has an explicit J1 `A4`
  `VBUS_5V` feed; `VBUS_CHG_SW` now lands on U3's VCC pad.
- `Q1` and `R4` now sit as a charger cluster above U3, with Q1's source pad
  off the J1 `A7` row.
- `D1` moved slightly upward and the A-side USB data escapes now dogleg before
  dropping, then run to U1 as separated D+/D- service lanes. The B-side USB-C
  data pads and D1's VBUS pad still need a broader connector-side cleanup.
- `SW1` moved left to relieve the MCU pad-column collision, and the shell CAD
  dome pocket moved with it. Its local footprint no longer models the grounded
  dome ring as a full copper disk overlapping the center click contact.
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
2. Close the 29 unconnected items without changing the shell-bound footprints
   unless the CAD packet moves with them.
3. Re-run ERC + DRC and update this file's counts in the same commit.
4. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
