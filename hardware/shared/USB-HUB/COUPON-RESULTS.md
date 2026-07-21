<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Physical Coupon Results

This is the source-controlled ingress point for real USB-HUB first physical
coupon observations. It closes the handoff from generated local worksheets back
to the checked-in hub evidence lane without treating generated STLs, previews,
or OpenSCAD logs as measured results.

## Ingest Command

Fill only the USB-HUB rows in a generated worksheet after physical checks run,
then ingest them from the repository root:

```sh
scripts/ingest-usb-hub-coupon-results.py build/first-board-mechanical-packet/FIRST-SWEEP/PHYSICAL-CHECK-WORKSHEET.md
```

For the USB-HUB-only first-print packet, use:

```sh
scripts/ingest-usb-hub-coupon-results.py build/usb-hub-mechanical/FIRST-PRINT/PHYSICAL-CHECK-WORKSHEET.md
```

Accepted generated-worksheet `Result` values are `pass`, `fail`, `partial`, and
`blocked`. Leave the worksheet result blank when a row has not been physically
checked; this checked-in ledger keeps unmeasured rows as `not-run`. Any
non-blank worksheet result must also include the fixture, host, board, or tool
used plus an observation or photo reference.

## Current Source-Controlled Results

| Check ID | Coupon artifact | Physical check | Result | Fixture / host / board used | Observation or photo reference | Source worksheet |
|---|---|---|---|---|---|---|
| `HUB-USB-A-SHOULDER` | `usb-hub-host-fit-coupon.stl` | USB-A shoulder seats against real host-port faces without printed body interference | not-run |  |  |  |
| `HUB-ADJACENT-PORT` | `usb-hub-host-fit-coupon.stl` | Wider body clears adjacent USB-A ports on real hosts | not-run |  |  |  |
| `HUB-MH1-MH2-ALIGNMENT` | `usb-hub-clamp-alignment-gauge.stl` | `MH1` / `MH2` clamp holes align against a board or board blank | not-run |  |  |  |
| `HUB-SERVICE-HATCH-REACH` | `usb-hub-service-hatch-reach-gauge.stl` | Probe and service opener reach the service row and hatch notch without pulling on the connector | not-run |  |  |  |
| `HUB-CONNECTOR-LOAD-PATH` | `usb-hub-validation-set.stl` | Clamp/enclosure path captures connector insertion/removal load instead of relying on solder joints alone | not-run |  |  |  |

## Downstream Truth Surfaces

| Check ID | Evidence surface | Closure rule |
|---|---|---|
| `HUB-USB-A-SHOULDER` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real host-port fixture and observation reference are ingested. |
| `HUB-ADJACENT-PORT` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real adjacent-port host or representative port fixture is checked. |
| `HUB-MH1-MH2-ALIGNMENT` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a board or board blank is checked against the printed gauge. |
| `HUB-SERVICE-HATCH-REACH` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the intended probe and opener are checked against a printed coupon. |
| `HUB-CONNECTOR-LOAD-PATH` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until connector insertion/removal load sharing is observed on the printed clamp/enclosure path. |

Do not remove a missing-artifact row from `MANIFEST.md` or check off a physical
proof row in `FIRST-BOARD-CHECKLIST.md` from generated artifacts alone.
