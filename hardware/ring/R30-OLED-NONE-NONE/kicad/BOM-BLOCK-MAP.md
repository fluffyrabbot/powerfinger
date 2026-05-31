<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE BOM To Block Map

This file keeps the first schematic capture aligned with the published BOM.
Read it together with [CAPTURE-BINDINGS.md](CAPTURE-BINDINGS.md) so the block
map and the actual symbol/footprint choices do not drift apart.

| Schematic block | BOM refs | What belongs in the first capture | Notes |
|-----------------|----------|-----------------------------------|-------|
| MCU / radio | `U1`, `ANT1` | `ESP32-C3-MINI-1-N4` module and any required decoupling local to the module | The onboard module antenna is the default path; no external antenna should be required for P0 |
| Optical tracking | `U2`, `LED1`, `LENS1`, `R5` | PAW3204-class sensor, illumination path, lens-aligned notes, and SPI clock damping | ADNS-2080 remains the real backup path, but not the first P0 capture target |
| Primary click | `SW1` | Metal snap dome and any debounce/support passives | Keep the click path mechanically independent from battery service |
| Battery | `BT1` | Protected `80-100 mAh` LiPo connection and pack notes | Capture assumes integrated PCM remains mandatory |
| Charge path | `U3`, `R1`, `Q1` | TP4054, 20 kohm RPROG, and non-BOM fixture-fed VBUS service jumper | Keep this close to USB/VBUS entry. This P0 does not claim MCU charge-enable control; fixture VBUS presence controls charge service |
| Regulation | `U4`, `C2` plus DNP placeholders `C1`/`C3` | RT9080 power tree and board-level decoupling | `C1`/`C3` are retained as schematic-only DNP placeholders until the next schematic-driven PCB update; do not substitute a high-Iq LDO in the P0 capture |
| Thermal safety | `NTC1`, `R3` | NTC divider and sensing path | Place to reflect cell temperature, not convenient routing |
| Battery / USB sense | `R7`, `R8`, `R9`, `R10` | BDFL-accepted first-board VBAT and VBUS resistor dividers to the MCU resources in the interface contract | `R7`/`R8` = `100k`/`100k`; `R9`/`R10` = `220k`/`100k`. Reintroducing onboard charge-enable switching remains an explicit substitution |
| Charge status | `TP_CHRG` | TP4054 `CHRG_STAT` fixture status pad with no onboard pull-up | This is fixture-testable status, not a firmware-consumed claim yet; the fixture must provide a pull-up if status is measured |
| USB / service entry | `J1`, `R2`, `D1` | Off-board same-net USB service pads, CC pull-downs, and rail-less USB data ESD protection | The service/debug USB path is a first-pass P0 assumption, not a hidden dev-only hack; keep D1 as a D+/D- shunt rather than a VBUS-clamped protection island |
| Shell / glide interface | `SHELL1`, `RIM1`, `PAD1` | Mechanical notes and keep-out references, not electrical symbols | These should still be called out in the project notes so layout respects them |

## Capture Boundary

The first schematic should capture everything needed for the Standard optical
ring claim and nothing that belongs exclusively to later Pro or hedge variants.

The first PCB pass used to expose `VBAT_SENSE`, `VBUS_DETECT`, and
`CHRG_STAT` as pads only. The active BOM CSV and KiCad files now carry
`R7`/`R8` (VBAT_SENSE 100k/100k divider), `R9`/`R10` (VBUS_DETECT 220k/100k
divider), and `TP_CHRG` as a fixture-observed CHRG_STAT pad without an onboard
pull-up. Treat those as the BDFL-accepted first rigid P0 packet decision, not
hidden extras. The remaining truth is narrower: the board is now ERC/DRC/parity
clean, board-house output constraints and physical fit/stackup evidence are
still open, onboard charge-enable switching needs an explicit packet update,
and `CHRG_STAT` is only a fixture status/test pad for this board pass.
Production status behavior needs an explicit MCU allocation, Kconfig symbol,
firmware consumer, and focused tests. See
[CURRENT-VIOLATIONS.md](CURRENT-VIOLATIONS.md) for the ERC/DRC snapshot that
gates this packet.
