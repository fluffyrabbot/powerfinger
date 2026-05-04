<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE CAD Skeleton

This directory contains the first editable mechanical source for the active
optical ring lane.

`r30_oled_none_none_shell_blank.scad` is now a parametric lower-shell plus
service-lid packet for the first board. It is still not a production-ready
enclosure, and it does not claim that the fit, comfort, closure hardware, or
focal-distance stack are already validated.

## Inputs Exposed Today

- `finger_circumference_mm`
- `sensor_angle_deg`
- `band_width_mm`
- `shell_height_mm`
- `rim_height_mm`
- `glide_pad_thickness_mm`
- `service_seam_height_mm`
- `export_mode` for either the full shell packet or quick-print fit coupons
- routed `pcb_size_mm`, KiCad-derived feature centers, and antenna keep-out
- lid skirt / seam clearance values
- USB-C opening and body keep-out for the `USB4215`-class receptacle
- board rail, side-stop, and lid compression-pad placeholders
- `SW1` dome pocket and actuator-relief placeholders
- battery, sensor, and module keep-out proxies
- battery lead channel and service-loop relief from the actual `J_BAT` area
- top-access screw-boss placeholders

## What The Model Intentionally Does

- Gives the ring lane a real editable CAD source file in the repo
- Encodes the 30-degree optical-lane posture as a default, not a hard-coded
  forever truth
- Maps the current routed `43 x 18 mm` PCB into a top pod so the mechanical
  story no longer depends on a generic electronics box
- Cuts a first-pass left-edge USB-C opening and local body clearance for the
  routed service connector
- Adds a non-USB board-retention concept with molded rails, side stop lugs, and
  service-lid compression pads
- Adds a `SW1` dome pocket and actuator relief as a first pass, not as proven
  click ergonomics
- Keeps the structural rim and four discrete glide-pad pockets on the lower
  shell so the focal-distance geometry does not move during service
- Uses a removable top service lid with a locating skirt and pry relief instead
  of pretending the first board can be glue-sealed
- Reserves service volume for the routed board, battery, sensor cavity, and
  connector bodies so stackup conflicts stay visible before fabrication
- Adds a battery lead channel and service-loop relief so the cell has a
  believable removal path from the actual `J_BAT` area
- Adds quick-print coupon/export modes for the fit unknowns that should be
  checked before spending time on a full shell print

## What Still Needs Real Validation

- Comfort across actual finger sizes
- Real sensor cavity dimensions for the chosen optical sensor and lens kit
- Measured USB-C plug insertion, board rail/stop tolerance, lid pad clearance,
  dome-click travel, battery lead routing, and lid removal against a populated
  board or representative coupons
- Whether two loose service screws are acceptable for limited-dexterity repair
  or need a captured-hardware follow-up
- Battery fit with real wiring, the chosen `BT1` interface, and real connector
  strain relief
- Focal-distance proof on physical surfaces
- Whether the larger top PCB pod is comfortable, accessible, and still inside
  the current `~$9` ring intent once fasteners and printable wall thickness are
  costed

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../STACKUP-VERIFY.md](../STACKUP-VERIFY.md) as the active-lane source of truth
for what this CAD needs to prove before secondary variants move forward.

## Current Service Story

- Battery service is top-side only: remove the service screws, lift the lid from
  the pry relief, then lift the cell through the upper opening
- The board service path is also top-side: once the lid is off, molded rails and
  side stops should allow the PCB to lift out without levering on USB-C, the
  antenna end, or the dome switch
- The lower shell owns the rim, glide pads, and sensor tunnel so battery
  service does not ask the user or repair tech to disturb the focal-distance
  features first
- The battery path is still an honest first pass until a real JST-SH harness,
  protected cell, and printed lid prove lead bend radius and lift clearance

## Quick-Print Fit Coupons

The OpenSCAD file supports these `export_mode` values:

| Mode | Artifact | Fit Unknown Targeted |
|------|----------|----------------------|
| `shell` | Full lower shell plus service lid | Whole-packet geometry sanity |
| `usb_c_coupon` | USB-C wall and board-edge pocket | Plug insertion and shell/board loading |
| `board_retention_coupon` | Board rails plus stop lugs | `43 x 18 mm` board slide, lift, and tolerance |
| `lid_pad_coupon` | Board-edge compression-pad channel | Lid pad path without crushing components |
| `battery_lead_coupon` | Lead channel, connector pocket, and service-loop relief | JST-SH lead bend and lift clearance |
| `service_lid_coupon` | Skirt/socket/pry-removal coupon | Reopenable lid handling around service hardware |
| `fit_coupons` | One sheet with all coupons | Fast print for the first fit sweep |

These are validation aids, not new product geometry. A coupon pass should update
[../STACKUP-VERIFY.md](../STACKUP-VERIFY.md) with measured results and leave
`MANIFEST.md` honest if any coupon fails.

## Local Sanity Check

On macOS, prefer installing OpenSCAD with:

```bash
brew install --cask openscad@snapshot
```

The binary is still exposed as `openscad`. A useful packet-level render check is:

```bash
openscad \
  -D 'show_exploded_view=false' \
  -D 'show_reference_solids=false' \
  -D 'show_reference_fasteners=false' \
  -o /tmp/r30_oled_none_none_shell.stl \
  hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad
```

That command is a geometry sanity check only. It does not prove the shell fits
the real board, battery, or service hardware.

A quick-print coupon sheet can be exported with:

```bash
openscad \
  -D 'export_mode="fit_coupons"' \
  -o /tmp/r30_oled_none_none_fit_coupons.stl \
  hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad
```

Render an individual coupon by replacing `fit_coupons` with one of the coupon
modes above.
