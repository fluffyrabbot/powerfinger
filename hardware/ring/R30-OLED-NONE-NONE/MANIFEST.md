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

- Locks the P0 rigid board as a `42 x 18 mm` validation pass split into
  power/service, sensor/click, and MCU/radio zones
- Preserves an ESP32-C3 antenna copper/component keep-out at the outward edge
  before any routing convenience
- Places the PAW3204 on the bottom side at the optical aperture datum and keeps
  the sensor lane P0-specific rather than adding PMW3360 copper
- Locks `J_BAT` to a JST-SH right-angle 2-pin 1.0 mm battery harness receptacle
  instead of direct cell solder
- Locks service USB-C to a 16-pin USB 2.0 receptacle class with through-hole
  shell stakes; SMD-only and power-only USB-C are rejected for P0
- Routes the charge path with TP4054, `20 kohm` RPROG, RT9080, NTC divider, and
  a logic-safe charge-gate driver so ESP32-C3 `GPIO10` is not tied to a 5 V
  pull-up

## Current CAD Packet Scope

- Models a lower shell that retains the structural rim, angled sensor tunnel,
  and four discrete glide-pad pockets
- Models the routed board as a `42 x 18 mm` PCB in a shallow top pod instead of
  a generic module pocket; the PCB datum is KiCad board center `(121, 100)`
- Adds a first-pass USB-C service opening at the left board edge for the
  `USB4215`-class receptacle body and shell-stake footprint
- Adds a non-USB board retention path: molded side rails, two side stop lugs,
  and small lid compression pads in board-edge keep-out zones
- Adds a first-pass pocket over the `SW1` 5 mm dome and a top relief for an
  accessible actuator; click force, cap material, and dome replacement are still
  unproven
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

- Completed schematic symbol capture that matches the first PCB pass
- Clean KiCad DRC/ERC; local KiCad CLI `10.0.1` DRC still reports violations
  and unconnected items, so the board is not fabrication-release
- Printed/measured results from the new CAD coupons and full-shell export
- Measured ring stackup with focal-distance verification
- Assembly photos, torque values, and validated fit tolerances
- Test notes tied to [docs/GO-NO-GO-RUBRIC.md](../../../docs/GO-NO-GO-RUBRIC.md)
- Measured validation for the first-pass USB-C opening, board rails/stops/lid
  pads, dome pocket, battery lead channel, and service-lid removal path; the
  coupon modes make these quick to print, but the rows remain red until tested
- Active BOM update for the charge-gate safety add (`Q2`/`R6`) if the BDFL
  accepts the board-pass recommendation
- Production routing for `VBAT_SENSE`, `VBUS_DETECT`, and `CHRG_STAT`; these
  are bring-up pads only until divider/pull-up BOM lines are intentionally
  added
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
