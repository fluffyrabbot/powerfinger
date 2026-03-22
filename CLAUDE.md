# CLAUDE.md — PowerFinger

## Project Overview

PowerFinger is an open-source assistive input device family: fingertip rings and
pen-shaped wands that function as surface-agnostic BLE HID mice/controllers.
Hardware designs under CERN-OHL-S 2.0, firmware/software under MIT.

## Hard Rules

- **Accessibility-first.** Every design decision must work for users with limited
  mobility, not just able-bodied desk users. If a feature helps gamers but hurts
  accessibility, cut it.
- **BOM ceiling.** No single variant may exceed $25 BOM at prototype scale. The
  cheapest ring variant must stay under $10.
- **License compliance.** All hardware files: CERN-OHL-S 2.0. All firmware/software:
  MIT. Never introduce proprietary dependencies. Never use a permissive hardware
  license — the reciprocal requirement and patent retaliation clause are load-
  bearing for the project's IP defense strategy.
- **No cloud dependency.** Every device must function fully offline. Cloud/API
  features (LLM for OCR, etc.) are optional enhancements, never requirements.
- **No planned obsolescence.** Every component must be replaceable. Design files
  must include assembly/disassembly instructions. Use commodity parts with
  multiple source vendors.
- **Surface agnosticism is a spectrum.** Direct optical doesn't work on glass —
  that's fine, document it honestly. Don't overclaim. The combinatorics matrix
  tracks surface compatibility per variant.

## Repository Structure

```
apps/powerfinger/
  README.md              — Mission, philosophy, quick start
  CLAUDE.md              — This file (agent instructions)
  LICENSE-HARDWARE       — CERN-OHL-S 2.0 (hardware)
  LICENSE-SOFTWARE       — MIT (firmware, companion app)
  docs/
    COMBINATORICS.md     — Full form factor x sensing x options matrix
    IP-STRATEGY.md       — Patent landscape, defense strategy, prior art
  hardware/
    ring/                — Fingertip ring designs (by angle, by sensor)
    wand/                — Pen/wand designs
    shared/              — Common components (BLE module, battery, etc.)
  firmware/
    ble-hid/             — ESP-IDF BLE HID mouse profile
    sensors/             — Sensor drivers (Hall, optical, laser)
    power/               — Power management, sleep/wake
  companion/             — Phone/laptop companion app (Flutter or PWA)
```

## Build & Dev Commands

*Not yet established.* When firmware development begins:
- Target MCU: ESP32-C3 (ring, wand basic) or ESP32-S3 (camera variants)
- Framework: ESP-IDF (Apache 2.0)
- Build: `idf.py build`
- Flash: `idf.py -p /dev/ttyUSBx flash`

## Key Technical Constraints

- **BLE HID latency < 15ms.** ESP32-C3 supports 7.5ms connection intervals.
- **Ring height budget: ~10mm** finger-to-surface. Sensor + PCB + battery must
  fit in ~7–8mm above surface with 2.5–3mm air gap below.
- **Optical sensor focal distance: 2.4–3.2mm.** Ring needs standoff feet or
  raised rim to maintain this.
- **Power target:** 1–2 weeks battery life on 80–100mAh (ring), 2–4 weeks on
  100–150mAh (wand).

## Design Philosophy References

- RFC-1318 (MeSH repo): OSS OCR thimble/wand scanner — sibling concept, OCR
  pipeline architecture
- RFC-1319 (MeSH repo): Fingertip ring mouse — full sensing analysis, IP
  landscape, multi-ring paradigm

## Code Smell Checklist

Same as MeSH project (see parent CLAUDE.md). Additionally:
- Never hard-code DPI/sensitivity — always configurable via BLE characteristic
- Never assume a specific hand size — parameterize ring geometry
- Never assume a specific surface — test on at least: wood desk, glass, fabric,
  paper, glossy magazine, matte plastic
- Never assume single-ring operation — firmware must be stateless enough for
  multi-ring composition via companion app

## Defensive Publication

Every hardware design committed to this repo constitutes a defensive publication
establishing prior art. Commit messages for new designs MUST include:
- The specific sensing mechanism
- The form factor and angle
- The intended use case (pointing, OCR, accessibility)
- The BOM estimate

This creates timestamped, enabling disclosure searchable by patent examiners.

## Pointers

- Patent landscape: `docs/IP-STRATEGY.md`
- Design matrix: `docs/COMBINATORICS.md`
- CERN-OHL-S 2.0: https://opensource.org/license/cern-ohl-s
- ESP-IDF BLE HID: https://github.com/espressif/esp-idf/tree/master/examples/bluetooth
