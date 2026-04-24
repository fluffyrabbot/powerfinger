<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Placement Constraints

References:

- [hardware/bom/USB-HUB.csv](../../bom/USB-HUB.csv)
- [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- [docs/REGULATORY-PRESCAN.md](../../../docs/REGULATORY-PRESCAN.md)

## Electrical Blocks To Capture

- `ESP32-S3-MINI-1-N8`
- USB connector and native USB data path
- `RT9080-33GJ5` regulation path with required input/output capacitors
- Decoupling and bulk capacitance
- Status LED and current limit resistor
- Boot/reset control
- Any required ESD or connector-side protection discovered during capture

## Placement Rules

- Place the ESP32-S3 module so its antenna keep-out is not shadowed by the USB
  connector shell or a metal enclosure feature.
- Keep the USB D+/D- path short, direct, and symmetric enough for a small full-
  speed device board.
- Place `R1A` / `R1B` at the ESP32-S3 side of the pair, not at the connector;
  connector-side protection belongs at `J1` / `D1`.
- Keep `VBUS_5V` entry, the LDO, and its output capacitor physically coherent so
  the board does not treat `VREG_3V3` like an abstract off-page supply.
- The connector must have mechanical reinforcement from both PCB footprinting
  and enclosure support; solder joints alone are not enough.
- Boot/reset access must be possible during bring-up without making accidental
  presses likely in normal use.
- Recovery pads for `EN`, `GPIO0`, and UART0 must remain reachable by probes or
  pogo pins after the board is installed.
- Status LED should be visible without forcing a non-serviceable enclosure seam.

## Service Rules

- The enclosure must reopen for connector replacement.
- Connector replacement should not require destroying the status LED or switch.
- Avoid board shapes that trap the PCB behind the connector shell with no
  extraction path.

## Packaging Direction

- The first routed source uses a stepped outline: a USB-A nose widened enough
  for the SOFNG USB-05 shell-tab pads, then a `54 x 26 mm` body once the
  ESP32-S3 module, service pads, and antenna keep-out are real. Treat the older
  BOM envelope as superseded by this board reality for enclosure work.
- Keep the antenna end as plastic-only clearance: no copper pour, no service
  pads, no screw insert, and no metal shell feature inside the keep-out.
- Keep the service-pad row on the reopenable body edge; do not hide it under a
  bonded seam or permanent strain-relief feature.
- Keep USB-A plug versus USB-C-plus-cable as an explicit design choice, not an
  accidental side effect of the first footprint used.
