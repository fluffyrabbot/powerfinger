<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Recommended First Capture

## Recommendation

Capture a **single-purpose P0 optical ring schematic first**, not a dual-
footprint Standard/Pro hybrid.

## Why

- `R30-OLED-NONE-NONE` is the active validation gate for the whole project.
- The Standard optical lane already has enough real risk in battery safety,
  sensor placement, antenna keep-out, and serviceable packaging.
- The PMW3360 path brings extra rails, different footprints, and higher-power
  tradeoffs that are valuable later but distract from the first proof.
- A clean P0 schematic is easier to review, easier to fabricate, and less likely
  to drift away from the current firmware and BOM reality.

## Preserve For Later

- Keep net names generic enough that a later branch to dual-footprint work is
  still readable.
- Keep the sensor zone localized so a later PMW3360 or alternate optical branch
  can diverge with minimal board-wide churn.
- Keep spare service notes for alternative sensor sourcing, especially the
  ADNS-2080 backup mentioned in [docs/VENDOR-VERIFICATION.md](../../../docs/VENDOR-VERIFICATION.md).

## Not Part Of This First Capture

- PMW3360 rails or footprints
- Piezo or haptic circuitry
- OCR / camera / laser options
- Ball+Hall support

