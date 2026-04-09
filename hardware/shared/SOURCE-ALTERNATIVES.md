<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Source Alternatives Baseline

This file summarizes the cross-variant source strategy already implied by the
current BOM intent files. It exists so the first PCB and CAD drops do not
accidentally collapse onto single-source parts or non-serviceable substitutions.

This is a sourcing baseline, not a permission slip to swap parts without
re-checking fit, current draw, thermal safety, or accessibility impact.

## Rules

- Prefer components with at least two realistic procurement paths.
- If a substitute changes current draw, pinout, or package, treat it as a design
  change and update the packet manifest.
- For body-worn variants, battery and charging substitutions must still satisfy
  [docs/BATTERY-SAFETY.md](../../docs/BATTERY-SAFETY.md).
- Do not substitute cheaper parts that break deep sleep, serviceability, or
  sensor honesty just to shave BOM cost.

## Common Components

| Subsystem | Preferred part | Acceptable alternatives | Do not substitute casually | Why it matters |
|-----------|----------------|-------------------------|----------------------------|----------------|
| Ring / wand MCU module | `ESP32-C3-MINI-1-N4` | `ESP32-C3-MINI-1-N4X`, `ESP32-C3-MINI-1-H4` | Modules with different keep-out, flash floor, or antenna geometry | Firmware lane is already built around ESP32-C3 timing and power behavior |
| Hub MCU module | `ESP32-S3-MINI-1-N8` | `ESP32-S3-MINI-1-N4R2` | S3 parts without native USB OTG exposure | The hub must remain a USB HID + BLE central without external USB silicon |
| Ring optical sensor | `PAW3204DB-TJ3L` | `ADNS-2080` class sensors | USB mouse controller chips marketed as “optical sensors” | Surface behavior and lens stack depend on a real standalone tracking sensor |
| Ring / wand Hall sensor | `DRV5053VAQDBZR` | `SS49E` with explicit power-budget re-check | Pin-incompatible Hall parts dropped in by footprint hope | Hall current dominates the ball+Hall power lane |
| Charge controller | `TP4054` | `LTC4054ES5-4.2` | `TP4056` or `MCP73831` without board re-layout | Pinout and thermal behavior matter in tiny enclosures |
| LDO | `RT9080-33GJ5` | `XC6220B331MR` with documented sleep penalty | `AP2112K`-class high-Iq regulators | Deep-sleep current is load-bearing for accessibility and battery life |
| USB ESD protection | Low-capacitance 2-channel USB TVS array | Equivalent 2-line USB ESD arrays from major vendors | Leaving USB data unprotected because the board is “just a prototype” | USB service and immunity realism matter before layout hardens |
| USB-C sink attach resistors | 5.1kΩ pull-downs on `CC1` / `CC2` | Any accurate commodity `5.1 kohm` resistors | Ad-hoc values or omitted CC resistors on a receptacle design | USB-C service/debug paths should enumerate honestly and attach cleanly |
| Ring / wand battery | 3.7V LiPo with integrated PCM and UN 38.3 docs | Alternate protected pouch cells within envelope and charge-rate limits | Bare cells, harvested cells, or undocumented packs | Replaceability cannot come at the cost of unsafe charging or transport ambiguity |
| Dome click | `Snaptron SQ-05400N` | Similar 4–6 mm domes from Murata or C&K | Adhesive-only switches that force destructive shell entry | Click feel and replacement path both matter |
| Wand barrel switch | `Kailh GM8.0` | `Omron D2FC-F-7N` class switches | Single-source boutique switches | Wand repair needs commodity mouse-switch replacements |
| Shell material | Nylon / POM for wear surfaces, PLA/PETG for rough prototypes | Any material with honest thermal and wear notes | Brittle or skin-unfriendly materials presented as final | Surface glide and skin-contact safety must stay truthful |

## Shared Procurement Notes

- ESP modules should be sourced from major distributors or directly traceable
  module vendors, not mystery relabel stock.
- Batteries should be spec-verified before enclosure lock-in: PCM present, size
  envelope measured, and vendor UN 38.3 summary archived with the packet.
- Where AliExpress is listed, treat it as a prototype-procurement lane, not the
  only acceptable source. The packet should keep at least one distributor-grade
  fallback wherever possible.
