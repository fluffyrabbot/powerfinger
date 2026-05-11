<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE KiCad Skeleton

This directory is the PCB/schematic starting point for the active optical ring
lane. It is intentionally honest about its state: the first routed PCB pass now
exists, and the power/USB plus MCU/radio plus sensor/click sheets now include
first-pass symbols, but the board DRC is still not clean enough for fabrication
release.

## Current Source Files

- `r30_oled_none_none.kicad_sch` — root sheet for the active optical ring lane
- `r30_oled_none_none.kicad_pcb` — first rigid P0 routed board pass, with
  footprint locks, placement zones, antenna keep-outs, and charge-path routing
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

1. Power tree: protected LiPo, TP4054 charge path, RT9080 LDO, charge-enable
   MOSFET, NTC divider
2. MCU/module block: `ESP32-C3-MINI-1-N4` with antenna keep-out reserved first
3. Sensor block: PAW3204-class optical sensor plus matching lens and aperture
4. Human input block: dome switch and any required debounce/passive support
5. USB-C and programming path for charging plus firmware bring-up

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
- JST-SH right-angle battery receptacle for a harnessed, replaceable protected
  cell
- USB-C service connector class locked to 16-pin USB 2.0 with through-hole
  shell stakes
- TP4054 path routed through a P-channel VBUS switch, `20 kohm` RPROG, RT9080
  regulation, and an NTC divider near the battery connector
- MCU/radio and sensor/click sheets populated with first-pass symbol
  counterparts for the routed `U1`, `U2`, `SW1`, decoupling, sense testpads,
  and sensor bring-up pads
- routed sheet nets promoted to passive global labels where they intentionally
  match flat PCB net names, with project-local SOT-23 pin-numbered
  `Q1`/`Q2` symbols, aligned USB ESD D+/D- labels, and USB-C shield stakes
  mapped to `SH`
- `J_BAT` backed by a project-local first-board symbol that models `MP1`/`MP2`
  as GND-tied mounting/shield pads, so schematic/PCB parity is clean
- first routing cleanup of USB-C SMT contact pad length, snap-dome contact
  geometry, and wrong first-pass power/USB endpoint coordinates
- second routing cleanup of the SDIO/motion fanout, VBUS-detect dogleg, long
  VBUS detector feed, and minimum routed trace width
- charger-cluster cleanup that rotates `Q1` off the J1 `A7` row and moves
  `R4` with it instead of routing raw and switched VBUS through the A7/A8 lane
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
- `R6` / `TP_LEDKIT` placement cleanup that lands the charge-enable pulldown on
  the existing local GND run and moves the PAW3204 LED test pad out of the
  `C1B` / GND pinch
- U3 VBAT escape cleanup that routes TP4054 BAT around the GND/CHRG pad row
  through an outer dogleg before rejoining the U4 VIN trunk
- C2 input-cap placement cleanup that moves and micro-retargets the 10uF
  capacitor closer to U4 VIN while leaving its GND pad as an honest unconnected
  blocker
- B-side sensor-pocket cleanup that reroutes the `C1B` ground return and
  `SENSOR_LED_KIT` test-pad stub without moving the shell-bound PAW3204 aperture
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

This pass accepts `Q2` = 2N7002 SOT-23 plus `R6` = `100k` as the logic-safe
charge-gate driver; the direct MCU-to-P-channel-gate option is rejected while
the gate can see `VBUS_5V`. The PCB now also carries the recommended
sense/status support parts: `R7`/`R8` = `100k`/`100k` for `VBAT_SENSE`,
`R9`/`R10` = `220k`/`100k` for `VBUS_DETECT`, and `R11` = `100k` as the TP4054
`CHRG_STAT` pull-up. `VBAT_SENSE` and `VBUS_DETECT` are routed to the ESP32-C3
resources named in [INTERFACE-CONTRACT.md](INTERFACE-CONTRACT.md). `CHRG_STAT`
is pulled up and locally testable, but no MCU GPIO or firmware consumer is
claimed yet. PAW3204 `SENSOR_NRESET` and `SENSOR_MOTION_N` are likewise local
bring-up pads only for the first optical board, not firmware reset or wake
signals. The `43 x 18 mm` rigid pass now drives the shell CAD fit pass, but it
still does not prove the package is comfortable, printable, RF-clean, or
serviceable with a populated board.

## Mechanical Binding Into CAD

`../cad/r30_oled_none_none_shell_blank.scad` now maps the routed board into the
shell using these KiCad facts:

- board outline: `99..142 x 91..109 mm`, treated as a `43 x 18 mm` PCB around
  center `(120.5, 100)`
- USB-C opening: `J1` at `(102.650, 100.000)` on the left service edge
- battery lead path: `J_BAT` at `(108.600, 94.250)`
- dome pocket: `SW1` at `(114.200, 95.000)`
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
- Treat the current schematic as first-pass capture, not proof that the board
  is fabrication-clean. The current local KiCad CLI `10.0.2` snapshot is
  ERC=0, DRC=102, unconnected=9, and schematic-parity=0; the remaining board
  story is dominated by unconnected items and hand-routed clearance/shorting
  failures.
- Treat the first PCB as a routed board pass, not a green fabrication release.
  Clear schematic backfill, net cleanup, and DRC/ERC before fabrication.
- Close `../FIRST-BOARD-CHECKLIST.md` and `../STACKUP-VERIFY.md` before treating
  this lane as hardware-ready.
