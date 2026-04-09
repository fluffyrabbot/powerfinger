<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Recommended First Capture

## Recommendation

Capture the first hub board as a **USB-A dongle-oriented design**, while
keeping a documented fallback path to a USB-C receptacle plus short cable if
connector mechanics or host clearance prove worse than expected.

## Why

- The current active BOM and prototype spec already point toward a small USB-A
  dongle as the cheapest P0 path.
- It keeps the first hub board aligned with the published `$5-6` target.
- It removes USB-C sink-attach circuitry from the first hub board and lets the
  native USB data path stay visually simple in the schematic.

## Fallback Trigger

Pivot the first physical board to a USB-C receptacle plus short cable if any of
the following become credible during capture or enclosure work:

- connector solder joints are carrying too much strain
- host-port clearance becomes a real accessibility problem
- the enclosure cannot reopen cleanly around the plug geometry

## Must-Haves Even In The Simple Path

- native USB direct to the ESP32-S3
- serviceable boot/reset access
- USB-side protection consistent with the regulatory pre-scan
- connector reinforcement by both PCB and enclosure, not solder alone

