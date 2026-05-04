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
| `pcb drc` violations | 170 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 11 | Net endpoints with no track to them |
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
This pass moves `R2B` next to J1 `B5`, shifts the local VBUS trunk inward to
avoid the former CC2 diagonal, and moves/flips the `NTC1`/`R3` pair downward
out of D1's immediate `USB_D+` corridor. Direct D1 VBUS and B-side data
fanouts were re-tested after this cleanup and still increased total DRC debt,
so those endpoints remained honest blockers at that snapshot.
This pass moves the J1 `A6`/`A7` to D1 data escape onto `B.Cu` immediately
after the A-side contacts, re-enters beside D1, and shifts the `USB_D-` entry
via left so the A-side data pair no longer cuts through the U3/R1 charger
service lane. A further `USB_D+` entry nudge toward U3 and a horizontal `R1`
orientation were tested and rejected because both increased total DRC debt.
This pass routes the mirrored J1 `B6`/`B7` USB data pads into the same data
escape with a separated `B.Cu` fanout and a short `USB_D-` front-layer overpass,
closing both B-side data unconnected items without moving the shell-bound USB-C
connector. Direct D1 VBUS was re-tested after the data fanout and still
increased total DRC debt, so D1 VBUS remained an honest blocker at that
snapshot.
This pass re-tested the D1 protection pocket as a route-only service island:
direct D1 ground to A12, via-assisted D1 ground to the existing GND via,
via-assisted D1 ground to the NTC-side ground pad, via-assisted D1 VBUS from the
inward trunk, and the combined GND/VBUS island. Each variant reduced one or two
unconnected rows but increased total DRC debt into the `249` to `251` range, so
no route-only D1 stitch is retained. That left a D1 blocker needing
footprint/placement-level cleanup, not another missing-wire patch.
This pass replaces the rail-clamped USBLC6 SOT-23-6 protection island with a
rail-less TPD2E2U06DCK-class SC-70/SOT-323 dual data-line shunt. `USB_D+` and
`USB_D-` now reach D1 through short shunt stubs, D1 has no `VBUS_5V` pad or
branch, and D1 ground ties into the nearby NTC-side return. That removes the
two D1 VBUS unconnected rows while keeping schematic ERC and schematic/PCB
parity clean. The remaining D1-adjacent unconnected row was part of the broader
service-edge ground/shield return, not a reason to reintroduce a VBUS-clamped
ESD footprint.
This pass adds a deliberate service-edge GND/shield spine: top and bottom
connector-side rails tie J1 `A1`/`B12`/upper `SH` and J1 `A12`/`B1`/lower
`SH`, the upper rail reaches `J_BAT` `MP1`, the lower rail pulls in `R2B` and
`R1`, and D1/NTC ground now drops through a local via to the B-side ground
return. That closes the J1 `A12` to D1 row without adding a VBUS clamp back to
D1. It raises the DRC violation count by four route/crossing rows, but lowers
unconnected items by ten and improves the combined R30 KiCad debt from `211`
to `205`.
This pass reroutes the battery-side ground fanout instead of adding a broad
switch-ring stitch: `R8`/`J_BAT` `MP2` now leave on a short dogleg away from
the `VBAT_SENSE` trace, `J_BAT` pad 2 joins that local return, `J_BAT` `MP1`
uses a B-side return with a local via, and `R2A`/the left `SW1` ring pad/`Q2`
source share a short local GND chain. The DRC violation count remains `190`,
but unconnected items drop from `15` to `12`, improving the combined R30 KiCad
debt from `205` to `202`.
This pass normalizes the six remaining undersized first-board vias from
`0.46` mm diameter / `0.20` mm drill to the board-rule minimum `0.50` mm
diameter / `0.30` mm drill. That clears the `via_diameter` and
`drill_out_of_range` buckets without changing pad/net parity; one extra local
clearance row remains, but total DRC drops from `190` to `179`, improving the
combined R30 KiCad debt from `202` to `191`.
A fresh KiCad rerun before this follow-on route cleanup reported the post-via
baseline as `DRC=180`, `unconnected=12`, and schematic-parity `0`. This pass
moves the `VREG_3V3` via from `(113.730, 102.900)` to `(114.900, 102.900)`,
routes the PAW3204 `VDD`/`VDDA` feed down a right-side `B.Cu` trunk, and keeps
a short `B.Cu` branch back to the sensor decoupler `C1B`. That pulls the sensor
rail off the PAW3204 `SENSOR_LED_KIT`/GND pad column, clears the single
`hole_clearance` row that had appeared in the fresh report, and drops the live
R30 DRC count from `180` to `176` while keeping unconnected items at `12`.
The combined R30 KiCad debt is now `188`; the full wrapper total is `224` once
the unchanged USB-HUB warning count is included.
This pass moves the `R6` charge-enable pulldown from `(110.900, 100.300)` to
`(111.470, 98.950)`, reties the `CHARGE_EN` route through the new pad
coordinate, and lands `R6`'s GND pad on the existing local ground run instead
of leaving it as a separate island. The same pass moves the non-shell-bound
`TP_LEDKIT` pad from `(113.700, 103.800)` to `(113.700, 105.400)` so the
PAW3204 LED test stub no longer sits in the `C1B` / GND pinch. That drops R30
DRC from `176` to `170`, unconnected items from `12` to `11`, combined R30
KiCad debt from `188` to `181`, and the full wrapper total from `224` to
`217`.

## ERC top categories

No current ERC messages. The previous `label_dangling`, `lib_symbol_issues`,
and `footprint_link_issues` buckets are closed by the project-local libraries
and sheet-interface cleanup in this snapshot.

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `copper_edge_clearance` | 38 | Copper inside the 0.5 mm board-edge clearance band |
| `solder_mask_bridge` | 36 | Adjacent pads of different nets share an unbroken mask aperture |
| `text_height` | 31 | Silkscreen text below the rule minimum |
| `tracks_crossing` | 17 | Tracks of different nets physically crossing on the same layer |
| `clearance` | 15 | Copper-to-copper or pad-to-track clearance failures |
| `silk_over_copper` | 13 | Silkscreen text crossing exposed copper |
| `shorting_items` | 13 | Nets physically shorted or crossing through pads in the hand-routed pass |
| `unconnected_items` | 11 | Routed pads with no track or via reaching them |

The shorting-class and crossing-class violations are the blocking ones for
fabrication. Mask-bridge and edge-clearance violations are easier mechanical
fixes but still block a clean release. `lib_footprint_mismatch` is intentionally
ignored in the project because the first-board custom footprints are now
source-controlled under `PowerFinger_Ring.pretty`; upstream-library comparison
is not useful signal for this hand-routed packet. The previous
`footprint_symbol_field_mismatch`, `footprint_symbol_mismatch`, and broad
sheet-local plus `J_BAT` mounting-pad `net_conflict` buckets are closed in this
snapshot. The previous `track_width` bucket is also closed by normalizing
first-board routes from 0.18 mm to the board setup's 0.20 mm minimum, and the
previous `via_diameter` / `drill_out_of_range` buckets are closed by
normalizing the remaining sub-minimum vias to `0.50` mm diameter / `0.30` mm
drill. The sensor-rail reroute also closes the transient `hole_clearance` row
without changing schematic/PCB parity. The `R6` relocation closes the pulldown
ground island, and the `TP_LEDKIT` move reduces the sensor LED/test-pad pinch
without moving the shell-bound PAW3204 aperture.

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
  board's `R2A`/`R2B` split. `R2B` now sits beside the connector `B5` CC2 pad
  instead of dragging `USB_CC2_RD` diagonally through the VBUS and data lanes.
  `R8`/`J_BAT` `MP2` and `J_BAT` pad 2 now share a doglegged local GND return,
  while `R2A`, the left `SW1` ring pad, and `Q2` source share a short local GND
  chain instead of remaining three separate islands.
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
- `D1` moved slightly upward and the A-side USB data escapes now leave J1 on
  short front-layer stubs, run under the charger cluster on `B.Cu`, re-enter
  beside D1, and then run to U1 as separated D+/D- service lanes. The mirrored
  B-side USB-C data pads now join that escape through a separated B-side fanout
  and a short `USB_D-` front-layer overpass. Route-only D1 GND/VBUS service
  islands were tested and rejected because they worsened total DRC. D1 is now a
  rail-less TPD2E2U06DCK-class SC-70/SOT-323 data-line shunt, which removes the
  D1 VBUS blocker. The follow-on service-edge ground spine now ties the J1
  ground contacts/shield stakes, `R2B`, `R1`, `J_BAT` `MP1`, and D1/NTC return
  into the board's GND return without changing the USB-C connector placement or
  reintroducing a VBUS-clamped protection island.
- `NTC1`/`R3` moved down inside the battery-side cluster so the thermistor
  divider is no longer sitting directly on D1's `USB_D+` pad/track corridor.
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

1. Rip up or reroute the remaining charger/regulator and MCU-side ground
   fanout clusters around `U3`, `U4`/`C2`, and the right-side
   `SW1`/`U1`/`C1A` GND chain; direct all-in ground stitches were tested and
   reduced unconnected rows only by increasing total debt, and simple R1/R8
   orientation shifts, click-trace doglegs, direct pad-to-pad GND stitches,
   pad vias, the first C1A relocation candidates, direct `U3`-to-`R6`/left-GND
   stitches, `U3` pad vias, `U4`/`C2` direct/bus stitches, `C2` pad vias, and a
   right-side `C1B` relocation also increased total debt. Keep the accepted
   `VREG_3V3` trunk off the PAW3204 LED/GND column.
2. Close the 11 unconnected items without changing the shell-bound footprints
   unless the CAD packet moves with them.
3. Re-run ERC + DRC and update this file's counts in the same commit.
4. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
