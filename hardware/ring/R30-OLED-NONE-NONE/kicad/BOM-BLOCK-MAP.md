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
| Charge path | `U3`, `R1`, `Q1`, `R4` | TP4054, 20 kohm RPROG, charge-enable MOSFET, default-off gate bias | Keep this close to USB/VBUS entry |
| Regulation | `U4`, `C1`, `C2` | RT9080 power tree and board-level decoupling | Do not substitute a high-Iq LDO in the P0 capture |
| Thermal safety | `NTC1`, `R3` | NTC divider and sensing path | Place to reflect cell temperature, not convenient routing |
| USB / service entry | `J1`, `R2`, `D1` | USB-C receptacle, CC pull-downs, USB ESD protection, and service pads | The service/debug USB path is a first-pass P0 assumption, not a hidden dev-only hack |
| Shell / glide interface | `SHELL1`, `RIM1`, `PAD1` | Mechanical notes and keep-out references, not electrical symbols | These should still be called out in the project notes so layout respects them |

## Capture Boundary

The first schematic should capture everything needed for the Standard optical
ring claim and nothing that belongs exclusively to later Pro or hedge variants.
