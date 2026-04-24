<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB CAD Skeleton

This directory contains the first editable enclosure source for the active hub
accessory lane.

`usb_hub_enclosure_blank.scad` is a serviceable enclosure blank. It creates a
simple top/bottom shell pair around a board envelope and a USB opening. It does
not claim the final fit, connector style, or strain-relief geometry is done.

## Inputs Exposed Today

- PCB envelope; the KiCad pass now uses a stepped `54 x 26 mm` board with a
  USB-A nose widened enough to carry the SOFNG USB-05 shell-tab pads
- wall thickness and clearances
- connector opening size
- top/bottom shell split and exploded-view spacing

## What The Model Intentionally Does

- Establish an editable source file for the hub enclosure
- Keep the enclosure reopenable instead of assuming heat-shrink or potting
- Reserve explicit space for connector support and module height

## What Still Needs Real Validation

- Carry `MH1` / `MH2` shell-clamp holes from `../kicad/usb_hub.kicad_pcb` into
  the printed shell so connector strain does not rely only on solder
- Keep the rear ESP32-S3 antenna zone plastic-only; do not add brass inserts,
  metal labels, screws, or copper-backed decoration near the antenna end
- Preserve a service seam or removable hatch over the pad row for `EN`,
  `BOOT_N`, UART0, USB, power, and ground access
- Verify host-port interference with the narrow USB-A nose and wider body

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../CONNECTOR-RETENTION-VERIFY.md](../CONNECTOR-RETENTION-VERIFY.md) as the
active-lane source of truth for what this enclosure needs to prove before
secondary hardware moves forward.
