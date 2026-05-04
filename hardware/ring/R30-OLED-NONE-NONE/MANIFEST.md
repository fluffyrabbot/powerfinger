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
- Optical sensor plus matched lens stack
- LiPo cell
- Service lid and closure screws
- USB-C charging connector
- Shell body and structural rim
- UHMWPE glide pads
- Dome click element

## Tracked First-Board Outputs

- [FIRST-BOARD-CHECKLIST.md](FIRST-BOARD-CHECKLIST.md) — active capture / routing / CAD closure checklist
- [STACKUP-VERIFY.md](STACKUP-VERIFY.md) — package-closure evidence template
- `kicad/r30_oled_none_none.kicad_pcb` — first routed rigid P0 PCB source
- Printable lower-shell and service-lid exports derived from `cad/r30_oled_none_none_shell_blank.scad`
- Quick-print fit coupons exported from the same OpenSCAD file with
  `export_mode` values for USB-C, board retention, lid pads, battery leads, and
  service-lid removal

## Current PCB Packet Scope

- Locks the P0 rigid board as a `43 x 18 mm` validation pass split into
  power/service, sensor/click, and MCU/radio zones
- Preserves an ESP32-C3 antenna copper/component keep-out at the outward edge
  before any routing convenience
- Places the PAW3204 on the bottom side at the optical aperture datum and keeps
  the sensor lane P0-specific rather than adding PMW3360 copper
- Locks `J_BAT` to a JST-SH right-angle 2-pin 1.0 mm battery harness receptacle
  instead of direct cell solder
- Locks service USB-C to a 16-pin USB 2.0 receptacle class with through-hole
  shell stakes; SMD-only and power-only USB-C are rejected for P0
- Locks D1 to a rail-less TPD2E2U06DCK-class SC-70/SOT-323 USB data ESD shunt
  so the constrained service-edge pocket no longer needs a VBUS clamp branch
- Routes the charge path with TP4054, `20 kohm` RPROG, RT9080, NTC divider, and
  a BDFL-accepted 2N7002 `Q2` logic-safe charge-gate driver so ESP32-C3
  `GPIO10` is not tied to a 5 V pull-up
- Recommends `R7`/`R8` = `100k`/`100k` for `VBAT_SENSE` to ESP32-C3 `GPIO0`;
  this accepts an always-on ~21 µA cell draw for first-board ADC reliability
- Recommends `R9`/`R10` = `220k`/`100k` for `VBUS_DETECT` to ESP32-C3 `GPIO3`;
  this draws ~16 µA only from USB VBUS while plugged in
- Recommends `R11` = `100k` as the TP4054 `CHRG_STAT` pull-up and keeps that
  signal as a local status/test pad only; no firmware consumer or MCU GPIO is
  claimed for it yet

## Current CAD Packet Scope

- Models a lower shell that retains the structural rim, angled sensor tunnel,
  and four discrete glide-pad pockets
- Models the routed board as a `43 x 18 mm` PCB in a shallow top pod instead of
  a generic module pocket; the PCB datum is KiCad board center `(120.5, 100)`
- Adds a first-pass USB-C service opening at the left board edge for the
  `USB4215`-class receptacle body and shell-stake footprint
- Adds a non-USB board retention path: molded side rails, two side stop lugs,
  and small lid compression pads in board-edge keep-out zones
- Adds a first-pass pocket over the `SW1` 5 mm dome at the current relieved
  board coordinate and a top relief for an accessible actuator; click force,
  cap material, and dome replacement are still unproven
- Adds a battery lead channel from the actual `J_BAT` area plus a service-loop
  relief and top lift window for a protected `80-100 mAh` cell
- Models a removable top service lid with a nested locating skirt, pry notch,
  and two top-access screw paths kept away from the antenna keep-out
- Reserves service volume for a protected `80-100 mAh` cell, a battery lead
  channel, and a small service-loop relief so battery removal is not forced to
  start at the USB connector
- Adds quick-print validation coupons for the remaining fit unknowns:
  `usb_c_coupon`, `board_retention_coupon`, `lid_pad_coupon`,
  `battery_lead_coupon`, `service_lid_coupon`, and a combined `fit_coupons`
  sheet
- Keeps this as a first-board fit pass only; it does not prove comfort, RF,
  screw clearance, dome actuation, or printable tolerances

## Missing Artifacts

- Clean KiCad DRC after the parity-clean first-pass schematic capture; local
  KiCad CLI `10.0.1` DRC still reports violations and unconnected items, so the
  board is not fabrication-release
- Printed/measured results from the new CAD coupons and full-shell export
- Measured ring stackup with focal-distance verification
- Assembly photos, torque values, and validated fit tolerances
- Test notes tied to [docs/GO-NO-GO-RUBRIC.md](../../../docs/GO-NO-GO-RUBRIC.md)
- Measured validation for the first-pass USB-C opening, board rails/stops/lid
  pads, dome pocket, battery lead channel, and service-lid removal path; the
  coupon modes make these quick to print, but the rows remain red until tested
- BDFL accepted the packet recommendation for the first rigid P0: `Q2` =
  2N7002 SOT-23, `R7`/`R8` = `100k`/`100k`, `R9`/`R10` = `220k`/`100k`, and
  `R11` = `100k`. BSS138-class alternates or a logic-level load switch remain
  explicit substitutions, not silent equivalents
- Clean routed-board DRC after full schematic/PCB parity; `R7`-`R11`, `U1`,
  `U2`, `SW1`, sensor support parts, bring-up pads, sheet-level routed nets,
  USB ESD labels, charge-gate MOSFET pin mappings, USB-C shield stakes, and
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
  and shell stakes, ties nearby `R2B`, `R1`, and `J_BAT` `MP1` into that
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
  rejoins U4 VIN. KiCad CLI `10.0.1` still reports DRC=168 and unconnected=11,
  with schematic-parity=0, because the board is still hand-routed
- Firmware allocation decision for `CHRG_STAT` if charger status needs to be
  reported in software rather than only checked at the local status pad
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
  pinch, shell cracking, or forced USB-C removal first
- Board can be removed after lid removal without prying on the USB-C receptacle,
  antenna end, or dome-switch contacts
- Record package-closure evidence in [STACKUP-VERIFY.md](STACKUP-VERIFY.md)
  before starting secondary ring or puck hardware
