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
| `BT1` | Vendor-specific | Harness-terminated 80-100 mAh protected LiPo pack that lands on the off-board battery service-pad interface | Direct cell-tab soldering is rejected because it defeats the reopenable-seam replaceability rule |
| `J_BAT` | Source-controlled packet footprint | `Offboard_Battery_Service_SameNet` service-pad footprint | Off-board same-net battery service pads preserve `VBAT+`, `VBAT-`, and grounded shield/service pads. First hardware still needs a replaceable harness or fixture so the cell is not direct-soldered |
| `U3` | Stock package | SOT-23-5 charger symbol + footprint | Exact TP4054 pinout matters; do not substitute MCP73831 without changing layout |
| `U4` | Source-controlled packet footprint | RT9080-33GJ5 SOT-23-5 LDO symbol plus compact service-clearance footprint | Keep the low-Iq part choice explicit; this retained land pattern is a packet-local DRC reducer, not a silent regulator substitution |
| `J1` | Source-controlled packet footprint | `Offboard_USB_Service_SameNet` service-pad footprint | Service/debug USB path is retained as off-board same-net pads for fixture or pogo access. No onboard USB-C receptacle body is claimed in this active packet |
| `C1`, `C2`, `C3` | Stock package | Standard MLCC symbols with 0402/0603 footprints as committed in capture | `C1`/`C3` are DNP schematic placeholders in this hand-routed pass; `C2` is the populated 0603 bulk capacitor. Only move `C2` if assembly or voltage-rating constraints require it |
| `R1`, `R2`, `R3`, `R5` | Stock package | Standard resistor symbols with 0402 footprints | Keep the USB-C Rd pair and the SPI damping resistor visible as separate functions |
| `NTC1` | Stock package | NTC thermistor symbol with 0402 footprint | Placement must follow the cell-temperature rule, not routing convenience |
| `Q1` | Source-controlled packet footprint | Non-BOM `VBUS_Service_Jumper_SOT23Land` copper jumper | Fixture-fed `VBUS_5V` reaches TP4054 `VCC` directly on this P0. `R4`, `Q2`, `R6`, and MCU charge-enable control are intentionally not populated |
| `R7`, `R8` | Stock package | 0402 `VBAT_SENSE` divider, packet value `100k` / `100k` | Present in the schematic and PCB as the production sense path to ESP32-C3 `GPIO0`; always-on draw is accepted for first-board ADC reliability |
| `R9`, `R10` | Stock package | 0402 `VBUS_DETECT` divider, packet value `220k` / `100k` | Present in the schematic and PCB as the production detect path to ESP32-C3 `GPIO3`; draw is only from USB VBUS while plugged in |
| `TP_CHRG` | Source-controlled packet footprint | TP4054 `CHRG_STAT` fixture status pad | Present in the schematic and PCB as a fixture-observed open-drain status pad. The onboard `R11` pull-up is cut from this packet; use an external pull-up if status is measured |
| `D1` | Stock package | TPD2E2U06DCK-class rail-less 2-channel USB ESD array in SC-70/SOT-323 | Use the committed data-line shunt layout; do not reintroduce a VBUS clamp branch in the constrained service-edge pocket |
| `PCB1`, `SHELL1`, `RIM1`, `PAD1`, `ANT1` | Custom / mechanical | Notes, keep-outs, and fabrication constraints rather than electrical symbols | These drive the board shape, service seam, glide system, and antenna clearance |

## Blocking Locks Before First Routed Board

- exact `J1` service interface footprint class. Resolved for this board pass as
  source-controlled off-board same-net USB service pads. A future onboard
  connector would be a new packet decision, not a commodity-source swap.
- exact PAW3204 lens/clip sourcing path. `LED1` resolved — dropped from first
  capture because `U2`/`LENS1` source path bundles the emitter. Reinstate
  discrete `LED1` only if a bare-sensor supply path becomes the only option.
- battery replacement interface for `BT1`. Resolved for this board pass as
  source-controlled off-board same-net battery service pads (`J_BAT`) plus a
  replaceable harness or fixture. The cell must not be direct-soldered.
- first-board flex-migration strategy. Resolved — P0 stays rigid; the
  zone-boundary rule in `PLACEMENT-CONSTRAINTS.md` preserves a later flex or
  rigid-flex respin without re-choosing footprints.
- charge-service topology. Resolved for this P0 by cutting the active
  charge-enable gate and using the non-BOM `Q1` VBUS service jumper. Reintroduce
  an onboard switch only with an explicit BOM, schematic, PCB, and firmware
  contract update.
- firmware sense completeness. Resolved for `VBAT_SENSE` and `VBUS_DETECT` at
  the KiCad/BOM level with `R7`-`R10`, but the board remains DRC-red and any
  BSS138/load-switch substitution needs a packet update. `CHRG_STAT` now keeps
  only a local fixture status pad; it is not pulled up on board and is not a
  firmware-consumed signal for this board pass. Adding production status
  behavior requires a later board-contract change with a real MCU allocation,
  firmware config, implementation, and focused tests.

## Sense/Status And Charge-Service Decision

Packet recommendation for the first rigid P0:

- Keep `Q1` as a non-BOM copper VBUS service jumper on the SOT-23 service land.
- Do not populate `R4`, `Q2`, or `R6`; ESP32-C3 `GPIO10` is no-connect in this
  P0 and charge service is controlled by external fixture VBUS presence.
- Populate `R7`/`R8` = `100k`/`100k` and `R9`/`R10` = `220k`/`100k`.
- Keep `CHRG_STAT` as a fixture-observed local status pad in this packet. Do
  not claim onboard pull-up behavior or firmware charge-status reporting for
  this board pass; any production status consumer must move the board,
  firmware, and test contracts together.

| Option | Safety | Quiescent draw | Repairability / BOM | Decision |
|--------|--------|----------------|---------------------|----------|
| Non-BOM `Q1` VBUS service jumper | Eliminates the local 5 V gate-control island and keeps MCU GPIO10 off charger control | No added cell draw; charging occurs only when fixture VBUS is present | No populated switch part, lowest BOM and easiest inspection | Recommended first-board choice |
| Reintroduced active charge-enable switch | Can restore firmware-controlled charge gating if a later board proves the need | Depends on exact switch/gate topology | Adds parts, routing pressure, and firmware contract surface | Defer unless the BDFL explicitly chooses the substitution |
| `R7`/`R8` = `100k`/`100k` | Keeps VBAT within ADC range for low-voltage and overvoltage safety checks | Always-on draw is about `4.2 V / 200kΩ = 21 µA`; this raises the RT9080-era sleep floor but is still inside the first-board power budget | Two commodity 0402 resistors and no IC dependency | Recommended first-board value |
| Higher-value or switched `VBAT_SENSE` divider | Can reduce sleep draw | Better sleep floor, but ADC settling/noise and switch leakage need proof | More characterization or more parts before the board has safety bring-up data | Defer to a measured sleep-floor respin |
| `R9`/`R10` = `220k`/`100k` | Produces about 1.56 V from USB 5 V for a clear `GPIO3` detect input | About `5 V / 320kΩ = 16 µA`, drawn from USB only while plugged in | Commodity 0402 resistors | Recommended first-board value |
| External `CHRG_STAT` pull-up | Lets a fixture read the TP4054 open-drain status pad during bench bring-up | No onboard quiescent draw in this packet | Fixture-side resistor keeps the ring PCB simpler and avoids stealing a GPIO | Use only in the external test fixture for this board pass |
