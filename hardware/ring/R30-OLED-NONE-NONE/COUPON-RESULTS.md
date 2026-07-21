<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Physical Coupon Results

This is the source-controlled ingress point for real R30 first physical coupon
observations. It closes the handoff from generated local worksheets back to the
checked-in ring evidence lane without treating generated STLs, previews, or
OpenSCAD logs as measured results.

## Ingest Command

Fill only the R30 rows in a generated worksheet after physical checks run, then
ingest them from the repository root:

```sh
scripts/ingest-r30-ring-coupon-results.py build/first-board-mechanical-packet/FIRST-SWEEP/PHYSICAL-CHECK-WORKSHEET.md
```

For the R30-only mechanical bundle, use:

```sh
scripts/ingest-r30-ring-coupon-results.py build/r30-oled-none-none-mechanical/PHYSICAL-CHECK-WORKSHEET.md
```

Accepted generated-worksheet `Result` values are `pass`, `fail`, `partial`,
and `blocked`. Leave the worksheet result blank when a row has not been
physically checked; this checked-in ledger keeps unmeasured rows as `not-run`.
Any non-blank worksheet result must also include the fixture, board, tool, or
coupon print used plus an observation or photo reference.

## Current Source-Controlled Results

| Check ID | Coupon artifact | Physical check | Result | Fixture / board / tool used | Observation or photo reference | Source worksheet |
|---|---|---|---|---|---|---|
| `R30-SERVICE-ACCESS` | `r30-oled-none-none-service-access-coupon.stl` | Service-access coupon lets the J1 service fixture/pogo path reach pads without binding, levering the board edge, or scraping the shell opening | not-run |  |  |  |
| `R30-BOARD-RETENTION` | `r30-oled-none-none-board-retention-coupon.stl` | `43 x 18 mm` board or blank slides onto rails, stops repeatably, and lifts out by hand | not-run |  |  |  |
| `R30-LID-PAD-CLEARANCE` | `r30-oled-none-none-lid-pad-coupon.stl` | Lid pads contact only intended board-edge keep-out zones and do not trap components | not-run |  |  |  |
| `R30-BATTERY-HARNESS` | `r30-oled-none-none-battery-harness-coupon.stl` | Off-board `J_BAT` harness and service loop clear the seam during protected-cell lift-out | not-run |  |  |  |
| `R30-SERVICE-LID-REMOVAL` | `r30-oled-none-none-service-lid-coupon.stl` | Lid/skirt/pry path reopens without destructive flex, tiny-tool dependence, or loose-hardware handling that blocks accessibility | not-run |  |  |  |
| `R30-COMBINED-FIT-SHEET` | `r30-oled-none-none-fit-coupons.stl` | Combined coupon print exposes no obvious generation-time geometry mismatch before individual physical checks | not-run |  |  |  |

## Downstream Truth Surfaces

| Check ID | Evidence surface | Closure rule |
|---|---|---|
| `R30-SERVICE-ACCESS` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real service fixture, board or board blank, and observation reference are ingested. |
| `R30-BOARD-RETENTION` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real board or representative board blank is checked against the printed rails and stop lugs. |
| `R30-LID-PAD-CLEARANCE` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until lid-pad contact is checked against the intended board-edge keep-out zones. |
| `R30-BATTERY-HARNESS` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the off-board battery harness and service loop are checked during cell lift-out. |
| `R30-SERVICE-LID-REMOVAL` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the intended lid, tool, and removal path are physically checked for reopenability and accessibility. |
| `R30-COMBINED-FIT-SHEET` | `STACKUP-VERIFY.md`, `MANIFEST.md` | Use only as print-sanity context; do not close individual stackup rows from the combined coupon sheet unless the individual checks above are also observed. |

Do not remove a missing-artifact row from `MANIFEST.md` or check off a physical
proof row in `FIRST-BOARD-CHECKLIST.md` from generated artifacts alone.
