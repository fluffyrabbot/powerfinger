<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Assembly Baseline

This is the minimum non-destructive assembly order for the first hub hardware
drop.

## Required Tools

- ESD-safe tweezers
- Fine-tip soldering iron and hot-air rework station
- Flush cutters
- Calipers
- Non-marring spudger

## Critical Build Constraints

- USB connector must have mechanical support beyond solder pads alone.
- Enclosure must reopen for switch, LED, and connector repair.
- ESP32-S3 antenna keep-out must remain unobstructed by metal shells or hardware.
- The stepped direct-plug board must be clamped through `MH1` / `MH2`; do not
  substitute adhesive, heat-shrink, or potting for connector retention.
- Service pads and the `SW1` pad-actuated `BOOT_N` footprint must remain
  reachable through the service hatch after the lid is installed.

## Intended Assembly Order

1. Populate the PCB, paying special attention to USB data routing and connector
   reinforcement features.
2. Install the status LED and boot/reset control so they remain accessible for
   service.
3. Fit the stepped PCB into the bottom shell with the USB-A nose at the host
   face and the wider body seated behind the `x=14 mm` shoulder.
4. Align the enclosure clamp hardware through `MH1` / `MH2`; tighten only
   enough to prevent connector rocking without bowing the PCB.
5. Install the lid and confirm the removable service hatch exposes `TP1`-`TP9`
   plus `SW1`.
6. Capture the exact connector style used so future replacements match the shell.

## Assembly Record To Capture

- Connector type and supplier
- Enclosure material and retention method
- Clamp hardware or printed-pin choice used at `MH1` / `MH2`
- Adjacent-port host fit observations with the stepped USB-A direct-plug body
- Any reinforcement used around the connector beyond the packeted shell geometry
