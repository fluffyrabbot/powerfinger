<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Connector Retention Verification

Fill this document with first-board mechanical evidence for the direct-plug hub.

## Connector Load Path

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Connector insertion/removal load is shared by PCB + enclosure | Required | Quick-print clamp gauge available, not measured | `usb_hub.kicad_pcb` uses plated connector shell tabs, guide holes, two shell-clamp holes behind the connector, and a stepped nose. `cad/usb_hub_enclosure_blank.scad` aligns bottom bosses, lid clearance holes, compression pads, and a `clamp_alignment_gauge` export mode to `MH1` / `MH2`; print-fit and load observation are still required. |
| Connector strain is not carried solely by solder joints | Required | CAD packet provisioned, not measured | `J1` has plated shell tabs tied to ground plus mechanical guide holes, and the enclosure now has an `MH1` / `MH2` clamp path behind the connector shoulder. First assembled board still needs insertion/removal observation before this can turn green. |
| Normal host insertion does not twist or pry the enclosure open | Required | Quick-print host coupon available, not measured | The enclosure source clips printed shell material at the USB-A host face and marks the `x=14 mm` shoulder where the wider body begins. Use the `host_fit_coupon` export to check the USB-A nose shoulder against real host-port faces before printing the full shell. |

## Serviceability

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Enclosure can reopen without destroying PCB retention | Required | CAD packet provisioned, not measured | The enclosure source is a top/bottom shell with removable clamp hardware assumptions and a separate service hatch; no heat-shrink, potting, or bonded retention is part of this packet. Printed-cycle validation remains open. |
| Boot/reset access remains usable after enclosure install | Required | Quick-print service gauge available, not measured | `TP6` / `TP7` expose `EN` and `BOOT_N`; `SW1` is pad-actuated for `BOOT_N`. The `service_hatch_reach_gauge` export gives a cheap print for checking probe and spudger reach before committing to a full enclosure print. |
| Connector style leaves enough clearance for adjacent ports | Required | Quick-print host coupon available, not measured | The direct-plug assumption forced a stepped `54 x 26 mm` board with a shell-tab-bearing nose. Use the `host_fit_coupon` or `validation_set` export to check the wider-body envelope against real adjacent USB-A ports; do not infer clearance from schematic or CAD alone. |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If any row is red, do not start secondary hub or accessory hardware
