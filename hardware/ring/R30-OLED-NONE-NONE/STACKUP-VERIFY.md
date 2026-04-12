<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Stackup Verification

Fill this document with measured evidence from the first active-lane hardware
drop. Do not replace measured values with estimates.

## Geometry

| Check | Target | Measured | Status | Notes |
|-------|--------|----------|--------|-------|
| Finger-to-surface height | `~10 mm` max | | | |
| Sensor-to-surface gap | `2.4-3.2 mm` | | | |
| Battery bay closes without compression risk | Required | | | |
| Shell can reopen after first full assembly | Required | | | |

## Optical And Mechanical Proof

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Sensor aperture stays aligned under realistic finger pressure | Required | | |
| Glide pads maintain focal distance without user technique | Required | | |
| Click actuation does not force destructive shell flex | Required | | |

## RF And Safety Proof

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Antenna keep-out preserved in final board + shell stack | Required | | |
| Battery choice still satisfies `docs/BATTERY-SAFETY.md` | Required | | |
| Charging and service access remain non-hermetic and repairable | Required | | |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If any row is red, do not start puck or secondary ring hardware
