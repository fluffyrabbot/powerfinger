<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen Substitute Risk And Factory Questions

This sheet is the first factory-exchange companion for the current Shenzhen /
Seeed packet. It is deliberately narrow:

- Quote path: `USB-HUB` PCB fab/assembly quote, connector/enclosure DFM review,
  and serviceability feedback.
- Annex path: `R30-OLED-NONE-NONE` DFM/pre-fab review only.

Do not use this sheet to quote secondary rings, wands, puck variants, OCR
features, cloud features, or companion-app work.

## Factory-Exchange Rule

Every proposed substitution must come back with:

| Required field | Why it matters |
|---|---|
| Exact manufacturer, MPN, package, and supplier link | Prevents "equivalent" parts from hiding a footprint, pinout, or quality change |
| Whether the current footprint is unchanged | Separates quote-safe sourcing from a PCB revision |
| Whether enclosure or fixture geometry changes | Prevents connector, service-hatch, battery, and focal-distance breakage |
| MOQ, quoted unit price, and availability basis | Keeps vendor data out of `REFERENCE-MANUFACTURERS.md` until it is actually verified |
| Serviceability impact | Accessibility depends on repairable hardware, not sealed disposable assemblies |
| Editable source they will return for DFM changes | Keeps CERN-OHL-S 2.0 source disclosure complete |

Safe substitutions are still not vendor commitments. They are only
quote-permissible candidates until a real quote, batch, or direct verification
exists.

## USB-HUB Send-Now Substitute Guidance

| Ref / subsystem | Active part | Safe within current quote | Footprint-breaking or contract-breaking | First factory ask |
|---|---|---|---|---|
| `U1` MCU/radio/USB | `ESP32-S3-MINI-1-N8` | `ESP32-S3-MINI-1-N4R2` only if the same module footprint is preserved and firmware flash/PSRAM assumptions are called out | ESP32-C3, ESP32-S3-WROOM, no-native-USB modules, or any module with different antenna keep-out / pads | Can you source the active `N8` module? If proposing `N4R2`, confirm exact footprint match, flash/PSRAM delta, and whether the firmware build must change. |
| `J1` USB connector | SOFNG `USB-05`, LCSC `C112454`, USB-A male plug | Same manufacturer/drawing, same shell-tab and through-hole geometry | USB-A receptacle, USB-C connector, cable/pigtail, connector with different shell tabs, or any part that invalidates the direct-plug nose / `MH1` / `MH2` clamp path | Can you source the SOFNG `USB-05` and confirm the manufacturer drawing is USB-A male? If not, send the drawing before quoting assembly. |
| `U2` regulator | RT9080-33GJ5, SOT-23-5 | Same RT9080-33GJ5 footprint from another distributor | AP2112K-class high-Iq regulator, DFN, SOT-89, pinout changes, or a regulator that changes required output-cap stability | Can you keep RT9080? If proposing an LDO substitute, state quiescent current, required output capacitor, pinout, and package. |
| `D1` USB ESD | USBLC6-2SC6, SOT-23-6 | USB 2.0-capable USBLC6-2SC6 with same pinout and package from a verified channel | Rail-less two-line shunt without the intended VBUS branch, different pinout, different package, or a part placed away from connector entry | Can you assemble USBLC6-2SC6 at the connector entry? If substituting ESD, provide capacitance, pinout, and whether VBUS protection remains. |
| `R1A` / `R1B` USB series resistors | 22R, 0402 | Commodity 22R 0402; 33R only if the factory explicitly recommends it for ESP32-S3 USB routing and notes the change | Omitting the resistors, moving them to the connector side of ESD, changing package, or choosing values without USB bring-up rationale | Which value do you recommend for this route length, 22R or 33R, and will you keep them near the ESP32-S3 module side? |
| `C2` / `C3` power capacitors | `10uF` 0603 input, `1uF` effective 0603 output | Commodity X5R/X7R 0603 MLCCs with adequate voltage rating and effective capacitance | Silent 0402 downsizing, low-voltage derating that loses effective capacitance, or changing `C3` below RT9080 stability needs | Which exact MLCCs would you place, and what effective capacitance do you expect at operating bias? |
| `C1`, `LED1`, `R2`, passives | Commodity 0402 support parts | Same value/package commodity substitutions | Package changes that crowd service pads, move the LED out of the visible/serviceable location, or hide status indication behind opaque enclosure material | Can you source these as standard commodity parts while preserving service-pad and enclosure visibility constraints? |
| `SW1` recovery control | Service-size tactile or pad-actuated recovery path | Smaller tact or pad-actuated `BOOT_N` path if the service hatch still reaches it | A large 6x6 tact that breaks the board envelope, hidden switch, destructive access, or omitting recovery pads | What switch or pad-actuated service method do you recommend, and can it remain reachable through the service hatch? |
| PCB / enclosure retention | Stepped USB-A dongle with `MH1` / `MH2` clamp load path | DFM tweaks that keep direct-plug topology, antenna keep-out, service hatch, and reversible clamp hardware | Adhesive, potting, heat-shrink-only retention, brass inserts near antenna, cable-only topology, or connector retention that relies on solder joints alone | What connector-retention and shell-clamp risks do you see before physical coupon evidence exists? |

## USB-HUB Accessibility And Serviceability Non-Negotiables

- The hub must remain a local BLE/USB HID bridge with no cloud dependency.
- The USB connector must be replaceable without discarding the enclosure.
- The enclosure must reopen; glue, potting, sealed heat-shrink, and destructive
  service paths fail the active lane.
- `MH1` / `MH2` remain the explicit connector load path unless the packet is
  revised.
- The service hatch must expose `EN`, `BOOT_N`, UART, power, ground, and trace
  access pads without pulling on the USB connector.
- The ESP32-S3 antenna keep-out must stay free of copper, metal enclosure
  features, and nearby clamp hardware.
- Adjacent-port clearance for the stepped USB-A nose and wider body is an
  open evidence item, not an assumption.

## R30 Annex Substitute Guidance

The R30 ring is not a fabrication quote path in this exchange. Ask for DFM,
costability, assembly, serviceability, and sourcing feedback only.

| Ref / subsystem | Active part | Safe for annex discussion | Footprint-breaking or contract-breaking | First factory ask |
|---|---|---|---|---|
| `U1` MCU/radio | `ESP32-C3-MINI-1-N4` | `ESP32-C3-MINI-1-N4X` or `ESP32-C3-MINI-1-H4` with same pinout; prefer `N4X` for new revisions | ESP32-C3 modules with different footprint, antenna, flash contract, or enclosure keep-out | Can you source `N4X` and confirm same footprint as `N4`? |
| `U2` optical sensor | PAW3204DB-TJ3L sensor/lens/emitter kit | Exact PAW3204 kit with matched lens/emitter | ADNS-2080 as a drop-in, YS8205, generic USB mouse ICs, or unverified SPI optical sensors | Can you source PAW3204 as a matched kit? If not, what proof would you need before an ADNS-2080 board profile is evaluated? |
| `LENS1` optical stack | Sensor-specific lens and clip | Lens/clip matched to the exact PAW3204 package | Bare sensor without a known lens/emitter path, mismatched aperture or focal geometry | Can you identify the lens/emitter kit as one sourced assembly, not loose uncertain parts? |
| `BT1` / `J_BAT` battery service | Protected `80-100 mAh` LiPo with replaceable service harness to off-board pads | Protected pouch cell within shell size, integrated PCM, UN 38.3 paperwork, replaceable harness or fixture tail | Harvested cells, direct cell-tab solder, no PCM, no paperwork, or inaccessible battery removal | Can you supply a protected cell and harness path that keeps battery replacement possible without soldering the cell? |
| `U3` charger | TP4054 SOT-23-5 | LTC4054ES5-4.2 if same pinout/footprint is confirmed | MCP73831 without layout change, TP4056 SOP-8, charger with no explicit charge-current review | Can you keep TP4054/LTC4054 pinout? If proposing MCP73831 or TP4056, treat it as a board revision, not a BOM swap. |
| `U4` regulator | RT9080-33GJ5 SOT-23-5 | Same RT9080; XC6220B331MR only as a documented low-power downgrade | AP2112K, different pinout, larger package, or regulator with sleep current that breaks ring battery assumptions | Can you source RT9080? If proposing XC6220, call out the higher Iq and battery-life impact. |
| `D1` USB service ESD | TPD2E2U06DCK-class rail-less dual TVS, SC-70/SOT-323 | Same pinout/package low-capacitance rail-less dual data-line shunt | USBLC6-style VBUS-clamped topology, different pinout, or package changes in the service-edge pocket | Can you source an equivalent rail-less SC-70/SOT-323 data shunt without adding a VBUS branch? |
| `SW1` click element | 5mm 150-250gf snap dome class | Commodity 4-6mm dome in the same force range after actuator review | Hidden, too-stiff, non-replaceable, or package changes that break limited-mobility click ergonomics | Which dome options can you source, and can replacement remain possible through the serviceable shell? |
| Service interfaces | Off-board USB pads and battery service pads | Fixture/pogo/harness improvements that keep same nets and access | Onboard USB-C body, direct soldered cell, or fixture path that blocks shell reopening | Can you manufacture/test the off-board service pad approach, and what fixture geometry would you recommend? |
| Shell / focal-distance system | Raised rim plus UHMWPE glide pads | Material/process suggestions that preserve focal gap and replaceable pads | Sealed shell, glued pads with no service path, rim changes that invalidate focal-distance evidence | Which shell tolerances, pad material, and inspection checks would you require before quoting ring assembly? |

## R30 Accessibility And Serviceability Non-Negotiables

- Battery service must be replaceable through a harness or fixture; direct cell
  soldering is rejected.
- The shell remains reopenable and non-hermetic through the service-lid path.
- The raised rim and glide pads are accessibility hardware because they control
  focal distance and glide effort; do not casually merge them into an
  unserviceable shell feature.
- The optical surface claim stays honest: opaque rigid surfaces only today,
  not glass.
- The ring is parametric for finger circumference; no fixed one-size shell
  assumption should enter a factory response.
- `ADNS-2080` is a future evaluated board profile, not a PAW3204 drop-in.
- `YS8205`-style integrated USB mouse controllers are not substitutes.

## First Factory Exchange Questions

Send these as the first response checklist after the introductory email:

1. For `USB-HUB`, can you quote PCB fabrication and assembly from the current
   source packet while also giving connector/enclosure DFM notes?
2. Which `USB-HUB` BOM lines would you substitute, if any? For each, provide
   manufacturer, MPN, package, supplier link, MOQ, footprint impact, and
   serviceability impact.
3. Can you source and assemble the SOFNG `USB-05` male USB-A direct-plug
   connector, and do you see any shell-tab, solder-joint, or enclosure-clamp
   risk in the current direct-plug topology?
4. What would you change in the `MH1` / `MH2` clamp path, service hatch, or
   enclosure material while keeping the hub reopenable?
5. Can you keep the ESP32-S3 antenna keep-out clear of copper, metal, screws,
   brass inserts, and enclosure features?
6. What test or fixture access do you need for `EN`, `BOOT_N`, UART, power,
   ground, USB D+/D-, and trace access during assembly?
7. Are you willing to return editable KiCad/OpenSCAD or equivalent source for
   any DFM modifications you make, so CERN-OHL-S 2.0 disclosure remains
   complete?
8. If you accept the R30 annex, can you review it only as DFM/pre-fab input and
   clearly avoid quoting ring fabrication until physical fit/stackup evidence is
   closed?
9. For R30, can you source the PAW3204 sensor/lens/emitter kit and protected
   `80-100 mAh` cell with replaceable harness? If not, which exact alternatives
   do you propose, and which ones require a board or shell revision?
10. What information should be added to the packet before your next response:
    fabrication outputs, centroid/BOM format, enclosure STLs, fixture drawings,
    physical coupon observations, or translated notes?

## Verification Notes

- Read against `docs/sensors-converge-2026/SHENZHEN-PAIRING.md`: this sheet
  keeps Shenzhen focused on supply-chain depth, component cost, factory access,
  and Seeed-style first-batch scoping.
- Read against `docs/VENDOR-VERIFICATION.md`: `USB-HUB` has no elevated
  component-sourcing risk today; R30's elevated risk is the PAW3204 kit path,
  with charger/regulator/Hall-style caveats already documented at part level.
- Read against `docs/REFERENCE-MANUFACTURERS.md`: Seeed, JLCPCB, PCBWay, and
  local prototype houses remain placeholders until quote/run data exists.
- Read against `hardware/bom/USB-HUB.csv` and
  `hardware/shared/USB-HUB/kicad/P0-COMPONENT-LOCKS.md`: USB-HUB substitutions
  are centered on `U1`, `J1`, `U2`, `D1`, USB resistors, power capacitors,
  recovery control, and the direct-plug enclosure clamp path.
- Read against `hardware/bom/R30-OLED-NONE-NONE.csv` and
  `hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md`: the annex guidance keeps
  PAW3204, battery service, charger, low-Iq regulation, rail-less USB ESD,
  focal-distance hardware, and service interfaces explicit.
- No new vendor claim, quote, availability claim, yield, realized COGS, or
  endorsement is created by this sheet.

## Recommended Followup

Use [`SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`](SHENZHEN-FACTORY-RESPONSE-CAPTURE.md)
as the factory-return form that mirrors this sheet as fillable tables for
proposed substitutions, DFM requests, source-return posture, quote-vs-verified
status, and explicit BDFL decisions. The form is included in the quote export so
every factory response can be pasted back into the repo without turning
ambiguous email prose into hidden design decisions.
