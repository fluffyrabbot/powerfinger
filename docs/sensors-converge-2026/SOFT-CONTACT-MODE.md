<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Soft-Contact Mode — Information Visit, No Pitch

**Recommended posture for the 2026 show given current project state.** The
project is pre-prototype. The GitHub repo is too rough to show without
forming an impression that's hard to overwrite later. There is no working
ring. There is no one-pager you'd be proud to hand over. The
[`FLOOR-PLAYBOOK.md`](FLOOR-PLAYBOOK.md) is the right doc to read once those
things exist; this doc is the right one for now.

The reframe: the show is a **reconnaissance and contact-opening** visit, not
a pitch visit. The conversion event is a follow-up email weeks or months
later, when the project is ready. The booth conversation only needs to give
you (a) a named contact and (b) a justification to email them later.

This is not a less ambitious plan. "Engineer doing due diligence at the right
show, asks intelligent questions, follows up months later with a polished
pitch" is a *more credible* posture than "guy with rough GitHub asks for
partnership." You're not lowering ambition; you're sequencing correctly.

## The core principle

**Ask, don't tell.** A booth conversation that consists of you asking
technical questions and them answering requires zero polish on your part.
Engineers ask FAEs questions all day; that is literally what FAEs are at the
booth for. You leave with information, a contact, and a relationship that
reads as "engineer who came by and asked good questions" — not "guy who
pitched me on his unfinished project."

Information flows toward you. You give nothing back except your email if
they request it.

## What this changes vs. the FLOOR-PLAYBOOK

**Drop entirely:**

- Bringing one-pagers to the booth.
- Saying the project name out loud.
- Mentioning the GitHub URL.
- The 30-second pitch.
- The 2-minute pitch.
- Asking for design-win partnerships, dev-kit allocations "for the project,"
  partner-program inclusion, featured-project placement.
- The 3D-printed ring shell on your finger as a conversation hook (it invites
  the "what is it?" question that you cannot fully answer yet).

**Keep:**

- Comfortable shoes, paper notebook, badge at chest height, plain bag.
- Floor-walk order: Murata → Bosch → ST/Microchip back-to-back, Tier A in
  any order, Tier B as drive-bys.
- Energy management — break every 90 minutes, last-90-minutes-of-day for
  Tier S.
- The same-night ritual, in modified form (see below).

## How to refer to your project without naming it

The trick is to refer to "the design" or "a project I'm evaluating" without
naming it. These phrasings are how every engineer doing pre-design research
talks:

- "I'm in early evaluation for a low-power BLE wearable HID design."
- "I'm comparing silicon options for an embedded sensing project."
- "I'm doing due diligence on BLE module options for a prototype."
- "I'm working on a wearable input device design — early stage."

The vendor's brain pattern-matches you to "early-stage technical buyer doing
due diligence" — the mode they're trained to handle and the mode where they
freely give brochures, app notes, dev kit catalogs, and FAE cards.

No name, no URL, no GitHub. They don't need them; they won't ask. If they
do ask, the answer is "I'm not ready to share publicly yet — I'll loop back
when I am" and they will accept that without friction.

## Opening lines for this mode

Pick one and use it the whole show. Variation costs mental energy.

- **"Hi — I'm in early evaluation for a low-power BLE wearable. Got a couple
  technical questions about your [WBA52 / WBZ451 / Type 2DL / BHI360] line.
  Got a minute?"**
- **"Hi — could I grab your dev kit catalog and ask one question about idle
  current?"**
- **"Hi — who'd be the right person to email when I'm ready to share more
  about a BLE HID design I'm working on?"**

The third opener is the most honest and works surprisingly well. It is a
*normal* request; marketing teams are paid to provide that contact. It also
explicitly licenses you to follow up later, which is the point.

## Questions to ask at booths

Pick one or two per booth — these are routine FAE questions that fit a
"doing due diligence" posture without requiring you to disclose project
state:

**General to silicon vendors (ST, Microchip, Bosch, ams, ADI, EM):**
- Idle / sleep current in BLE peripheral mode at 15ms and 30ms connection
  intervals.
- Whether their BLE HID example code is in the public SDK or NDA-walled.
- Whether they have an open-source / maker / education program.
- Dev kit availability and lead time.
- Whether OTA / firmware update is in the BLE stack out of the box.

**Specific to Bosch:**
- Whether the BSX fusion library is open-source-redistributable, source-
  available under NDA, or binary-only. *This is the load-bearing question
  for the IMU variant — get it on the record at the booth in writing if
  possible.*

**Specific to Murata:**
- Antenna integration constraints for flex PCB / small wearable form factors.
- Whether their pre-cert modules can be designed into open-hardware reference
  designs without licensing friction (general posture, not project-specific).
- Module-vs-raw-silicon BOM cost trade-off literature.

**Specific to TDK / ADI / ams:**
- Piezo-film signal-conditioning app notes (TDK, ADI).
- Eye-safety class on lowest-power VCSEL emitters (ams).
- Sample availability via distributor for sub-100-unit eval runs.

**Specific to Mouser:**
- Whether they offer a "BOM bundle / project bundle" merchandising feature.
- Editorial program contact (no project pitch — just the contact).

Each of these can sustain a 2–5 minute conversation without revealing
anything about project state. Each gives you genuine intel. Each is a
perfect setup for a follow-up email when you're ready.

## Identity hygiene

- **Personal email, not project email.** A clean Gmail / iCloud / personal-
  domain address. Nothing that ties back to the GitHub or the project name.
- **A simple personal business card.** Your name, email, an unpretentious
  title ("Embedded Engineer," "Hardware Designer," or none at all). No
  project name. No URL. ~$25 from MOO / Vistaprint.
- **Take their cards. Don't lead with handing them yours.** If they ask
  for yours, hand it over without ceremony. Don't volunteer it.
- **If they Google your name and find the GitHub:** that's fine *if your
  GitHub username is generic and not project-named*. If your GitHub points
  directly to the rough repo, consider using a different email or a name-
  only card so the trail isn't immediate.

## The graceful close that licenses follow-up

End every Tier S conversation with this line, or very close to it:

> **"Thanks — really helpful. I'll be in touch when the design's at a state I
> can share more about. What's the best email for you?"**

This is the entire ask. It does three things at once: closes the conversation
politely, sets the expectation that you *will* follow up, and gets the email.
They will give you the email. Almost nobody refuses this.

If they push for more ("what's the project, exactly?"), the answer is:

> "It's an open-source design I'm not ready to share publicly yet — I'd
> rather come back to you when I have something polished. Sound okay?"

This works on every reasonable counterparty. People in this industry
understand stealth-mode posture even from solo open-source builders.

## Per-vendor doc relationship

The "ask" sections in each per-vendor strategy doc
([`01-stmicroelectronics.md`](01-stmicroelectronics.md) through
[`12-excelitas.md`](12-excelitas.md)) are still correct — they just become
**email asks for later**, not booth asks for now. The booth visit is to
acquire the contact and the technical-question pretext for following up.
The asks themselves shift forward in time by weeks or months.

When reading those docs in soft-contact mode, treat:
- "Minimum viable" ask → use as a follow-up email ask.
- "Stretch" ask → use only after the follow-up email has had a positive
  response.
- "Conversation cues" → still apply; they help you find the right team.
- "Risks / why they might say no" → less relevant in soft-contact mode
  because you're not asking for anything yet.

## Same-night ritual — modified shape

You're not sending "nice to meet you" emails the same night — you have
nothing to attach yet, and a content-free email weakens the future cold-warm
follow-up. Instead:

1. **Log the conversation.** Notebook page or a single spreadsheet row per
   contact: name, company, role, date, what you asked, what they said, the
   email. Be detailed enough that the follow-up email three months later can
   reference specifics.
2. **Triage.** Which contacts get a real follow-up when the project is ready?
   Star the Tier S; mark Tier A as "maybe"; archive Tier B unless
   surprisingly engaged.
3. **Schedule the follow-up.** Calendar reminder for the date you expect to
   be ready. The follow-up email is the conversion event; everything before
   it is setup.
4. **Update the per-vendor docs in this directory** with a "Sensors Converge
   2026 booth notes" section. Capture the contact email, the conversation
   detail, and any updates to the conversation cues / risks based on what
   you actually saw.

## The follow-up email — the conversion point

In whatever future month the project is ready (working ring or close to it,
polished one-pager, GitHub presentable, README that doesn't apologize), you
send:

```
Subject: Re: low-power BLE HID design — Sensors Converge 2026 follow-up

Hi [name] —

We met briefly at Sensors Converge in May; I asked you about [the specific
question — idle current on WBA52 / BSX licensing / antenna keepout / etc.].
The project I was evaluating silicon for is now at a state I can share.

It's an open-source assistive input device — wearable rings + a wand-style
stylus that work as BLE HID mice. Sub-$25 BOM, fully offline, designed for
users with limited mobility. One-pager attached. Repo at [URL].

Three things I'd want to ask, if you have ten minutes:

1. [specific technical question]
2. [specific operational question — sample, partner program, app note]
3. [specific commercial question — relevant only at this point]

Happy to take it to email or a call — whichever works.

Thanks,
[your name]
```

This email lands completely differently than a cold email. You are a known
engineer re-emerging with something concrete. They remember meeting you. The
bar to engagement is low. This is the entire reason you did the show.

## When to switch back to FLOOR-PLAYBOOK

The full pitch playbook becomes the right doc when **all four** of these are
true:

1. There is a working ring or wand that demos cleanly. Click works, cursor
   tracks, BLE pairs reliably to a laptop.
2. There is a one-pager you would hand to a stranger without explanation.
3. The GitHub repo's top-level README is something a first-time visitor
   could read in two minutes and walk away with a correct understanding of
   the project.
4. There is at least one consumer-tier BOM committed (Standard or Pro) with
   real, validated power numbers.

Until all four are true, soft-contact mode is the right posture. Even at a
later show. Even at CSUN, ATIA, AWE, or any other event in the partnership
pipeline. The pitch mode is for when you're ready to transact; soft-contact
is for when you're not.

## Mental reframe

You are not at the show to sell anyone anything. You are at the show to
**learn what they offer, learn who to talk to, and license future contact**.
Each booth conversation is a small, low-stakes intel-gathering exchange. The
expected value of approaching a Tier S booth in this mode is positive
because the only thing being asked of you is "would you like an email
contact?"

Twelve calm five-minute conversations across three days, ending each with
"what's the best email for you?", produces twelve named contacts to email
months later when you have something to show. That is a successful Sensors
Converge 2026 trip in soft-contact mode.

## See also

- [`README.md`](README.md) — triage and floor-walk order. Still applies; the
  posture is the only thing that changes.
- [`FLOOR-PLAYBOOK.md`](FLOOR-PLAYBOOK.md) — pitch-mode booth strategy. Use
  later, not at this show.
- Per-vendor docs ([`01-`](01-stmicroelectronics.md) through
  [`12-`](12-excelitas.md)) — booth-specific cues and the asks that move into
  the follow-up email.
- [`../OUTREACH.md`](../OUTREACH.md) — the broader partnership pipeline; the
  follow-up emails feed it.
