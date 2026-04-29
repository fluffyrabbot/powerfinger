<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE First Board Checklist

This checklist is the execution contract for the active ring hardware lane.

## PCB Capture And Layout

- [x] Lock the first-board footprint classes for the active lane in
  `kicad/CAPTURE-BINDINGS.md`
- [x] Complete the first routed rigid P0 source at
  `kicad/r30_oled_none_none.kicad_pcb`
- [x] Preserve the module antenna keep-out in the PCB pass and document the
  no-copper/no-component zones
- [x] Validate the charge path around `RT9080-33GJ5`, `20 kohm` charge resistor,
  NTC divider, and charge-enable MOSFET against `docs/BATTERY-SAFETY.md`
- [x] Validate the PAW3204-class sensor placement against the aperture datum in
  the PCB pass
- [ ] Backfill schematic symbols to match the PCB pass and clear KiCad ERC/DRC
  (current snapshot in `kicad/CURRENT-VIOLATIONS.md`: ERC=34, DRC=324,
  unconnected=41, parity=27 — regenerate with
  `scripts/verify-firmware-local.sh --kicad-only`)
- [x] Accept the `Q2`/`R6` charge-gate safety add into the active BOM CSV
- [x] Add starter `VBAT_SENSE` (`R7`/`R8`), `VBUS_DETECT` (`R9`/`R10`), and
  `CHRG_STAT` (`R11`) divider / pull-up lines to the active BOM CSV
- [ ] Lock BDFL-approved values for `R7`–`R11` and the production part for
  `Q2` (2N7002 vs BSS138 vs an equivalent logic-level load switch) before
  schematic capture
- [ ] Prove the `42 x 18 mm` rigid board, USB-C opening, JST-SH service loop,
  and antenna keep-out fit the current shell CAD or revise the shell/board

## Mechanical Closure

- [x] Keep `cad/r30_oled_none_none_shell_blank.scad` as a printable lower-shell
  plus service-lid packet, not a monolithic shell
- [x] Replace the generic electronics placeholders with the actual `42 x 18 mm`
  routed PCB datum and a board-sized top pod
- [x] Add a first-pass USB-C service opening for the left-edge `USB4215`-class
  receptacle; do not use the connector as the board-retention feature
- [x] Add a first-pass board retention path using molded side rails, side stop
  lugs, and lid compression pads that become accessible when the service lid is
  removed
- [x] Add a first-pass `SW1` 5 mm dome pocket and top actuator relief so the lid
  does not silently crush the click element
- [x] Add battery lead clearance from the actual `J_BAT` location plus a
  service-loop relief and top lift window
- [x] Keep the seam honest: nested locating skirt, pry relief, and accessible
  top-side closure hardware rather than glue or one-shot snaps
- [x] Add quick-print OpenSCAD coupon/export modes for USB-C fit, board
  rails/stops, lid compression pads, battery lead/service loop, and service-lid
  removal
- [x] Preserve glide-pad pockets and structural rim geometry on the lower shell
- [x] Lock the first board to a replaceable JST-SH `J_BAT` interface instead of
  direct cell solder
- [ ] Print/measure the USB-C opening, board rails/stops/lid pads, dome pocket,
  battery lead path, and lid removal path against the assembled first board or
  the matching quick-print coupon
- [ ] Verify the chosen service fastener path still fits inside the current
  `~$9` ring BOM assumptions
- [ ] Confirm the shell still supports battery replacement without destructive
  rework, USB-C removal, antenna disturbance, or glide-rim disturbance
- [ ] Decide whether limited-dexterity repair needs captured screws or another
  no-loose-hardware service-lid follow-up

## Bring-Up Evidence Required Before Secondary Variants

- Complete [STACKUP-VERIFY.md](STACKUP-VERIFY.md)
- Link assembled board / shell observations from `MANIFEST.md`
- Record which seam and battery-service assumptions survived the first build,
  and which did not
- Keep the packet honest if any item remains provisional or fails
