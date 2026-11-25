<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30 electrical and layout verification

The retained four-layer rigid board passes KiCad 10.0.6 ERC and DRC with all reported severities, zero unconnected pads and zero schematic-parity issues. Ground zones were refilled before export. Regenerate the two reports with `scripts/verify-firmware-local.sh --kicad-only`. Earlier route development and its violation counts are preserved in repository history.

The optical circuit, module land pattern and reset network, dome contacts and USB-presence buffer are present in the current schematic and PCB. The manufacturer's exact optical carrier geometry, assembled height, cell temperature protection in the fixture, USB signal integrity and RF operation still require review or physical measurement. The R30 remains a DFM annex. [Capture bindings](CAPTURE-BINDINGS.md) · [First-board checklist](../FIRST-BOARD-CHECKLIST.md)
