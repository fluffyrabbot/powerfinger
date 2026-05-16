<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Current ERC / DRC Snapshot

This packet is **not** fabrication-released. The first routed PCB pass still
carries DRC violations that block fab and any release claim, even though the
current unconnected-item bucket is closed.

This file is a snapshot of those violations so the manifest cannot quietly
drift. It is intentionally not a generated artifact: regenerate the raw reports
locally before relying on these counts (see "How to regenerate" below). When
you close violations, update the counts here in the same commit so the snapshot
stays honest.

## Snapshot

Toolchain: `kicad-cli 10.0.2` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc` messages | 0 | No current ERC errors or warnings |
| `pcb drc` violations | 21 | Mixed errors and warnings in the current KiCad CLI report; not fab-clean |
| `pcb drc` unconnected items | 0 | All current KiCad unconnected rows are closed |
| `pcb drc` schematic-parity issues | 0 | Schematic and PCB pad/net parity is clean |

The schematic capture now has real first-pass symbols in `power_and_charge`,
`usb_and_service`, `mcu_radio`, and `sensor_and_click`, plus a project-local
`PowerFinger` symbol library and `PowerFinger_Ring.pretty` footprint library.
The root and sub-sheet interfaces are now wired enough for ERC to pass. The
PAW3204 reset/motion nets stay local to the sensor sheet/test pads instead of
pretending to be MCU GPIOs. The PCB footprint metadata now carries the matching
schematic BOM fields for the populated power/USB parts, the C2 bulk capacitor
footprint matches the routed 0603 part, and the populated value labels are
aligned. Routed sheet nets are now passive global labels where they intentionally
match the flat PCB net names, `Q1` is now a non-BOM fixture-fed VBUS service
jumper, the USB ESD labels match the routed D+/D- nets, and the off-board
service pad footprint carries same-net `SH` pads for the schematic shield pin.
`J_BAT` now uses a project-local
first-board symbol whose `MP1`/`MP2` mounting pads are explicit GND-tied
shield pins. The current DRC `--schematic-parity` output is clean, including
the project-local `TP_CHRG` fixture-status pad footprint. The
first routing cleanup
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
without moving the then-shell-bound battery connector or adding a new via. KiCad CLI
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
without moving shell-bound `SW1`, `U1`, or the then-battery-connector body. KiCad CLI
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
clearance row while preserving the then-USB-C connector placement. KiCad CLI
`10.0.2` now reports R30 `DRC=44`, `unconnected=9`, and schematic-parity `0`.
This follow-on rejects the `VBUS_5V`/`CHARGE_GATE`/`J_BAT` pocket doglegs and
Q1/R2A micro-nudges because they held total DRC or introduced shorting,
clearance, mask, or courtyard debt. It instead moves shell-bound `SW1` from
`(114.200, 95.300)` to `(114.350, 95.780)`, retargets the click via
and right ring-pad GND leg, and moves the shell CAD dome pocket with it.
Repeated KiCad CLI `10.0.2` runs later reported R30 `DRC=43`, so this was the
fresh baseline before the courtyard follow-up. The first courtyard pass narrows
only the custom first-board courtyard envelopes for `J1`, `J_BAT`, `U1`, and
`SW1` in the PCB plus their `PowerFinger_Ring.pretty` sources. It does not move
placements, pads, nets, copper, or the shell-bound CAD coordinates. That clears
the `SW1`/`J_BAT` and `SW1`/`U1` courtyard rows while leaving the tighter
`J1`/`J_BAT` service-edge courtyard overlap visible for real connector/battery
clearance review. A follow-up scratch pass tested `J_BAT` upward moves to
`y=93.650`, `93.250`, `93.000`, and `92.850` with local pad/via endpoint
retargets; every variant raised total DRC into `47..49` and reintroduced
`shorting_items`, so no placement move is retained. This pass then narrows the
source-controlled `J1` courtyard envelope to the actual USB-C pad row in the
service-edge direction, which clears the remaining `J1`/`J_BAT` courtyard row
without moving shell-bound pads, copper, placements, or CAD bindings. A
same-slice C2 GND via/stub candidate lowered unconnected items to `8` but
raised DRC to `47`, and an R8/VBAT_SENSE divider nudge raised DRC to `43`; both
were rejected. This follow-up then tested a via-assisted `NTC_SENSE` B-side
reroute and a local GND via move around the U4/C2 return; both raised total
debt or reintroduced shorting / unconnected drift, so neither is retained. The
accepted regulator/input-cap redraw rotates non-polar C2 so `VBAT_PROTECTED`
lands on C2 pad 1 from the near side rather than running through the C2 ground
pad column. This follow-up tested a coherent rotated-`U4` topology with local
`VBAT_PROTECTED`, `VREG_3V3`, and GND spoke retargets: the first repaired
variant reached `DRC=38` with `unconnected=9`, but still introduced current
`shorting_items`, while the right-shifted correction rose to `DRC=44`; no
rotated-regulator topology is retained. KiCad CLI `10.0.2` wrapper reruns at
that point conservatively reported R30 `DRC=40`, `unconnected=9`,
schematic-parity `0`, no courtyard rows, and no current `shorting_items`
bucket.

The follow-on corridor scratch redraw also rejects three broader power/status
variants. Pulling `R11`/`TP_CHRG` back beside `U3` shortened `CHRG_STAT` but
raised total DRC to `55`; a B-side `CHARGE_EN` hop raised DRC to `46`; and a
right-shifted `Q2` placement with retargeted `CHARGE_GATE`, `CHARGE_EN`, and
GND legs raised DRC to `43` and `unconnected` to `10`. None of those corridor
topologies are retained. The retained board still needs a more fundamental
left power-switch corridor replacement rather than additional one-net escapes.
A true placement-group scratch move then shifted `Q1`, `Q2`, and `U3` left as
a charger/switch group while shifting `U4` slightly right and retargeting the
local `VBUS_CHG_SW`, `VBAT_PROTECTED`, `VREG_3V3`, `PROG_R`, `CHARGE_GATE`,
`CHARGE_EN`, `CHRG_STAT`, and Q2-GND legs. That worsened the board to
`DRC=72`, `unconnected=13`, so the group translation is also rejected and no
new PCB topology is retained.
The next scratch topology split `U4`/`C2` out as a regulator island above the
current charge/switch corridor while leaving `Q1`/`Q2`/`U3` in place; retargeted
`VBAT_PROTECTED` and `VREG_3V3` spokes raised the result to `DRC=49` with
`unconnected=10`, so that regulator-island placement is rejected too. The
retained follow-up instead normalizes the long `VREG_3V3` rail from U4 to U1
and the short C2 `VBAT_PROTECTED` spoke to the board-rule minimum `0.20 mm`.
That keeps placements, pads, nets, and CAD bindings fixed while dropping the
live R30 report to `DRC=39`, `unconnected=9`, schematic-parity `0`, no
courtyard rows, and no current `shorting_items` bucket.
This pass then closes six GND-continuity rows without moving shell-bound
footprints or changing any schematic net contracts: the lower `SW1` ring pad
gets a short local GND stitch, U3 GND returns through a pad-local B-side via,
C2 GND ties into both the accepted lower return and the U1-side GND rail
through B-side vias, `J_BAT` pad 2 gets a via-assisted return to the existing
left-service GND via, and the left GND trunk gets a via-assisted B-side bridge
to the sensor-side return. Direct `J1` A12 closure and top-`SW1` closure
variants were re-tested and rejected because they reintroduced current
`shorting_items`. KiCad CLI `10.0.2` now reports R30 `DRC=39`,
`unconnected=3`, schematic-parity `0`, no courtyard rows, and no current
`shorting_items` bucket.
This follow-up then closes the `J1` A12 service-ground row by adding a B-side
return from the existing lower ground rail endpoint at `(104.650,104.100)` to
the accepted service-edge GND via at `(107.400,105.750)`. Direct A12 pad vias
and doglegs were rejected because they collided with the nearby `USB_D-`
service fanout. Top-`SW1` closure variants were re-tested after the A12 fix;
each reduced raw unconnected count to `1` but reintroduced current shorts in
the U4 `VBAT_PROTECTED`/GND column, the U1 `VBAT_SENSE`/GND pad column, and/or
the Q2 `CHARGE_EN`/`VBAT_PROTECTED` corridor, so no top-`SW1` copper is
retained. KiCad CLI `10.0.2` then reported R30 `DRC=39`, `unconnected=2`,
schematic-parity `0`, no courtyard rows, and no current `shorting_items`
bucket.
This follow-up closes the last two unconnected rows by changing `SW1` from a
four-contact dome-ring footprint to a three-contact dome-ring footprint in both
the board and `PowerFinger_Ring.pretty`, then tying the remaining right and
lower ring contacts together on F.Cu. Direct four-contact top-ring closure
remained rejected because it exposed the same regulator, sense, and
charge-enable shorts. KiCad CLI `10.0.2` now reports R30 `DRC=40`,
`unconnected=0`, schematic-parity `0`, no courtyard rows, and no current
`shorting_items` bucket.
The follow-on clearance-only pass tested the newly visible `SW1`/`VREG_3V3`
pressure first, then the adjacent power/sense diagonals. B-side and doglegged
`SW1` returns either stayed at `DRC>=42` or introduced shorts/hole-clearance
debt. Moving the long `VREG_3V3` diagonal to the existing B-side rail lowered
raw non-library count in scratch but introduced three current shorting rows, so
it is rejected. Normalizing the remaining `0.24 mm` `VBAT_PROTECTED` and
`VREG_3V3` spokes merely traded one clearance row for one crossing row. Small
`VBAT_SENSE`, `NTC_SENSE`, `USB_CC1_RD`, `CHARGE_GATE`, `CHARGE_EN`, and
`CHRG_STAT` doglegs likewise stayed at `DRC>=40` or reopened shorting. No PCB
topology from this pass is retained; the live board remains `DRC=40`,
`unconnected=0`, schematic-parity `0`, with no current shorting or courtyard
buckets.
The placement-level follow-up then tested the recommended non-shell-bound
`Q2`/`U4`/`C2`/`R11`/`TP_CHRG` corridor moves with connected endpoint retargets
rather than detached component moves. The best Q2-only clean move reached the
same `DRC=40` baseline; the only below-40 Q2 scratch variant reached raw
`DRC=39` by introducing a `VBAT_PROTECTED`/`CHARGE_EN` short, and local
`CHARGE_EN` retargets did not remove that short without giving the DRC back.
U4 moves opened unconnected or shorting rows, R11/TP_CHRG moves raised total
DRC or reopened shorts, and C2 moves raised total DRC, shorting, or
hole-clearance debt. No placement topology from this pass is retained.
The controlled corridor rip-up follow-up then removed the old local
`VBAT_PROTECTED`/`VREG_3V3`/`CHARGE_EN`/`CHRG_STAT` diagonal spokes in scratch
and replaced them with explicit local corridor routes. Frozen-placement rip-up
variants came back at `DRC=48..50` with reopened shorting, dangling-track, and
unconnected debt. Group-island variants that moved the non-shell-bound
regulator, C2, Q2, and status pull-up/test point together came back at
`DRC=57..72` with shorting, hole-clearance, and unconnected debt. No controlled
rip-up topology from this pass is retained.
The blank-slate local-spine follow-up then widened that proof to remove and
redraw all local `VBUS_CHG_SW`, `VBAT_PROTECTED`, `VREG_3V3`, `PROG_R`,
`CHARGE_GATE`, `CHARGE_EN`, and `CHRG_STAT` copper inside the active corridor.
The fixed-placement spine sketches returned raw `DRC=89..98` in scratch before
library-noise filtering, with real shorting, dangling-track, and unconnected
rows; the best non-library category mix still failed well above the retained
`DRC=40` board. No blank-slate local-spine topology is retained.
The left-service USB/charge fanout repartition follow-up is also rejected.
Moving `J1` left inside the service edge while retargeting its exact pad
endpoints returned non-library DRC debt of `75..81` with shorting, dangling,
copper-edge, courtyard, and unconnected rows. Coupled service-pocket spreads
that moved `J1` with `Q1`/`R4`/`R2A`, `D1`/`R2B`, or both returned non-library
DRC debt of `111..156`. Route-only service fanout rewrites for the USB data
pair, `R2B` ground, `USB_CC1_RD`, and `VBUS_5V`/Q1 either reopened shorts or
unconnected rows; the best USB_D+ route-only sweep candidates merely matched
the retained board at non-library `DRC=40`. No left-service fanout topology is
retained.
The right-side regulator/click density follow-up is rejected too. Direct `SW1`
moves with retargeted click/GND endpoints returned non-library DRC debt of
`49..65` and reopened shorting plus unconnected rows; coupling `SW1` with
`C1A` stayed at non-library `DRC=62..66`. Moving the `U4`/`C2`/`R11`/`TP_CHRG`
island together returned non-library `DRC=78..90`, and combining that island
with `SW1` or the VREG sensor branch returned non-library `DRC=89..103`, again
with shorting, dangling, and unconnected rows. No right-side density topology
is retained.
The clean-room active-R30 scratch follow-up is rejected as well. A no-placement
change all-pad lane sketch returned non-library DRC debt of `564` with
`unconnected_items=10`; the corresponding non-shell-bound support-placement
sketch returned non-library DRC debt of `560` with `unconnected_items=9`.
Both variants reopened large shorting, hole-clearance, copper-edge, and
solder-mask buckets, so they were not eligible for retention. A safer
function-lane reroute with no edge lanes reduced the scratch debt but still
returned non-library `DRC=405` on existing placements and `DRC=416` after
support-placement moves, both with `unconnected_items=10` and reopened
shorting, dangling, hole-clearance, and mask debt. No clean-room lane topology
from this pass is retained; the live packet stays at `DRC=40`,
`unconnected=0`, schematic-parity `0`, with no current shorting or courtyard
buckets.
The first four-layer architecture scratch is also not retained. A KiCad
10-format stackup-only scratch board remained effectively tied with the live
two-layer packet at non-library `DRC=41` in the upgraded scratch-report context
and did not reduce the active debt. Moving the worst long `VREG_3V3` and
`VBAT_PROTECTED` diagonals onto `In2.Cu`/`In1.Cu` returned non-library
`DRC=58` with `unconnected_items=8`, `shorting_items=13`, and reopened
hole-clearance debt. Extending the VREG inner bus returned non-library
`DRC=74` with `unconnected_items=11`; the VREG bus rewrite returned
non-library `DRC=76` with `unconnected_items=7`. These scratch results reject
route-migrating the current two-layer hand-route onto inner layers as the next
closure mechanism; no four-layer scratch topology is promoted to the live PCB.
The placement-level four-layer scratch then allowed non-shell-bound charger,
regulator, divider, service, and test-pad parts to move while keeping `J1`,
`J_BAT`, `U1`, `U2`, `SW1`, the PAW3204 aperture, antenna keep-out, outline,
and first-board BOM/netlist fixed. Keeping the old route geometry after the
placement move returned non-library `DRC=127` with `unconnected_items=32`;
adding targeted inner-layer replacements for the worst `VREG_3V3` and
`VBAT_PROTECTED` diagonals worsened that to non-library `DRC=137` with
`unconnected_items=36`. A full via-per-pad four-layer reroute across three
support-placement variants returned non-library `DRC=549..573` with
`unconnected_items=74..75` and large shorting/hole-clearance/mask debt. No
placement-level four-layer topology from this pass is retained. This rejects
the current fixed-anchor `43 x 18 mm` packet as a route/placement cleanup
problem rather than proving the board fabrication-clean.
The first envelope scratch then moved the service edge, `J1`, `J_BAT`, `SW1`,
and non-shell-bound support parts together while keeping `U1`, `U2`, the
PAW3204 aperture, antenna keep-out, first-board BOM, and netlist fixed.
Retaining the old route geometry after those coordinated service-anchor moves
returned non-library `DRC=149..157` with `unconnected_items=49..50`,
`shorting_items=17..21`, and a reopened courtyard row. Replacing all routes
with generated service/envelope bus reroutes returned non-library
`DRC=307..317` with `unconnected_items=18`, `shorting_items=88..92`, and the
same courtyard debt. No envelope scratch topology is retained; moving the
existing service anchors and shell openings together is not enough to make this
first-board BOM/netlist fabrication-clean.

## ERC top categories

No current ERC messages. The previous `label_dangling`, `lib_symbol_issues`,
and `footprint_link_issues` buckets are closed by the project-local libraries
and sheet-interface cleanup in this snapshot.

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `tracks_crossing` | 10 | Tracks of different nets physically crossing on the same layer |
| `solder_mask_bridge` | 5 | Adjacent pads of different nets share an unbroken mask aperture |
| `clearance` | 6 | Copper-to-copper or pad-to-track clearance failures in the live wrapper report |
| `unconnected_items` | 0 | Closed by the three-contact `SW1` ring topology |

The explicit unconnected and shorting buckets are now closed in the current
report, but crossing-class, clearance, and mask-bridge violations still block
fabrication release.
`lib_footprint_mismatch` is intentionally
ignored in the project when it only compares intentional source-controlled
first-board custom footprints under `PowerFinger_Ring.pretty`; upstream-library comparison
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
pulls the battery trace off the TP4054 GND/CHRG pad row; later ground-return
cleanup closes the remaining U3-side unconnected debt without hiding it behind
a short.
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
total DRC while keeping the shorting bucket closed and moving the shell CAD
dome pocket with the PCB click footprint. The latest GND-continuity cleanup is
retained because it closes six isolated return rows while preserving the closed
shorting and courtyard buckets; the remaining direct `J1` A12 and top-`SW1`
closure probes still turn into real shorts.
The latest service-edge ground cleanup is retained because it closes the `J1`
A12 row through the existing lower GND rail without colliding with the USB-D-
fanout; the remaining top-`SW1` closure still exposes regulator, sense, and
charge-enable shorts instead of a simple missing stitch.
The latest `SW1` dome-ring topology cleanup is retained because it closes the
last two unconnected rows without reopening shorting or courtyard buckets:
the top ring contact is removed from the custom footprint and the remaining
left/right/lower ring contacts provide the serviceable dome return. The latest
component-class service cut is retained because it replaces the
through-hole-staked USB-C receptacle with a source-controlled off-board
same-net service-pad footprint, adds the one GND continuity via formerly
provided by a shell stake, and moves `USB_CC1_RD` onto a B-side jog. The live
KiCad proof is now `ERC=0`, `DRC=38`, `unconnected=0`, and schematic parity
`0`, with no current shorting or courtyard buckets. This does not make the
board fab-clean, and it changes the mechanical truth: the packet now needs an
external service/programming/charge fixture rather than an onboard USB-C plug.
The regulator/input-cap component-class follow-up is not retained. Scratch
`U4` pad-shrink and `C2` 0402 substitutions either tied the live non-library
`DRC=38` state or worsened mask debt; a tiny same-pin DFN-style regulator
reopened unconnected and shorting rows. Route-migration sketches for the
`VREG_3V3` and `VBAT_PROTECTED` diagonals lowered crossing counts only by
reopening shorting and unconnected buckets. A connected C2 placement sweep
around the `U4` island found several no-new-bucket ties at non-library
`DRC=38`, but no `DRC<38` candidate.
The latest battery-service component-class cut is retained because it replaces
the onboard JST-SH right-angle body with source-controlled off-board same-net
battery service pads, preserving `VBAT+`, `VBAT-`, `MP1`, and `MP2` schematic
parity while shrinking the mounting-pad keep-out that was driving local
clearance and mask debt. That pass proved `ERC=0`, `DRC=36`,
`unconnected=0`, schematic parity `0`, with no current shorting or courtyard
bucket. This changes the mechanical truth: the first board now needs a
replaceable battery service harness or fixture instead of an onboard JST-SH
plug body.
The retained regulator-pocket component-class cut then moves `U4` by
`(+0.05, -0.05) mm`, changes only the RT9080 land pattern to the
source-controlled `RT9080_33GJ5_SOT23_5_ServiceClearance` footprint, and leaves
the regulator part, pinout, BOM, `C2`, shell-bound `J1`/`J_BAT`/`SW1`, and shell
CAD coordinates unchanged. That clears two U4-adjacent mask/clearance rows
without reopening shorting or courtyard debt. The live proof was `ERC=0`,
`DRC=34`, `unconnected=0`, schematic parity `0`.
The retained `R8` divider nudge then moves only non-shell-bound `R8` from
`(111.800, 93.200)` to `(112.100, 93.400)` and retargets the two local
`VBAT_SENSE` and GND endpoints tied to that resistor. This closes the
`R8`/`VBAT_SENSE` clearance row and two nearby mask rows without touching
shell-bound `J1`, `J_BAT`, `SW1`, the USB service interface, the regulator
footprint, the antenna keep-out, or the schematic/BOM contract. The live proof
was `ERC=0`, `DRC=33`, `unconnected=0`, schematic parity `0`.
The retained U4 land-pattern refinement then keeps the RT9080 part, pinout,
placement, and source-controlled footprint name fixed while shrinking the local
SOT-23-5 pads from `0.45 x 0.80 mm` to `0.40 x 0.78 mm`. This closes one
remaining U4-adjacent mask row without moving `C2`, `J1`, `J_BAT`, `SW1`, the
antenna keep-out, or the schematic/BOM contract. The live proof is now
`ERC=0`, `DRC=32`, `unconnected=0`, schematic parity `0`, with no current
shorting or courtyard bucket.
The retained CHRG_STAT fixture-status cut then removes the onboard `R11`
pull-up and its local `VREG_3V3` spoke while keeping `TP_CHRG` as a
fixture-observed TP4054 status pad. This closes one net DRC row without
claiming firmware charge-status behavior; an external fixture must provide a
pull-up if the open-drain status signal is measured. The live proof is now
`ERC=0`, `DRC=31`, `unconnected=0`, schematic parity `0`, with no current
shorting or courtyard bucket.
The retained `TP_CHRG` micro-retarget then moves only the fixture status pad
and its local `CHRG_STAT` segment from `(116.100, 105.000)` to
`(116.000, 104.000)`, preserving the fixture-observed status contract while
removing one net DRC row. KiCad CLI `10.0.2` now reports `ERC=0`, `DRC=30`,
`unconnected=0`, schematic parity `0`, with no current shorting or courtyard
bucket.
The retained charge-service topology cut then removes the onboard active
`Q1`/`R4`/`Q2`/`R6` charge-gate from this P0. `Q1` is now a non-BOM copper
service jumper that ties fixture-fed `VBUS_5V` directly to TP4054 `VCC`;
`GPIO10` is marked no-connect, and `CHARGE_GATE`, `CHARGE_EN`, and
`VBUS_CHG_SW` no longer exist in the routed/schematic contract. The local U4
GND via moves to `(112.000, 102.700)` to preserve GND continuity without
reopening the shorting bucket. KiCad CLI `10.0.2` now reports `ERC=0`,
`DRC=27`, `unconnected=0`, schematic parity `0`, with no current shorting,
dangling, or courtyard bucket.
The retained left-service return topology then moves the `R2B` / lower service
shield GND leg from the front layer to a B-side via pair between
`(100.880, 101.500)` and `(100.650, 104.100)`. This removes the remaining
local `VBUS_5V` crossing at the lower off-board service-pad row without moving
`J1`, `R2A`, `R2B`, `SW1`, the fixture-fed charge-service jumper, or any shell
CAD anchor. KiCad CLI `10.0.2` now reports `ERC=0`, `DRC=26`, `unconnected=0`,
schematic parity `0`, with no current shorting, dangling, or courtyard bucket.
The retained `TP_CHRG` endpoint retarget then moves only the fixture status pad
and its local `CHRG_STAT` segment from `(116.000, 104.000)` to
`(109.800, 101.600)`. This shortens the U3-local charger-status spur while
preserving the fixture-observed status contract, no onboard pull-up, no MCU
consumer, no service-anchor move, no shell change, and no BOM change. KiCad CLI
`10.0.2` now reports `ERC=0`, `DRC=23`, `unconnected=0`, schematic parity `0`,
with no current shorting, dangling, or courtyard bucket.
The retained battery-service VBAT dogleg then replaces the direct
`R7`-to-`J_BAT` `VBAT_PROTECTED` diagonal with a front-layer dogleg through
`(107.500, 92.400)` and `(107.500, 95.300)`. This keeps `J_BAT`, `R7`/`R8`,
the fixture-fed charge-service jumper, shell CAD anchors, BOM, and schematic
netlist fixed while removing two live crossing rows. KiCad CLI `10.0.2` now
reports `ERC=0`, `DRC=21`, `unconnected=0`, schematic parity `0`, with no
current shorting, dangling, hole-clearance, or courtyard bucket.
The follow-on local reducer scratch is not retained. `C1A` translation,
rotation, pad-shape, front-layer dogleg, and B-side GND-return variants either
held `DRC=23` or lowered the raw count only by reopening
`VBAT_PROTECTED`/GND shorting, unconnected, or extra clearance/mask debt. U4
NC/GND pad-shape and GND-via variants likewise failed to beat `DRC=23` without
shorting, hole-clearance, or unconnected debt. Moving the `VREG_3V3`,
`VBAT_PROTECTED`, and `NTC_SENSE` junctions traded crossing rows for new bad
rows or stayed at the baseline. `SW1` dome-ring pad-shape variants and a USB
service-pair sanity check also failed the retention gate.
The follow-on clearance-only scratch pass is not retained because every tested
route-only clearance fix either held total DRC at `40` or reintroduced shorts.
This confirms that one-net doglegs are not the next useful reduction.
The placement-level `Q2`/`U4`/`C2`/`R11`/`TP_CHRG` scratch pass is likewise not
retained: bounded component moves with endpoint retargets failed to beat
`DRC=40` without reopening shorting, unconnected, or hole-clearance debt.
The controlled corridor rip-up scratch pass is not retained either: both the
frozen-placement route replacement and the grouped regulator-island sketches
were materially worse than the retained board.
The blank-slate local-spine scratch pass is also rejected because removing and
redrawing all local power/status/control copper reopened shorting, dangling,
and unconnected rows while staying far above the retained `DRC=40` board.
The clean-room active-R30 lane scratch is rejected for the same packet-facing
reason at a wider scope: rerouting from fixed shell anchors and first-board
BOM/netlist did not beat the retained hand-route, and both existing-placement
and support-placement variants reopened unconnected and shorting buckets.

## What this means for downstream packets

- The `cad/r30_oled_none_none_shell_blank.scad` board-coordinate bindings
  reference the current PCB pass. If a violation fix moves `J1`, `J_BAT`,
  `SW1`, `U2`, or the antenna keep-out, the shell CAD must move with it.
- `VBAT_SENSE` and `VBUS_DETECT` now have BDFL-accepted first-board divider
  parts and MCU-side sense/testpad counterparts in the schematic and PCB, so the
  remaining blocker is PCB DRC cleanup rather than a missing packet decision, a
  schematic-parity gap, or an ERC setup problem.
- `CHRG_STAT` now keeps a local `TP_CHRG` fixture status pad, but the onboard
  `R11` pull-up is cut from this packet and no firmware-consumed claim is made
  for that signal.
- `J_BAT`, including its `MP1`/`MP2` service shield pads, `R2A`, `R2B`, and
  `TP_CHRG` now have schematic counterparts; `R2` and `R12` were renamed to the
  board's `R2A`/`R2B` split. `R2B` now sits beside the connector `B5` CC2 pad
  instead of dragging `USB_CC2_RD` diagonally through the VBUS and data lanes.
  `R8`/`J_BAT` `MP2` and `J_BAT` pad 2 now share a doglegged local GND return,
  while `R2A`, the left `SW1` ring pad, and `Q2` source share a short local GND
  chain instead of remaining three separate islands.
- `J1` no longer claims an onboard USB-C receptacle. It keeps the same logical
  USB2/VBUS/CC/GND service nets on a source-controlled off-board service-pad
  footprint at the left service edge, with same-net shield pads preserved for
  schematic parity. J1 `A4`/`A9`/`B4`/`B9` remain tied as one VBUS contact
  group, and the latest CC1 B-side jog lowers live DRC without reopening
  unconnected, shorting, or courtyard debt.
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
  ground contacts/service shield pads, `R2B`, `R1`, `J_BAT` `MP1`, and D1/NTC
  return into the board's GND return without reintroducing a VBUS-clamped
  protection island.
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

1. Stop trying route-only or corridor-only power/status rewrites as the next
   reducer. The current blocker rows are crossings, mask bridges, and dense
   `U4`/`C2` plus left-service clearance debt rather than explicit unconnected
   or shorting rows, and the latest full local-spine rewrite reopened both.
   Direct all-in ground stitches were tested and
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
   `J_BAT` `MP1` short without moving the then-shell-bound battery connector.
   The latest retained battery-service component-class cut replaces that
   connector body with off-board service pads while preserving schematic parity.
   The
   accepted `R9` in-place flip is retained because it removes the
   `VBUS_5V`/`VBUS_DETECT` divider crossing and one mask bridge without changing
   ERC, unconnected count, parity, or the closed shorting bucket. The
   earlier accepted `R11` / `TP_CHRG` relocation is superseded by the current
   fixture-status cut, which removes the onboard pull-up while preserving ERC,
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
   unconnected debt. The latest accepted courtyard-envelope cleanup removes the
   remaining service-edge courtyard row without changing copper or placement;
   the same-slice C2 ground-return and R8 divider placement probes are rejected
   because they reduce one local symptom only by raising total DRC. The latest
   accepted C2 rotation is retained because it lowers the regulator/input-cap
   crossing and mask debt without changing ERC, unconnected count, parity, or
   the closed shorting bucket; the NTC B-side reroute and local GND-via move
   remain rejected because they reintroduced shorting or unconnected drift.
   The follow-on VREG/status probes also stay rejected: a top-edge `VREG_3V3`
   dogleg, a B-side `VREG_3V3` split from the existing sensor-rail via, an
   inward `R11` / `TP_CHRG` placement, and a B-side `CHRG_STAT` hop all raised
   total DRC instead of beating the previous stable `DRC=40` baseline. The
   accepted rail-width normalization is retained because it lowers the live
   report to `DRC=39` without changing placement, schematic parity, the
   unconnected count, or the closed shorting bucket. The accepted
   GND-continuity cleanup is retained because it closes six unconnected rows
   without changing shell-bound placements, reopening courtyard rows, or
   reintroducing current shorting. The accepted A12 service-ground cleanup is
   retained because it closes one more unconnected row without touching `J1` or
   the USB data fanout. The accepted three-contact `SW1` footprint cleanup is
   retained because it closes the final unconnected rows without reopening
   shorting or courtyard debt.
2. The component-class and local endpoint cuts have crossed the gate for the USB
   service interface, battery service interface, U4 regulator land pattern and
   pad refinement, `R8` divider clearance, charge-service topology, the
   left-service `R2B` / lower service-shield return topology, and the
   `TP_CHRG` fixture-status endpoint retarget, and the `R7`/`J_BAT`
   `VBAT_PROTECTED` service-feed dogleg: the
   live packet now uses source-controlled service/clearance footprints and
   proves `ERC=0`, `DRC=21`, `unconnected=0`, schematic parity `0`, and no
   shorting, dangling, or courtyard bucket. Do not silently
   reintroduce an onboard USB-C receptacle, JST-SH battery body, stock U4 land
   pattern, the larger U4 pads, the old `R8` endpoint geometry, the onboard
   `R11` pull-up, the old long `TP_CHRG` status spur, the active
   `Q1`/`R4`/`Q2`/`R6` charge gate, or the former front-layer lower
   service-shield return, or the old direct `R7`-to-`J_BAT`
   `VBAT_PROTECTED` diagonal unless the
   schematic, PCB, shell CAD where applicable, BOM/manifest, stackup notes,
   packet counts, and first-board checklist move together and beat this
   retained state.
3. The next honest reducer should stop treating the remaining rows as isolated
   pad or one-junction symptoms. Sketch a coupled source-controlled topology
   cut for the `VBAT_PROTECTED` / `VREG_3V3` / `NTC_SENSE` crossing stack:
   either move the non-shell-bound regulator input/output decoupling and sense
   divider endpoints together, or replace the long diagonal rails with a
   schematic-matched local distribution spine. Keep the fixture-fed
   charge-service jumper, off-board service anchors, shell-bound placements,
   PAW3204 aperture, antenna keep-out, and active BOM contract fixed. Retain
   only a live `DRC<21` result with `unconnected=0`, no ERC errors, no new
   schematic-parity errors, and no shorting, dangling, hole-clearance, or
   courtyard bucket.
4. Re-run ERC + DRC and update this file's counts in the same commit.
5. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
