<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen Pairing — Coordinating the Two Trips

If a Shenzhen visit is on the 2026 calendar, the Sensors Converge floor walk
shrinks. Some Tier S booths become deferrable because their core value
(supply chain, component cost, factory access) is better served by a direct
Shenzhen visit. Others stay must-visit because their value (speculative-
product programs, app-note depth, US-side relationships) does not exist in
the Shenzhen channel.

This doc triages the two trips against each other so neither one duplicates
the other.

## The principle

The two trips have different roles:

- **Sensors Converge (US):** Western relationships, speculative-friendly
  silicon programs, partner-program enrollment, app-note authors, academic
  context, BLE / fusion / DSP IP licensing posture, the soft-contact email
  pipeline that converts into design-in conversations months later.
- **Shenzhen:** supply-chain depth, component cost, pre-cert module
  factory access, contract-manufacturer scoping, the actual COGS that lands
  in [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md), and
  fast iteration on physical prototypes via local board houses.

Almost every booth at Sensors Converge falls into one of those two
categories. The pairing question is: for each vendor, does their value live
in the *relationship* (Sensors Converge) or in the *supply chain* (Shenzhen)?

## Per-vendor pairing decisions

### Skip at Sensors Converge — Shenzhen replaces cleanly

| Vendor | Shenzhen replacement | Rationale |
|---|---|---|
| **Murata** | RF-star, Minew, Feasycom, SKYLAB factory visits | Pre-cert BLE modules at ~half the cost. Antenna integration docs are usable. FCC-certified parts confirmed (Minew MS48SF2 has FCC ID 2ABU6-MS48SF2). The biggest single Shenzhen-substitution win — turns the highest Tier S booth into a Shenzhen target. |
| **EM Microelectronic** | Telink (TLSR8258 family) | Telink's 1µA deep-sleep with SRAM retention is in the same conversation as EM9305 for "good enough" coin-cell BLE. Dramatically cheaper. Visit Telink in Shenzhen instead. |
| **Mouser** | LCSC (Shenzhen), Huaqiangbei in person | LCSC is the Mouser-equivalent for Chinese silicon. For Western parts, ordering online suffices — no booth visit needed. Skip the Mouser booth entirely. |
| **Excelitas** | Vertilite (VCSEL), broad Shenzhen photodiode supply | Only relevant if guidance laser activates; Shenzhen photonics covers consumer-tier needs. Class-1 cert is the gating concern, but that's true of the Western part too. |

### Visit at Sensors Converge — Shenzhen alternative is partial / inadequate

| Vendor | Why Sensors Converge still matters |
|---|---|
| **STMicroelectronics** | ST has a Shenzhen presence, but the **speculative-friendly lane** (Partner Program, developer relations, accessibility-flavored co-marketing, free dev kits for low-volume open-source) lives in the Western channel. Shenzhen ST sales offices are oriented toward volume customers. |
| **Microchip** | Same logic. The Atmel-lineage maker culture, MASTERs Conference contacts, and educational-program engagement only happen via the Western booth. |
| **Bosch Sensortec** | The **BSX fusion library licensing** question must be answered authoritatively, and the only place to get a verbal answer on the record is from a Bosch rep at the Western booth. Chinese IMU substitutes exist (QST QMI8658) but BHI's smart-sensor-hub story has no clean equivalent. |
| **Analog Devices** | ADI's value is **app notes and academic engagement**, not component cost. Chinese precision-analog vendors are closing the parts gap but cannot replicate the documentation depth. ADI booth is for tribal knowledge and FAE pointers, not parts. |
| **Edge Impulse** | Fully SaaS — no Shenzhen presence relevant. Booth visit is purely contact-warming for a future TinyML feature. |

### Visit at Sensors Converge but downgraded — Shenzhen has real alternatives

| Vendor | Notes |
|---|---|
| **TDK InvenSense** | QST QMI8658 covers the IMU substrate at the consumer tier; ICM-42688's gyroscope precision is hard to match. Visit TDK only if Bosch BSX licensing question goes badly and you need a fallback IMU partner. |
| **ams OSRAM** | Sensortek and Vertilite cover proximity / VCSEL at the consumer tier. Visit ams only if the wand-tip proximity or guidance-laser feature becomes active. |
| **Ceva** | Not replaceable; not visited anyway. Reconnaissance only at either trip. |

## Revised Sensors Converge floor walk if Shenzhen is committed

The 12-booth list shrinks to **5 must-visits** plus 2 conditional:

1. **ST** — Tier S; speculative-program lane; no Shenzhen substitute
2. **Microchip** — Tier S; speculative-program lane; no Shenzhen substitute
3. **Bosch Sensortec** — Tier S; BSX licensing on the record + BHI roadmap
4. **ADI** — Tier A; app-note depth and academic engagement
5. **Edge Impulse** — Tier B contact-warming only

Conditional:
- **TDK InvenSense** — only if Bosch BSX licensing closes the BHI path
- **ams OSRAM** — only if guidance-laser or proximity feature reactivates

This collapses 3 days of floor work into approximately 1 day of focused
Tier S conversations.

## Shenzhen targets to scope

Replace the deferred booth conversations with these visits / inquiries on
the Shenzhen trip:

### Pre-cert BLE module factories (Murata replacement)

- **Shenzhen RF-star Technology Co., Ltd.** — TI CC2652P-based modules, FCC
  / CE / BQB / SRRC certified, +20 dBm PA on some lines. Reachable in
  Shenzhen.
- **Shenzhen Minew Technologies Co., Ltd.** — broad BLE module catalog,
  multiple FCC IDs on file, used in many Western consumer products.
- **Feasycom (Shenzhen)** — BLE modules, audio-focused but BLE HID-capable.
- **Ai-Thinker (Shenzhen)** — Espressif-based modules; useful since the
  current prototype is on ESP32-C3.
- **SKYLAB (Shenzhen)** — broader IoT module catalog, BLE included.

For each: scope antenna keepout rules for ring/wand form factors, MOQ,
cert reuse posture, sample availability for sub-100-unit eval runs.

### Low-power BLE silicon (EM replacement)

- **Telink Semiconductor (Wuxi HQ, Shenzhen sales)** — TLSR8258 / TLSR9 family.
  TLSR8258 deep-sleep at 1µA with SRAM retention. Documentation in English
  is workable but thinner than Nordic's.
- **Phy+ (Shenzhen)** — PHY6222 BLE SoC. Even lower-cost; tooling is
  immature.
- **Bouffalo Lab (Shenzhen)** — BL616 / BL602 BLE+Wi-Fi combo. Newer; active
  community.

### IMU substitutes (Bosch / TDK fallback)

- **QST Corp (Wuxi)** — QMI8658 IMU. Confirmed real, used in budget wearables.
  Lower-spec than BMI270's wearable-optimized features (no on-chip step
  counter / activity recognition); fine for raw 6-DoF and host-side fusion.
  Distribution via LCSC and direct.

### VCSEL / photonics (ams / Excelitas substitutes)

- **Vertilite (Sunnyvale HQ, mainland-China roots)** — 940nm VCSEL arrays,
  used in face-auth devices. Class-1 eye-safety lines exist; verify per
  part.
- **Sensortek (Taiwan, Shenzhen distribution)** — proximity, ALS. Replaces
  ams TMD-series for consumer-tier proximity.

### Distribution / parts cash-and-carry

- **LCSC (Shenzhen)** — Mouser-equivalent for Chinese silicon. Use for
  initial parts kits when validating the Chinese-substitute BOM.
- **Huaqiangbei (markets in Futian)** — physical electronics market for
  cash-and-carry of Chinese parts. Useful for one-off prototype parts when
  shipping is too slow. Document any vendor visited per the conventions in
  [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md).

### Contract manufacturers

- **Seeed Studio (Shenzhen — Bao'an)** — already named in
  [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) as the
  candidate for first-batch production. Tour the Seeed Fusion facility and
  scope the Propagate productization program in person. Use
  [`SHENZHEN-SEEED-QUOTE-PACKET.md`](SHENZHEN-SEEED-QUOTE-PACKET.md) as the
  current starter packet: hub quote first, ring DFM/pre-fab review until the
  ring PCB DRC is closed.
- **JLCPCB / PCBWay (Shenzhen)** — PCB fab + assembly at small scale.
- **Local Bao'an / Longgang prototype houses** — for cases where Seeed is
  the wrong scale.

## Things to verify in Shenzhen that you cannot verify from the US

- **Actual COGS** vs. quoted price (the column
  [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) explicitly
  asks for "realized" not "quoted").
- **Defect rates** at low volume — Chinese vendors often have a "first-batch
  yield reality" that doesn't appear in marketing material.
- **IP-handling reputation** — does the factory respect customer designs or
  do they re-sell them? This is the load-bearing CERN-OHL-S question for
  Chinese vendors.
- **Communication channels** — many Chinese factories prefer WeChat over
  email. Documenting their preferred channel is half the operational
  battle.
- **Payment terms** — Western customers often expect Net-30; Chinese
  factories typically ask for 30/70 or 50/50 split with TT.
- **Sample turnaround in person** — bringing a board to a fab in person can
  shrink iteration time from weeks to days.

## CERN-OHL-S compatibility — the unaddressed reality

Chinese factories typically have **no public posture** on reciprocal-license
handling. The license is foreign, unfamiliar, and has no precedent in their
customer base. Two practical implications:

1. **You will be pioneering.** Most factories will neither understand nor
   care about CERN-OHL-S; their concern is whether you'll pay on time and
   whether they can reuse your design for other customers (which the license
   actually permits, with reciprocity). Don't expect informed counsel from
   the factory side.
2. **The reciprocal obligation runs through the manufacturer.** Per the
   conveyance terms, a factory that ships PowerFinger-derived units must
   also publish their modifications under CERN-OHL-S. Most Chinese factories
   will not understand this obligation, which means **you may need to
   provide the disclosure on their behalf**, or insist via contract that
   any modifications they make come back to you for upstream publication.
   Document this in any contract.

This is not insurmountable, but it is *invisible* friction that doesn't
appear in any other open-hardware project's playbook — most open-hardware
projects ship under permissive licenses or use OSHWA-certified Western
manufacturers. PowerFinger's reciprocal posture is unusual enough to
require explicit handling.

## Pre-Shenzhen prep checklist

- [ ] Read [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md)
      end-to-end. Pre-fill empty rows with the vendors above so the trip
      adds verified data, not speculation.
- [ ] Read [`SHENZHEN-SEEED-QUOTE-PACKET.md`](SHENZHEN-SEEED-QUOTE-PACKET.md)
      and keep the packet status honest: the active hub is quoteable as a
      first-board candidate, while the active ring is DFM/pre-fab review until
      its DRC/unconnected blockers are cleared.
- [ ] Use [`SHENZHEN-FIRST-CONTACT-TEMPLATE.md`](SHENZHEN-FIRST-CONTACT-TEMPLATE.md)
      for the first Shenzhen / Seeed outbound message so the contact starts
      from the same hub-quote / ring-review boundary as the packet.
- [ ] WeChat account with English profile + project-neutral handle. Most
      factory communication will route through WeChat after first contact.
- [ ] Translated one-pager (Chinese, simplified):
      [`SHENZHEN-FACTORY-ONE-PAGER.zh-CN.md`](SHENZHEN-FACTORY-ONE-PAGER.zh-CN.md).
      The Western one-pager is not appropriate for factory floor conversations;
      specs and BOM in Chinese on a single page is what's expected.
- [ ] Sample physical artifact (3D-printed shell or populated PCB) — same
      principle as the FLOOR-PLAYBOOK, except in Shenzhen the artifact is
      a quote-enabling document, not a pitch device.
- [ ] Pre-paid local SIM with mobile data; Western roaming is too slow for
      WeChat / map-app workflows in Shenzhen.
- [ ] Hotel near Huaqiangbei (Futian) or near Bao'an (Seeed area) depending
      on emphasis.

## Sequencing the two trips

If Sensors Converge is May (already done) and Shenzhen is later in 2026:

1. **Sensors Converge** runs in soft-contact mode (per
   [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md)) for the 5 must-visit
   booths.
2. **Between trips:** complete the BOM evaluation against Chinese-substitute
   parts. The follow-up email to Tier S Sensors Converge contacts can
   reference "evaluating against Chinese-substitute parts in parallel" as
   honest framing.
3. **Shenzhen trip** validates the Chinese-substitute BOM against actual
   factory quotes and physical samples.
4. **Post-Shenzhen:** the data-driven follow-up to Western partners is
   either "we're going Chinese-substitute, not engaging further" or
   "Chinese-substitute didn't pan out for [reason]; let's talk dev kit."
   Either is a real conversation; soft-contact gave you the license to
   have it.

If Shenzhen comes first:

1. **Shenzhen** establishes the Chinese-substitute BOM and factory
   relationships.
2. **Sensors Converge** soft-contact visits become *informed* — you can
   tell ST that you've already priced WBA52 against Bouffalo BL616, which
   sharpens the conversation considerably.

## Cross-references

- [`README.md`](README.md) — top-level triage; pairing decisions modify the
  Tier S / Tier A list.
- [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md) — the posture for the
  remaining must-visit booths at Sensors Converge.
- [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) — where
  Shenzhen factory data lands. Pre-fill rows before the trip; populate
  realized COGS after.
- [`../IP-STRATEGY.md`](../IP-STRATEGY.md) — CERN-OHL-S posture; the gap
  in Chinese-factory understanding is documented above and should feed
  back into the IP strategy doc if patterns emerge.
- [`../CONSUMER-TIERS.md`](../CONSUMER-TIERS.md) — the BOM target the
  Chinese-substitute exercise is validating against.
