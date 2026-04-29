<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE KiCad Skeleton

This directory is the PCB/schematic starting point for the active optical ring
lane. It is intentionally honest about its state: the first routed PCB pass now
exists, but the schematic sheets are still capture scaffolds and the board is
not fabrication-released.

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
  component keep-outs marked before any convenience routing
- bottom-side PAW3204 placement centered on the `6.2 mm` optical aperture datum
- JST-SH right-angle battery receptacle for a harnessed, replaceable protected
  cell
- USB-C service connector class locked to 16-pin USB 2.0 with through-hole
  shell stakes
- TP4054 path routed through a P-channel VBUS switch, `20 kohm` RPROG, RT9080
  regulation, and an NTC divider near the battery connector

The board also records three honest open items. `Q2`/`R6` are included because
the P-channel gate needs a logic-safe driver; add them to the active BOM before
fabrication or replace them with an equivalent load-switch solution. `VBAT_SENSE`,
`VBUS_DETECT`, and `CHRG_STAT` are bring-up pads only until the missing
divider/pull-up BOM lines are intentionally added. The `42 x 18 mm` rigid pass
now drives the shell CAD fit pass, but it still does not prove the package is
comfortable, printable, RF-clean, or serviceable with a populated board.

## Mechanical Binding Into CAD

`../cad/r30_oled_none_none_shell_blank.scad` now maps the routed board into the
shell using these KiCad facts:

- board outline: `100..142 x 91..109 mm`, treated as a `42 x 18 mm` PCB around
  center `(121, 100)`
- USB-C opening: `J1` at `(102.650, 100.000)` on the left service edge
- battery lead path: `J_BAT` at `(108.600, 94.250)`
- dome pocket: `SW1` at `(116.550, 94.450)`
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
- Treat the current schematic files as hierarchy scaffolds, not proof that the
  active components or footprints have already been validated in KiCad.
- Treat the first PCB as a routed board pass, not a green fabrication release.
  Local KiCad CLI `10.0.1` DRC currently reports violations and unconnected
  items; clear schematic backfill, net cleanup, and DRC/ERC before fabrication.
- Close `../FIRST-BOARD-CHECKLIST.md` and `../STACKUP-VERIFY.md` before treating
  this lane as hardware-ready.
