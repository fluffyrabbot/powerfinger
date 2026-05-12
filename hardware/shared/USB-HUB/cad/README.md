<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB CAD Packet

This directory contains the first editable enclosure source for the active hub
accessory lane.

`usb_hub_enclosure_blank.scad` is now the first mechanical packet for the
stepped direct-plug board in `../kicad/usb_hub.kicad_pcb`. It models a
reopenable top/bottom shell pair, removable service hatch, shell-clamp bosses,
host-fit reference gauges, and quick-print validation coupons. It still does
not claim printed fit, connector strain, or adjacent-port clearance has been
measured.

## Inputs Exposed Today

- PCB envelope; the KiCad pass now uses a stepped `54 x 26 mm` board with a
  USB-A nose widened enough to carry the SOFNG USB-05 shell-tab pads
- `MH1` / `MH2` shell-clamp hole locations copied from the board source
- service window over the `TP1`-`TP9` row plus the `SW1` pad-actuated
  `BOOT_N` service footprint
- rear ESP32-S3 antenna keep-out shown as a plastic-only reference volume
- host insertion shoulder and adjacent-port reference gauges
- quick-print export modes for the USB-A shoulder / adjacent-port coupon,
  `MH1` / `MH2` clamp alignment gauge, and service-hatch reach gauge
- wall thickness and clearances
- top/bottom shell split, removable hatch, and exploded-view spacing

## What The Model Intentionally Does

- Follows the stepped board outline instead of the older rectangular envelope
- Keeps printed shell material from protruding in front of the USB-A host face
- Uses two serviceable clamp points aligned to `MH1` / `MH2`; the intended
  low-cost hardware path is removable small screws or an equivalent reversible
  fastener, not adhesive, heat-shrink, potting, or brass inserts
- Keeps `EN`, `BOOT_N`, UART, power, ground, and trace-inspection pads reachable
  through a removable hatch
- Makes adjacent-port and insertion/removal constraints visible with reference
  gauges instead of treating them as implied pass conditions
- Provides a `host_fit_coupon` mode for cheaply checking the USB-A nose shoulder
  and wider-body adjacent-port envelope before printing the full shell
- Provides a `clamp_alignment_gauge` mode for checking whether `MH1` / `MH2`
  and the printed clamp path line up with the board or board blank
- Provides a `service_hatch_reach_gauge` mode for checking probe/spudger access
  to the service row and removable hatch outline

## What Still Needs Real Validation

- Print fit against a real board or board blank before claiming the shell is
  fabrication-ready
- Confirm `MH1` / `MH2` clamp hardware carries insertion/removal load without
  cracking the printed bosses or bowing the PCB
- Keep the rear ESP32-S3 antenna zone plastic-only; do not add brass inserts,
  metal labels, screws, or copper-backed decoration near the antenna end
- Confirm the service hatch can be opened with a non-marring tool and that the
  pads remain probeable without pulling on the USB connector
- Verify host-port interference with real adjacent USB-A ports; the CAD gauges
  make the constraint explicit but do not prove clearance
- Print the quick coupons and record whether the USB-A shoulder, adjacent-port
  body width, clamp holes, and service hatch reach pass on actual hardware

## Local Sanity Check

Render the full enclosure syntax/manifold check from the repository root:

```sh
openscad -o /tmp/powerfinger-usb-hub-enclosure.stl hardware/shared/USB-HUB/cad/usb_hub_enclosure_blank.scad
```

Regenerate the quick-print validation coupon set, preview PNGs, hash manifest,
OpenSCAD logs, bundle README, and blank physical-check worksheet with:

```sh
scripts/generate-usb-hub-validation-coupons.sh
```

Use [../FIRST-BOARD-CHECKLIST.md](../FIRST-BOARD-CHECKLIST.md) and
[../CONNECTOR-RETENTION-VERIFY.md](../CONNECTOR-RETENTION-VERIFY.md) as the
active-lane source of truth for what this enclosure needs to prove before
secondary hardware moves forward.
