<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Sensors Converge 2026 — Partnership Outreach Triage

**Event:** May 5–7, 2026, Santa Clara Convention Center.
**Status of this doc:** Self-facing prep notes. Not for external distribution.

This directory triages confirmed-exhibiting companies into outreach tiers and
gives a one-page strategy per company. The goal is to walk the floor with a
ranked target list, a known posture per booth, and a clear post-show plan.

## Recommended posture for the 2026 show: SOFT CONTACT

The project is pre-prototype. The GitHub repo is too rough to show without
forming an impression that's hard to overwrite later. There is no working
ring and no one-pager you'd hand to a stranger.

**Read [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md) first.** It is the
current default for the 2026 show. The reframe: information visit, not pitch
visit. Booth conversations exist to acquire named contacts and a justification
to email them later, when the project is ready. The conversion event is a
follow-up email weeks or months out — not a booth pitch.

[`FLOOR-PLAYBOOK.md`](FLOOR-PLAYBOOK.md) describes the pitch-mode posture and
becomes the right doc only when (1) there's a working ring, (2) there's a
one-pager you'd be proud of, (3) the GitHub README is presentable, and (4)
at least one consumer-tier BOM has validated power numbers. Use it at a later
event, not at Sensors Converge 2026.

The triage, floor-walk order, and per-vendor docs below apply to **both**
postures. Only the booth-conversation behavior differs.

If a **Shenzhen visit is also on the 2026 calendar**, read
[`SHENZHEN-PAIRING.md`](SHENZHEN-PAIRING.md) before finalizing the floor
plan. Several Tier S booths (Murata, EM Microelectronic, Mouser, Excelitas)
become deferrable to Shenzhen because their value is supply-chain rather
than relationship. The shrunken Sensors Converge floor walk is 5 booths,
not 12.

If the Shenzhen / Seeed conversation needs a factory-facing artifact, use
[`SHENZHEN-SEEED-QUOTE-PACKET.md`](SHENZHEN-SEEED-QUOTE-PACKET.md). It is the
current repo-backed quote starter: hub quote first, ring DFM/pre-fab review
until the ring PCB DRC is closed.

## Caveats

- "Confirmed" here means surfaced in (a) the smallworldlabs A–F slice that is
  publicly fetchable or (b) Questex / Fierce Sensors press coverage of headline
  sponsors. The full directory is 150+ exhibitors; the M–Z range was not
  fetchable. Verify each booth assignment in the on-site mobile app before
  arriving.
- Three of the most directly load-bearing silicon partners — **PixArt**,
  **Nordic**, and **Espressif** — are not visible in the confirmed list.
  Search the on-site app for them; if absent, pursue them through their own
  partner programs after the show, not on the floor.
- This is a partnership-prospecting trip, not a customer-prospecting trip. The
  show's audience is engineers and component buyers; PowerFinger has no
  customers in that audience. The value is silicon, modules, distribution, and
  fab introductions — not orders.

## Pre-show prep checklist (soft-contact mode — current)

- [ ] **Read [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md) end-to-end** the
      day before. Internalize the opening lines and the graceful close.
- [ ] Personal business cards: name + email only. No project name, no GitHub
      URL. ~$25 from MOO / Vistaprint, 24-hour turnaround.
- [ ] Personal email separate from any project-tied identity.
- [ ] Pre-load the floor map and mark the 12 confirmed booths in two colors:
      Tier S (must-visit) and Tier A (visit if time).
- [ ] Pre-pick 1–2 specific technical questions per Tier S booth from the
      "Questions to ask at booths" section of `SOFT-CONTACT-MODE.md`.
- [ ] Memorize one of the three opening lines and the graceful close. Practice
      out loud five times.
- [ ] Paper notebook + pen. One page per conversation; capture name, email,
      what you asked, what they said, the date.

### Pitch-mode prep checklist (deferred — for a later event)

The following apply only when switching back to
[`FLOOR-PLAYBOOK.md`](FLOOR-PLAYBOOK.md), i.e. when the four readiness
conditions are met:

- [ ] Print 50 single-page project one-pagers (BOM, power numbers, surface
      compatibility table, license posture, GitHub URL). One side, no fluff.
- [ ] Bring a working ring or a non-functional 3D-printed shell with a
      populated PCB visible.
- [ ] Have a 30-second pitch and a 2-minute pitch rehearsed.
- [ ] Pre-write a follow-up email template per tier.

## Tier S — must visit, specific ask prepared

These four are the highest leverage on the floor. Each can replace multiple
BOM lines or collapse a regulatory step. Prepare a specific ask before going.

| # | Company | One-line angle | Doc |
|---|---------|----------------|-----|
| 1 | STMicroelectronics | STM32WB/WBA as full nRF52840 alternative + sensor portfolio + ST Partner Program | [01-stmicroelectronics.md](01-stmicroelectronics.md) |
| 2 | Microchip | PIC32CX-BZ / WBZ family BLE + active maker/edu program + design-in support at low volume | [02-microchip.md](02-microchip.md) |
| 3 | Bosch Sensortec | BMI / BHI series IMU for the IMU-variant ring; on-chip fusion = simpler firmware | [03-bosch-sensortec.md](03-bosch-sensortec.md) |
| 4 | Murata | Pre-certified BLE modules collapse FCC/CE/RED prescan; biggest regulatory de-risk | [04-murata.md](04-murata.md) |

## Tier A — visit if time, narrower fit

| # | Company | One-line angle | Doc |
|---|---------|----------------|-----|
| 5 | TDK InvenSense | ICM-series IMU + MEMS + piezo film for Pro click | [05-tdk-invensense.md](05-tdk-invensense.md) |
| 6 | Analog Devices | ADCs (piezo click), IMUs, ADuCM4050 BLE MCU; AppNote depth | [06-analog-devices.md](06-analog-devices.md) |
| 7 | ams OSRAM | Optical/proximity sensing IP; tip detection on the wand | [07-ams-osram.md](07-ams-osram.md) |

## Tier B — drive-by only if convenient

| # | Company | One-line angle | Doc |
|---|---------|----------------|-----|
| 8 | EM Microelectronic | EM9304/EM9305 ultra-low-power BLE; coin-cell wearable angle | [08-em-microelectronic.md](08-em-microelectronic.md) |
| 9 | Mouser Electronics | Distribution; "buy the BOM" merchandising / open-hardware showcase | [09-mouser.md](09-mouser.md) |
| 10 | Edge Impulse | TinyML for IMU gesture classification; speculative | [10-edge-impulse.md](10-edge-impulse.md) |
| 11 | Ceva | Sensor fusion IP licensing; probably wrong scale for the project | [11-ceva.md](11-ceva.md) |
| 12 | Excelitas | Photonics / VCSEL; only matters if a laser variant is pursued | [12-excelitas.md](12-excelitas.md) |

## Floor-walk order (recommended)

1. **Open with Murata.** Cheapest conversation to win and biggest immediate
   regulatory unlock. A pre-cert module is the kind of thing where five minutes
   with an FAE replaces a month of prescan work.
2. **Bosch Sensortec next.** They're the IMU vendor that matters; the BHI
   smart-sensor-hub story is exactly the kind of thing PowerFinger could
   showcase.
3. **STMicroelectronics and Microchip back-to-back.** These are the two big
   silicon platforms competing for your design. Visit them in sequence so you
   can compare what each is willing to offer for a design win.
4. **TDK + ADI + ams** in any order — Tier A, narrower fit, shorter visits.
5. **Tier B as drive-bys** if there's time on day 2 or day 3.

## What "win" looks like per conversation (soft-contact mode)

For each Tier S / Tier A booth, the minimum viable outcome is:

- **A named FAE or developer-relations email**, given willingly after a 2–5
  minute technical conversation. That email + the specific question you
  asked = your justification to follow up months later.
- **Their literature in hand** — dev kit catalog, partner-program brochure,
  app-note pointer.
- **One useful technical answer logged** in your notebook for reference (idle
  current numbers, BSX licensing posture, antenna keepout rules, etc.).

If a booth offers none of those, mark it "no path" and do not pursue post-show.

## What "win" does NOT look like

- A business card with no email exchanged in either direction (cards alone
  are weak signal; an explicit email handover is the contract).
- An "email our partnerships inbox" hand-off — that inbox is a dead letter
  office for projects at PowerFinger's scale.
- A booth conversation in which you ended up pitching the project despite
  the soft-contact plan. If this happens, log it but treat the contact as
  weakened — the next email needs to overcome the immature first impression.

### Pitch-mode "win" criteria (deferred)

What "win" looks like when in pitch mode (per `FLOOR-PLAYBOOK.md`) — for
reference at a future event:

- Named contact in their partner / design-win / maker program with willingness
  to take a follow-up.
- Free or sampled parts tied to a promise of public reference-design
  publication.
- FAE / app-engineer hours offered for a specific firmware or BOM question.
- Listing or featured-project placement in their open-hardware showcase or
  developer blog.

## Post-show (soft-contact mode)

- **Same night:** log every conversation into the per-vendor doc's "Sensors
  Converge 2026 booth notes" section. Capture name, email, what you asked,
  what they said, the date.
- **Triage:** mark Tier S contacts as starred for follow-up; Tier A as
  "maybe"; Tier B as archive-unless-engaged.
- **Schedule the follow-up.** Calendar reminder for the date you expect to
  be ready. Do **not** send same-night "nice to meet you" emails — a
  content-free email weakens the future warm follow-up.
- **The follow-up email is the conversion event** — see
  [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md) for the template.
- Update [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) and
  [`../OUTREACH.md`](../OUTREACH.md) with anything learned that updates the
  vendor stack or the partner pipeline.

### Pitch-mode post-show (deferred)

For reference at a future event in pitch mode (per `FLOOR-PLAYBOOK.md`):

- Within 48 hours: email each Tier S contact with a recap, the one-pager
  attached, and one specific next-step question.
- Within one week: Tier A follow-ups.
- Tier B contacts can wait or be skipped.

## See also

- [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md) — **read first.** Current
  recommended posture for the 2026 show: information visit, no pitch.
  Includes opening lines, questions to ask, the graceful close, and the
  follow-up email template that is the actual conversion event.
- [`FLOOR-PLAYBOOK.md`](FLOOR-PLAYBOOK.md) — pitch-mode booth strategy.
  Defer until working ring + polished one-pager + presentable GitHub +
  validated power numbers. Useful at a later event.
- [`SHENZHEN-PAIRING.md`](SHENZHEN-PAIRING.md) — coordinates the Sensors
  Converge floor walk with a Shenzhen visit. Deferrable booths, must-visits,
  and Shenzhen factory targets that replace deferred booth conversations.
- [`SHENZHEN-SEEED-QUOTE-PACKET.md`](SHENZHEN-SEEED-QUOTE-PACKET.md) —
  repo-backed quote starter for Seeed/Shenzhen conversations.
- [`../OUTREACH.md`](../OUTREACH.md) — accessibility-community outreach (Tier
  3+ in the broader partnership stack: Microsoft Adaptive, Logitech, MMC,
  e-NABLE, etc.). Sensors Converge does not address that tier.
- [`../IP-STRATEGY.md`](../IP-STRATEGY.md) — the CERN-OHL-S posture every
  silicon partner needs to understand before a design-in conversation makes
  sense.
- [`../REFERENCE-MANUFACTURERS.md`](../REFERENCE-MANUFACTURERS.md) — where any
  contract-manufacturer leads from the show should be logged.
- [`../CONSUMER-TIERS.md`](../CONSUMER-TIERS.md) — the Standard / Pro tier model
  that frames every silicon swap conversation.
