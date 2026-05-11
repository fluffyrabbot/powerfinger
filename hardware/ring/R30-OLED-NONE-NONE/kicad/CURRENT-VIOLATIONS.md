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

Toolchain: `kicad-cli 10.0.2` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc` violations | 0 | Project-local symbols/footprints and sheet-interface labels are now ERC-clean |
| `pcb drc` violations | 44 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 9 | Net endpoints with no track to them |
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
A fresh KiCad rerun before this follow-on route cleanup reported the previous
board as `DRC=171`, `unconnected=11`, and schematic-parity `0`. This pass
reroutes the `U3`/TP4054 `VBAT_PROTECTED` escape out of the BAT/GND/CHRG pad
row: the trace now leaves U3 BAT through an outer dogleg at
`(110.800, 99.650)` / `(110.800, 102.700)` and rejoins the `U4` VIN trunk at
`(113.350, 103.650)` instead of running vertically through U3's GND pad. That
drops the live R30 DRC count from `171` to `168` while keeping unconnected
items at `11`; the combined R30 KiCad debt is now `179`, and the full wrapper
total is `215`.
This pass moves the C2 input capacitor from `(113.950, 105.500)` to
`(113.550, 105.450)` and retargets the `VBAT_PROTECTED` leg from U4 VIN to
C2 pad 1 at `(113.550, 106.200)` while preserving the existing `0.32 mm` power
trace width. The direct `U4`/`C2` GND stitches, ground-via variants, and
pad-via variants still failed the total-debt gate in scratch, so the C2 ground
pad remains an explicit unconnected item instead of being hidden by a bad
stitch. The accepted placement relieves the regulator input-cap pinch, drops
R30 DRC from `168` to `166`, keeps unconnected items at `11`, lowers combined
R30 KiCad debt to `177`, and drops the full wrapper total to `213`.
A fresh KiCad rerun before this regulator-pocket follow-on reported the board
as `DRC=167`, `unconnected=11`, and schematic-parity `0`. This pass keeps the
accepted U3 `VBAT_PROTECTED` dogleg and shell-bound footprints intact, then
micro-retargets C2 from `(113.550, 105.450)` to `(113.400, 105.850)` and moves
the `VBAT_PROTECTED` landing to C2 pad 1 at `(113.400, 106.600)`. Direct
`SW1`/`U1`/`C1A` stitches and close C1A relocation candidates were rechecked and
still increased total DRC, so they are not retained. The live R30 DRC count is
now `166`, unconnected items remain `11`, and schematic-parity remains `0`.
This pass keeps the reset/motion local-pad decision intact and reroutes only
the B-side `C1B` ground return plus the non-contract `SENSOR_LED_KIT` test-pad
stub. The PAW3204 LED pad now escapes around the decoupler instead of straight
through it, and the C1B ground return approaches from the left side of the
sensor pocket. KiCad CLI `10.0.1` now reports R30 `DRC=165`, `unconnected=11`,
and schematic-parity `0`.
This pass moves non-shell-bound `C1A` from `(121.600, 93.900)` to
`(118.450, 94.950)`, retargets its `VREG_3V3` leg directly from U1 `3V3`, and
adds a short local GND leg from C1A pad 2 to U1 `GND1`. The shell-bound `SW1`
and `U1` footprints stay fixed. Direct `SW1`-to-`U1` / `SW1`-to-`C1A` GND ties
were rechecked after the C1A move; they lower unconnected count further only by
raising total DRC, so they are not retained. Repeated KiCad CLI `10.0.1` reruns
reported R30 `DRC=164..165`, `unconnected=10`, and schematic-parity `0`; this
snapshot records the conservative `165` count.
This pass moves non-shell-bound `TP_VBAT` from `(116.100, 92.600)` to
`(116.800, 93.600)` and reroutes `VBAT_SENSE` below the top `SW1` GND pad before
it enters U1 `GPIO0`. With that sense route out of the immediate GND pinch, the
right `SW1` ring pad now ties into the local U1/C1A ground leg without increasing
the conservative total DRC count. KiCad CLI `10.0.1` now reports R30 `DRC=164`,
`unconnected=9`, and schematic-parity `0`.
This pass reroutes `CLICK_PRIMARY_N` through a B-side via pair from the `SW1`
center pad to U1 `GPIO6` instead of dragging the front-layer click trace through
the lower `SW1` GND pad. Direct top/bottom `SW1` GND stitches were re-tested and
still increased total DRC, so they remain explicit unconnected items. KiCad CLI
`10.0.1` now reports R30 `DRC=163`, `unconnected=9`, and schematic-parity `0`.
This follow-up re-tested the remaining shared `C2`/`U4` and lower-`SW1` ground
closure as one topology problem. The lower-left `SW1` GND stitch reduced
unconnected rows only by increasing shorting rows; direct C2 pad-to-stub,
via-assisted C2, C2-to-right-ground, and small C2 retarget variants all
increased total DRC into the high 160s or 170s. No new copper is retained from
those variants, and the verified snapshot remains R30 `DRC=163`,
`unconnected=9`, and schematic-parity `0`.
This pass reroutes the B-side C1B/U2 ground leg around the PAW3204 LED pad
column, replacing the direct `(112.770, 102.900)` to `(113.700, 100.635)` run
with a left dogleg through `(112.200, 100.300)`. That preserves the shell-bound
PAW3204 aperture and the accepted C1B placement while clearing one sensor-pocket
short/mask pinch. KiCad CLI `10.0.1` now reports R30 `DRC=161`,
`unconnected=9`, and schematic-parity `0`.
This follow-up re-tested the recommended U3/R1-to-B-side-GND continuity path.
Direct U3-to-R1, U3-to-service-GND, pad-via, offset-via, and paired U3/R1
B-side return variants reduced one unconnected row only by increasing shorting,
mask-bridge, or total DRC debt. The U3 pad-via to `(111.900, 104.000)` was the
least bad candidate, but it still raised the conservative DRC count on repeat
runs and expanded the shorting bucket, so no U3/R1 copper is retained. The
verified snapshot remains R30 `DRC=161`, `unconnected=9`, and
schematic-parity `0`.
This pass then moves non-shell-bound `R1` from `(106.500, 102.800)` rotated 90
degrees to `(108.400, 102.400)` rotated horizontal, retargets the `PROG_R` leg
from U3 `PROG` to R1 pad 1, and drops R1 pad 2 into the accepted B-side GND
return through a local via. The front-layer service-GND sweep was tested and
rejected because it shorted through R1 pad 1; the retained version keeps R1 GND
on the B-side return instead. KiCad CLI `10.0.1` now reports R30 `DRC=158`,
`unconnected=9`, and schematic-parity `0`.
This pass then nudges non-shell-bound U4 from `(112.400, 102.700)` to
`(112.400, 103.000)` and retargets the local VIN/EN/VOUT segments to the new
pad coordinates while preserving the accepted C2 placement and the U3
`VBAT_PROTECTED` dogleg. Moving C2 with U4 was re-tested and rejected because it
either held no net gain or increased shorting/unconnected rows. KiCad CLI
`10.0.1` now reports R30 `DRC=156`, `unconnected=9`, and schematic-parity `0`.
This follow-up re-tested C2 GND as a route-only micro-slice with the accepted U4
nudge in place. Direct front-layer C2-to-return stubs held `unconnected=9` while
raising total DRC, and via-assisted C2 returns reduced unconnected rows only by
raising total DRC to `158..162` plus shorting or hole-clearance debt. No C2 GND
route is retained; the verified snapshot remains R30 `DRC=156`,
`unconnected=9`, and schematic-parity `0`.
This follow-up re-tested the `J_BAT` / left-service GND cluster named by the
remaining unconnected and shorting rows. Direct `J_BAT` pad-2-to-left-return
routes reduced unconnected rows only by expanding the shorting bucket; `J_BAT`
`MP1` B-side doglegs, simple `R8` relocation candidates, `VBUS_5V` doglegs
around `MP1`, and `VBAT_PROTECTED` trunk retargets either held `unconnected=9`
or raised shorting despite occasional raw DRC-count reductions. The adjacent
`Q1`/`R2A` `CHARGE_GATE` and `USB_CC1_RD` service-lane doglegs were also
rejected: the best candidate still reported `DRC=157`, `unconnected=9`, and
`shorting_items=5`, worse than the accepted snapshot's `shorting_items=4`.
No copper is retained from this slice; the verified snapshot remains R30
`DRC=156`, `unconnected=9`, and schematic-parity `0`.
This pass then moves the local D1/NTC ground return off the front-layer
`NTC_SENSE` crossing by adding one offset GND via at `(107.400, 105.750)` and
reusing the existing NTC ground via at `(109.080, 107.400)` for a short B-side
dogleg. That removes the `GND`/`NTC_SENSE` short without touching shell-bound
footprints, the accepted U3 `VBAT_PROTECTED` dogleg, or the accepted C2
placement. KiCad CLI `10.0.1` then reported R30 `DRC=154`, `unconnected=9`,
and schematic-parity `0`.
This pass also micro-retargets non-shell-bound `TP_VBAT` from
`(116.800, 93.600)` to `(116.500, 92.900)` and reroutes the local
`VBAT_SENSE` leg so the sense pad no longer shorts the upper `SW1` GND pad.
KiCad CLI `10.0.1` now reports R30 `DRC=153`, `unconnected=9`, and
schematic-parity `0`; the shorting bucket is down to three rows.
This follow-up re-tested the recommended `R7`/`R8` divider closure after the
`TP_VBAT` micro-retarget. Full `R7`/`R8` pair moves, `R8`-only moves, direct
`R7` sense doglegs, and via-assisted `R7` escapes all failed the gate: the best
raw-count candidates held `DRC=153` only by expanding the shorting bucket, while
the via candidates rose to `DRC=156` and added hole-clearance or unconnected
debt. No divider copper or placement is retained from this slice; the verified
snapshot remains R30 `DRC=153`, `unconnected=9`, and schematic-parity `0`.
This pass moves the PAW3204 optical aperture datum circle from `Edge.Cuts` to
`Cmts.User`. The circle is a package/shell alignment datum for the bottom-side
sensor and CAD tunnel, not a fabrication cutout through the populated PCB. That
removes the false internal-board-edge clearance bucket without changing the
board outline, U2 placement, or CAD aperture binding. The same pass detours the
long `VBUS_5V` feed to the `R9`/`R10` detect divider farther right of the
bring-up pads. This pass also keeps the ESP32-C3 external antenna keep-out
strict for tracks, vias, pads, and copper pour while allowing the owning module
footprint; the module footprint's antenna courtyard intentionally extends to
the board edge, so forbidding footprints in that keep-out only flags U1 itself.
This pass moves the dense left-service reference fields for `J_BAT`, `Q1`,
`R8`, and `R2A` from `F.SilkS` to `F.Fab`; those marks were clipped by nearby
pads/copper in the bring-up pocket and are not useful as printed silkscreen at
this density. KiCad CLI `10.0.2` then reported R30 `DRC=105`, `unconnected=9`,
and schematic-parity `0`.
This pass moves non-shell-bound `R2A` from `(111.650, 96.200)` to
`(111.650, 95.000)` and retargets the attached `USB_CC1_RD` pull-down plus
local GND return endpoints. That pulls the CC1 run out of the `R4`
`CHARGE_GATE` pull-up pad and reduces the shorting bucket without changing ERC,
unconnected count, or schematic/PCB parity. KiCad CLI `10.0.2` then reported
R30 `DRC=104`, `unconnected=9`, and schematic-parity `0`.
This pass retargets the `VBAT_SENSE` divider junction from `(113.000, 93.600)`
to `(113.000, 91.900)`, moving the `R7`/`R8`/`TP_VBAT` sense trunk above the
`SW1` upper dome-ring ground pad without moving shell-bound `SW1` or the CAD
dome pocket. KiCad CLI `10.0.2` then reported R30 `DRC=102`, `unconnected=9`,
and schematic-parity `0`.
This pass retargets the top-edge `VBUS_5V` trunk start from `(105.820, 91.600)`
to `(105.600, 91.600)`, pulling the diagonal VBUS feed off `J_BAT` `MP1`
without moving the shell-bound battery connector or adding a new via. KiCad CLI
`10.0.2` reported R30 `DRC=101`, `unconnected=9`,
and schematic-parity `0`.
This pass flips non-polar `R9` in place so the right-side `VBUS_5V` trunk lands
on its VBUS pad instead of crossing the `VBUS_DETECT` pad, reroutes
`VBUS_DETECT` from the flipped divider pad into the existing top node, and
moves `R9`'s reference field to `F.Fab` so the source keeps the label without
printing it in the copper-dense divider pocket. KiCad CLI `10.0.2` reported
R30 `DRC=98`, `unconnected=9`,
and schematic-parity `0`.
This pass moves non-shell-bound `R11` and `TP_CHRG` down into the charger-status
pocket at `y=105.000`, shortening the `CHRG_STAT` run and keeping the TP4054
status pull-up local while avoiding the prior U4/C2 mask-bridge crossings.
Their source reference fields move to `F.Fab` so the labels remain in the board
source without printing in the dense regulator pocket. KiCad CLI `10.0.2` now
reported R30 `DRC=95`, `unconnected=9`,
and schematic-parity `0`.
This pass keeps the accepted `VBUS_5V` and `VBAT_SENSE` copper because scratch
top-trunk, divider-junction, and `R7`/`R8` placement variants all introduced
shorting, copper-edge, or unconnected debt. It instead moves the top-edge
source-only `R7`, `TP_VBAT`, and `SW1` reference fields to `F.Fab`, clearing
their silkscreen DRC rows without changing electrical geometry. KiCad CLI
`10.0.2` reported R30 `DRC=90`, `unconnected=9`,
and schematic-parity `0`.
This pass keeps the accepted `CHARGE_EN`, `VREG_3V3`, and `VBAT_PROTECTED`
copper because scratch `CHARGE_EN` route variants and `R6` placement/orientation
variants all reintroduced shorting or unconnected debt. It instead moves the
source-only `Q2`, `U4`, and `R6` reference fields to `F.Fab`, clearing local
silkscreen DRC rows without changing electrical geometry. KiCad CLI `10.0.2`
reported R30 `DRC=86`, `unconnected=9`,
and schematic-parity `0`.
This pass keeps the accepted `USB_CC1_RD`, `VBUS_5V`, and `CHARGE_GATE` copper
because scratch CC1 and gate route variants reintroduced shorting or
unconnected drift. It instead moves the source-only `R4` and `R1` reference
fields to `F.Fab`, clearing local silkscreen DRC rows without changing
electrical geometry. KiCad CLI `10.0.2` reported R30 `DRC=83`,
`unconnected=9`,
and schematic-parity `0`.
This pass keeps the accepted `NTC_SENSE`, `VREG_3V3`, and `CHRG_STAT` copper
because scratch variants for that crossing cluster reintroduced shorting or
raised total DRC. It instead moves the remaining source-only silkscreen marks
for `J1`, `ANT`, `U2`, `U3`, `D1`, `R10`, `R3`, `NTC1`, `R2B`, `R5`,
`C1A`/`C1B`, `C2`, `TP_RST`, `TP_MOT`, `TP_VBUS`, and `TP_LEDKIT` to
`F.Fab`/`B.Fab`, clearing all remaining silkscreen text buckets without
changing electrical geometry. KiCad CLI `10.0.2` reported R30 `DRC=60`,
`unconnected=9`,
and schematic-parity `0`.
This pass keeps the board setup's `solder_mask_min_width=0.05` rule, but
changes the first-board pad mask expansion from `0.05` to `0` so pad apertures
no longer grow into nearby service/regulator tracks. Scratch variants that
relaxed the min-width rule were rejected as rule changes rather than geometry
cleanup. KiCad CLI `10.0.2` reported R30 `DRC=57`, `unconnected=9`, and
schematic-parity `0`.
This pass keeps U4 and C2 placement fixed after scratch placement variants
either held total DRC or reintroduced shorts. It instead narrows the two
`VBAT_PROTECTED` regulator/C2 spokes from `0.32` to `0.24`, reducing local
mask/clearance pressure while keeping the route above the first-board minimum
trace width. KiCad CLI `10.0.2` reported R30 `DRC=56`, `unconnected=9`, and
schematic-parity `0`.
This pass keeps the rejected R4 orientation and CC1 dogleg variants out because
they added unconnected drift or shorting risk. It instead nudges non-shell-bound
Q1 upward by `0.20` mm and retargets its three local service endpoints, reducing
left-service clearance debt without moving J1, R4, or R2A. KiCad CLI `10.0.2`
reported R30 `DRC=55`, `unconnected=9`, and schematic-parity `0`.
This follow-on rejects the top-edge `VBUS_5V`/`VBAT_SENSE` variants because
they traded clearance for shorting or copper-edge debt, then nudges Q1 upward
by another `0.10` mm and retargets its three local service endpoints. That
removes one Q1/CC1 mask bridge while keeping shorts closed and preserving
`unconnected=9` plus schematic-parity `0`. KiCad CLI `10.0.2` reported R30
`DRC=54`, `unconnected=9`, and schematic-parity `0`.
This follow-on rejects the regulator/power-pocket width, R11/TP_CHRG, U4, R6,
C2 GND, C1A, and layer-hop probes because they held total DRC or traded the
target row for shorts, unconnected, mask, clearance, or crossing debt. It
instead nudges non-shell-bound `TP_VBAT` right by `0.10` mm and retargets the
two local `VBAT_SENSE` endpoints, removing one top power-pocket mask bridge
without moving shell-bound `SW1`, `U1`, or the battery connector. KiCad CLI
`10.0.2` reported R30 `DRC=53`, `unconnected=9`, and schematic-parity `0`.
This follow-on rejects direct SCLK, top-dogleg SCLK, `VBUS_DETECT`, and
`SENSOR_MOTION_N` route variants because they introduced shorts or traded the
target row for new clearance debt. It instead raises the local
`SENSOR_SCLK` jog from `y=99.900` to `y=99.980`, clearing two MCU-column
clearance rows while keeping `unconnected=9`, no shorts, and schematic-parity
`0`. KiCad CLI `10.0.2` reported R30 `DRC=51`, `unconnected=9`, and
schematic-parity `0`.
This follow-on rejects VBUS spine, VBUS dogleg, B-side motion-hop, and
right-of-VBUS `TP_MOT` variants because they held total DRC or added dangling
vias, shorts, mask, or clearance debt. It instead moves non-shell-bound
`TP_MOT` from `(122.050, 97.900)` to `(121.250, 99.500)` and retargets the
local `SENSOR_MOTION_N` endpoint, clearing the `VBUS_DETECT` crossing while
keeping `unconnected=9`, no shorts, and schematic-parity `0`. KiCad CLI
`10.0.2` now reports R30 `DRC=48`, `unconnected=9`, and
schematic-parity `0`.
This follow-on rejects left-service Q1/R2A/R4/CC1 micro-nudges, CC1/gate
layer-hop probes, and direct `VBAT_PROTECTED`/`NTC_SENSE` trunk moves because
they held total DRC or traded crossings for shorts, dangling vias, clearance,
mask, or unconnected debt. It instead lowers the `VBAT_SENSE` divider junction
to `(113.000, 92.100)`, moves the serviceable `SW1` dome from
`(114.200, 95.000)` to `(114.200, 95.300)`, and moves the shell dome pocket with
it. KiCad CLI `10.0.2` now reports R30 `DRC=46`, `unconnected=9`, and
schematic-parity `0`.
This follow-on rejects USB data-pair doglegs, D+ branch-via moves, D+ rail
reroutes, and broad `NTC_SENSE` layer-hop/width/dogleg variants because they
held total DRC or traded the target row for shorts, clearance, mask, or
track-width debt. It instead moves only the local `NTC_SENSE` junction from
`(110.420, 106.700)` to `(109.900, 106.700)`, clearing one front solder-mask
bridge while preserving the existing NTC divider endpoints and MCU route. KiCad
CLI `10.0.2` now reports R30 `DRC=45`, `unconnected=9`, and schematic-parity
`0`.
This follow-on rejects left-service VBUS doglegs, VBUS junction moves, and broad
R2B/CC2 resistor shifts because they held total DRC or introduced shorts,
copper-edge, hole-clearance, mask, or extra clearance debt. It instead moves
only `R2B` from `(100.500, 101.500)` to `(100.400, 101.500)` and retargets the
short local `USB_CC2_RD` and GND endpoints, clearing the local R2B/VBUS
clearance row while preserving the USB-C connector placement. KiCad CLI
`10.0.2` now reports R30 `DRC=44`, `unconnected=9`, and schematic-parity `0`.

## ERC top categories

No current ERC messages. The previous `label_dangling`, `lib_symbol_issues`,
and `footprint_link_issues` buckets are closed by the project-local libraries
and sheet-interface cleanup in this snapshot.

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `tracks_crossing` | 18 | Tracks of different nets physically crossing on the same layer |
| `solder_mask_bridge` | 12 | Adjacent pads of different nets share an unbroken mask aperture |
| `clearance` | 11 | Copper-to-copper or pad-to-track clearance failures |
| `unconnected_items` | 9 | Routed pads with no track or via reaching them |
| `courtyards_overlap` | 3 | Footprint courtyard overlaps in the dense service/MCU pockets |

The explicit shorting bucket is now closed in the current report, but
crossing-class, clearance, mask-bridge, courtyard, and unconnected
violations still block fabrication release.
`lib_footprint_mismatch` is intentionally
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
without moving the shell-bound PAW3204 aperture. The accepted U3 VBAT escape
pulls the battery trace off the TP4054 GND/CHRG pad row; U3 GND still remains
one of the explicit unconnected items rather than being hidden by a short.
The accepted C2 move, plus this follow-on C2 micro-retarget, keeps the regulator
input capacitor nearer U4 VIN while avoiding the rejected C2 ground stitches and
pad vias. The latest B-side sensor-pocket cleanup reduces the shorting bucket
while preserving the local-pad reset/motion choice and keeping the shell-bound
sensor footprint fixed. The SW1 click reroute then removes the front-layer
`CLICK_PRIMARY_N`/GND pass through the lower dome pad without changing the
shell-bound click footprint. The latest C2/SW1 ground-topology probes showed
that simple local stitches are now the wrong closure mechanism: they either
raise shorting rows or move the regulator pocket into a worse clearance state.
The accepted sensor-pocket GND dogleg then removes the remaining direct
GND/`SENSOR_LED_KIT` pinch through U2's LED pad column without moving U2 or C1B.
The accepted R1 retarget opens the charger-programming resistor pocket by
moving R1 onto the B-side return and removing the rejected front-layer GND sweep
through the R1 `PROG_R` pad.
The accepted U4 nudge relieves the regulator VIN/VOUT/EN pad pinch while
leaving the accepted C2 placement and U3 dogleg in place.
The accepted D1/NTC B-side GND dogleg removes the local `GND`/`NTC_SENSE`
short while preserving the existing battery, regulator, and shell-bound
placement decisions. The follow-on `TP_VBAT` micro-retarget removes the upper
`SW1`/`VBAT_SENSE` short and lowers the remaining shorting bucket to three rows.
The aperture-datum layer fix removes the previous `copper_edge_clearance`
bucket by keeping the sensor/cad datum visible without representing it as a
routed internal cutout. The `R2A` placement cleanup removes the `USB_CC1_RD` /
`CHARGE_GATE` short, and the latest `VBAT_SENSE` divider-junction retarget
removes the `SW1` upper-ring short without moving the shell-bound click
footprint. The latest top-edge `VBUS_5V` retarget removes the remaining
`J_BAT` `MP1` short; the current report has no `shorting_items` rows, though
the board still has DRC debt and remains not fab-clean. The latest left-service
source-label cleanup is retained because it removes `R4` and `R1` printed-label
rows after CC1 and gate copper variants reintroduced shorting or unconnected
drift. The latest remaining-label cleanup is retained because the
`NTC_SENSE`/`VREG_3V3`/`CHRG_STAT` copper probes were worse, while moving
source-only marks to fabrication layers clears the last silkscreen DRC buckets
without touching copper. The latest pad-mask expansion cleanup is retained
because it reduces mask-bridge rows while preserving the project minimum mask
web rule and avoiding new shorts, unconnected rows, or parity drift. The latest
`VBAT_PROTECTED` spoke-width cleanup is retained because it lowers total DRC
without moving U4/C2 or disturbing ERC, unconnected count, or schematic parity,
even though one crossing-class row remains newly visible in that bucket mix.
The latest Q1 nudge is retained because it removes one left-service mask bridge
without accepting the R4 orientation, CC1 dogleg, or top-edge VBUS/VBAT variants
that added unconnected, shorting, or copper-edge debt.
The latest `SW1`/`VBAT_SENSE` topology cleanup is retained because it lowers
total DRC while keeping the shorting bucket closed, retaining `unconnected=9`,
and moving the shell CAD dome pocket with the PCB click footprint.

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
- `SW1` moved left to relieve the MCU pad-column collision and later down by
  `0.30` mm to relieve the top-edge sense/switch crossing cluster; the shell CAD
  dome pocket moved with both board-coordinate changes. Its local footprint no
  longer models the grounded dome ring as a full copper disk overlapping the
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

1. Move to placement-level cleanup for the left-service and power-ground
   pockets before adding more route-only stitches: the remaining blocker rows
   are now dominated by unconnected items, crossings, mask bridges, and dense
   `U4`/`C2` plus left-service clearance/text debt rather than explicit
   shorting rows. Direct all-in ground stitches were tested and
   reduced unconnected rows only by increasing total debt, and simple R1/R8
   orientation shifts, click-trace doglegs, direct pad-to-pad GND stitches,
   pad vias, the first C1A relocation candidates, direct `U3`-to-`R6`/left-GND
   stitches, `U3` pad vias, `U4`/`C2` direct/bus stitches, `C2` pad vias, direct
   `SW1`/`U1`/`C1A` stitches, the rejected close C1A relocation candidates, and
   a right-side `C1B` relocation, lower-left `SW1` GND stitch, direct C2
   pad-to-stub routes, via-assisted C2 routes, C2-to-right-ground route, and
   small C2 retarget candidates also increased total debt. Direct U3-to-R1,
   U3-to-service-GND, U3 pad-via, offset-via, and paired U3/R1 B-side return
   candidates also increased shorting or conservative total DRC. C2 moves paired
   with the accepted U4 nudge were also re-tested and rejected because they held
   no net gain or increased shorting/unconnected rows. Route-only C2 GND stubs
   after the accepted U4 nudge also increased total DRC or introduced
   shorting/hole-clearance debt. The `J_BAT` pad-2-to-left-return, `J_BAT`
   `MP1` dogleg, simple `R8` relocation, `VBUS_5V` dogleg, `VBAT_PROTECTED`
   trunk retarget, and pre-placement `Q1`/`R2A` service-lane dogleg probes also
   failed the gate by expanding shorting or raising total DRC. The accepted
   `R2A` placement retarget is retained because it reduces both total DRC and
   shorting rows without changing ERC, unconnected count, or parity. The
   accepted `VBAT_SENSE` junction retarget is retained because it removes the
   `SW1` upper-ring short while preserving the shell/CAD click coordinate. The
   accepted `VBUS_5V` trunk retarget is retained because it clears the final
   `J_BAT` `MP1` short without moving the shell-bound battery connector. The
   accepted `R9` in-place flip is retained because it removes the
   `VBUS_5V`/`VBUS_DETECT` divider crossing and one mask bridge without changing
   ERC, unconnected count, parity, or the closed shorting bucket. The
   accepted `R11` / `TP_CHRG` relocation is retained because it reduces the
   charger-status mask-bridge cluster and total DRC while preserving ERC,
   unconnected count, parity, and the closed shorting bucket. The
   accepted top-edge reference-field cleanup is retained because it removes
   source-only `R7`, `TP_VBAT`, and `SW1` silkscreen DRC without moving
   electrical copper; the rejected VBUS/VBAT copper variants expanded
   shorting, copper-edge, or unconnected debt. The
   accepted Q2/U4/R6 reference-field cleanup is retained because it removes
   source-only labels in the charge/regulator pocket after `CHARGE_EN` route
   variants and R6 placement/orientation variants reintroduced shorting or
   unconnected debt. The
   accepted R1 horizontal
   retarget with B-side-only GND return and accepted U4 nudge are retained. The
   accepted C1A local-MCU relocation, accepted TP_VBAT / right-SW1 ground
   cleanup, and accepted B-side SW1 click reroute are now retained. The accepted
   C1B/U2 B-side GND dogleg and accepted D1/NTC B-side GND dogleg are also
   retained. The post-`TP_VBAT` `R7`/`R8` divider pair move, `R8`-only move,
   direct `R7` sense dogleg, and via-assisted `R7` escape probes also failed
   because they expanded shorting, total DRC, or unconnected debt. Keep the
   accepted `VREG_3V3` trunk off the PAW3204 LED/GND column, keep the accepted
   U3 VBAT dogleg out of the TP4054 pad row, and preserve the micro-retargeted
   C2 input-cap placement unless a real GND closure lowers total debt further.
   The latest accepted `SW1`/`VBAT_SENSE` topology cleanup keeps the shorting
   bucket closed by moving the shell-bound dome coordinate and its CAD pocket
   together; the rejected Q1/R2A/R4/CC1 and via/layer-hop probes either held
   total DRC or introduced shorts, dangling vias, clearance, mask, or
   unconnected debt.
2. Close the 9 unconnected items without changing the shell-bound footprints
   unless the CAD packet moves with them.
3. Re-run ERC + DRC and update this file's counts in the same commit.
4. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
