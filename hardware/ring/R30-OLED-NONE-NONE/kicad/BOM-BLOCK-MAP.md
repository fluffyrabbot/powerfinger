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
| Charge path | `U3`, `R1`, `Q1`, `R4`, `Q2`, `R6` | TP4054, 20 kohm RPROG, charge-enable MOSFET, default-off gate bias, and BDFL-accepted 2N7002 logic-safe gate driver | Keep this close to USB/VBUS entry. `Q2`/`R6` are a safety add because a 5 V pull-up on `Q1` gate must not touch ESP32-C3 `GPIO10` directly |
| Regulation | `U4`, `C2` plus DNP placeholders `C1`/`C3` | RT9080 power tree and board-level decoupling | `C1`/`C3` are retained as schematic-only DNP placeholders until the next schematic-driven PCB update; do not substitute a high-Iq LDO in the P0 capture |
| Thermal safety | `NTC1`, `R3` | NTC divider and sensing path | Place to reflect cell temperature, not convenient routing |
| Battery / USB sense | `R7`, `R8`, `R9`, `R10` | BDFL-accepted first-board VBAT and VBUS resistor dividers to the MCU resources in the interface contract | `R7`/`R8` = `100k`/`100k`; `R9`/`R10` = `220k`/`100k`. BSS138-class alternates or a logic-level load switch remain explicit substitutions |
| Charge status | `R11` | TP4054 `CHRG_STAT` `100k` pull-up to `VREG_3V3` plus a local status pad | This is hardware-testable status, not a firmware-consumed claim yet |
| USB / service entry | `J1`, `R2`, `D1` | USB-C receptacle, CC pull-downs, rail-less USB data ESD protection, and service pads | The service/debug USB path is a first-pass P0 assumption, not a hidden dev-only hack; keep D1 as a D+/D- shunt rather than a VBUS-clamped protection island |
| Shell / glide interface | `SHELL1`, `RIM1`, `PAD1` | Mechanical notes and keep-out references, not electrical symbols | These should still be called out in the project notes so layout respects them |

## Capture Boundary

The first schematic should capture everything needed for the Standard optical
ring claim and nothing that belongs exclusively to later Pro or hedge variants.

The first PCB pass used to expose `VBAT_SENSE`, `VBUS_DETECT`, and
`CHRG_STAT` as pads only. The active BOM CSV and KiCad files now carry
`R7`/`R8` (VBAT_SENSE 100k/100k divider), `R9`/`R10` (VBUS_DETECT 220k/100k
divider), and `R11` (CHRG_STAT 100k pull-up to `VREG_3V3`). Treat those as the
BDFL-accepted first rigid P0 packet decision, not hidden extras. The remaining
truth is narrower: the board is still ERC/DRC-red, BSS138-class or load-switch
substitutions need an explicit packet update, and `CHRG_STAT` is only a
pulled-up local status/test pad until firmware receives an explicit GPIO and
config symbol. See
[CURRENT-VIOLATIONS.md](CURRENT-VIOLATIONS.md) for the ERC/DRC snapshot that
gates this packet.
