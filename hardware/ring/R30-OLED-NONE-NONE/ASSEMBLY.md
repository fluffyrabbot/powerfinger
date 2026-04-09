<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Assembly Baseline

This is the pre-CAD assembly baseline for the first optical ring hardware drop.
It defines the non-destructive build order the design must support. It is not a
claim that the package is already mechanically validated.

## Required Tools

- ESD-safe tweezers
- Fine-tip soldering iron and hot-air rework station
- Non-marring spudger
- Flush cutters
- Calipers
- Isopropyl alcohol for cleanup and removable-adhesive release

## Allowed Consumables

- Removable battery adhesive tabs or thin double-sided adhesive that can be
  released with IPA and gentle lift
- Threadlocker only on removable mechanical fasteners, not on service seams
- Opaque sealant only where it does not block later sensor or battery service

## Not Allowed

- Potting compound
- Cyanoacrylate or epoxy bridging the primary service seam
- Battery adhesive that requires bending or puncturing the cell to remove
- USB-C connector acting as the sole mechanical retention feature

## Intended Assembly Order

1. Verify the selected LiPo has integrated PCM, documented size, and UN 38.3
   paperwork before shell fit is finalized.
2. Populate the PCB or carrier assembly, leaving the antenna keep-out free of
   shields, screws, carbon fiber, or metal trim.
3. Install the optical sensor and matched lens stack so the optical path is
   mechanically constrained rather than glue-positioned by luck.
4. Mount the dome click so replacement does not require removing the battery.
5. Install the battery using removable adhesive and route leads so cell removal
   does not require de-soldering the USB connector first.
6. Close the shell with a reversible seam: screws, snaps, or another clearly
   re-openable method.
7. Apply replaceable glide pads last so worn pads can be swapped independently
   of the shell and electronics.

## Assembly Record To Capture

- Actual battery dimensions and supplier
- Sensor and lens part numbers used
- Shell size parameters used for the wearer
- Closure method used
- Any adhesive used near the battery, lens, or service seam

