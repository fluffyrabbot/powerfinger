<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE KiCad Skeleton

This directory is the PCB/schematic starting point for the active optical ring
lane. It is intentionally honest about its state: the first routed PCB pass now
exists, and the power/USB plus MCU/radio plus sensor/click sheets now include
first-pass symbols. The current KiCad wrapper snapshot is ERC/DRC/parity clean,
but fabrication outputs and physical first-board evidence are still separate
packet gates.

## Current Source Files

- `r30_oled_none_none.kicad_sch` — root sheet for the active optical ring lane
- `r30_oled_none_none.kicad_pcb` — first rigid P0 routed board pass, with
  footprint locks, placement zones, antenna keep-outs, and charge-path routing
- `FABRICATION-OUTPUTS.md` — local Gerber/drill/POS plus active-BOM/POS review
  commands, hashes, and caveats for the DRC-clean first-board source
- `BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` — prototype-house intake checklist for
  turning the local outputs into specific pre-fab feedback without claiming
  physical ring closure
- `sheets/power_and_charge.kicad_sch`
- `sheets/mcu_radio.kicad_sch`
- `sheets/sensor_and_click.kicad_sch`
- `sheets/usb_and_service.kicad_sch`

## What Belongs Here Next

- Local symbol and footprint references only where upstream libraries are not
  enough
- Plot and fabrication presets once routing exists
- `INTERFACE-CONTRACT.md` — first-pass electrical pin contract for capture and firmware
- `CAPTURE-BINDINGS.md` — which active BOM lines need stock, vendor, or custom binding
- `SCHEMATIC-CAPTURE.md` — sheet split, net naming, and capture order
- `BOM-BLOCK-MAP.md` — turn the BOM into schematic blocks without drift
- `RECOMMENDED-FIRST-CAPTURE.md` — the recommended P0 scope boundary
- `../FIRST-BOARD-CHECKLIST.md` — the execution checklist that decides when this
  packet is allowed to move on to secondary variants

## Capture Order

1. Power tree: protected LiPo, TP4054 charge path, RT9080 LDO, fixture-fed
   VBUS service jumper, NTC divider
2. MCU/module block: `ESP32-C3-MINI-1-N4` with antenna keep-out reserved first
3. Sensor block: PAW3204-class optical sensor plus matching lens and aperture
4. Human input block: dome switch and any required debounce/passive support
5. Off-board USB service/programming path for charging plus firmware bring-up

## First Routed Board Pass

`r30_oled_none_none.kicad_pcb` locks the current P0 direction:

- rigid validation board with separate power/service, sensor/click, and
  MCU/radio zones
- ESP32-C3-MINI-1 antenna end facing the shell exterior, with copper and
  antenna keep-outs marked before any convenience routing; the external
  keep-out forbids tracks, vias, pads, and copper pour while allowing the owning
  module footprint whose antenna courtyard reaches the board edge
- bottom-side PAW3204 placement centered on the `6.2 mm` optical aperture datum
  recorded as a `Cmts.User` alignment circle, not an internal `Edge.Cuts`
  fabrication cutout through the populated PCB
- off-board same-net battery service pads for a replaceable protected-cell
  harness or fixture; no onboard JST-SH receptacle body is claimed
- Off-board same-net USB service pads for USB2 data, VBUS, CC pulldowns, GND,
  and shield continuity; charge/program service now requires a fixture or pogo
  harness rather than an onboard USB-C receptacle
- TP4054 path routed through a non-BOM fixture-fed VBUS service jumper,
  `20 kohm` RPROG, RT9080 regulation, and an NTC divider near the battery
  service pads
- MCU/radio and sensor/click sheets populated with first-pass symbol
  counterparts for the routed `U1`, `U2`, `SW1`, decoupling, sense testpads,
  and sensor bring-up pads
- routed sheet nets promoted to passive global labels where they intentionally
  match flat PCB net names, with the project-local `Q1` VBUS service jumper,
  aligned USB ESD D+/D- labels, and service shield pads mapped to `SH`
- `J_BAT` backed by a project-local first-board symbol that models `MP1`/`MP2`
  as GND-tied shield/service pads, so schematic/PCB parity is clean
- first routing cleanup of USB-C SMT contact pad length, snap-dome contact
  geometry, and wrong first-pass power/USB endpoint coordinates
- second routing cleanup of the SDIO/motion fanout, VBUS-detect dogleg, long
  VBUS detector feed, and minimum routed trace width
- charger-service cleanup that replaces the active `Q1`/`R4`/`Q2`/`R6`
  charge-gate with a non-BOM fixture-fed VBUS jumper into TP4054 `VCC`
- USB/D1 fanout cleanup that nudges `D1` upward, doglegs J1 `A6`/`A7` before
  the drop, and separates the long `USB_D+`/`USB_D-` lanes into U1
- connector-edge cleanup that extends the fixed J1 service lip leftward and
  ties J1 `A4`/`A9`/`B4`/`B9` as one VBUS contact group
- connector-cluster cleanup that moves `R2B` beside J1 `B5`, shifts the local
  VBUS trunk inward, and moves `NTC1`/`R3` out of D1's immediate `USB_D+`
  corridor
- A-side USB data cleanup that moves the J1 `A6`/`A7` to D1 escape onto `B.Cu`
  under the charger cluster and re-enters beside D1 with the `USB_D-` entry via
  shifted left for clearance
- mirrored USB-C data cleanup that routes J1 `B6`/`B7` into the same escape
  through separated `B.Cu` lanes and a short front-layer `USB_D-` overpass,
  while leaving D1 VBUS disconnected because the direct branch still regresses
  total DRC; route-only D1 GND/VBUS service-island attempts also regress, so the
  follow-up needed footprint/placement movement rather than another local
  stitch
- D1 footprint cleanup that replaces the rail-clamped USBLC6 SOT-23-6 island
  with a rail-less TPD2E2U06DCK-class SC-70/SOT-323 data-line shunt, removing
  the D1 VBUS branch from the constrained service-edge pocket
- service-edge GND/shield spine that ties the J1 ground contacts and shell
  stakes, pulls `R2B` and `R1` into the lower return, reaches `J_BAT` `MP1`
  from the upper return, and drops D1/NTC ground through a local B-side via
  without reintroducing VBUS-clamped ESD protection
- battery-side GND fanout cleanup that doglegs `R8`/`J_BAT` `MP2` away from
  the `VBAT_SENSE` trace, ties `J_BAT` pad 2 into that local return, moves
  `J_BAT` `MP1` onto a B-side return via, and joins `R2A`/left `SW1`/`Q2`
  source into one local ground chain
- via-rule cleanup that brings the six remaining sub-minimum first-board vias
  from `0.46` mm diameter / `0.20` mm drill to the board-rule minimum
  `0.50` mm diameter / `0.30` mm drill
- sensor-rail cleanup that moves the `VREG_3V3` via off the PAW3204
  `SENSOR_LED_KIT`/GND pad column, routes the sensor `VDD`/`VDDA` feed down a
  right-side `B.Cu` trunk, and keeps `C1B` tied in with a short local branch
- `TP_LEDKIT` contract cleanup that supersedes the earlier PAW3204 LED test-pad
  move by deleting the non-BOM pad/stub and leaving the PAW3204 LED-kit pin as
  an explicit local no-connect
- U3 VBAT escape cleanup that routes TP4054 BAT around the GND/CHRG pad row
  through an outer dogleg before rejoining the U4 VIN trunk
- C2 input-cap placement cleanup that moves and micro-retargets the 10uF
  capacitor closer to U4 VIN while leaving its GND pad as an honest unconnected
  blocker
- B-side sensor-pocket cleanup that reroutes the `C1B` ground return around the
  PAW3204 LED pad column without moving the shell-bound PAW3204 aperture
- C1A MCU-side decoupler cleanup that moves the non-shell-bound capacitor into
  the U1 `3V3` / `GND1` pocket while leaving the shell-bound `SW1` and U1
  placements fixed
- `TP_VBAT` / right-SW1 cleanup that reroutes `VBAT_SENSE` below the top dome
  ground pad and ties the right `SW1` ring pad into the local U1/C1A ground leg
- SW1 click cleanup that reroutes `CLICK_PRIMARY_N` through a B-side via pair
  instead of through the lower dome ground pad
- C1B/U2 B-side GND cleanup that doglegs around the PAW3204 LED pad column
  without moving the shell-bound sensor aperture
- R1 charger-programming cleanup that moves non-shell-bound `R1` into a
  horizontal pocket and returns its GND pad through the accepted B-side trunk
- U4 regulator-pocket cleanup that nudges non-shell-bound U4 while preserving
  the accepted C2 placement and U3 `VBAT_PROTECTED` dogleg
- D1/NTC ground-return cleanup that moves the local GND leg through a B-side
  dogleg so it no longer shorts the `NTC_SENSE` divider trace
- `TP_VBAT` micro-retarget that moves the sense test pad upward and removes the
  upper `SW1`/`VBAT_SENSE` short
- left-service silkscreen cleanup that moves the dense `J_BAT`, `Q1`, `R8`,
  and `R2A` reference fields to `F.Fab` instead of printing clipped labels over
  the bring-up pads/copper
- `R2A` placement cleanup that moves the CC1 pull-down to `(111.650, 95.000)`
  and retargets its `USB_CC1_RD` plus GND endpoints, clearing the
  `USB_CC1_RD` / `CHARGE_GATE` short without changing ERC, unconnected count,
  or schematic/PCB parity
- `VBAT_SENSE` divider-junction cleanup that retargets the `R7`/`R8`/`TP_VBAT`
  sense trunk through `(113.000, 91.900)`, clearing the upper `SW1` dome-ring
  short without moving shell-bound `SW1` or the CAD dome pocket
- `SW1` / `VBAT_SENSE` topology cleanup that lowers the divider junction to
  `(113.000, 92.100)` and moves the serviceable dome plus shell pocket to
  `(114.200, 95.300)`, reducing total DRC while keeping shorts closed
- top-edge `VBUS_5V` trunk cleanup that retargets the VBUS feed from
  `(105.820, 91.600)` to `(105.600, 91.600)`, clearing the `J_BAT` `MP1`
  short without moving the then-shell-bound battery connector
- in-place `R9` divider flip so the right-side `VBUS_5V` trunk lands on the
  VBUS pad and `VBUS_DETECT` exits the opposite pad into the existing top node,
  plus moving `R9`'s source reference field to `F.Fab` in the copper-dense
  divider pocket
- `SW1` dome-pocket relief that moves the shell-bound click center down to
  `(114.350, 95.780)` and moves the CAD dome pocket with it; this established
  the earlier short-free topology that later ground-continuity and
  three-contact dome-ring closures brought to the current verified
  `DRC=40`, `unconnected=0`, parity `0` snapshot
- non-shell-bound `R11` / `TP_CHRG` relocation down into the charger-status
  pocket at `y=105.000`, shortening the `CHRG_STAT` run and moving their source
  reference fields to `F.Fab` in the dense regulator pocket
- top-edge source-label cleanup that keeps the accepted `VBUS_5V` /
  `VBAT_SENSE` copper after scratch retargets introduced shorting or edge debt,
  and moves the `R7`, `TP_VBAT`, and `SW1` reference fields to `F.Fab`
- charge/regulator-pocket source-label cleanup that keeps the accepted
  `CHARGE_EN`, `VREG_3V3`, and `VBAT_PROTECTED` copper after scratch retargets
  reintroduced shorting or unconnected debt, and moves the `Q2`, `U4`, and `R6`
  reference fields to `F.Fab`
- left-service source-label cleanup that keeps the accepted `USB_CC1_RD`,
  `VBUS_5V`, and `CHARGE_GATE` copper after scratch CC1/gate reroutes
  reintroduced shorting or unconnected drift, and moves the `R4` and `R1`
  reference fields to `F.Fab`
- remaining source-label cleanup that keeps the accepted `NTC_SENSE`,
  `VREG_3V3`, and `CHRG_STAT` copper after scratch reroutes raised total DRC or
  reintroduced shorting, and moves the remaining source-only silkscreen marks
  to `F.Fab`/`B.Fab`
- pad-mask expansion cleanup that keeps `solder_mask_min_width=0.05` but
  changes first-board pad mask expansion from `0.05` to `0`, reducing
  mask-bridge rows without relaxing the minimum mask-web rule
- regulator/C2 spoke-width cleanup that keeps U4 and C2 placement fixed after
  scratch placement variants reintroduced shorts or held total DRC, and narrows
  the two `VBAT_PROTECTED` spokes from `0.32` to `0.24`
- Q1 left-service cleanup that rejects R4 orientation, CC1 dogleg, and
  top-edge `VBUS_5V`/`VBAT_SENSE` variants with unconnected, shorting, or
  copper-edge debt, then nudges Q1 upward by a retained total `0.30` mm and
  retargets its three local service endpoints
- TP_VBAT top power-pocket cleanup that rejects regulator width, R11/TP_CHRG,
  U4, R6, C2 GND, C1A, and layer-hop probes that held total DRC or added debt,
  then nudges `TP_VBAT` right by `0.10` mm and retargets its two local
  `VBAT_SENSE` endpoints
- SENSOR_SCLK MCU-column cleanup that rejects direct SCLK, top-dogleg SCLK,
  `VBUS_DETECT`, and `SENSOR_MOTION_N` route variants with shorts or clearance
  debt, then raises the local SCLK jog from `y=99.900` to `y=99.980`
- TP_MOT MCU-column cleanup that rejects VBUS spine, VBUS dogleg, B-side
  motion-hop, and right-of-VBUS `TP_MOT` variants with dangling vias, shorts,
  mask, or clearance debt, then moves `TP_MOT` to `(121.250, 99.500)` and
  retargets its local `SENSOR_MOTION_N` endpoint
- NTC sense-spine cleanup that rejects USB data-pair and broad `NTC_SENSE`
  trunk/layer-hop variants with shorts, clearance, mask, or track-width debt,
  then moves only the local `NTC_SENSE` junction from `(110.420, 106.700)` to
  `(109.900, 106.700)`
- Left-service CC2 pulldown cleanup that rejects VBUS doglegs, VBUS junction
  moves, and broad R2B shifts with shorts, copper-edge, hole-clearance, mask, or
  extra clearance debt, then moves only `R2B` from `(100.500, 101.500)` to
  `(100.400, 101.500)` and retargets its local `USB_CC2_RD` and GND endpoints
- GND-continuity cleanup that keeps the shell-bound placements and schematic
  contracts intact while closing six unconnected rows: a lower `SW1` local GND
  stitch, via-assisted U3 and C2 returns, a via-assisted `J_BAT` pad-2 return,
  a left-trunk B-side bridge, and a C2-to-U1-side GND return; direct `J1` A12
  and top-`SW1` closure variants were rejected because they reintroduced
  current shorting rows
- `J1` A12 service-ground cleanup that closes the lower USB-C ground row from
  the existing lower GND rail endpoint to the accepted service-edge via while
  leaving the USB-D- fanout intact; top-`SW1` closure remains blocked by the
  latent U4, U1, and Q2 power/sense shorts exposed by direct top-ring closure
- three-contact `SW1` dome-ring cleanup that removes the top ring contact from
  the board and source footprint, keeps the left/right/lower dome contacts, and
  ties the remaining right/lower GND contacts together without reopening shorts
  or courtyard debt
- follow-on scratch-only routing proofs that reject local patch moves after the
  unconnected bucket closed: clearance-only doglegs, placement-level corridor
  moves, local-spine redraws, left-service USB/charge fanout repartitions, and
  right-side regulator/click density moves all failed to beat the retained
  `DRC=40` board without reopening shorting, dangling, or unconnected debt
- clean-room active-R30 lane scratches inside the same packet envelope are also
  rejected for this two-layer pass: all-pad lane sketches returned non-library
  `DRC=560..564`, while safer function-lane sketches still returned
  non-library `DRC=405..416`; none kept `unconnected=0` or the closed shorting
  bucket, so the next reducer is a stackup/envelope architecture decision rather
  than another local route patch
- broad four-layer architecture scratches are rejected as route migrations: a
  KiCad 10-format stackup-only scratch stayed effectively tied with the live
  packet, while inner-layer moves for the existing `VREG_3V3` /
  `VBAT_PROTECTED` diagonals and VREG bus returned non-library `DRC=58..76`
  with reopened unconnected and shorting buckets; the retained four-layer cut is
  narrower, moving only the existing via-to-via `VREG_3V3` support span to
  `In1.Cu`
- placement-level four-layer scratches are rejected while all shell/service
  anchors remain fixed: support-part moves plus retained routes returned
  non-library `DRC=127`, targeted inner-power replacements returned
  non-library `DRC=137`, and full via-per-pad reroutes returned non-library
  `DRC=549..573`; the next reducer must let the service edge, `J1`, `J_BAT`,
  and `SW1` move with shell CAD, or change component classes
- envelope scratches that moved the service edge, `J1`, `J_BAT`, `SW1`, and
  support parts together are also rejected with the current first-board
  BOM/netlist: retained-route variants returned non-library `DRC=149..157`, and
  generated bus reroutes returned non-library `DRC=307..317`; the next reducer
  is a component-class cut, starting with the USB-C service connector or
  off-board charge/program service
- component-class USB service cut that replaces the through-hole-staked USB-C
  receptacle with source-controlled off-board same-net service pads, adds a GND
  continuity via at the former shell-stake return, and moves `USB_CC1_RD` onto
  a B-side jog; KiCad CLI `10.0.2` then reported ERC `0`, DRC `38`,
  unconnected `0`, and schematic parity `0` with no current shorting or
  courtyard bucket
- regulator/input-cap component-class scratches are rejected: same-center `U4`
  pad shrink, `C2` 0402 substitution, tiny same-pin DFN regulator sketches,
  `VREG_3V3` / `VBAT_PROTECTED` route migration, and connected C2 placement
  sweeps did not produce a retainable `DRC<38` board without reopening
  unconnected or shorting debt
- battery-service component-class cut that replaces the onboard JST-SH body
  with source-controlled off-board same-net service pads while preserving
  `VBAT+`, `VBAT-`, `MP1`, and `MP2` parity; KiCad CLI `10.0.2` then reported
  ERC `0`, DRC `36`, unconnected `0`, and schematic parity `0` with no current
  shorting or courtyard bucket
- U4 regulator land-pattern cut that keeps the RT9080 part and pinout, moves
  non-shell-bound `U4` by `(+0.05, -0.05) mm`, and switches only that footprint
  to `PowerFinger_Ring:RT9080_33GJ5_SOT23_5_ServiceClearance`; KiCad CLI
  `10.0.2` then reported ERC `0`, DRC `34`, unconnected `0`, and schematic parity
  `0` with no current shorting or courtyard bucket
- `R8` divider clearance cleanup that moves only non-shell-bound `R8` to
  `(112.100, 93.400)` and retargets its local `VBAT_SENSE`/GND endpoints;
  KiCad CLI `10.0.2` then reported ERC `0`, DRC `33`, unconnected `0`, and
  schematic parity `0` with no current shorting or courtyard bucket
- U4 land-pattern refinement that keeps the RT9080 part, pinout, placement, and
  source-controlled footprint name fixed while shrinking the local pads to
  `0.40 x 0.78 mm`; KiCad CLI `10.0.2` now reports ERC `0`, DRC `32`,
  unconnected `0`, and schematic parity `0` with no current shorting or
  courtyard bucket
- CHRG_STAT fixture-status cut that removes the onboard `R11` pull-up and its
  local `VREG_3V3` spoke while keeping `TP_CHRG` as a fixture-observed
  TP4054 status pad; KiCad CLI `10.0.2` now reports ERC `0`, DRC `31`,
  unconnected `0`, and schematic parity `0` with no current shorting or
  courtyard bucket
- fixture-fed charge-service cut that removes `R4`, `Q2`, `R6`,
  `CHARGE_GATE`, `CHARGE_EN`, and `VBUS_CHG_SW` from this P0, reties U3 `VCC`
  directly to `VBUS_5V` through the non-BOM `Q1` service jumper, marks ESP32-C3
  `GPIO10` no-connect, and moves the local U4 GND via to `(112.000, 102.700)`;
  KiCad CLI `10.0.2` now reports ERC `0`, DRC `27`, unconnected `0`, and
  schematic parity `0` with no current shorting or courtyard bucket
- left-service return-topology cleanup that moves the `R2B` / lower service
  shield GND leg onto B.Cu through a via pair from `(100.880, 101.500)` to
  `(100.650, 104.100)`, clearing the local `VBUS_5V` crossing without moving
  `J1`, `R2A`, `R2B`, `SW1`, the service jumper, or shell CAD anchors; KiCad
  CLI `10.0.2` now reports ERC `0`, DRC `26`, unconnected `0`, and schematic
  parity `0` with no current shorting, dangling, or courtyard bucket
- TP_CHRG endpoint cleanup that moves only the fixture-observed status pad and
  its local `CHRG_STAT` segment from `(116.000, 104.000)` to
  `(109.800, 101.600)`, shortening the charger-status spur beside `U3` without
  adding an onboard pull-up, firmware consumer, shell change, service-anchor
  move, or BOM change; KiCad CLI `10.0.2` now reports ERC `0`, DRC `23`,
  unconnected `0`, and schematic parity `0` with no current shorting, dangling,
  or courtyard bucket
- battery-service `VBAT_PROTECTED` endpoint cleanup that replaces the direct
  `R7`-to-`J_BAT` diagonal with a front-layer dogleg through
  `(107.500, 92.400)` and `(107.500, 95.300)`, keeping `J_BAT`, `R7`/`R8`,
  the service jumper, shell CAD anchors, BOM, and schematic netlist fixed while
  clearing two live crossing rows; KiCad CLI `10.0.2` now reports ERC `0`,
  DRC `21`, unconnected `0`, and schematic parity `0` with no current shorting,
  dangling, hole-clearance, or courtyard bucket
- C1A support-island cleanup that moves the MCU `VREG_3V3` decoupler from
  `(118.450, 94.950)` to `(118.450, 94.700)` and retargets only its local
  `VREG_3V3` / GND endpoints, keeping U1, SW1, J_BAT, the battery-service
  dogleg, shell CAD anchors, BOM, and schematic netlist fixed while clearing
  one live clearance row; KiCad CLI `10.0.2` now reports ERC `0`, DRC `20`,
  unconnected `0`, and schematic parity `0` with no current shorting, dangling,
  hole-clearance, or courtyard bucket
- `VBAT_SENSE` support-corridor cleanup that moves the long ADC leg onto a
  via-assisted B-side escape between `(116.600, 93.250)` and
  `(118.550, 96.850)`, and bends only the local C1A GND return through
  `(119.200, 95.300)` before U1 `GND1`; KiCad CLI `10.0.2` now reports ERC
  `0`, DRC `17`, unconnected `0`, and schematic parity `0` with no current
  shorting, dangling, hole-clearance, or courtyard bucket
- `VREG_3V3` support-corridor cleanup that removes the long front-layer
  U4-to-U1/C1A rail, reuses the existing sensor-rail via at
  `(114.900, 102.900)`, adds one U1-side via at `(118.000, 94.550)`, and lands
  at U1 `3V3` with a short F.Cu hop; KiCad CLI `10.0.2` now reports ERC `0`,
  DRC `15`, unconnected `0`, and schematic parity `0` with no current shorting,
  dangling, hole-clearance, or courtyard bucket
- sensor-LED endpoint-column cleanup that moves only the B-side
  `SENSOR_LED_KIT` return column from `x=115.600` to `x=116.000`, clearing the
  local `VREG_3V3` clearance row while keeping U1, U2, SW1's shell-bound click
  coordinate, C1A, J_BAT, the off-board service anchors, PAW3204 aperture,
  antenna keep-out, active BOM, and schematic netlist fixed; KiCad CLI
  `10.0.2` now reports ERC `0`, DRC `14`, unconnected `0`, and schematic
  parity `0` with no current shorting, dangling, hole-clearance, or courtyard
  bucket
- live `DRC=14` corridor re-probe that keeps the retained board geometry fixed
  after USB data-pair doglegs, U4 GND doglegs, `SENSOR_LED_KIT` column moves,
  and `NTC_SENSE` via/dogleg escapes either tied the current count or reopened
  shorting, extra clearance, solder-mask, dangling, or hole-clearance debt
- `TP_LEDKIT` contract cleanup that removes the non-BOM PAW3204 LED-kit test pad
  and its B-side spur, models U2 pin 5 as an explicit schematic no-connect, and
  keeps U2, the aperture, shell-bound placements, service anchors, charge-service
  jumper, antenna keep-out, active BOM, and schematic/PCB parity fixed; KiCad CLI
  `10.0.2` now reports ERC `0`, DRC `13`, unconnected `0`, and schematic parity
  `0` with no current shorting, dangling, hole-clearance, or courtyard bucket
- four-layer support-span cleanup that adds `In1.Cu` / `In2.Cu` and moves only
  the long via-to-via `VREG_3V3` span from `B.Cu` to `In1.Cu`, keeping the
  fixture-fed charge-service jumper, service anchors, shell-bound placements,
  active BOM, antenna keep-out, and schematic parity fixed; KiCad CLI `10.0.2`
  now reports ERC `0`, DRC `11`, unconnected `0`, and schematic parity `0` with
  no current shorting, dangling, hole-clearance, or courtyard bucket
- USB service-span cleanup that moves only the local `USB_D+` span from
  `(109.200, 105.650)` to `(120.800, 105.650)` onto `In2.Cu` with two
  board-rule vias, keeping `J1`, U1, U4, C2, NTC/R3, shell-bound placements,
  active BOM, antenna keep-out, and schematic parity fixed; KiCad CLI `10.0.2`
  now reports ERC `0`, DRC `10`, unconnected `0`, and schematic parity `0` with
  no current shorting, dangling, hole-clearance, or courtyard bucket
- left-service USB-pair cleanup that reuses the existing `USB_D-` endpoint vias
  and moves only their via-to-via span from `B.Cu` to `In1.Cu`, keeping fixed
  `J1`, D1, U1, service anchors, shell-bound placements, active BOM, antenna
  keep-out, and schematic parity fixed; KiCad CLI `10.0.2` now reports ERC `0`,
  DRC `9`, unconnected `0`, and schematic parity `0` with no current shorting,
  dangling, hole-clearance, track-width, or courtyard bucket
- regulator-corridor layer-partition cleanup that keeps the local
  `NTC_SENSE` divider junction on `F.Cu`, moves its long U1 ADC span to
  `In2.Cu` through two board-rule vias, and moves the existing local `USB_D+`
  service span from `In2.Cu` to `In1.Cu`; KiCad CLI `10.0.2` now reports ERC
  `0`, DRC `6`, unconnected `0`, and schematic parity `0` with no current
  shorting, dangling, hole-clearance, track-width, or courtyard bucket
- U4 VIN/EN topology cleanup that replaces the direct front-layer
  `VBAT_PROTECTED` VIN-to-EN tie through the U4 GND mask pinch with a widened
  right-side dogleg around the GND pad and away from the local `VREG_3V3`
  diagonal; KiCad CLI `10.0.2` now reports ERC `0`, DRC `4`, unconnected `0`,
  and schematic parity `0` with no current shorting, dangling, hole-clearance,
  track-width, solder-mask-bridge, or courtyard bucket
- U4 pad-mask/support-footprint follow-up scratch that is not retained: U4 pad 2
  solder-mask-margin and land-shrink probes tied `DRC=10`, the custom pad-2
  mask/paste aperture raised the live board to `DRC=12`, and the extra-via
  left USB_D- inner-layer hop raised the live board to `DRC=11`; no new copper,
  footprint, or rule change beat the then-retained `DRC=10` state without new
  debt
- remaining left regulator-feed scratch that is not retained: lower
  `VBAT_PROTECTED` doglegs, right-side or lower U4 GND returns, U4 VOUT doglegs,
  upper-junction VREG inner-layer feeds, and small U4 left/right micro-nudges
  all raised the live board to `DRC=5..19` or reopened unconnected, shorting,
  clearance, mask, or inner-span crossing debt; the retained snapshot stays
  ERC `0`, DRC `4`, unconnected `0`, and schematic parity `0`
- remaining two-net layer-partition scratch that is not retained: non-pad U4
  VOUT-to-`In1.Cu` hops, U3 BAT-to-U4 VIN `In1.Cu` hops, and extended
  `VBAT_PROTECTED` service-span inner doglegs either tied the live board at
  DRC `4` without improvement or raised it to DRC `5..7` with shorting,
  clearance, hole-clearance, or retained-inner-span debt
- C2/NTC/R3 support-topology follow-up scratch that is not retained: lower
  `NTC_SENSE` doglegs, offset inner-layer `NTC_SENSE` hops, rotated C2 support
  placement, right-shifted NTC/R3 junctions, and a narrowed C2 VBAT spur either
  raised live DRC to `11..15`, reopened current shorting, or violated the
  `0.20 mm` minimum track-width rule; no geometry beats the retained `DRC=10`
  state
- U4/C2/NTC/R3 micro-placement follow-up scratch that is not retained: isolated
  U4 x/y nudges raised live DRC to `13..14`, a coupled U4+C2/VBAT/GND upward
  retarget raised live DRC to `13`, and a full U4+C2+NTC/R3 upward support
  shift raised live DRC to `15`; no non-shell-bound micro-placement beats the
  retained `DRC=10` state without new mask/clearance pressure
- U4/C2 core scratch that rejects C2-only endpoint moves, C2 pad-size probes,
  coupled U4 VIN/EN/VOUT/GND moves, U4 NC/GND pad refinements, and U4 GND-via
  relocations because the clean variants hold `DRC=20` and the lower-looking
  variants reopen shorting, hole-clearance, unconnected, or extra mask debt
- coupled C2/`NTC_SENSE`/`USB_D+` scratch that rejects the full exposed trade:
  C2-lowered, C2-land-shrink, NTC-junction, local `USB_D+` lane-retarget, and
  via-assisted `NTC_SENSE` variants either tied `DRC=20`, rose to clean
  `DRC=21+`, or reopened shorting; no `DRC<20` topology is retained
- regulator-island VREG/VIN topology cleanup that reroutes U4 VOUT through the
  existing R3/VREG upper lane before dropping to the retained VREG via, then
  shortens the paired U4 VIN/EN `VBAT_PROTECTED` dogleg from `x=115.500` to
  `x=114.250`; the right-shifted VREG-via continuation and lower U4 GND-via
  move were rejected with shorting, hole-clearance, sliver, click, NTC, or
  extra clearance debt, while the retained topology verifies at ERC `0`, DRC
  `3`, unconnected `0`, and schematic parity `0` with no current shorting,
  dangling, hole-clearance, track-width, solder-mask-bridge, or courtyard
  bucket
- C2-node `VBAT_PROTECTED` service-feed cleanup that moves the former long
  front-layer diagonal onto `In1.Cu` through board-rule vias at the C2 VBAT node
  and retained service-feed endpoint; the final retained front bend through
  `(111.200, 103.050)` plus the `In1.Cu` dogleg through
  `(111.000, 105.000)` / `(109.800, 101.000)` clears the two remaining local
  GND-via clearance rows without moving `J1`, `J_BAT`, `SW1`, `U2`, the antenna
  keep-out, or the shell-bound/service anchors; the straight inner span and
  front-layer C2-node diagonal were rejected with shorting, mask, or extra
  clearance debt, while the retained topology verifies at ERC `0`, DRC `0`,
  unconnected `0`, and schematic parity `0` with no current shorting, dangling,
  hole-clearance, track-width, solder-mask-bridge, clearance, crossing, or
  courtyard bucket

This pass cuts the onboard active charge-enable switch from the first P0:
fixture VBUS now feeds TP4054 `VCC` through a non-BOM `Q1` service jumper, and
ESP32-C3 `GPIO10` is intentionally no-connect. The PCB also carries the
recommended sense support parts: `R7`/`R8` = `100k`/`100k` for `VBAT_SENSE` and
`R9`/`R10` = `220k`/`100k` for `VBUS_DETECT`. `VBAT_SENSE` and `VBUS_DETECT`
are routed to the ESP32-C3 resources named in
[INTERFACE-CONTRACT.md](INTERFACE-CONTRACT.md). `CHRG_STAT` remains locally
testable at `TP_CHRG`, but it has no onboard pull-up and no MCU GPIO or
firmware consumer is claimed yet. A fixture must provide a pull-up if charger
status is measured. PAW3204 `SENSOR_NRESET` and `SENSOR_MOTION_N` are likewise
local bring-up pads only for the first optical board, not firmware reset or wake
signals. The `43 x 18 mm` rigid pass now drives the shell CAD fit pass, but it
still does not prove the package is comfortable, printable, RF-clean, or
serviceable with a populated board.

## Mechanical Binding Into CAD

`../cad/r30_oled_none_none_shell_blank.scad` now maps the routed board into the
shell using these KiCad facts:

- board outline: `99..142 x 91..109 mm`, treated as a `43 x 18 mm` PCB around
  center `(120.5, 100)`
- off-board service-pad access: `J1` at `(102.650, 100.000)` on the left
  service edge
- off-board battery service path: `J_BAT` at `(108.600, 94.250)`
- dome pocket: `SW1` at `(114.350, 95.780)`
- optical datum: `U2`/board aperture at `(116.500, 100.000)`
- antenna keep-out: external no-footprint zone from `x=134.150` to board edge

Keep these coordinates in sync if the board moves; the shell should fail loudly
rather than drifting back to anonymous module pockets.

## Hard Constraints

- Treat [PLACEMENT-CONSTRAINTS.md](PLACEMENT-CONSTRAINTS.md) as a blocker list,
  not optional polish.
- Do not substitute the LDO, charge resistor, or battery safety path without
  updating the variant manifest and BOM.
- Do not let convenience routing eat the antenna keep-out.
- Treat the current schematic as first-pass capture with clean local electrical
  and layout gates, not proof that the assembled hardware is validated. The
  current local KiCad CLI `10.0.2` snapshot is ERC=0, DRC=0, unconnected=0, and
  schematic-parity=0.
- Treat the first PCB as a DRC-clean routed board pass, not a completed
  hardware release. Keep ERC/DRC/parity clean through fabrication export and
  physical first-board evidence.
- Close `../FIRST-BOARD-CHECKLIST.md` and `../STACKUP-VERIFY.md` before treating
  this lane as hardware-ready.
