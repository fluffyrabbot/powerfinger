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

## Intended Assembly Order

1. Populate the PCB, paying special attention to USB data routing and connector
   reinforcement features.
2. Install the status LED and boot/reset control so they remain accessible for
   service.
3. Fit the PCB into the enclosure or sleeve using a reversible retention method.
4. Capture the exact connector style used so future replacements match the shell.

## Assembly Record To Capture

- Connector type and supplier
- Enclosure material and retention method
- Whether the build uses USB-A plug geometry or USB-C plus cable
- Any reinforcement used around the connector

