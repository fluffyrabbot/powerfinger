<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB First Board Checklist

This checklist is the execution contract for the active hub hardware lane.

## PCB Capture And Layout

- Lock the first-board USB / power path parts from `kicad/P0-COMPONENT-LOCKS.md`
- Complete `kicad/usb_hub.kicad_pcb`
- Validate native USB routing, ESD placement, and 3.3 V regulation against the
  packet constraints
- Keep boot/reset access and service pads reachable after enclosure assembly

## Enclosure Closure

- Turn `cad/usb_hub_enclosure_blank.scad` into a printable reopenable enclosure
- Provide deliberate connector reinforcement in the PCB + shell system
- Confirm enclosure retention does not block normal host insertion/removal

## Bring-Up Evidence Required Before Secondary Variants

- Complete [CONNECTOR-RETENTION-VERIFY.md](CONNECTOR-RETENTION-VERIFY.md)
- Link assembled board / enclosure observations from `MANIFEST.md`
- Keep the packet honest if any item remains provisional or fails
