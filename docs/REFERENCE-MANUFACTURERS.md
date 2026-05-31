<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Reference Manufacturers

**Public knowledge. Not endorsement. No referral fees exist.**

This is a living doc of factories and services that have actually run
PowerFinger or PowerFinger-compatible designs at various volumes. The
purpose is to remove the "which factory in Huaqiangbei do I even talk to"
footgun for downstream entrants — to lower the activation energy for
someone who wants to ship a derivative or run a small batch for a care
facility, a school district, or an accessibility nonprofit.

## Disclaimer

> This document is documentation of public knowledge at specific
> timestamps. Factories change. MOQs change. Lead times change. Yields
> drift. **Verify with each vendor before placing orders.** No relationship,
> no endorsement, and no referral fees exist between the PowerFinger
> project and any listed vendor. Listings are not recommendations; they are
> records of what has been tried.

This disclaimer is load-bearing. The supply-chain misinformation risk if
this doc is treated as endorsement is real (see
`docs/scoping/LICENSE-REVISION-SCOPE.md` §5.2 risk 4). Treat it as a
starting point for due diligence, not the conclusion.

---

## How to Read This Document

Each tier has a table. Each row is a vendor that has run the design at the
listed volume. Each row includes:

- **Vendor** — name and primary URL
- **Last verified** — date the entry was last confirmed accurate. Entries
  more than 12 months unverified are visually flagged with `⚠️ STALE` so
  the reader does not mistake stale data for current data.
- **What was run** — which variant(s) and at what stage (rev, count)
- **MOQ** — minimum order quantity at last verification
- **Realized COGS** — actual landed cost per unit, if known. **Not the
  vendor's quote — the actual COGS after defects, shipping, customs, and
  any rework.** If only the quote is available, label as "quoted, not
  realized."
- **Lead time** — quoted vs. realized
- **Yield / defects** — defect rate and notes on failure modes
- **Notes** — operational quirks, communication channels, payment terms,
  IP-handling reputation, anything a future builder should know

Empty cells are honestly empty. Do not infer from absence; infer from data
or the absence of an entry entirely.

---

## Tier 1 — DIY (1 unit)

For builders making one device for themselves or one user.

### Vendor table

| Vendor | Last verified | What was run | MOQ | Realized COGS | Lead time | Yield | Notes |
|---|---|---|---|---|---|---|---|
| *(no entries yet)* | | | | | | | |

### Notes

- For one-off DIY, the relevant "vendors" are the component distributors
  (Digi-Key, Mouser, LCSC) and a 3D printer (personal or local maker
  space) for the shell. PCB fab at quantity 1 is best handled by JLCPCB
  or PCBWay's individual-order service, but neither has been run for
  PowerFinger yet.
- Tier 1 is the most permissive tier. The CERN-OHL-S 2.0 license imposes
  source-disclosure obligations only on conveyance (distribution outside
  your organization). One-off DIY for personal use is not conveyance and
  triggers no disclosure obligation. See
  `docs/NAMING-AND-COMPATIBILITY.md` §6.

---

## Tier 2 — Small Batch (25–500 units)

For care facilities, school districts, accessibility nonprofits, university
research labs, and prosumer makers who want to ship to a small population.

### Vendor table

| Vendor | Last verified | What was run | MOQ | Realized COGS | Lead time | Yield | Notes |
|---|---|---|---|---|---|---|---|
| Seeed Studio Fusion + Propagate | *not yet run* | Candidate for `USB-HUB` first PCB/assembly quote, with `R30-OLED-NONE-NONE` only as a DFM/pre-fab review annex | TBD | TBD | TBD | TBD | **Placeholder. No quote or batch has been run yet.** Current packet: [`docs/sensors-converge-2026/SHENZHEN-SEEED-QUOTE-PACKET.md`](sensors-converge-2026/SHENZHEN-SEEED-QUOTE-PACKET.md). Use the send-now path for `USB-HUB`; treat ring materials as annex-only DFM/pre-fab review until board-house output constraints and physical fit/stackup evidence close. |
| JLCPCB / EasyEDA ecosystem | *not yet run* | Candidate fallback for active packet PCB fab/assembly only; enclosure/serviceability still needs separate handling | TBD | TBD | TBD | TBD | **Placeholder. No quote or batch has been run yet.** Listed because `PCB1` BOM notes already name JLCPCB/PCBWay-style prototype rigid PCB sourcing. Do not record as PowerFinger-proven until an actual quote or run exists. |
| PCBWay | *not yet run* | Candidate fallback for active packet PCB fab/assembly and prototype mechanical review | TBD | TBD | TBD | TBD | **Placeholder. No quote or batch has been run yet.** Listed for Shenzhen scoping only. Any row update must separate quoted PCB/assembly cost from realized landed COGS and defects. |
| Local Bao'an / Longgang prototype house | *not yet identified* | In-person DFM/prototype-house fallback for ring shell/board serviceability questions | TBD | TBD | TBD | TBD | **Placeholder class, not a vendor.** Fill with a named shop only after contact details, quote scope, communication channel, payment terms, and license/source-return posture are known. |

### Notes

- At this tier, source-disclosure obligations under CERN-OHL-S 2.0
  activate on conveyance. Care facilities distributing to residents are
  conveying; internal use within a single facility may not be (consult
  counsel).
- Documentation expectations rise sharply at this tier. Each unit shipped
  needs assembly date, firmware version, and a path back to the
  certification UID (when OSHWA cert exists for the variant).

---

## Tier 3 — Mid-Batch (500–10,000 units)

For accessibility-focused commercial entrants, regional distributors, and
larger nonprofit deployments.

### Vendor table

| Vendor | Last verified | What was run | MOQ | Realized COGS | Lead time | Yield | Notes |
|---|---|---|---|---|---|---|---|
| *(no entries yet)* | | | | | | | |

### Notes

- At this tier, regulatory compliance (FCC for US, CE/UKCA/RED for EU/UK,
  IC for Canada) is a hard requirement. `docs/REGULATORY-PRESCAN.md`
  documents the prescan; actual certification is the entrant's
  responsibility.
- Commercial entrants at this tier should pursue OSHWA Certified status
  per variant. See `docs/IP-STRATEGY.md` Action Items for the submission
  order.
- IP handling becomes material at this scale. Vendors with a track record
  of respecting CERN-OHL-S 2.0 reciprocity (publishing modified source on
  conveyance) should be preferred. Vendors known to ignore copyleft
  obligations should be flagged.

---

## Tier 4 — Large Batch (10,000+ units)

For consumer-product entrants and large institutional deployments
(insurance-reimbursed assistive devices, large hospital networks, etc.).

### Vendor table

| Vendor | Last verified | What was run | MOQ | Realized COGS | Lead time | Yield | Notes |
|---|---|---|---|---|---|---|---|
| *(no entries yet)* | | | | | | | |

### Notes

- At this tier, MCU migration to nRF52840 should be evaluated for the
  3–5x battery life improvement. See `docs/NRF52840-MIGRATION.md`.
- Consumer-product framing introduces FDA / EU MDR considerations if
  the device is positioned as a medical device. The project's posture is
  that PowerFinger is an assistive HID device, not a medical device. See
  `docs/scoping/LICENSE-REVISION-SCOPE.md` §4.5 and `docs/REGULATORY-PRESCAN.md`.
- Tier 4 entrants should also evaluate OIN / Unified Patents membership
  for patent-litigation defensive coverage. See `docs/IP-STRATEGY.md`
  Defensive Alliances section.

---

## What Goes In a New Entry

When you (anyone — maintainer, contributor, downstream productizer) add a
vendor to this document:

1. Open a PR adding the row to the appropriate tier's table.
2. Fill **Last verified** with today's date in `YYYY-MM-DD` format.
3. Fill **Realized COGS** only with actual landed cost; quoted-only data
   should be marked `(quoted)` and treated as approximate.
4. Add operational notes that would have saved you a week if you'd had
   them when you started — payment terms, communication channels,
   common failure modes, IP-handling reputation, language barriers.
5. Be honest about defects and yield. The doc's value is honesty.

---

## Maintenance

This document depends on contributors keeping entries current. The
**Last verified** column drives this:

- Entries verified within the last 12 months: trusted as current.
- Entries 12–24 months unverified: prefix with `⚠️ STALE — last verified
  YYYY-MM-DD` and treat as historical reference only.
- Entries more than 24 months unverified: candidates for removal unless
  someone re-verifies and updates the timestamp.

Entries are not deleted when vendors disappear. The historical record is
useful — it tells future builders which approaches have been tried and
why they didn't continue. Mark abandoned vendors with `🪦 ABANDONED —
[reason]` rather than removing.

---

## Cross-References

- [`docs/NAMING-AND-COMPATIBILITY.md`](NAMING-AND-COMPATIBILITY.md) —
  conveyance vs. internal modification under CERN-OHL-S 2.0.
- [`docs/CONFORMANCE.md`](CONFORMANCE.md) — conformance criteria for
  ecosystem participants.
- [`docs/IP-STRATEGY.md`](IP-STRATEGY.md) — IP defense strategy and OSHWA
  cert action items.
- [`docs/REGULATORY-PRESCAN.md`](REGULATORY-PRESCAN.md) — regulatory
  compliance prescan.
- [`docs/CONSUMER-TIERS.md`](CONSUMER-TIERS.md) — consumer-product Standard
  / Pro tier definitions.
- [`docs/NRF52840-MIGRATION.md`](NRF52840-MIGRATION.md) — MCU migration
  analysis for consumer-product generation.
- [`docs/VENDOR-VERIFICATION.md`](VENDOR-VERIFICATION.md) — component
  sourcing verification (sibling concept at the part level rather than
  factory level).

---

*This document is part of the PowerFinger documentation set, licensed under
CC-BY-SA 4.0. It is not legal advice. The project consists of unincorporated
contributors, not a legal entity. No vendor relationships, endorsements, or
referral fees exist.*
