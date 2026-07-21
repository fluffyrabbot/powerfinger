<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen / Seeed Quote Packet

This is the smallest current offline packet to hand to a Shenzhen prototype
house or Seeed Fusion / Propagate contact for first conversation and DFM
scoping. It is not a purchase order, not a vendor endorsement, and not a claim
that every attached design is fabrication-ready.

Use [`SHENZHEN-SUBSTITUTE-RISK-QUESTIONS.md`](SHENZHEN-SUBSTITUTE-RISK-QUESTIONS.md)
as the first-exchange sheet for substitute-risk classification, factory
questions, and accessibility/serviceability non-negotiables.
Use [`SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`](SHENZHEN-FACTORY-RESPONSE-CAPTURE.md)
as the paste-back sheet for proposed substitutions, DFM requests,
source-return posture, and quote-vs-verified status after a factory replies.
Before recording a reply, create one dated evidence directory with
`scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD` so incoming quote
files, substitutions, DFM asks, and source-return artifacts are captured once
under `docs/sensors-converge-2026/factory-replies/`.

The packet is intentionally split into two paths:

- Send-now quote path: `USB-HUB` PCB fab/assembly quote plus
  connector/enclosure DFM review.
- Optional annex: `R30-OLED-NONE-NONE` DFM/pre-fab review only. The KiCad
  ERC/DRC/parity gate is now clean, but do not treat the ring as a fab/assembly
  quote path until the board-house output-constraints checklist and physical
  fit/stackup evidence are closed.

## Scope To Quote

Quote only the send-now hub path unless the factory explicitly accepts the ring
annex as a DFM/pre-fab review input.

| Item | Packet | Request | Current status |
|---|---|---|---|
| USB hub dongle | `hardware/shared/USB-HUB/` | PCB fab/assembly quote, enclosure/connector-retention DFM review, and serviceability feedback | BOM-backed packet and routed first-board source exist; schematic ERC=0, PCB DRC=0, unconnected=0, parity=0; consume `hardware/shared/USB-HUB/kicad/CURRENT-VIOLATIONS.md`, `hardware/shared/USB-HUB/kicad/FABRICATION-OUTPUTS.md`, and `hardware/shared/USB-HUB/kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` as the checked-in status sources |
| Optical ring annex | `hardware/ring/R30-OLED-NONE-NONE/` | DFM/pre-fab review, BOM costability review, output-constraints review, and assembly/serviceability feedback only | BOM-backed packet and first routed rigid P0 source exist; KiCad snapshot is ERC=0, DRC=0, unconnected=0, parity=0; consume `hardware/ring/R30-OLED-NONE-NONE/kicad/CURRENT-VIOLATIONS.md`, `hardware/ring/R30-OLED-NONE-NONE/kicad/FABRICATION-OUTPUTS.md`, and `hardware/ring/R30-OLED-NONE-NONE/kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` as the checked-in status sources |

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

## Active USB Hub Snapshot

Packet: `hardware/shared/USB-HUB/`

- Variant: `USB-HUB`, shared accessory / hub dongle.
- Use case: BLE central for the active optical ring pair; USB HID mouse bridge
  to the host OS; optional local companion control port over the same USB link.
- PCB: stepped direct USB-A dongle, not an earlier compact placeholder.
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
- Fabrication-review packet: `kicad/FABRICATION-OUTPUTS.md` records local
  Gerber, split drill, POS, active-BOM/POS review, and archive hashes. The
  current active-BOM/POS review has no open rows. The Shenzhen/Seeed export
  script regenerates and includes this review bundle by default.
- Board-house intake packet:
  `kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` records the current answers and
  open questions for file set, layer order, drill/slot handling, mask, paste,
  panelization, module assembly, no-BOM copper features, connector-retention
  DFM, antenna keep-out handling, and editable source return.
- Quote go/no-go: go for PCB fab/assembly quote and connector/enclosure DFM
  review from the current source packet; no-go for build release until printed
  host-fit, clamp-alignment, service-hatch reach, adjacent-port clearance, and
  connector-retention evidence are recorded. Board-house output constraints
  also remain open until a vendor response is recorded.
- Open blockers: printed host-fit, clamp-alignment, service-hatch reach,
  adjacent-port clearance, and connector-retention evidence are not measured
  yet.

## R30 DFM / Pre-Fab Review Annex

Packet: `hardware/ring/R30-OLED-NONE-NONE/`

Status source: `hardware/ring/R30-OLED-NONE-NONE/kicad/CURRENT-VIOLATIONS.md`
is the checked-in quote-facing source for the R30 KiCad snapshot. Do not
copy-edit numeric R30 ERC/DRC/unconnected/parity counts into this packet; keep
this annex pointed at the status source so exported packets cannot drift from
the actual KiCad proof.

Use this annex only when the factory is willing to give early manufacturability,
assembly, serviceability, sourcing, and costability feedback before measured
ring fit/stackup evidence exists. The annex is not a ring PCB fab/assembly quote
request and is not a release-to-build package.

- Variant: `R30-OLED-NONE-NONE`, optical ring, `30 deg` sensor posture.
- Use case: one of two identical rings in the default cursor + scroll pair.
- Surface claim today: opaque rigid surfaces only; not glass.
- PCB: rigid P0, `43 x 18 mm`, split into power/service, sensor/click, and
  MCU/radio zones.
- KiCad status: ERC=0, DRC=0, unconnected=0, and schematic parity=0 per
  `kicad/CURRENT-VIOLATIONS.md`. `kicad/FABRICATION-OUTPUTS.md` records local
  Gerber/drill/POS review outputs plus an active-BOM/POS review with no open
  rows. `kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` turns that generated-output
  state into explicit prototype-house intake questions. The board must not be
  treated as fabrication-release until that checklist and physical fit/stackup
  evidence are closed.
- MCU/radio: `ESP32-C3-MINI-1-N4` module, with antenna keep-out preserved.
- Sensor: `PAW3204DB-TJ3L` optical sensor plus matched lens/emitter kit;
  `LED1` is dropped from the first capture unless bare-sensor sourcing forces
  it back in. `ADNS-2080` is the evaluated fallback class for a future board
  profile, not a drop-in substitution for this packet; `YS8205`-style
  integrated USB mouse controllers are not acceptable replacements.
- Battery: protected `80-100 mAh` LiPo pack, connected through off-board
  same-net battery service pads on `J_BAT`; direct cell-tab soldering is
  rejected, and battery service must remain replaceable through a harness or
  fixture.
- Service interface: off-board same-net USB service pads on `J1` for USB2 data,
  VBUS, CC pulldowns, GND, and shield continuity; charge/program service now
  requires an external fixture or pogo harness rather than an onboard USB-C
  receptacle.
- Power path: TP4054 charger, `20 kohm` charge resistor, RT9080-33GJ5 LDO, NTC
  divider, and non-BOM `Q1` VBUS service jumper from fixture VBUS to TP4054
  `VCC`; no onboard MCU charge-enable switch is claimed in this P0.
- Sense/status support: `R7`/`R8 = 100k`/`100k` for `VBAT_SENSE`,
  `R9`/`R10 = 220k`/`100k` for `VBUS_DETECT`, and `TP_CHRG` as a
  fixture-observed `CHRG_STAT` pad with no onboard pull-up. `CHRG_STAT` is not
  claimed as a firmware-consumed GPIO yet.
- USB ESD: TPD2E2U06DCK-class rail-less dual data-line shunt in SC-70/SOT-323.
- Mechanical packet: OpenSCAD lower shell and service lid with raised rim,
  glide-pad pockets, board rails/stops, lid compression pads, service-pad
  access opening, dome pocket, battery harness channel, service-loop relief, and
  quick-print fit coupon modes.
- Open blockers: vendor response to the R30 board-house output-constraints
  checklist, printed/measured fit, stackup, focal-distance, RF, click force,
  battery service, and fastener choice remain evidence items.

## Quote Questions

Ask the factory to respond to the `USB-HUB` send-now path with:

1. Hub PCB fab/assembly quote using the current `USB-HUB` packet and BOM target
   as a starting point.
2. Confirmation that the generated hub Gerber, split drill, Gerber job, POS,
   active-BOM/POS review, and source BOM are sufficient for intake, or the exact
   regenerated file set they require.
3. Connector/enclosure DFM notes for the SOFNG USB-05 direct-plug topology,
   including whether they see manufacturability risk before physical
   connector-retention evidence exists.
4. Any part substitutions they propose, with exact MPN, footprint impact,
   source, MOQ, and whether the substitution preserves serviceability and
   offline operation.
5. What editable source they would return for any DFM modifications so the
   CERN-OHL-S hardware disclosure remains complete.
6. Whether they can fill or mirror the response-capture fields in
   `SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`, keeping quote-only statements
   separate from verified evidence.

If the factory accepts the `R30-OLED-NONE-NONE` annex as DFM/pre-fab review
input, ask separately:

1. Whether they can review the active ring's DRC-clean KiCad source and local
   fabrication-output packet as a DFM/pre-fab input, and what exact output,
   stackup, panelization, drill, solder-mask, paste, POS/BOM, or CAM-intake
   changes they require before a ring PCB fab/assembly quote.
2. Whether they can source or assemble the PAW3204 sensor/lens/emitter path, or
   whether they require the ADNS-2080 alternate path to be evaluated first.
3. Whether the ring's off-board USB service-pad fixture path and replaceable
   battery harness or fixture path are manufacturable and serviceable at the
   intended prototype scale.
4. Which board-house output, stackup, focal-distance, shell-fit,
   serviceability, or sourcing issues they would require closed before quoting
   ring PCB fab/assembly.
5. Whether they can label every annex answer as DFM/pre-fab feedback, not a
   ring fabrication or assembly quote, in
   `SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`.

## USB-HUB Send-Now Source Files To Send Or Link

- `docs/sensors-converge-2026/SHENZHEN-FIRST-CONTACT-TEMPLATE.md`
- `docs/sensors-converge-2026/SHENZHEN-SUBSTITUTE-RISK-QUESTIONS.md`
- `docs/sensors-converge-2026/SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`
- `docs/sensors-converge-2026/SHENZHEN-FACTORY-ONE-PAGER.zh-CN.md`
- `hardware/bom/USB-HUB.csv`
- `hardware/shared/USB-HUB/MANIFEST.md`
- `hardware/shared/USB-HUB/FIRST-BOARD-CHECKLIST.md`
- `hardware/shared/USB-HUB/CONNECTOR-RETENTION-VERIFY.md`
- `hardware/shared/USB-HUB/kicad/CURRENT-VIOLATIONS.md`
- `hardware/shared/USB-HUB/kicad/FABRICATION-OUTPUTS.md`
- `hardware/shared/USB-HUB/kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md`
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
- `scripts/generate-usb-hub-fabrication-review.py`
- `LICENSE-HARDWARE`
- `LICENSE-SOFTWARE`

## R30-OLED-NONE-NONE DFM / Pre-Fab Review Annex Files

- `hardware/bom/R30-OLED-NONE-NONE.csv`
- `hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md`
- `hardware/ring/R30-OLED-NONE-NONE/FIRST-BOARD-CHECKLIST.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/CURRENT-VIOLATIONS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/FABRICATION-OUTPUTS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/CAPTURE-BINDINGS.md`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pro`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb`
- `hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad`
- `scripts/generate-r30-assembly-review.py`
- `LICENSE-HARDWARE`
- `LICENSE-SOFTWARE`

## Local Export

Regenerate the send-ready local source packet for the `USB-HUB` quote path:

```bash
scripts/export-shenzhen-seeed-quote-packet.sh
```

Default output: `build/quote-packets/shenzhen-seeed-usb-hub-current/`. The
send-now export includes the first-contact template and simplified-Chinese
factory one-pager under `docs/sensors-converge-2026/`, plus a regenerated
`USB-HUB-FABRICATION-REVIEW/` directory containing the hub Gerber/drill/POS/BOM
review bundle and archive. Send or link that directory for the hub quote.
The export also writes two operator-facing files at the packet root:
`OUTBOUND-DRAFT.md`, generated from
`SHENZHEN-FIRST-CONTACT-TEMPLATE.md` with subject/body placeholders preserved
and the optional R30 body section included only in annex-mode exports, and
`ATTACHMENT-MANIFEST.md`, generated from the same packet-document sections that
drive the copied USB-HUB files and optional R30 annex files. Use
`PACKET-MANIFEST.md` as the byte-count and SHA-256 hash manifest for the whole
generated bundle.

To include the R30 DFM/pre-fab review annex in a separate subdirectory:

```bash
scripts/export-shenzhen-seeed-quote-packet.sh --include-r30-annex
```

That keeps the hub quote packet at
`build/quote-packets/shenzhen-seeed-usb-hub-current/` and adds the optional
annex under
`build/quote-packets/shenzhen-seeed-usb-hub-current/R30-OLED-NONE-NONE-DFM-ANNEX/`.
Send the annex only for DFM/pre-fab review, not for ring PCB fab/assembly
quote.

The script copies the listed source files, regenerates the USB-HUB
fabrication-review artifacts, writes `OUTBOUND-DRAFT.md` and
`ATTACHMENT-MANIFEST.md`, and writes `PACKET-MANIFEST.md` with file sizes and
SHA-256 hashes. It also fails if the generated attachment manifest drifts from
the packet-doc file sections, if the regenerated USB-HUB fabrication-review
archive no longer matches the SHA-256 recorded in
`hardware/shared/USB-HUB/kicad/FABRICATION-OUTPUTS.md`, or if manifest rows and
exported file counts drift. It does not generate STLs or physical-evidence
records.

After the first factory reply arrives, scaffold the source-controlled intake
directory before updating response tables:

```bash
scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD
```

Use `--include-r30-annex` only if the same response explicitly accepts
`R30-OLED-NONE-NONE` as DFM/pre-fab review input. Put raw messages in
`incoming/`, quote sheets and DFM reports in `quote-files/`, returned editable
source or CAM-only artifacts in `source-return/`, then cite the dated directory
from `SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`.

For one local first-board mechanical print/preview packet covering both the
USB-HUB coupon lane and the R30 DFM/pre-fab coupon lane:

```bash
scripts/generate-first-board-mechanical-packet.sh
```

Default output: `build/first-board-mechanical-packet/`, with `USB-HUB/` and
`R30-OLED-NONE-NONE/` sub-bundles. This command only assembles generated
STLs, preview PNGs, hash manifests, OpenSCAD logs, README files, and blank
physical-check worksheets from the two active coupon generators. It does not
turn either lane into measured fit, connector-retention, stackup,
focal-distance, comfort, click, or RF evidence.

To also isolate the first physical-check print sweep in `FIRST-SWEEP/`, use:

```bash
scripts/generate-first-board-mechanical-packet.sh --first-sweep
```

That first sweep selects only the USB-HUB host-fit coupon, USB-HUB clamp
alignment gauge, R30 off-board service-pad access coupon, and R30
board-retention coupon plus matching previews/logs and a blank worksheet.
Treat `build/first-board-mechanical-packet/FIRST-SWEEP/` as the one obvious
starting print bundle for this packet, but keep result fields blank until those
printed coupons are checked against real hosts, plugs, boards, board blanks, or
fixtures.

For the USB-HUB physical coupon lane, regenerate the local STL/preview coupon
bundle and blank evidence worksheet separately:

```bash
scripts/generate-usb-hub-validation-coupons.sh
```

Default output: `build/usb-hub-mechanical/`. The generated `README.md`,
`COUPON-MANIFEST.md`, `previews/*.png`, `PHYSICAL-CHECK-WORKSHEET.md`, and
`FIRST-PRINT/` proof-capture packet are local evidence scaffolding only; they
are not proof of fit, strain, or clearance until printed coupon observations are
ingested into `hardware/shared/USB-HUB/COUPON-RESULTS.md` with
`scripts/ingest-usb-hub-coupon-results.py`. Use that ledger before changing
`CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, or checklist closure state.
`FIRST-PRINT/` is the concrete USB-HUB print queue for host-fit, adjacent-port
clearance, clamp alignment, service-hatch reach, and connector-retention
capture; it is generated locally and is not part of the source-only Shenzhen
send-now export.
