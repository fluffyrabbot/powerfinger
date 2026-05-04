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
| `J_BAT` | Vendor-specific | `JST_SH_SM02B-SRSS-TB_RA_1x02_P1.00mm` class right-angle SMD footprint | Internal battery harness connector; not user-visible, so retention posts are not required. Exact JST `SM02B-SRSS-TB(LF)(SN)` geometry is the board-pass lock; cheaper equivalents are acceptable only if the mating harness and shell service loop still fit |
| `U3` | Stock package | SOT-23-5 charger symbol + footprint | Exact TP4054 pinout matters; do not substitute MCP73831 without changing layout |
| `U4` | Stock package | SOT-23-5 LDO symbol + footprint | Keep the low-Iq part choice explicit |
| `J1` | Vendor-specific | `USB_C_Receptacle_GCT_USB4215_Class_16P_TH_Stakes` footprint | Service/debug USB path is baseline; ring shell will see repeated cable-pull stress, so four through-hole shell stakes are required. SMD-only receptacles and 6-pin power-only receptacles are rejected. A lower-cost supplier part may replace `USB4215` only if it preserves the same 16-pin USB 2.0 service path, through-hole-stake retention, and shell-opening envelope |
| `C1`, `C2`, `C3` | Stock package | Standard MLCC symbols with 0402/0603 footprints as committed in capture | `C1`/`C3` are DNP schematic placeholders in this hand-routed pass; `C2` is the populated 0603 bulk capacitor. Only move `C2` if assembly or voltage-rating constraints require it |
| `R1`, `R2`, `R3`, `R4`, `R5` | Stock package | Standard resistor symbols with 0402 footprints | Keep the USB-C Rd pair and the SPI damping resistor visible as separate functions |
| `NTC1` | Stock package | NTC thermistor symbol with 0402 footprint | Placement must follow the cell-temperature rule, not routing convenience |
| `Q1` | Stock package | SOT-23 P-channel MOSFET symbol + footprint | High-side VBUS switch before TP4054 `VCC`; safe-default gate bias is part of the electrical contract |
| `Q2`, `R6` | Active BOM safety add | SOT-23 2N7002 low-side gate driver plus 0402 pulldown | Required because pulling the P-channel gate up to `VBUS_5V` and wiring it directly to ESP32-C3 `GPIO10` would expose the GPIO to 5 V. This is the BDFL-accepted packet decision for first rigid P0; BSS138-class parts need footprint/pinout confirmation, and a logic-level load switch is a BDFL substitution |
| `R7`, `R8` | Stock package | 0402 `VBAT_SENSE` divider, packet value `100k` / `100k` | Present in the schematic and PCB as the production sense path to ESP32-C3 `GPIO0`; always-on draw is accepted for first-board ADC reliability |
| `R9`, `R10` | Stock package | 0402 `VBUS_DETECT` divider, packet value `220k` / `100k` | Present in the schematic and PCB as the production detect path to ESP32-C3 `GPIO3`; draw is only from USB VBUS while plugged in |
| `R11` | Stock package | 0402 TP4054 `CHRG_STAT` pull-up, packet value `100k` | Present in the schematic and PCB so the charger status pad is pulled up and testable; no MCU GPIO or firmware consumer is claimed yet |
| `D1` | Stock package | TPD2E2U06DCK-class rail-less 2-channel USB ESD array in SC-70/SOT-323 | Use the committed data-line shunt layout; do not reintroduce a VBUS clamp branch in the constrained service-edge pocket |
| `PCB1`, `SHELL1`, `RIM1`, `PAD1`, `ANT1` | Custom / mechanical | Notes, keep-outs, and fabrication constraints rather than electrical symbols | These drive the board shape, service seam, glide system, and antenna clearance |

## Blocking Locks Before First Routed Board

- exact `J1` USB-C connector footprint class. Resolved for this board pass as
  `USB4215`-class: 16-pin USB 2.0, top-mount horizontal, four through-hole
  shell stakes. The commodity-source follow-up is allowed to change the MPN,
  not the retention class or shell envelope.
- exact PAW3204 lens/clip sourcing path. `LED1` resolved — dropped from first
  capture because `U2`/`LENS1` source path bundles the emitter. Reinstate
  discrete `LED1` only if a bare-sensor supply path becomes the only option.
- battery replacement interface for `BT1`. Resolved — JST-SH 2-pin 1.0 mm
  right-angle receptacle on the PCB (`J_BAT`) with a harness-terminated
  protected pack.
- first-board flex-migration strategy. Resolved — P0 stays rigid; the
  zone-boundary rule in `PLACEMENT-CONSTRAINTS.md` preserves a later flex or
  rigid-flex respin without re-choosing footprints.
- charge-enable gate safety. Resolved in the PCB and active BOM with `Q2`/`R6`.
  Keep them unless the BDFL explicitly selects and documents a real load-switch
  substitution.
- firmware sense completeness. Resolved for `VBAT_SENSE` and `VBUS_DETECT` at
  the KiCad/BOM level with `R7`-`R10`, but the board remains DRC-red and any
  BSS138/load-switch substitution needs a packet update. `CHRG_STAT` now has
  `R11` and a pulled-up local status pad; it is not a firmware-consumed signal
  until a spare MCU GPIO and matching firmware symbol are explicitly allocated.

## Sense/Status And Charge-Gate Decision

Packet recommendation for the first rigid P0:

- Keep `Q1` as the SI2301-class high-side P-channel VBUS switch.
- Populate `Q2` as a SOT-23 2N7002 and keep `R6` = `100k` so `CHARGE_EN` cannot
  expose ESP32-C3 `GPIO10` to the 5 V `Q1` gate pull-up.
- Populate `R7`/`R8` = `100k`/`100k`, `R9`/`R10` = `220k`/`100k`, and `R11` =
  `100k`.
- Keep `CHRG_STAT` as a pulled-up local status/test pad in this packet. Do not
  claim firmware charge-status reporting until a GPIO and firmware symbol are
  deliberately allocated.

| Option | Safety | Quiescent draw | Repairability / BOM | Decision |
|--------|--------|----------------|---------------------|----------|
| `Q2` = 2N7002 SOT-23 | Keeps the 5 V `Q1` gate pull-up off the MCU, defaults off through `R6`, and only sinks the `R4` pull-up current while charging is enabled | No cell draw when USB is absent; `R6` draws only while the MCU actively drives `CHARGE_EN` high | Commodity SOT-23, easy to rework, low-cent BOM add | Recommended first-board choice |
| `Q2` = BSS138/BSS123 SOT-23 | Electrically adequate for this low-current gate-driver job if pinout matches | Same class of draw as 2N7002 | Good alternate, but no first-board advantage over 2N7002 | Keep as verified alternate, not primary |
| Logic-level load switch replacing `Q1`/`Q2`/`R6` | Cleaner integrated switch path if the exact part has suitable enable polarity, ESD, UVLO, and 3.3 V control behavior | Part-specific Iq must be checked; may improve or worsen sleep/USB draw | More package-specific, less repairable in tiny DFN/WLCSP options, and likely a new BOM/placement decision | Defer unless the BDFL explicitly chooses the substitution |
| `R7`/`R8` = `100k`/`100k` | Keeps VBAT within ADC range for low-voltage and overvoltage safety checks | Always-on draw is about `4.2 V / 200kΩ = 21 µA`; this raises the RT9080-era sleep floor but is still inside the first-board power budget | Two commodity 0402 resistors and no IC dependency | Recommended first-board value |
| Higher-value or switched `VBAT_SENSE` divider | Can reduce sleep draw | Better sleep floor, but ADC settling/noise and switch leakage need proof | More characterization or more parts before the board has safety bring-up data | Defer to a measured sleep-floor respin |
| `R9`/`R10` = `220k`/`100k` | Produces about 1.56 V from USB 5 V for a clear `GPIO3` detect input | About `5 V / 320kΩ = 16 µA`, drawn from USB only while plugged in | Commodity 0402 resistors | Recommended first-board value |
| `R11` = `100k` | Gives TP4054 open-drain `CHRG_STAT` a defined local status level | About `3.3 V / 100kΩ = 33 µA` only while STAT is asserted low | Commodity 0402 resistor; leaves status testable without stealing a GPIO | Recommended first-board value |
