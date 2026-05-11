<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen / Seeed Quote Packet

This is the smallest current packet to hand to a Shenzhen prototype house or
Seeed Fusion / Propagate contact for first conversation and DFM scoping. It is
not a purchase order, not a vendor endorsement, and not a claim that the ring is
fabrication-ready.

## Scope To Quote

Quote the active validation-lane pair:

| Item | Packet | Request | Current status |
|---|---|---|---|
| Optical ring | `hardware/ring/R30-OLED-NONE-NONE/` | DFM/pre-fab review, BOM costability review, assembly/serviceability feedback, and quote once PCB DRC is closed | BOM-backed packet and first routed rigid P0 source exist; KiCad snapshot is still red: ERC=0, PCB DRC=60, unconnected=9, parity=0 |
| USB hub dongle | `hardware/shared/USB-HUB/` | PCB fab/assembly quote, enclosure/connector-retention DFM review, and serviceability feedback | BOM-backed packet and routed first-board source exist; schematic ERC=0, PCB DRC=0, unconnected=0, parity=0 |

Do not quote secondary ring, wand, puck, OCR, cloud, or companion-app product
features as part of this starter packet.

## Factory-Facing Constraints

- Accessibility-first: the ring and hub must remain serviceable and usable by
  people with limited mobility; do not replace reopenable service paths with
  glue, potting, sealed heat-shrink, or destructive battery access.
- No cloud dependency: the hardware must work as local BLE/USB HID. Cloud/API
  features are outside this quote.
- License posture: hardware files are CERN-OHL-S 2.0; firmware/software are
  MIT. Any manufacturer-side hardware modifications that are conveyed must come
  back as editable source suitable for upstream publication.
- BOM honesty: the active ring target is `~$9` prototype-scale BOM; the hub
  target is `~$5-6`. Treat these as repo targets, not vendor commitments.
- No invented measurements: printed fit, focal-distance, RF, click ergonomics,
  connector strain, adjacent-port clearance, yield, lead time, and realized COGS
  are not measured yet unless a packet evidence file says otherwise.

## Active Ring Snapshot

Packet: `hardware/ring/R30-OLED-NONE-NONE/`

- Variant: `R30-OLED-NONE-NONE`, optical ring, `30 deg` sensor posture.
- Use case: one of two identical rings in the default cursor + scroll pair.
- Surface claim today: opaque rigid surfaces only; not glass.
- PCB: rigid P0, `43 x 18 mm`, split into power/service, sensor/click, and
  MCU/radio zones.
- MCU/radio: `ESP32-C3-MINI-1-N4` module, with antenna keep-out preserved.
- Sensor: `PAW3204DB-TJ3L` optical sensor plus matched lens/emitter kit;
  `LED1` is dropped from the first capture unless bare-sensor sourcing forces
  it back in. `ADNS-2080` is the evaluated fallback class for a future board
  profile, not a drop-in substitution for this packet; `YS8205`-style
  integrated USB mouse controllers are not acceptable replacements.
- Battery: protected `80-100 mAh` LiPo pack, harness-terminated to a JST-SH
  2-pin 1.0 mm plug; direct cell-tab soldering is rejected.
- Battery connector: JST-SH right-angle 2-pin 1.0 mm class on `J_BAT`.
- Service connector: USB-C 2.0 16-pin receptacle class with through-hole shell
  stakes; SMD-only and power-only USB-C are rejected for P0.
- Power path: TP4054 charger, `20 kohm` charge resistor, RT9080-33GJ5 LDO, NTC
  divider, SI2301-class high-side `Q1`, and 2N7002 `Q2` logic-safe charge-gate
  driver with `R6 = 100k`.
- Sense/status support: `R7`/`R8 = 100k`/`100k` for `VBAT_SENSE`,
  `R9`/`R10 = 220k`/`100k` for `VBUS_DETECT`, and `R11 = 100k` for local
  `CHRG_STAT`. `CHRG_STAT` is not claimed as a firmware-consumed GPIO yet.
- USB ESD: TPD2E2U06DCK-class rail-less dual data-line shunt in SC-70/SOT-323.
- Mechanical packet: OpenSCAD lower shell and service lid with raised rim,
  glide-pad pockets, board rails/stops, lid compression pads, USB-C opening,
  dome pocket, battery lead channel, service-loop relief, and quick-print fit
  coupon modes.
- Open blockers: board is not fabrication-release; printed/measured fit,
  stackup, focal-distance, RF, click force, battery service, and fastener choice
  remain evidence items.

## Active USB Hub Snapshot

Packet: `hardware/shared/USB-HUB/`

- Variant: `USB-HUB`, shared accessory / hub dongle.
- Use case: BLE central for the active optical ring pair; USB HID mouse bridge
  to the host OS; optional local companion control port over the same USB link.
- PCB: stepped direct USB-A dongle, not the older `~20 x 12 mm` placeholder.
  The packet documents a host-side USB-A nose and wider `54 x 26 mm`
  module/service body.
- MCU/radio/USB: `ESP32-S3-MINI-1-N8` module with native USB.
- USB connector: `SOFNG USB-05` (`LCSC C112454`) male USB-A direct-plug
  connector.
- Regulator: RT9080-33GJ5 (`LCSC C882092`).
- USB ESD: USBLC6-2SC6 (`LCSC C7519`).
- USB series resistors: `R1A`/`R1B = 22R` in 0402 at the module side.
- Capacitors: `C2 = 10uF` 0603 input bulk; `C3 = 1uF` 0603 output capacitor;
  0402 local decoupling where documented in the BOM/packet.
- Mechanical packet: OpenSCAD enclosure with stepped outline, `MH1`/`MH2`
  clamp-hole load path, removable service hatch, rear antenna reference volume,
  host-clearance gauges, and quick-print modes for host-fit, clamp alignment,
  service-hatch reach, and combined validation.
- KiCad packet: project-local symbol and footprint libraries are configured;
  current local verification is ERC=0, PCB DRC=0, unconnected=0, parity=0.
- Quote go/no-go: go for PCB fab/assembly quote and connector/enclosure DFM
  review from the current source packet; no-go for build release until printed
  host-fit, clamp-alignment, service-hatch reach, adjacent-port clearance, and
  connector-retention evidence are recorded.
- Open blockers: printed host-fit, clamp-alignment, service-hatch reach,
  adjacent-port clearance, and connector-retention evidence are not measured
  yet.

## Quote Questions

Ask the factory to respond with:

1. Whether they can review the active ring while its PCB is still DRC-red, and
   whether they prefer a DFM-only quote before release-to-build.
2. Hub PCB fab/assembly quote using the current `USB-HUB` packet and BOM target
   as a starting point.
3. Any part substitutions they propose, with exact MPN, footprint impact,
   source, MOQ, and whether the substitution preserves serviceability and
   offline operation.
4. Whether they can source or assemble the PAW3204 sensor/lens/emitter path, or
   whether they require the ADNS-2080 alternate path to be evaluated first.
5. Whether the ring USB-C through-hole-stake class and hub SOFNG USB-05 direct
   plug are manufacturable at the intended prototype scale.
6. What editable source they would return for any DFM modifications so the
   CERN-OHL-S hardware disclosure remains complete.

## Source Files To Send Or Link

- `hardware/bom/R30-OLED-NONE-NONE.csv`
- `hardware/bom/USB-HUB.csv`
- `hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md`
- `hardware/ring/R30-OLED-NONE-NONE/FIRST-BOARD-CHECKLIST.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/CURRENT-VIOLATIONS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/CAPTURE-BINDINGS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pro`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb`
- `hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad`
- `hardware/shared/USB-HUB/MANIFEST.md`
- `hardware/shared/USB-HUB/FIRST-BOARD-CHECKLIST.md`
- `hardware/shared/USB-HUB/CONNECTOR-RETENTION-VERIFY.md`
- `hardware/shared/USB-HUB/kicad/CURRENT-VIOLATIONS.md`
- `hardware/shared/USB-HUB/kicad/P0-COMPONENT-LOCKS.md`
- `hardware/shared/USB-HUB/kicad/fp-lib-table`
- `hardware/shared/USB-HUB/kicad/sym-lib-table`
- `hardware/shared/USB-HUB/kicad/PowerFinger.kicad_sym`
- `hardware/shared/USB-HUB/kicad/usb_hub.kicad_pro`
- `hardware/shared/USB-HUB/kicad/usb_hub.kicad_sch`
- `hardware/shared/USB-HUB/kicad/sheets/usb_and_power.kicad_sch`
- `hardware/shared/USB-HUB/kicad/sheets/mcu_radio.kicad_sch`
- `hardware/shared/USB-HUB/kicad/sheets/controls_and_indicators.kicad_sch`
- `hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/README.md`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/C_0402_1005Metric_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/C_0603_1608Metric_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/ESP32-S3-MINI-1-N8_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/LED_0402_1005Metric_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/MountingHole_1.4mm_Clamp.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/PadActuated_Boot_Service.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/R_0402_1005Metric_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/SOT-23-5_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/SOT-23-6_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/TestPoint_Pad_D1.0mm_FirstBoard.kicad_mod`
- `hardware/shared/USB-HUB/kicad/PowerFinger_USB.pretty/USB_A_Plug_SOFNG_USB-05.kicad_mod`
- `hardware/shared/USB-HUB/cad/usb_hub_enclosure_blank.scad`
- `LICENSE-HARDWARE`
- `LICENSE-SOFTWARE`

## Local Export

Regenerate a local copy of exactly the file list above:

```bash
scripts/export-shenzhen-seeed-quote-packet.sh
```

Default output: `build/quote-packets/shenzhen-seeed-current/`.

The script copies existing source files only and writes
`PACKET-MANIFEST.md` with file sizes and SHA-256 hashes. It does not generate
Gerbers, STLs, zip archives, or any other derived fabrication artifacts.
