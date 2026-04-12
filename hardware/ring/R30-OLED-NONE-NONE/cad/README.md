<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE CAD Skeleton

This directory contains the first editable mechanical source for the active
optical ring lane.

`r30_oled_none_none_shell_blank.scad` is a parametric OpenSCAD shell blank. It
is not a production-ready enclosure and it does not claim that the fit, comfort,
or focal-distance stack are already validated.

## Inputs Exposed Today

- `finger_circumference_mm`
- `sensor_angle_deg`
- `band_width_mm`
- `shell_height_mm`
- `rim_height_mm`
- `glide_pad_thickness_mm`
- battery and module keep-out placeholders

## What The Model Intentionally Does

- Gives the ring lane a real editable CAD source file in the repo
- Encodes the 30-degree optical-lane posture as a default, not a hard-coded
  forever truth
- Marks battery and module envelopes so serviceability and stackup are visible
- Cuts an angled sensor aperture and four glide-pad pockets

## What Still Needs Real Validation

- Comfort across actual finger sizes
- Real sensor cavity dimensions for the chosen optical sensor and lens kit
- Final shell seam geometry
- Battery fit with real wiring and connector strain relief
- Focal-distance proof on physical surfaces

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../STACKUP-VERIFY.md](../STACKUP-VERIFY.md) as the active-lane source of truth
for what this CAD needs to prove before secondary variants move forward.
