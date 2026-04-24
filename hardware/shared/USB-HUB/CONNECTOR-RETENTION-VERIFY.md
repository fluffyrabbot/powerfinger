<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Connector Retention Verification

Fill this document with first-board mechanical evidence for the direct-plug hub.

## Connector Load Path

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Connector insertion/removal load is shared by PCB + enclosure | Required | Board-source provisioned, not measured | `usb_hub.kicad_pcb` uses plated connector shell tabs, guide holes, two shell-clamp holes behind the connector, and a stepped nose. The CAD shell still needs clamp geometry around those holes. |
| Connector strain is not carried solely by solder joints | Required | Board-source provisioned, not measured | `J1` has plated shell tabs tied to ground plus mechanical guide holes. First assembled board still needs insertion/removal observation before this can turn green. |
| Normal host insertion does not twist or pry the enclosure open | Required | Open | The current board uses a widened USB-A nose to carry the SOFNG USB-05 shell-tab pads and a wider body starting behind the host-side clearance step. Verify on adjacent-port hosts before secondary variants. |

## Serviceability

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Enclosure can reopen without destroying PCB retention | Required | Open | Board now exposes shell-clamp holes and service pads, but `cad/usb_hub_enclosure_blank.scad` has not yet been fit around the stepped outline. |
| Boot/reset access remains usable after enclosure install | Required | Board-source provisioned, not measured | `TP6` / `TP7` expose `EN` and `BOOT_N`; `SW1` is pad-actuated for `BOOT_N`. The enclosure needs a recessed actuator or probe window that cannot be bumped during normal use. |
| Connector style leaves enough clearance for adjacent ports | Required | Open | The direct-plug assumption forced a stepped `54 x 26 mm` board with a shell-tab-bearing nose. Real host-port spacing must be checked; do not infer adjacent-port clearance from the schematic. |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If any row is red, do not start secondary hub or accessory hardware
