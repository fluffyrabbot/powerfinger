<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Capture Bindings

This file turns the active optical ring BOM into capture-ready binding guidance
for the first KiCad schematic and PCB pass.

It is intentionally not a library-name dump. The goal is to make the required
symbol and footprint decisions explicit before capture starts, while staying
honest about which parts still need an exact MPN lock in the same commit as the
land pattern.

## Binding Status Meanings

- `Stock package`: a normal package footprint and generic symbol are acceptable.
- `Vendor-specific`: use the vendor module or connector geometry, not a generic
  placeholder body.
- `Custom / mechanical`: requires custom copper, keep-outs, or non-electrical
  alignment notes.
- `Intentionally open`: function is required, but the exact physical interface
  still needs to be locked during capture.

## Active Ring Binding Plan

| Refs | Binding status | Capture representation | Notes |
|------|----------------|------------------------|-------|
| `U1` | Vendor-specific | Exact `ESP32-C3-MINI-1` module symbol/footprint pair | Use the module geometry and antenna keep-out from the Espressif module docs, not a generic RF module outline |
| `U2` | Vendor-specific | Exact `PAW3204DB-TJ3L` symbol and package footprint | Sensor aperture alignment matters more than library convenience |
| `LED1` | Intentionally open | Discrete illumination LED only if the sourced PAW3204 kit actually needs one | Do not force a standalone LED into the schematic if the sourced sensor/lens kit already includes the emitter path |
| `LENS1` | Custom / mechanical | Mechanical note, alignment datum, or footprint courtyard companion only | Lens/clip alignment is load-bearing even though it is not an electrical symbol |
| `SW1` | Custom / mechanical | Dome contact geometry and actuation area, not a generic tact switch footprint | The dome path is a service/ME problem as much as an EE one |
| `BT1` | Intentionally open | Removable battery interface plus pack note | Do not hard-code direct cell-tab soldering if it defeats the replaceability rule |
| `U3` | Stock package | SOT-23-5 charger symbol + footprint | Exact TP4054 pinout matters; do not substitute MCP73831 without changing layout |
| `U4` | Stock package | SOT-23-5 LDO symbol + footprint | Keep the low-Iq part choice explicit |
| `J1` | Vendor-specific | Exact 16-pin USB-C receptacle symbol/footprint pair | Lock the connector MPN and footprint in the same commit; generic USB-C placeholders are not enough |
| `C1`, `C2` | Stock package | Standard MLCC symbols with 0402/0603 footprints as committed in capture | Only move `C2` to 0603 if assembly or voltage-rating constraints require it |
| `R1`, `R2`, `R3`, `R4`, `R5` | Stock package | Standard resistor symbols with 0402 footprints | Keep the USB-C Rd pair and the SPI damping resistor visible as separate functions |
| `NTC1` | Stock package | NTC thermistor symbol with 0402 footprint | Placement must follow the cell-temperature rule, not routing convenience |
| `Q1` | Stock package | SOT-23 P-channel MOSFET symbol + footprint | Safe-default gate bias is part of the electrical contract |
| `D1` | Stock package | 2-channel USB ESD array in SOT-23-6 or equivalent footprint | Lock the exact footprint when the device MPN is chosen |
| `PCB1`, `SHELL1`, `RIM1`, `PAD1`, `ANT1` | Custom / mechanical | Notes, keep-outs, and fabrication constraints rather than electrical symbols | These drive the board shape, service seam, glide system, and antenna clearance |

## Blocking Locks Before First Routed Board

- exact `J1` USB-C connector MPN and land pattern
- exact PAW3204 lens/clip sourcing path, including whether `LED1` remains a
  discrete part
- battery replacement interface choice for `BT1`
- whether the first board is rigid lash-up only or already trying to preserve
  a flex/rigid-flex migration path in the footprint geometry
