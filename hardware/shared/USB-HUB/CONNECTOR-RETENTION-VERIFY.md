<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Connector Retention Verification

Fill this document with first-board mechanical evidence for the direct-plug hub.

## Connector Load Path

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Connector insertion/removal load is shared by PCB + enclosure | Required | Clamp gauge STL generated, not physically measured | `usb_hub.kicad_pcb` uses plated connector shell tabs, guide holes, two shell-clamp holes behind the connector, and a stepped nose. `cad/usb_hub_enclosure_blank.scad` aligns bottom bosses, lid clearance holes, compression pads, and a `clamp_alignment_gauge` export mode to `MH1` / `MH2`; print-fit and load observation are still required. |
| Connector strain is not carried solely by solder joints | Required | CAD packet provisioned, not measured | `J1` has plated shell tabs tied to ground plus mechanical guide holes, and the enclosure now has an `MH1` / `MH2` clamp path behind the connector shoulder. First assembled board still needs insertion/removal observation before this can turn green. |
| Normal host insertion does not twist or pry the enclosure open | Required | Host-fit coupon STL generated, not physically measured | The enclosure source clips printed shell material at the USB-A host face and marks the `x=14 mm` shoulder where the wider body begins. Use the `host_fit_coupon` export to check the USB-A nose shoulder against real host-port faces before printing the full shell. |

## Serviceability

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Enclosure can reopen without destroying PCB retention | Required | CAD packet provisioned, not measured | The enclosure source is a top/bottom shell with removable clamp hardware assumptions and a separate service hatch; no heat-shrink, potting, or bonded retention is part of this packet. Printed-cycle validation remains open. |
| Boot/reset access remains usable after enclosure install | Required | Service gauge STL generated, not physically measured | `TP6` / `TP7` expose `EN` and `BOOT_N`; `SW1` is pad-actuated for `BOOT_N`. The `service_hatch_reach_gauge` export gives a cheap print for checking probe and spudger reach before committing to a full enclosure print. |
| Connector style leaves enough clearance for adjacent ports | Required | Host-fit coupon STL generated, not physically measured | The direct-plug assumption forced a stepped `54 x 26 mm` board with a shell-tab-bearing nose. Use the `host_fit_coupon` or `validation_set` export to check the wider-body envelope against real adjacent USB-A ports; do not infer clearance from schematic or CAD alone. |

## Local Print / Preview Bundle

Regenerate the local coupon artifacts from the repository root:

```sh
scripts/generate-usb-hub-validation-coupons.sh
```

Default output: `build/usb-hub-mechanical/`.

For the combined first-board first sweep, regenerate the selected print bundle:

```sh
scripts/generate-first-board-mechanical-packet.sh --first-sweep
```

Selected output: `build/first-board-mechanical-packet/FIRST-SWEEP/`, including
the USB-HUB host-fit coupon, USB-HUB clamp-alignment gauge, matching previews,
OpenSCAD logs, manifest, README, and a blank worksheet.

The command renders printable STLs:

- `usb-hub-host-fit-coupon.stl`
- `usb-hub-clamp-alignment-gauge.stl`
- `usb-hub-service-hatch-reach-gauge.stl`
- `usb-hub-validation-set.stl`

It also writes matching `previews/*.png` files for visual review before
printing, `README.md` for the bundle handoff, `COUPON-MANIFEST.md` with
OpenSCAD version, file sizes, SHA-256 hashes, and render results, plus
`PHYSICAL-CHECK-WORKSHEET.md` as the blank capture sheet for host-fit,
clamp-alignment, service-hatch reach, adjacent-port clearance, and
connector-retention checks. The same run also assembles
`build/usb-hub-mechanical/FIRST-PRINT/` as the concrete first-print
proof-capture packet: all four USB-HUB STLs, matching previews, OpenSCAD logs,
hash manifest, README, and a blank worksheet that covers USB-A shoulder seating,
adjacent-port clearance, `MH1` / `MH2` alignment, service-hatch reach, and
connector-retention load capture.

These are print/preview artifacts only. They do not prove USB-A host seating,
adjacent-port clearance, clamp load sharing, hatch reach, or connector strain.
Those rows must stay non-green until physical coupon prints are checked against
real hosts, a board or board blank, and the intended service tools.

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If any row is red, do not start secondary hub or accessory hardware
