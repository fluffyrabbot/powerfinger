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
- final KiCad U1 radio anchor at board-local `(+13.20, 0)`, populated body
  `16.6 x 13.2 x 2.4 mm`, and copper void `x=+16.1..+21.5` across the full
  board width
- explicit KiCad U2 board-local sensor datum at `(-4.0, 0.0)` and a nominal
  `2.9 mm` lens-to-installed-glide-pad gap as a fixture gauge setting
- lid skirt / seam clearance values
- off-board service-pad access opening and fixture keep-out at the `J1` edge
- board rail, side-stop, and lid compression-pad placeholders
- `SW1` dome pocket and actuator-relief placeholders
- battery, sensor, and module keep-out proxies
- battery harness channel and service-loop relief from the actual `J_BAT`
  service-pad area
- top-access screw-boss placeholders

## What The Model Intentionally Does

- Gives the ring lane a real editable CAD source file in the repo
- Encodes the 30-degree optical-lane posture as a default, not a hard-coded
  forever truth
- Maps the current routed `43 x 18 mm` PCB into a lateral `46.6 x 26.6 mm`
  electronics pod centered at board-local `(0, +23.4)`. The protected cell is
  in a separate `24.0 x 26.6 mm` pod centered at `(-36.0, +23.4)`, with a
  `4.15 mm` nominal x-clearance from the PCB and a routed service bridge.
  This is mechanically non-overlapping but creates a large lateral profile;
  comfort and accessibility remain unmeasured gates.
- Cuts a first-pass left-edge service-pad access opening and local fixture
  clearance for the routed `J1` service interface
- Adds a non-USB board-retention concept with molded rails, side stop lugs, and
  service-lid compression pads
- Adds a `SW1` dome pocket and actuator relief as a first pass, not as proven
  click ergonomics
- Uses four flat adhesive glide-pad seats across the battery pod, board pod and ring on the lower
  shell so the focal-distance geometry does not move during service
- Uses a `7.8 mm` shell and `1.7 mm` rim with the `0.5 mm` glide pads exposed
  below the rim; the nominal top-to-installed-pad height is `10.0 mm` before
  print/pad tolerance
- Uses a removable top service lid with a locating skirt and pry relief instead
  of pretending the first board can be glue-sealed
- Reserves service volume for the routed board, battery, sensor cavity, and
  service fixture path so stackup conflicts stay visible before fabrication
- Adds a battery harness channel and service-loop relief so the cell has a
  believable removal path from the actual `J_BAT` service-pad area
- Adds quick-print coupon/export modes for the fit unknowns that should be
  checked before spending time on a full shell print

## What Still Needs Real Validation

- Comfort across actual finger sizes
- Real sensor cavity dimensions for the chosen optical sensor and lens kit
- Measured service-fixture access, board rail/stop tolerance, lid pad
  clearance, dome-click travel, battery harness routing, and lid removal
  against a populated board or representative coupons
- Whether four loose service screws are acceptable for limited-dexterity repair
  or need a captured-hardware follow-up
- Battery fit with real wiring, the chosen `BT1` interface, and real connector
  strain relief
- Focal-distance proof on physical surfaces; the CAD assertion only checks the
  stated first-board stack assumption and is not a lens measurement
- Whether the larger top PCB pod is comfortable, accessible, and still inside
  the current `~$9` ring intent once fasteners and printable wall thickness are
  costed

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../STACKUP-VERIFY.md](../STACKUP-VERIFY.md) as the active-lane source of truth
for what this CAD needs to prove before secondary variants move forward.

## Current Service Story

- Battery service is top-side only: remove the four service screws, lift the
  lid from the pry relief, then lift the cell through the dedicated lateral
  pod opening; the harness stays in the service-side bridge channel
- The board service path is also top-side: once the lid is off, molded rails and
  side stops should allow the PCB to lift out without levering on service pads,
  the antenna end, or the dome switch
- The lower shell owns the rim, glide pads, and sensor tunnel so battery
  service does not ask the user or repair tech to disturb the focal-distance
  features first
- The battery path is still an honest first pass until a real replaceable
  harness or fixture, protected cell, and printed lid prove lead bend radius
  and lift clearance

## Quick-Print Fit Coupons

The OpenSCAD file supports these `export_mode` values.  Its top-level
assertions are a regression check for the current PCB envelope, screw
clearance, KiCad sensor datum, nominal optical gap, and nominal height; they
do not substitute for a printed fit or human comfort check:

| Mode | Artifact | Fit Unknown Targeted |
|------|----------|----------------------|
| `shell` | Full lower shell plus service lid | Whole-packet geometry sanity |
| `service_access_coupon` | Off-board service-pad access pocket | Fixture/pogo access and shell/board loading |
| `board_retention_coupon` | Board rails plus stop lugs | `43 x 18 mm` board slide, lift, and tolerance |
| `lid_pad_coupon` | Board-edge compression-pad channel | Lid pad path without crushing components |
| `battery_harness_coupon` | Harness channel, service-pad access pocket, and service-loop relief | Replaceable harness bend and lift clearance |
| `service_lid_coupon` | Skirt/socket/pry-removal coupon | Reopenable lid handling around service hardware |
| `fit_coupons` | One sheet with all coupons | Fast print for the first fit sweep |
| `collision_probe` | Empty CSG intersection export | PCB/cell/finger/rail/lid mesh collision regression |
| `contact_probe` | Positive CSG contact export | Installed pad exposure and joined battery bridge regression |

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

For the durable local first-board coupon bundle, use:

```bash
scripts/generate-r30-ring-fit-coupons.sh
```

That regenerates coupon STLs, preview PNGs, hashes, and a blank physical-check
worksheet under `build/r30-oled-none-none-mechanical/`. The bundle is local
print/preview scaffolding only; transfer measured observations back into
[../STACKUP-VERIFY.md](../STACKUP-VERIFY.md) and
[../MANIFEST.md](../MANIFEST.md) only after physical checks.

For a source-level geometry regression that runs every export mode and the
actual KiCad PCB DRC, use:

```bash
scripts/verify-r30-enclosure-geometry.sh
```

The command requires `openscad` and `kicad-cli` (or `OPENSCAD_BIN` and
`KICAD_CLI_BIN` overrides). It requires every normal/contact STL to be
non-empty, fails on any OpenSCAD `ERROR:`, and requires the CSG
`collision_probe` STL to be empty. Passing means the CAD assertions, mesh
intersection probe, and PCB DRC pass; it does not claim printed fit, lens
measurement, RF performance, comfort, or wearability.

The lateral split-pod shell is a non-wearable validation fixture for checking
board/cell separation, service routing, lid closure, and optical datum. Do not
put it on a finger or treat its profile as an accessibility result.
