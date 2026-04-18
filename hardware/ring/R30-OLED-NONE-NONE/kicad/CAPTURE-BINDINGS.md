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
- `Dropped`: reference exists for historical and defensive-publication reasons,
  but is intentionally not part of the first routed board and does not consume
  copper, footprint area, or BOM cost.

## Active Ring Binding Plan

| Refs | Binding status | Capture representation | Notes |
|------|----------------|------------------------|-------|
| `U1` | Vendor-specific | Exact `ESP32-C3-MINI-1` module symbol/footprint pair | Use the module geometry and antenna keep-out from the Espressif module docs, not a generic RF module outline |
| `U2` | Vendor-specific | Exact `PAW3204DB-TJ3L` symbol and package footprint | Sensor aperture alignment matters more than library convenience |
| `LED1` | Dropped | Not part of first capture | PAW3204 is sourced as a sensor+lens+emitter kit, so the illumination path ships with `U2`/`LENS1`. Reinstate as a discrete LED only if a bare-sensor supply path becomes the only option |
| `LENS1` | Custom / mechanical | Mechanical note, alignment datum, or footprint courtyard companion only | Lens/clip alignment is load-bearing even though it is not an electrical symbol |
| `SW1` | Custom / mechanical | Dome contact geometry and actuation area, not a generic tact switch footprint | The dome path is a service/ME problem as much as an EE one |
| `BT1` | Vendor-specific | Harness-terminated 80-100 mAh protected LiPo pack with JST-SH 2-pin 1.0 mm mating plug | Pack terminates in the mating plug for `J_BAT`; direct cell-tab soldering is rejected because it defeats the reopenable-seam replaceability rule |
| `J_BAT` | Vendor-specific | JST-SH 2-pin 1.0 mm SMD right-angle receptacle footprint | Internal battery harness connector; not user-visible, so retention posts are not required. Any JST-SH 2-pin 1.0 mm equivalent receptacle is acceptable; specific MPN lock pending verified LCSC research pass |
| `U3` | Stock package | SOT-23-5 charger symbol + footprint | Exact TP4054 pinout matters; do not substitute MCP73831 without changing layout |
| `U4` | Stock package | SOT-23-5 LDO symbol + footprint | Keep the low-Iq part choice explicit |
| `J1` | Vendor-specific | 16-pin USB-C receptacle with through-hole retention posts, exact symbol/footprint pair | Service/debug USB path is baseline; ring shell will see repeated cable-pull stress, so through-hole mechanical retention posts are required. SMD-only receptacles and 6-pin power-only receptacles are rejected. Specific MPN lock pending verified LCSC research pass |
| `C1`, `C2` | Stock package | Standard MLCC symbols with 0402/0603 footprints as committed in capture | Only move `C2` to 0603 if assembly or voltage-rating constraints require it |
| `R1`, `R2`, `R3`, `R4`, `R5` | Stock package | Standard resistor symbols with 0402 footprints | Keep the USB-C Rd pair and the SPI damping resistor visible as separate functions |
| `NTC1` | Stock package | NTC thermistor symbol with 0402 footprint | Placement must follow the cell-temperature rule, not routing convenience |
| `Q1` | Stock package | SOT-23 P-channel MOSFET symbol + footprint | Safe-default gate bias is part of the electrical contract |
| `D1` | Stock package | 2-channel USB ESD array in SOT-23-6 or equivalent footprint | Lock the exact footprint when the device MPN is chosen |
| `PCB1`, `SHELL1`, `RIM1`, `PAD1`, `ANT1` | Custom / mechanical | Notes, keep-outs, and fabrication constraints rather than electrical symbols | These drive the board shape, service seam, glide system, and antenna clearance |

## Blocking Locks Before First Routed Board

- exact `J1` USB-C connector MPN and land pattern. Retention class resolved
  (16-pin SMD with through-hole retention posts); specific MPN lock pending
  verified LCSC research pass.
- exact PAW3204 lens/clip sourcing path. `LED1` resolved — dropped from first
  capture because `U2`/`LENS1` source path bundles the emitter. Reinstate
  discrete `LED1` only if a bare-sensor supply path becomes the only option.
- battery replacement interface for `BT1`. Resolved — JST-SH 2-pin 1.0 mm pad
  on the PCB (`J_BAT`) with a harness-terminated protected pack.
- first-board flex-migration strategy. Resolved — P0 stays rigid; the
  zone-boundary rule in `PLACEMENT-CONSTRAINTS.md` preserves a later flex or
  rigid-flex respin without re-choosing footprints.
