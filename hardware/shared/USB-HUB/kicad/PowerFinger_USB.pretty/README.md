<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# PowerFinger USB Footprints

This directory is reserved for local first-board footprints that cannot
honestly be represented by an untracked upstream KiCad library cache.

Current local footprints:

- `USB_A_Plug_SOFNG_USB-05.kicad_mod` for the locked direct-plug hub connector
- `ESP32-S3-MINI-1-N8_FirstBoard.kicad_mod` for the first board's module
  placement and antenna keep-out
- `PadActuated_Boot_Service.kicad_mod` for the recessed/probe-actuated
  `BOOT_N` service short
- `SOT-23-5_FirstBoard.kicad_mod` for the locked `RT9080-33GJ5` regulator
- `SOT-23-6_FirstBoard.kicad_mod` for the locked `USBLC6-2SC6` ESD device
- `R_0402_1005Metric_FirstBoard.kicad_mod` for the native-USB/support
  0402 resistors
- `C_0402_1005Metric_FirstBoard.kicad_mod` and
  `C_0603_1608Metric_FirstBoard.kicad_mod` for commodity MLCC placements
- `LED_0402_1005Metric_FirstBoard.kicad_mod` for the commodity status LED
- `TestPoint_Pad_D1.0mm_FirstBoard.kicad_mod` for probeable service pads
- `MountingHole_1.4mm_Clamp.kicad_mod` for the no-BOM shell-clamp holes

The USB-A footprint keeps the `J1` schematic property anchored to the intended
male direct-plug connector, including plated shell tabs and guide holes. The S3
module footprint keeps the first board independent of an external library while
preserving the native USB pins and the no-copper antenna zone needed by the
stepped dongle layout.

Commodity footprints keep package provenance local without pretending the final
assembler part number is locked before fab review. The locked vendor parts
carry their manufacturer/LCSC source in the footprint description or tags.
