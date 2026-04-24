<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB First Board Checklist

This checklist is the execution contract for the active hub hardware lane.

## PCB Capture And Layout

- [x] Lock the first-board USB / power path parts from
  `kicad/P0-COMPONENT-LOCKS.md`
- [x] Resolve `R1` for native USB as `R1A` / `R1B`, 22R 0402 parts placed at
  the ESP32-S3 side of the pair; `C5` / `C6` remain no-BOM mechanical DNI
  placeholders in this DRC-clean pass rather than connected shunt capacitors
- [x] Create `kicad/usb_hub.kicad_pcb` as a first routed placement pass
- [x] Represent native USB routing, connector-side ESD placement, and local
  3.3 V regulation in the PCB source
- [x] Keep boot/recovery and UART service pads reachable along the reopenable
  body edge
- [x] Run KiCad CLI DRC/ERC once KiCad is available locally
- [x] Clear KiCad ERC and PCB DRC failures before fabrication release review

## Enclosure Closure

- [ ] Turn `cad/usb_hub_enclosure_blank.scad` into a printable reopenable
  enclosure around the stepped `54 x 26 mm` first-board outline
- [x] Provide deliberate connector reinforcement in the PCB source: plated
  shell tabs, guide holes, and shell-clamp holes are present before the CAD
  shell claims retention
- [ ] Confirm enclosure retention does not block normal host insertion/removal
- [ ] Confirm the wider body behind the narrow USB-A nose clears adjacent ports

## Bring-Up Evidence Required Before Secondary Variants

- [ ] Complete [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md)
- [ ] Link assembled board / enclosure observations from `MANIFEST.md`
- [x] Keep the packet honest that this is a board-source pass, not measured
  mechanical evidence
