<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Manifest

## Status

- Variant ID: `R30-OLED-NONE-NONE`
- Form factor: ring
- Lane: active validation lane
- Publication state: BOM-backed hardware packet plus first routed rigid P0 PCB pass
- BOM source: [hardware/bom/R30-OLED-NONE-NONE.csv](../../bom/R30-OLED-NONE-NONE.csv)
- BOM target: `~$9` at prototype scale
- Source skeletons:
  - `kicad/` — KiCad-oriented schematic/layout inputs, hierarchy scaffolds, placement notes, and first routed PCB pass
  - `cad/` — OpenSCAD lower-shell plus service-lid packet for the first-board ring pass
  - `FIRST-BOARD-CHECKLIST.md` — concrete first-board outputs required before secondary variants
  - `STACKUP-VERIFY.md` — measured package-closure template for height, focal distance, battery, and RF evidence

This packet documents the minimum repairability and assembly expectations for
the first optical ring hardware drop. It does not claim the package geometry,
focal distance, RF behavior, or click ergonomics are already proven in hardware.

## Intended Use

- Primary use case: one of the two identical rings in the default cursor +
  scroll pair
- Accessibility claim under evaluation: a user with limited wrist mobility can
  point and click on common opaque surfaces without host-side remapping
- Surface claim today: best-case on opaque rigid surfaces only; not glass

## Key Physical Assumptions

- Sensor angle: `30 deg`
- Shell sizing: parametric for finger circumference, not one fixed ring size
- Focal-distance strategy: raised rim and glide pads maintain the optical gap
- Shell closure stays non-hermetic and reopenable through a top service lid,
  nested seam skirt, pry relief, and two commodity service screws
- The current CAD keeps the rim and glide-pad geometry on the lower shell so
  battery service does not require disturbing the focal-distance features
- The current seam assumption should remain inside the existing `~$9` ring BOM
  target; if the fasteners stay required after fit validation, lock them into
  the BOM rather than treating them as invisible hardware

## Replaceable Subassemblies

- PCB or module assembly
- PAW3204 optical sensor plus matched lens/emitter stack
- LiPo cell
- Service lid and closure screws
- Off-board USB service/charge fixture interface
- Off-board replaceable battery service harness or fixture interface
- Shell body and structural rim
- UHMWPE glide pads
- Dome click element

## Tracked First-Board Outputs

- [FIRST-BOARD-CHECKLIST.md](FIRST-BOARD-CHECKLIST.md) — active capture / routing / CAD closure checklist
- [STACKUP-VERIFY.md](STACKUP-VERIFY.md) — package-closure evidence template
- `kicad/r30_oled_none_none.kicad_pcb` — first routed rigid P0 PCB source
- Printable lower-shell and service-lid exports derived from `cad/r30_oled_none_none_shell_blank.scad`
- Quick-print fit coupons exported from the same OpenSCAD file with
  `export_mode` values for service-pad access, board retention, lid pads,
  battery harness routing, and service-lid removal
- Local generated coupon bundle from `scripts/generate-r30-ring-fit-coupons.sh`
  under `build/r30-oled-none-none-mechanical/`; this includes STL/PNG/hash
  scaffolding and a blank worksheet only, not physical fit evidence

## Current PCB Packet Scope

- Locks the P0 rigid board as a `43 x 18 mm` validation pass split into
  power/service, sensor/click, and MCU/radio zones
- Preserves an ESP32-C3 antenna copper/component keep-out at the outward edge
  before any routing convenience
- Places the PAW3204 on the bottom side at the optical aperture datum and keeps
  the sensor lane P0-specific rather than adding PMW3360 copper
- Treats ADNS-2080 as a sourcing fallback that needs a future evaluated board
  profile, not a drop-in substitution for this PAW3204 first-board packet
- Replaces the onboard JST-SH `J_BAT` body with same-net off-board battery
  service pads for `VBAT+`, `VBAT-`, and grounded `MP1`/`MP2`; battery service
  must remain replaceable through a harness or fixture, not direct cell solder
- Replaces the onboard USB-C receptacle with a same-net off-board service-pad
  interface for USB2 data, VBUS, CC pulldowns, GND, and shield continuity; a
  fixture or pogo harness is now required for charge/program service
- Locks D1 to a rail-less TPD2E2U06DCK-class SC-70/SOT-323 USB data ESD shunt
  so the constrained service-edge pocket no longer needs a VBUS clamp branch
- Routes the charge path with TP4054, `20 kohm` RPROG, RT9080, NTC divider, and
  a non-BOM `Q1` copper service jumper from fixture-fed `VBUS_5V` to TP4054
  `VCC`; no onboard MCU charge-enable switch is claimed in this P0
- Uses a source-controlled compact RT9080-33GJ5 SOT-23-5 land pattern for `U4`
  to relieve the first-board regulator/click corridor without changing the
  regulator, pinout, or BOM line
- Recommends `R7`/`R8` = `100k`/`100k` for `VBAT_SENSE` to ESP32-C3 `GPIO0`;
  this accepts an always-on ~21 µA cell draw for first-board ADC reliability
- Recommends `R9`/`R10` = `220k`/`100k` for `VBUS_DETECT` to ESP32-C3 `GPIO3`;
  this draws ~16 µA only from USB VBUS while plugged in
- Keeps `TP_CHRG` as a TP4054 `CHRG_STAT` fixture status pad without an onboard
  pull-up; no firmware consumer or MCU GPIO is claimed for it yet
- Keeps PAW3204 `SENSOR_NRESET` and `SENSOR_MOTION_N` as local bring-up pads
  only. The first optical board spends no MCU GPIOs on sensor reset or optical
  motion wake; firmware polls the PAW3204 and uses dome-only wake until real
  hardware evidence justifies a board-contract change.

## Current CAD Packet Scope

- Models a lower shell that retains the structural rim, angled sensor tunnel,
  and four discrete glide-pad pockets
- Models the routed board as a `43 x 18 mm` PCB in a shallow top pod instead of
  a generic module pocket; the PCB datum is KiCad board center `(120.5, 100)`
- Adds first-pass left-edge access for the off-board service-pad interface
  instead of an onboard USB-C receptacle body or shell-stake footprint
- Adds a non-USB board retention path: molded side rails, two side stop lugs,
  and small lid compression pads in board-edge keep-out zones
- Adds a first-pass pocket over the `SW1` 5 mm dome at the current relieved
  board coordinate and a top relief for an accessible actuator; click force,
  cap material, and dome replacement are still unproven
- Adds a battery harness channel from the actual `J_BAT` service-pad area plus
  a service-loop relief and top lift window for a protected `80-100 mAh` cell
- Models a removable top service lid with a nested locating skirt, pry notch,
  and two top-access screw paths kept away from the antenna keep-out
- Reserves service volume for a protected `80-100 mAh` cell, a battery harness
  channel, and a small service-loop relief so battery removal is not forced to
  start at the service connector
- Adds quick-print validation coupons for the remaining fit unknowns:
  `service_access_coupon` for off-board service-pad access,
  `board_retention_coupon`, `lid_pad_coupon`,
  `battery_harness_coupon`, `service_lid_coupon`, and a combined `fit_coupons`
  sheet
- Keeps this as a first-board fit pass only; it does not prove comfort, RF,
  screw clearance, dome actuation, or printable tolerances

## Missing Artifacts

- Clean KiCad DRC after the parity-clean first-pass schematic capture; local
  KiCad CLI `10.0.2` DRC still reports violations, so the board is not
  fabrication-release even though the unconnected-item bucket is now closed
- Printed/measured results from the new CAD coupons and full-shell export
- Completed physical-check worksheet copied from the local
  `build/r30-oled-none-none-mechanical/PHYSICAL-CHECK-WORKSHEET.md` scaffold
  after real coupon or board checks
- Measured ring stackup with focal-distance verification
- Assembly photos, torque values, and validated fit tolerances
- Test notes tied to [docs/GO-NO-GO-RUBRIC.md](../../../docs/GO-NO-GO-RUBRIC.md)
- Measured validation for the first-pass off-board service-pad access, board
  rails/stops/lid pads, dome pocket, battery harness channel, and service-lid
  removal path; the coupon modes make these quick to print, but the rows remain
  red until tested
- BDFL accepted the packet recommendation for the first rigid P0:
  fixture-controlled charge VBUS, `R7`/`R8` = `100k`/`100k`, and
  `R9`/`R10` = `220k`/`100k`. Reintroducing an onboard charge-enable switch or
  a future onboard `CHRG_STAT` pull-up remains an explicit substitution, not a
  silent equivalent
- Clean routed-board DRC after full schematic/PCB parity; `R7`-`R10`, `U1`,
  `U2`, `SW1`, sensor support parts, bring-up pads, sheet-level routed nets,
  USB ESD labels, charge-service jumper mapping, service shield pads, and
  `J_BAT` `MP1`/`MP2` mounting pads now have first-pass schematic counterparts,
  and schematic ERC is clean with project-local `PowerFinger` libraries loaded.
  The first routing cleanup fixed the USB-C contact pad geometry, snap-dome
  contact geometry, and the first batch of wrong power/USB endpoint
  coordinates. The second routing cleanup removed the bad SDIO-to-motion tie,
  normalized routed traces to the board minimum width, doglegged the
  sensor/VBUS-detect fanout, and moved the long VBUS detector feed out of the
  board middle. This pass moved the `SW1` dome left of the MCU pad column and
  moved the matching shell pocket with it. The next cleanup corrected the
  charge-service endpoints for `Q1`/`R4` `CHARGE_GATE` and `U3`/`R1` `PROG_R`.
  This pass moved `R4` away from J1's `A5`/CC1 crowding, added an explicit
  `A4`-to-`R4` `VBUS_5V` feed, and corrected `VBUS_CHG_SW` onto U3's VCC pad,
  leaving the remaining USB-edge shorts as real routing/placement work rather
  than wrong-pad connectivity. This pass rotates `Q1` off the J1 `A7` row,
  moves `R4` into the same charger cluster, and keeps J1 `A9` disconnected
  until the broader USB/D1 fanout can be rerouted cleanly. This pass moves
  `D1` slightly upward, doglegs the J1 `A6`/`A7` data escapes, and separates
  the long `USB_D+`/`USB_D-` lanes into U1 without reconnecting the still-red
  VBUS branch. This pass extends the left service edge to contain the fixed
  USB-C B-row contacts, updates the shell CAD to the new `43 x 18 mm` board
  datum, and ties J1 `A4`/`A9`/`B4`/`B9` as a single VBUS contact group. This
  pass moves `R2B` next to J1 `B5`, shifts the local VBUS trunk inward, and
  moves the `NTC1`/`R3` pair down out of D1's immediate `USB_D+` corridor.
  This pass reroutes the J1 `A6`/`A7` to D1 data escape onto `B.Cu` under the
  charger cluster, then re-enters beside D1 with the `USB_D-` entry via shifted
  left for clearance. This pass routes the mirrored J1 `B6`/`B7` USB data pads
  into that escape with a separated `B.Cu` fanout and front-layer `USB_D-`
  overpass, while leaving direct D1 VBUS out because it still regresses total
  DRC. Route-only D1 GND/VBUS service-island attempts were also tested and
  rejected because they reduced unconnected rows only by increasing total DRC
  debt, so no D1 stitch is retained. This pass replaces the rail-clamped USBLC6
  D1 island with a rail-less TPD2E2U06DCK-class SC-70/SOT-323 data shunt,
  removing the D1 VBUS branch while keeping ERC and schematic-parity clean.
  This pass adds a service-edge GND/shield spine across the J1 ground contacts
  and service shield pads, ties nearby `R2B`, `R1`, and `J_BAT` `MP1` into that
  return, and drops D1/NTC ground through a B-side via without reintroducing
  VBUS-clamped ESD protection. This pass reroutes the battery-side GND fanout:
  `R8`/`J_BAT` `MP2` now dogleg away from `VBAT_SENSE`, `J_BAT` pad 2 joins
  that local return, `J_BAT` `MP1` uses a B-side via return, and
  `R2A`/left `SW1`/`Q2` source share one local ground chain. This pass also
  normalizes the six remaining sub-minimum first-board vias to the board-rule
  minimum `0.50` mm diameter / `0.30` mm drill, then moves the `VREG_3V3`
  sensor feed off the PAW3204 LED/GND pad column through a right-side B-side
  trunk with a short `C1B` branch. This pass also moves `R6` onto the existing
  local GND run, shifts `TP_LEDKIT` out of the `C1B` / GND pinch, and doglegs
  the U3 `VBAT_PROTECTED` escape around the TP4054 GND/CHRG pad row before it
  rejoins U4 VIN. This pass then moves C2 closer to U4 VIN and retargets the
  C2 `VBAT_PROTECTED` leg while keeping the C2 GND pad as an explicit blocker
  rather than accepting a bad stitch. This follow-on micro-retargets C2 again
  inside the U4 input-cap pocket, preserves the U3 dogleg and shell-bound
  footprints, and leaves the rejected direct `SW1`/`U1`/`C1A` stitches out
  because they still increase total debt. This follow-on also reroutes the
  `C1B` ground return and `SENSOR_LED_KIT` test-pad stub inside the B-side
  sensor pocket without moving the shell-bound PAW3204 aperture. This follow-on
  moves non-shell-bound `C1A` into the MCU-side decoupling pocket and ties it to
  U1 `3V3` / `GND1`. This follow-on then moves non-shell-bound `TP_VBAT`,
  reroutes `VBAT_SENSE` below the top `SW1` GND pad, and ties the right `SW1`
  ring pad into the local U1/C1A ground leg. This follow-on then reroutes
  `CLICK_PRIMARY_N` through a B-side via pair so the click trace no longer runs
  through the lower `SW1` GND pad. This follow-on also reroutes the C1B/U2
  B-side GND leg around the PAW3204 LED pad column. KiCad CLI `10.0.1` still
  reports DRC=161 and unconnected=9, with schematic-parity=0. This follow-on
  then moves non-shell-bound `R1` into a horizontal charger-programming pocket
  and returns its GND pad through the accepted B-side GND trunk; KiCad CLI
  `10.0.1` then reported DRC=158 and unconnected=9, with schematic-parity=0.
  This follow-on nudges non-shell-bound U4 inside the regulator pocket while
  preserving the accepted C2 placement and U3 dogleg; KiCad CLI `10.0.1` now
  reports DRC=156 and unconnected=9, with schematic-parity=0. This follow-on
  then moves the local D1/NTC ground return to a short B-side dogleg, removing
  the `GND`/`NTC_SENSE` short while preserving shell-bound footprints, the
  accepted C2 placement, and the accepted U3 dogleg. This follow-on also
  micro-retargets non-shell-bound `TP_VBAT` upward to clear the upper
  `SW1`/`VBAT_SENSE` short; KiCad CLI `10.0.1` then reported DRC=153 and
  unconnected=9, with schematic-parity=0. This pass moves the PAW3204 optical
  aperture datum circle from `Edge.Cuts` to `Cmts.User` so it remains a
  package/shell alignment datum instead of a false fabrication cutout, and
  detours the long `VBUS_5V` feed to the `R9`/`R10` detect divider farther
  right of the bring-up pads. This pass also keeps the ESP32-C3 external antenna
  keep-out strict for tracks, vias, pads, and copper pour while allowing the
  owning module footprint whose antenna courtyard extends to the board edge.
  This follow-on moves the dense left-service reference fields for `J_BAT`,
  `Q1`, `R8`, and `R2A` from `F.SilkS` to `F.Fab`, keeping the bring-up labels
  in the source file without printing clipped silkscreen in the service pocket.
  KiCad CLI `10.0.2` then reported DRC=105 and unconnected=9, with
  schematic-parity=0. This follow-on moves non-shell-bound `R2A` to
  `(111.650, 95.000)` and retargets its `USB_CC1_RD` plus local GND endpoints,
  clearing the `USB_CC1_RD` / `CHARGE_GATE` short while keeping ERC,
  unconnected count, and schematic/PCB parity unchanged. KiCad CLI `10.0.2`
  then reported DRC=104 and unconnected=9, with schematic-parity=0. This
  follow-on retargets the `VBAT_SENSE` divider junction to `(113.000, 91.900)`,
  pulling the `R7`/`R8`/`TP_VBAT` sense trunk above the upper `SW1` ground ring
  without moving shell-bound `SW1` or its CAD dome pocket. KiCad CLI `10.0.2`
  then reported DRC=102 and unconnected=9, with schematic-parity=0. This
  follow-on retargets the top-edge `VBUS_5V` trunk start to
  `(105.600, 91.600)`, clearing the `J_BAT` `MP1` short without moving the
  then-shell-bound battery connector or adding a via. KiCad CLI `10.0.2` then reported
  DRC=101 and unconnected=9, with schematic-parity=0. This follow-on flips the
  non-polar `R9` divider resistor in place so the right-side `VBUS_5V` trunk
  lands on the VBUS pad and `VBUS_DETECT` leaves from the opposite pad into the
  existing top node, while moving `R9`'s source reference field to `F.Fab` to
  avoid printed copper overlap in that dense pocket. KiCad CLI `10.0.2` then
  reported DRC=98 and unconnected=9, with schematic-parity=0. This follow-on
  moves non-shell-bound `R11` and `TP_CHRG` down into the charger-status pocket
  at `y=105.000`, retargets their `CHRG_STAT` and `VREG_3V3` local segments,
  and moves their source reference fields to `F.Fab` so the dense regulator
  pocket is not burdened with printed labels. KiCad CLI `10.0.2` then reported
  DRC=95 and unconnected=9, with schematic-parity=0. This follow-on keeps the
  accepted top-edge `VBUS_5V` and `VBAT_SENSE` copper after scratch retargets
  introduced shorting, copper-edge, or unconnected debt, and instead moves the
  source-only `R7`, `TP_VBAT`, and `SW1` reference fields to `F.Fab`. KiCad CLI
  `10.0.2` then reported DRC=90 and unconnected=9, with schematic-parity=0.
  This follow-on keeps the accepted `CHARGE_EN`, `VREG_3V3`, and
  `VBAT_PROTECTED` copper after scratch `CHARGE_EN` route and `R6` placement
  variants reintroduced shorting or unconnected debt, and instead moves the
  source-only `Q2`, `U4`, and `R6` reference fields to `F.Fab`. KiCad CLI
  `10.0.2` then reported DRC=86 and unconnected=9, with schematic-parity=0.
  This follow-on keeps the accepted `USB_CC1_RD`, `VBUS_5V`, and `CHARGE_GATE`
  copper after scratch CC1 and gate route variants reintroduced shorting or
  unconnected drift, and instead moves the source-only `R4` and `R1` reference
  fields to `F.Fab`. KiCad CLI `10.0.2` then reported DRC=83 and unconnected=9,
  with schematic-parity=0. This follow-on keeps the accepted `NTC_SENSE`,
  `VREG_3V3`, and `CHRG_STAT` copper after scratch variants reintroduced
  shorting or raised total DRC, and instead moves the remaining source-only
  silkscreen marks to `F.Fab`/`B.Fab`. KiCad CLI `10.0.2` then reported DRC=60
  and unconnected=9, with schematic-parity=0. This follow-on keeps
  `solder_mask_min_width=0.05` but changes the first-board pad mask expansion
  from `0.05` to `0`, reducing mask-bridge rows without relaxing the minimum
  mask-web rule. KiCad CLI `10.0.2` then reported DRC=57 and unconnected=9, with
  schematic-parity=0. This follow-on keeps U4 and C2 placement fixed after
  scratch placement variants either held total DRC or reintroduced shorts, and
  instead narrows the two `VBAT_PROTECTED` regulator/C2 spokes from `0.32` to
  `0.24`. KiCad CLI `10.0.2` then reported DRC=56 and unconnected=9, with
  schematic-parity=0. This follow-on rejects R4 orientation and CC1 dogleg
  variants that added unconnected drift or shorting risk, and instead nudges
  non-shell-bound Q1 upward by `0.20` mm while retargeting its three local
  service endpoints. KiCad CLI `10.0.2` then reported DRC=55 and unconnected=9,
  with schematic-parity=0. This follow-on rejects top-edge
  `VBUS_5V`/`VBAT_SENSE` variants that introduced shorting or copper-edge debt,
  and instead nudges Q1 upward by another `0.10` mm while retargeting its three
  local service endpoints. KiCad CLI `10.0.2` then reported DRC=54 and
  unconnected=9, with schematic-parity=0. This follow-on rejects
  regulator/power-pocket width, R11/TP_CHRG, U4, R6, C2 GND, C1A, and layer-hop
  probes that held total DRC or traded the target row for new debt, and instead
  nudges non-shell-bound `TP_VBAT` right by `0.10` mm while retargeting its two
  local `VBAT_SENSE` endpoints. KiCad CLI `10.0.2` then reported DRC=53 and
  unconnected=9, with schematic-parity=0. This follow-on rejects direct SCLK,
  top-dogleg SCLK, `VBUS_DETECT`, and `SENSOR_MOTION_N` route variants that
  introduced shorts or clearance debt, and instead raises the local
  `SENSOR_SCLK` jog from `y=99.900` to `y=99.980`. KiCad CLI `10.0.2` then
  reported DRC=51 and unconnected=9, with schematic-parity=0. This follow-on
  rejects VBUS spine, VBUS dogleg, B-side motion-hop, and right-of-VBUS
  `TP_MOT` variants that held total DRC or added dangling vias, shorts, mask, or
  clearance debt, and instead moves non-shell-bound `TP_MOT` from
  `(122.050, 97.900)` to `(121.250, 99.500)` while retargeting its local
  `SENSOR_MOTION_N` endpoint. KiCad CLI `10.0.2` now reports DRC=48 and
  unconnected=9, with schematic-parity=0. This follow-on rejects left-service
  Q1/R2A/R4/CC1 micro-nudges plus via/layer-hop trunk moves that held total DRC
  or introduced shorts, dangling vias, clearance, mask, or unconnected debt, and
  instead lowers the `VBAT_SENSE` divider junction to `(113.000, 92.100)` while
  moving `SW1` and the CAD dome pocket from `(114.200, 95.000)` to
  `(114.200, 95.300)`. KiCad CLI `10.0.2` now reports DRC=46 and unconnected=9,
  with schematic-parity=0. This follow-on rejects USB data-pair and broad
  `NTC_SENSE` trunk/layer-hop variants that held total DRC or added shorts,
  clearance, mask, or track-width debt, and instead moves only the local
  `NTC_SENSE` junction from `(110.420, 106.700)` to `(109.900, 106.700)`. KiCad
  CLI `10.0.2` now reports DRC=45 and unconnected=9, with schematic-parity=0.
  This follow-on rejects left-service VBUS doglegs, VBUS junction moves, and
  broad R2B/CC2 shifts that held total DRC or added shorts, copper-edge,
  hole-clearance, mask, or extra clearance debt, and instead moves only `R2B`
  from `(100.500, 101.500)` to `(100.400, 101.500)` while retargeting its local
  `USB_CC2_RD` and GND endpoints. KiCad CLI `10.0.2` now reports DRC=44 and
  unconnected=9, with schematic-parity=0. This follow-on rejects
  `VBUS_5V`/`CHARGE_GATE`/`J_BAT` pocket doglegs and Q1/R2A micro-nudges that
  held total DRC or opened shorting, clearance, mask, or courtyard debt, and
  instead moves shell-bound `SW1` from `(114.200, 95.300)` to
  `(114.350, 95.780)` while moving the CAD dome pocket with it. KiCad CLI
  `10.0.2` repeat runs later reported DRC=43; the next courtyard-only cleanup
  narrows the source-controlled `J1`, `J_BAT`, `U1`, and `SW1` first-board
  courtyards without moving pads, copper, placements, or shell CAD bindings.
  A follow-up scratch pass then tested `J_BAT` upward moves to create real
  service-edge courtyard separation, but each candidate raised total DRC to
  `47..49` and reintroduced shorting rows, so no placement move is retained.
  This pass then narrows the `J1` service-edge courtyard to the actual USB-C
  pad row, clearing the remaining `J1`/`J_BAT` courtyard row without moving
  shell-bound pads, copper, placements, or CAD bindings. Same-slice C2
  ground-return and R8 divider placement probes were rejected because they
  raised total DRC. This follow-up rejects a via-assisted `NTC_SENSE` B-side
  reroute and local GND-via move that reintroduced shorting or unconnected
  drift, then rotates non-polar C2 so `VBAT_PROTECTED` reaches the near pad
  without crossing through the C2 ground pad column. This follow-up rejects a
  rotated-`U4` power/status topology: the best repaired variant reached
  `DRC=38` with `unconnected=9`, but still introduced current shorting rows,
  and the right-shifted correction rose to `DRC=44`. This follow-up also
  rejects a broader left power-switch corridor scratch redraw: moving
  `R11`/`TP_CHRG` beside `U3` raised DRC to `55`, a B-side `CHARGE_EN` hop
  raised DRC to `46`, and a right-shifted `Q2` placement raised DRC to `43`
  with `unconnected=10`. A true grouped placement translation that shifted
  `Q1`/`Q2`/`U3` left and `U4` right raised DRC to `72` with
  `unconnected=13`, so that topology is also rejected. Splitting `U4`/`C2` out
  as a separate regulator island above the charge/switch corridor raised DRC to
  `49` with `unconnected=10`, so that topology is rejected as well. This
  follow-on then keeps the broader corridor placement intact and normalizes the
  long `VREG_3V3` rail plus the short C2 `VBAT_PROTECTED` spoke to the
  board-rule minimum `0.20 mm`; KiCad CLI `10.0.2` wrapper reruns now
  conservatively reported DRC=39 with unconnected=9, schematic-parity=0, no
  courtyard rows, and no current shorting bucket, because the board is still
  hand-routed. This follow-on closes six GND-continuity rows through a lower
  `SW1` local stitch plus via-assisted U3, C2, `J_BAT`, left-trunk, and
  U1-side C2 returns. Direct `J1` A12 and top-`SW1` closure variants were
  rejected because they reintroduced current shorting rows. KiCad CLI `10.0.2`
  then reported DRC=39 with unconnected=3, schematic-parity=0, no courtyard
  rows, and no current shorting bucket. This follow-on closes the `J1` A12
  service-ground row through the existing lower GND rail instead of adding a
  pad-local via that collides with `USB_D-`. Top-`SW1` closure remains rejected
  as a four-contact footprint because the latent U4, U1, and Q2 power/sense
  shorts are still exposed by direct top-ring closure. This follow-on instead
  removes the top `SW1` dome-ring contact from the board and source footprint,
  keeps the left/right/lower ring contacts, and ties the remaining right/lower
  contacts together. KiCad CLI `10.0.2` then reported DRC=40 with unconnected=0,
  schematic-parity=0, no courtyard rows, and no current shorting bucket.
  A follow-on clearance-only scratch pass tested B-side and doglegged `SW1`
  returns, `VREG_3V3` rail moves, remaining `0.24 mm` power-spoke
  normalization, and small `VBAT_SENSE`/`NTC_SENSE`/`USB_CC1_RD`/
  `CHARGE_GATE`/`CHARGE_EN`/`CHRG_STAT` doglegs; no candidate beat the live
  `DRC=40` baseline without reopening shorting, so no additional PCB topology
  is retained. A placement-level follow-up then tested bounded
  `Q2`/`U4`/`C2`/`R11`/`TP_CHRG` moves with connected endpoint retargets rather
  than detached components; the only below-40 scratch result introduced a
  `VBAT_PROTECTED`/`CHARGE_EN` short, and the clean variants failed to beat
  `DRC=40`, so no placement topology is retained either. A controlled corridor
  rip-up then removed and redrew the local power/status spokes in scratch;
  frozen-placement rewrites returned `DRC=48..50`, and grouped regulator-island
  sketches returned `DRC=57..72` with shorting, hole-clearance, or unconnected
  debt, so no rip-up topology is retained. A fuller blank-slate local-spine
  scratch pass then removed and redrew all local `VBUS_CHG_SW`,
  `VBAT_PROTECTED`, `VREG_3V3`, `PROG_R`, `CHARGE_GATE`, `CHARGE_EN`, and
  `CHRG_STAT` copper in the active corridor; it returned raw `DRC=89..98` with
  real shorting, dangling-track, and unconnected rows, so no local-spine
  topology is retained. The follow-on left-service USB/charge fanout
  repartition is rejected too: `J1` translations returned non-library
  `DRC=75..81`, coupled service-pocket spreads returned non-library
  `DRC=111..156`, and route-only USB/CC/VBUS fanout rewrites either reopened
  shorting or unconnected rows or merely matched the retained `DRC=40` board.
  No left-service fanout topology is retained. The right-side regulator/click
  density follow-up is rejected as well: direct `SW1` moves returned
  non-library `DRC=49..65`, `SW1`+`C1A` moves returned `DRC=62..66`,
  `U4`/`C2`/`R11`/`TP_CHRG` island moves returned `DRC=78..90`, and
  island-plus-`SW1`/VREG-branch variants returned `DRC=89..103`, all with
  reopened shorting, dangling, or unconnected debt. No right-side density
  topology is retained.
  The clean-room active-R30 scratch-board follow-up is rejected too. The
  no-placement-change all-pad lane sketch returned non-library `DRC=564` with
  `unconnected_items=10`; the corresponding support-placement sketch returned
  non-library `DRC=560` with `unconnected_items=9`. The safer function-lane
  reroute still returned non-library `DRC=405` on existing placements and
  `DRC=416` after support-placement moves, both with `unconnected_items=10`.
  All of those scratch candidates reopened shorting and other fabrication
  buckets, so no clean-room lane topology is retained.
  The first four-layer architecture scratch is also not retained. A KiCad
  10-format stackup-only scratch stayed effectively tied with the retained
  board at non-library `DRC=41` in the upgraded scratch-report context. Moving
  the worst `VREG_3V3` and `VBAT_PROTECTED` diagonals onto inner layers returned
  non-library `DRC=58` with `unconnected_items=8` and `shorting_items=13`;
  extending the VREG inner bus returned non-library `DRC=74` with
  `unconnected_items=11`; and the VREG bus rewrite returned non-library
  `DRC=76` with `unconnected_items=7`. No route-migration four-layer topology
  is retained.
  The retained component-class USB service cut replaces the
  through-hole-staked USB-C receptacle with source-controlled off-board
  same-net service pads, adds the GND continuity via formerly supplied by the
  lower shell-stake return, and moves `USB_CC1_RD` onto a B-side jog. The live
  packet then verified with KiCad CLI `10.0.2` at `DRC=38`, `unconnected=0`,
  schematic-parity `0`, with no current shorting or courtyard bucket.
  The regulator/input-cap component-class follow-up is rejected: same-center
  `U4` pad-shrink and `C2` 0402 substitutions failed to beat the retained
  state, tiny same-pin DFN regulator sketches reopened unconnected/shorting
  buckets, diagonal route-migration sketches reopened shorting/unconnected
  buckets, and the connected C2 placement sweep only tied non-library
  `DRC=38`.
  The retained battery-service component-class cut then replaces the onboard
  JST-SH body with source-controlled off-board same-net battery service pads,
  preserving `VBAT+`, `VBAT-`, `MP1`, and `MP2` parity. KiCad CLI `10.0.2`
  then verified at `DRC=36`, `unconnected=0`,
  schematic-parity `0`, with no current shorting or courtyard bucket.
  This follow-on keeps the regulator choice and BOM fixed while moving
  non-shell-bound `U4` by `(+0.05, -0.05) mm` and switching only that regulator
  to the source-controlled `RT9080_33GJ5_SOT23_5_ServiceClearance` footprint.
  Shell-bound `J1`, `J_BAT`, `SW1`, the shell CAD openings, and `C2` stay fixed.
  KiCad CLI `10.0.2` then verified at `DRC=34`, `unconnected=0`,
  schematic-parity `0`, with no current shorting or courtyard bucket.
  This follow-on moves only non-shell-bound `R8` from `(111.800, 93.200)` to
  `(112.100, 93.400)` and retargets its local `VBAT_SENSE` and GND endpoints,
  closing the divider-pocket clearance row plus two nearby mask rows without
  touching shell-bound placements, the U4 land-pattern cut, BOM values, or
  schematic parity. KiCad CLI `10.0.2` then verified at `DRC=33`,
  `unconnected=0`, schematic-parity `0`, with no current shorting or courtyard
  bucket.
  This follow-on keeps the RT9080 part, pinout, `U4` placement, and
  source-controlled footprint name fixed while shrinking that local U4 land
  pattern from `0.45 x 0.80 mm` pads to `0.40 x 0.78 mm` pads. KiCad CLI
  `10.0.2` then verified at `ERC=0`, `DRC=32`, `unconnected=0`,
  schematic-parity `0`, with no current shorting or courtyard bucket.
  This follow-on removes the onboard `R11` `CHRG_STAT` pull-up and its local
  `VREG_3V3` spoke while keeping `TP_CHRG` as a fixture-observed TP4054 status
  pad. A fixture must provide a pull-up if charger status is measured.
  This follow-on moves only `TP_CHRG` and its local `CHRG_STAT` segment from
  `(116.100, 105.000)` to `(116.000, 104.000)`, keeping the fixture-observed
  status contract while removing one net DRC row. KiCad CLI `10.0.2` now
  verifies at `ERC=0`, `DRC=30`, `unconnected=0`, schematic-parity `0`, with
  no current shorting or courtyard bucket.
  This follow-on removes the active onboard charge-gate island from this first
  P0: `Q1` becomes a non-BOM copper VBUS service jumper, `R4`/`Q2`/`R6` drop
  from the active BOM and schematic, `GPIO10` is marked no-connect, and
  TP4054 `VCC` is fed directly from fixture `VBUS_5V`. The local U4 GND via is
  moved to `(112.000, 102.700)` so the cut does not reopen shorting. KiCad CLI
  `10.0.2` now verifies at `ERC=0`, `DRC=27`, `unconnected=0`,
  schematic-parity `0`, with no current shorting, dangling, or courtyard
  bucket.
  The placement-level four-layer scratch is rejected too. Moving only
  non-shell-bound support parts while keeping `J1`, `J_BAT`, `U1`, `U2`, `SW1`,
  the PAW3204 aperture, antenna keep-out, outline, and first-board BOM/netlist
  fixed returned non-library `DRC=127` with `unconnected_items=32` when the old
  route geometry was retained, and non-library `DRC=137` with
  `unconnected_items=36` when the worst power diagonals were replaced on inner
  layers. Full via-per-pad four-layer reroutes across three support-placement
  variants returned non-library `DRC=549..573` with `unconnected_items=74..75`.
  No fixed-anchor placement-level four-layer topology is retained.
  The first envelope scratch is rejected too. Coordinated scratch moves for the
  service edge, `J1`, `J_BAT`, `SW1`, and support parts returned non-library
  `DRC=149..157` with `unconnected_items=49..50`, `shorting_items=17..21`, and
  a reopened courtyard row when the old route geometry was retained. Generated
  bus reroutes for the same envelope candidates returned non-library
  `DRC=307..317` with `unconnected_items=18`, `shorting_items=88..92`, and the
  same courtyard debt. No same-BOM/netlist envelope topology is retained.
- Firmware allocation decision for `CHRG_STAT` if charger status needs to be
  reported in software rather than only checked by an external pull-up fixture
- Later-board allocation decision for PAW3204 `SENSOR_NRESET` or
  `SENSOR_MOTION_N` if bench evidence shows the first optical board needs
  firmware reset control or optical motion wake
- Cleared ERC/DRC against the routed PCB; current snapshot lives in
  [kicad/CURRENT-VIOLATIONS.md](kicad/CURRENT-VIOLATIONS.md) and is
  regenerated by `scripts/verify-firmware-local.sh --kicad-only`
- Confirmed service fastener size and whether limited-dexterity repair needs a
  captured-hardware follow-up instead of loose screws
- Confirmation that the larger top PCB pod still preserves the active `~$9`
  ring intent after printable wall thickness, fasteners, and service features
  are costed

## Required First-Hardware Evidence

- Real stackup fits the height budget from
  [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- Battery selection still satisfies
  [docs/BATTERY-SAFETY.md](../../../docs/BATTERY-SAFETY.md)
- Shell can be opened and reclosed without destroying the battery bay, sensor
  mount, or antenna keep-out
- Battery can actually leave the bay through the service opening without lead
  pinch, shell cracking, or forced service-fixture removal first
- Board can be removed after lid removal without prying on service pads, antenna
  end, or dome-switch contacts
- Record package-closure evidence in [STACKUP-VERIFY.md](STACKUP-VERIFY.md)
  before starting secondary ring or puck hardware
