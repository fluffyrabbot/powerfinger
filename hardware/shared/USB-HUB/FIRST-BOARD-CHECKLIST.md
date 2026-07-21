<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB First Board Checklist

This checklist is the execution contract for the active hub hardware lane.

## PCB Capture And Layout

- [x] Lock the first-board USB / power path parts from
  `kicad/P0-COMPONENT-LOCKS.md`
- [x] Resolve `R1` for native USB as `R1A` / `R1B`, 22R 0402 parts placed at
  the ESP32-S3 side of the pair; `C5` / `C6` remain no-BOM mechanical DNI
  placeholders in this routing pass rather than connected shunt capacitors
- [x] Create `kicad/usb_hub.kicad_pcb` as a first routed placement pass
- [x] Represent native USB routing, connector-side ESD placement, and local
  3.3 V regulation in the PCB source
- [x] Keep boot/recovery and UART service pads reachable along the reopenable
  body edge
- [x] Run KiCad CLI DRC/ERC once KiCad is available locally
- [x] Clear KiCad ERC and PCB DRC failures before fabrication release review:
  the latest KiCad CLI run passes ERC and PCB DRC at error severity after the
  connector escape, service-channel nets, common ground, support-part schematic
  parity cleanup, project-local footprint provenance cleanup, project-local
  symbol library cleanup, endpoint-grid cleanup, no-BOM service-pad modeling,
  and power-flag removal. The follow-up all-severity schematic ERC and PCB DRC
  passes have 0 violations, 0 unconnected items, and 0 schematic parity issues.
- [x] Generate and review local USB-HUB fabrication outputs from the DRC-clean
  KiCad source: `kicad/FABRICATION-OUTPUTS.md` records Gerber, split drill, POS,
  active-BOM/POS review, and archive hashes; the current active-BOM/POS review
  has no open rows.
- [x] Add board-house output-constraints questions for the generated USB-HUB
  fabrication outputs so file-set, layer, drill/slot, solder-mask, paste,
  panelization, module, connector-retention DFM, and source-return questions
  are explicit instead of handled through ad hoc manual notes.
- [ ] Record prototype-house / Seeed response to
  `kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md`; regenerate and re-hash
  `kicad/FABRICATION-OUTPUTS.md` if their intake requires different Gerber,
  drill, POS, BOM, stackup, mask, paste, panelization, or source-return output.

## Enclosure Closure

- [x] Turn `cad/usb_hub_enclosure_blank.scad` into a first reopenable
  mechanical packet around the stepped `54 x 26 mm` first-board outline
- [x] Provide deliberate connector reinforcement in the PCB source: plated
  shell tabs, guide holes, and shell-clamp holes are present before the CAD
  shell claims retention
- [x] Carry `MH1` / `MH2` shell-clamp geometry into the CAD source as aligned
  bottom bosses, lid clearance holes, and clamp pads
- [x] Preserve installed service access in CAD with a removable hatch over the
  `TP1`-`TP9` row and `SW1`
- [x] Make adjacent-port and insertion/removal constraints explicit in CAD
  with host-face, shoulder, and neighboring-port reference gauges
- [x] Add quick-print CAD export modes for the USB-A shoulder / adjacent-port
  host-fit coupon, `MH1` / `MH2` clamp alignment gauge, and service-hatch reach
  gauge
- [ ] Print and check the `host_fit_coupon` against real host ports for USB-A
  shoulder seating and wider-body adjacent-port clearance
- [ ] Print and check the `clamp_alignment_gauge` against a board or board
  blank before relying on `MH1` / `MH2` clamp hardware
- [ ] Print and check the `service_hatch_reach_gauge` against probe/spudger
  access to the service row and hatch notch
- [ ] Confirm the printed enclosure retention does not block normal host
  insertion/removal
- [ ] Confirm the wider body behind the narrow USB-A nose clears adjacent ports
  on real hosts

## Bring-Up Evidence Required Before Secondary Variants

- [ ] Complete [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md)
- [ ] Link assembled board / enclosure observations from `MANIFEST.md`
- [x] Keep the packet honest that this is a board-source pass, not measured
  mechanical evidence
- [x] Add a durable local command for OpenSCAD quick-print validation coupon
  and preview-bundle regeneration:
  `scripts/generate-usb-hub-validation-coupons.sh`
- [x] Have that command assemble `build/usb-hub-mechanical/FIRST-PRINT/` as the
  concrete USB-HUB first-print proof-capture packet for host-fit,
  adjacent-port clearance, clamp alignment, service-hatch reach, and
  connector-retention capture, with blank result fields until real checks run
- [x] Expose the USB-HUB host-fit and clamp-alignment coupons in the combined
  first-sweep packet:
  `scripts/generate-first-board-mechanical-packet.sh --first-sweep`
- [x] Add a source-controlled USB-HUB coupon-result ingest path:
  `hardware/shared/USB-HUB/COUPON-RESULTS.md` plus
  `scripts/ingest-usb-hub-coupon-results.py`, so filled generated worksheets
  can be merged into the checked-in hub evidence lane without ad hoc paste-back
- [ ] Ingest real USB-HUB first-sweep coupon observations into
  `COUPON-RESULTS.md` before changing connector-retention, manifest, or
  checklist closure state from generated worksheet rows
- [x] Teach the Shenzhen/Seeed quote export to regenerate and include the
  USB-HUB fabrication-review bundle by default so the send-now quote path
  includes source files plus local Gerber/drill/POS/BOM review artifacts.
