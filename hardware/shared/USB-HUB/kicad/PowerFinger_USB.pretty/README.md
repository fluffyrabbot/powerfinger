<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# PowerFinger USB Footprints

This directory is reserved for local connector footprints that cannot honestly
be represented by a generic upstream KiCad library part.

Current local footprints:

- `USB_A_Plug_SOFNG_USB-05.kicad_mod` for the locked direct-plug hub connector
- `ESP32-S3-MINI-1-N8_FirstBoard.kicad_mod` for the first board's module
  placement and antenna keep-out
- `PadActuated_Boot_Service.kicad_mod` for the recessed/probe-actuated
  `BOOT_N` service short

The USB-A footprint keeps the `J1` schematic property anchored to the intended
male direct-plug connector, including plated shell tabs and guide holes. The S3
module footprint keeps the first board independent of an external library while
preserving the native USB pins and the no-copper antenna zone needed by the
stepped dongle layout.
