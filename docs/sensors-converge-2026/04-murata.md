<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Murata — Tier S

**What they make:** Pre-certified BLE / Wi-Fi / cellular modules built around
third-party silicon (Nordic nRF52, NXP, Cypress/Infineon, ST). Inductors,
capacitors, oscillators, RF components. The "module that ships with FCC ID
already on the label" vendor.

## Why they matter for PowerFinger

A Murata pre-certified BLE module is the single highest-leverage regulatory
de-risk available on the floor. It collapses a six-figure cost item.

- **FCC / CE / RED / IC / TELEC pre-certification:** Module-level certification
  inherits to the end product if the module's antenna and PCB integration
  rules are followed. This means:
  - The PowerFinger BOM no longer requires a full intentional-radiator
    certification campaign — only an unintentional-radiator (FCC Part 15B)
    test, which is dramatically cheaper.
  - Saves $10k–$50k+ in cert costs at Tier 3 / Tier 4 manufacturing per
    `../REFERENCE-MANUFACTURERS.md`.
  - Saves 2–3 months of cert-lab queue time.
- **nRF52840-based Murata modules** (e.g. Type 2DL, Type 2BL) bring the
  documented production-target silicon in a pre-certified, smaller, antenna-
  matched package. The BOM cost goes up vs. raw nRF52840 chip, but the
  regulatory cost goes way down.
- **Antenna matching is solved.** Custom antenna design on a flex-PCB ring is
  one of the highest-risk parts of going from prototype to production. A
  module with an integrated ceramic chip antenna eliminates this.

## The ask

1. **Minimum viable:** Murata's BLE module selection guide + FAE contact for
   antenna integration questions on flex PCB. Both are routine asks.
2. **Stretch:** Sample Type 2DL (or equivalent nRF52840 Murata module) with
   an explicit acknowledgment that it can be designed into an open-source
   reciprocally-licensed product. The licensing posture is unusual enough that
   getting it on the record matters.

## What you bring

- An open-hardware reference design that uses a Murata module, with
  publication of the integration files (footprint, antenna keepout, sample
  circuit) under CERN-OHL-S. Murata's customers are mostly closed-source
  consumer-electronics OEMs; PowerFinger would be a visibility win in the
  open-hardware / accessibility / wearable segment they don't currently
  showcase.
- Public power-budget validation that the Murata module hits the documented
  nRF52840 numbers in a real wearable form factor — useful data their other
  customers can cite.

## Conversation cues

- If the booth shows **Type 2DL / 2BL / 2EL** (nRF52-based modules), you're
  in the right place.
- If the booth foregrounds **automotive RF / 5G** — wrong day, ask for a
  wearable-vertical or BLE-module FAE.
- Look for **module selection guides** as printed handouts; grab one.

## Risks / why they might say no

- Module BOM cost is roughly 2–3x the raw nRF52840 chip cost (~$5–8 per module
  vs. ~$2–3 for the raw chip). At PowerFinger's $25 BOM ceiling this is
  consequential — for the Standard tier, it would push BOM toward the ceiling.
  **However:** the ceiling exists for prototype validation; the consumer
  product can absorb the module premium in exchange for cert savings.
- Murata is a Japanese company with conservative IP-licensing posture. The
  reciprocal-license question may need explicit handling. Reassure them:
  CERN-OHL-S applies to PowerFinger's design files, not to Murata's module
  internals.
- Some Murata BLE modules require a Nordic SoftDevice license that has its
  own redistribution constraints. Surface this immediately if a Nordic-based
  module is the path.

## Minimum viable outcome from a 5-minute booth visit

- Module selection guide in hand.
- FAE contact email for the wearable / BLE module vertical.
- Verbal confirmation that an open-hardware reference design using their
  module is acceptable from their licensing posture.

## Post-show follow-up path

1. Email contact within 48 hours; attach one-pager; ask one specific
   integration question (e.g. "antenna keepout requirements for Type 2DL on
   a flex PCB ring with 8mm × 30mm footprint").
2. If positive, request module samples for a Standard-tier prototype iteration.
3. Document module-vs-chip BOM comparison in
   [`../CONSUMER-TIERS.md`](../CONSUMER-TIERS.md) once data is in hand.
4. Update [`../REGULATORY-PRESCAN.md`](../REGULATORY-PRESCAN.md) to reflect the
   pre-cert path if Murata is committed.

## Cross-references

- [`../REGULATORY-PRESCAN.md`](../REGULATORY-PRESCAN.md) — current FCC/CE
  prescan; pre-cert module collapses several sections.
- [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) Tier 3 — cert
  cost as a hard requirement at 500–10k unit volume.
- [`../NRF52840-MIGRATION.md`](../NRF52840-MIGRATION.md) — Murata module is the
  cheapest path to nRF52840 silicon in a small open-hardware project.
