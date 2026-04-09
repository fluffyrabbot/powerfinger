<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# PowerFinger USB Footprint Stub

This directory is reserved for local connector footprints that cannot honestly
be represented by a generic upstream KiCad library part.

Current planned local footprint:

- `USB_A_Plug_SOFNG_USB-05.kicad_mod` for the locked direct-plug hub connector

This footprint file does not exist yet. The point of creating the library
directory now is to keep the `J1` schematic property anchored to the intended
local library name while the next capture step turns the manufacturer PCB layout
into a real footprint and courtyard.
