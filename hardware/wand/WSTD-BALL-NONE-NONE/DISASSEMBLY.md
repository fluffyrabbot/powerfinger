<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# WSTD-BALL-NONE-NONE Disassembly Baseline

This teardown path defines the minimum replaceability standard for the wand.

## Safety First

- Disconnect USB power before opening the wand.
- Remove the battery before high-heat rework near the charge path.
- Do not use the tube body as a pry bar against the pouch cell.

## Intended Removal Order

1. Open the rear service path and remove the battery first.
2. Remove the tip module as a discrete assembly.
3. Remove the ball and roller components for cleaning or replacement.
4. Remove the barrel switch without cutting the body tube.
5. Remove the electronics carrier or PCB after the mechanical orientation is documented.

## Replacement Expectations

- Tip wear should not require replacing the entire wand body.
- The battery should be replaceable independently of the tip module.
- A failed barrel switch should be a routine repair, not a body write-off.

## Failure Conditions

If teardown requires cutting the tube, forcing bonded end caps, or destroying
the tip module to replace the battery or switch, the design fails this baseline.

