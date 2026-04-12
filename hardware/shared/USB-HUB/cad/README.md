<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB CAD Skeleton

This directory contains the first editable enclosure source for the active hub
accessory lane.

`usb_hub_enclosure_blank.scad` is a serviceable enclosure blank. It creates a
simple top/bottom shell pair around a board envelope and a USB opening. It does
not claim the final fit, connector style, or strain-relief geometry is done.

## Inputs Exposed Today

- PCB envelope
- wall thickness and clearances
- connector opening size
- top/bottom shell split and exploded-view spacing

## What The Model Intentionally Does

- Establish an editable source file for the hub enclosure
- Keep the enclosure reopenable instead of assuming heat-shrink or potting
- Reserve explicit space for connector support and module height

## What Still Needs Real Validation

- Exact connector footprint and board origin
- Antenna clearance once the PCB is actually placed
- Final retention details for top/bottom shells
- Host-port interference if USB-A plug geometry is used

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../CONNECTOR-RETENTION-VERIFY.md](../CONNECTOR-RETENTION-VERIFY.md) as the
active-lane source of truth for what this enclosure needs to prove before
secondary hardware moves forward.
