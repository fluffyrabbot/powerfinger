<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# License Revision Scoping — Layered Documentation Around an Unchanged License Stack

**Status:** Scoping only. No code/file changes have been made. BDFL approval
required before any phase begins.

**Scope:** Evaluate adding three (possibly four) documentation-only layers
*around* the existing CERN-OHL-S 2.0 (hardware) + MIT (firmware/software)
license stack. The load-bearing licenses do not change.

**TL;DR recommendation:**

- **Ship layers A + B + C.** They are low-cost, mission-aligned, and add
  protections the existing license stack doesn't articulate.
- **Drop layer D (Prusa OCL dual-licensing).** The license is unsuitable on
  legal-drafting grounds, conflicts with CERN-OHL-S 2.0 in subtle ways, has
  near-zero adoption, and solves a problem CERN-OHL-S §3.1 already solves
  (internal modification ≠ conveyance, no disclosure trigger).
- **Surface a NEW blocker the original prompt didn't anticipate:** the repo
  has no `LICENSE-DOCS` artifact. This blocks OSHWA certification today and
  should be folded into layer A.
- **Surface a NEW certification opportunity:** OSHWA Open Healthware
  Certification was announced 2026-01-09. Probably out of scope for an
  assistive HID device, but worth a parking-lot decision.

This doc is structured for non-lawyer readers. Each section stands alone.
Skip to §3 for the file-level diff plan if you only have five minutes.

---

## 1. External Research Findings

Three independent research passes (Prusa OCL, OSHWA cert process, conformance
models). Citations and full sub-briefs are preserved in this section's tables;
the headline finding for each is in the first row.

### 1.1 Prusa Open Community License (OCL) v1

| Question | Finding |
|---|---|
| **Headline** | Do not adopt OCL, even for documentation. |
| Released | 2025-12-19 alongside Prusa CORE One CAD release. |
| OSI / FSF / OSHWA recognition | None of the three has reviewed it. **Michael Weinberg (OSHWA Board Member, NYU Engelberg Center) wrote a personal critique 2026-01-30 calling OCL "an 'open' license that does not meet any common definition of open"** and frames it as IP maximalism. |
| Patent retaliation clause | **None.** Material gap vs. CERN-OHL-S 2.0. |
| Patent grant | Implied via "copyrights, designs, and patents… are community open" but no clean grant clause. Kyle Mitchell (/dev/lawyer) flags this as imprecise drafting. |
| Commercial use | Forbidden for "BUSINESS USERS" except for "internal production use." Cannot sell or replicate without separate paid Prusa license. |
| Reciprocity fallback chain | Derivatives must be under "OCL or any non-commercial share-alike license." In practice this matches only CC-BY-NC-SA — unsuitable for software, and **not satisfied by CERN-OHL-S 2.0** (which permits commercial sale). |
| "Non-commercial" definition | Undefined — used as a *user* descriptor, not a *use* descriptor. Mitchell calls this the central failure mode. |
| Adoption beyond Prusa | None located. ~55 stars / 7 forks / 4 commits on the OCL GitHub repo as of 2026-05. Functions as a single-licensor instrument. |
| Community framing | "Openwashing" / "non-commercial license wearing an open source costume" repeated in Adafruit, Make, Fabbaloo coverage. Stargirl Flowers published a satirical "SOCL" stripping OCL to a copyright notice. |

**Compatibility with CERN-OHL-S 2.0** is the dispositive question for our use
case, and it fails on three independent grounds:

1. **Commercial asymmetry.** CERN-OHL-S permits sale; OCL forbids it. A
   dual-licensed file gives recipients a choice — they pick CERN-OHL-S, sell,
   and the OCL channel goes inert. Recipient-choice dual-licensing is unsafe
   when one branch is strictly more permissive than the other on a load-bearing
   axis.
2. **Reciprocity chains do not interlock.** OCL demands derivatives go under
   "OCL or any non-commercial share-alike." A CERN-OHL-S derivative cannot
   satisfy that requirement. The two copyleft chains do not feed each other.
3. **No equivalent defensive value.** OCL has no patent-retaliation clause, so
   stacking it on a CERN-OHL-S file adds zero defensive protection while
   adding legal complexity.

**The problem OCL was designed to solve does not exist for us.** CERN-OHL-S
2.0 §3.1 obligations trigger only on **conveyance** (distribution outside the
organization). A care facility internally adapting our build guides for
in-house use is not conveyance and triggers no disclosure. OCL's "internal
production use" carve-out is already CERN-OHL-S's default behavior. If care
facilities express friction, the right move is a one-page interpretation
note, not a license change.

### 1.2 OSHWA Certification Process

| Question | Finding |
|---|---|
| **Headline** | Closer to ready than `docs/DEFENSIVE-PUBLICATION.md` §3 currently suggests. The big new blocker is documentation licensing, not hardware completeness. |
| Cost | Free. Confirmed across 2025–2026 sources. |
| Working prototype required | **No.** FAQ explicit: "You will need to provide links to the final documentation that you will make available at launch." Schematic-only certification is allowed. |
| Responsible party | Individual is allowed. No legal entity, DUNS, or corporate filing required. |
| UID format | `[ISO-3166-1 alpha-2][6-digit serial]` e.g. `US000237`. Country prefix mandatory and auto-assigned. |
| Display requirements | Permissive. README badge + directory listing satisfies the program. PCB silkscreen of UID is best practice, not mandated. |
| Multi-variant policy | **Each variant gets its own UID.** No umbrella cert. The application form has a "builds upon" dropdown citing prior UIDs — this creates a de facto family chain in the directory. |
| Renewal | Annual reaffirmation email. Lapse = listing remains as historical record but project is no longer active-certified. |
| Audit posture | Self-certification with post-submission staff review. Links must resolve and licenses must be on the approved list. |
| New: Open Healthware Certification | Announced 2026-01-09, layered on top of standard OSHWA. Targets medical hardware. Assistive devices not explicitly named as in-scope. Probably parking-lot for now. |

**Hard blockers for PowerFinger today (May 2026):**

1. **No `LICENSE-DOCS`.** OSHWA accepts CC-BY or CC-BY-SA for documentation;
   CC-BY-NC and CC-BY-ND are explicitly prohibited. The repo currently has
   no documentation license declared. **This is the most common
   first-rejection reason and is the silent killer.**
2. **No completed BOM with manufacturer part numbers and source links per
   variant.** The publication packets are "BOM-backed" but the BOM file
   itself must be public and specific enough to source every component.
3. **Schematics in progress.** OSHWA needs final, committed schematics
   (KiCad source, not exported PDF) at the certified version.
4. **No routed Gerbers/PCB layout for ring + hub.** Schematic-only is
   technically allowed but unusual; expect a clarification request.
5. **No firmware version tag pinned to the certified hardware version.**

**Submission order, when ready:** `R30-OLED-NONE-NONE` ring → `USB-HUB`
(citing the ring) → `WSTD-BALL-NONE-NONE` PowerPen → `P-OLED-NONE-NONE`
PowerPuck. The citation chain creates a visible family relationship despite
the absence of umbrella certification.

**Accessibility tagging:** No accessibility-specific badge exists. Use
free-text keywords: `assistive technology`, `assistive-technology`,
`disability`, `digital accessibility`, `mobility`. Precedent: CA000073 (Shrub
Hub, Neil Squire Society).

### 1.3 Conformance Test Models

| Model | What we'd borrow | What we'd skip |
|---|---|---|
| USB-IF | Spec + reference test harness + public registry triad | Trademark, fees, workshops, $6k vendor ID |
| "Linux-compatible" (historical) | Reputational gravity + public mailing-list shaming | Nothing formal; that was the model |
| RISC-V Architectural Compatibility Tests (ACTs) | Self-checking test ELFs run by implementer on their own DUT — no central authority | Member-org gating + trademarked logo |
| PJRC Teensy | **Two-sentence trademark policy.** "Publish derivatives, just don't call it Teensy." | The trademark itself (BDFL-decided) |
| Adafruit CircuitPython | **PR-as-registry pattern.** Third party submits a PR adding a YAML to `compatible-devices/`, merge = listing | Adafruit's brand chain via VID/PIDs |

**Synthesis for PowerFinger:** PJRC + CircuitPython hybrid is the right
shape, but the BDFL has decided against a trademark filing. So the PJRC
"don't call it Teensy" leg is gone. The remaining mechanics:

- Self-attestation as a PR
- A `CONFORMANCE.md` defining four checklist categories
- Test scripts in a `conformance/` directory (when written)
- An OSHWA-style triple disclaimer pattern (no warranty, self-cert framing,
  trademark license ≠ endorsement) that travels with every listing

**Important pushback from the conformance research agent, which I endorse:**

> "Conformance" is overengineered for the current scale. At BOM-$9 and
> pre-firmware, you don't need a conformance program; you need a checklist
> and a test script in `tests/conformance/`. Promote it to a "program" only
> if (a) a third-party productizer ships, AND (b) at least one user reports
> interoperability breakage. Until then, the file `CONFORMANCE.md` plus
> four scripts *is* the program. Don't build a registry until there is
> something to register.

This pushback shapes the layer-B file plan in §3 below. **No
`compatible-devices/` directory, no PR-registry instructions, no listing
template until a third-party productizer actually exists.** The
`CONFORMANCE.md` will reserve the registry mechanism as a "future activation"
section so the door is open without us building scaffolding for a community
that doesn't exist yet.

### 1.4 Liability disclaimer pattern

The four reference programs (USB-IF, OSHWA, Adafruit, RISC-V) all use the
same triad:

1. **Explicit no-warranty / no-legal-advice disclaimer**
2. **Self-certification framing** that places legal posture on the certifier,
   not the standards body
3. **Trademark license framed as ≠ endorsement** (or equivalent — for us,
   the descriptive-name framing serves the same role)

OSHWA has operated this way since 2016 with no notable liability exposure to
the association itself. We can crib this triad directly into
`docs/CONFORMANCE.md` and `docs/NAMING-AND-COMPATIBILITY.md`.

---

## 2. Recommendation

**Ship layers A + B + C. Drop layer D entirely.**

### 2.1 Why ship A+B+C

The existing CERN-OHL-S 2.0 + MIT stack is well-chosen and load-bearing.
It does *not* by itself address three real concerns the project has:

1. **Naming posture.** "PowerFinger" looks like a product name. Casual readers
   may assume a trademark exists. Without explicit guidance, derivative makers
   may either (a) avoid using the name even in honest "compatible with"
   claims (overcautious), or (b) ship products under the name in ways that
   confuse end users about provenance (bad-faith). Layer A makes the
   descriptive-name posture explicit and gives derivative makers a phrasing
   playbook.
2. **Compatibility claims.** The project will eventually have downstream
   makers claiming compatibility. Without a published criterion, those claims
   are unverifiable. Layer B publishes the criterion. **It does not require
   any new code today** — the file plus four lightweight scripts (when those
   exist) is the entire program.
3. **Manufacturing knowledge.** "Where do I even fab this at scale" is a real
   footgun for downstream productizers. Layer C documents public knowledge of
   factories that have actually run open-hardware projects. This is the
   highest-leverage layer for *encouraging competition without
   footgunning entrants* — a stated mission goal.

All three layers are pure documentation. Zero changes to load-bearing
licenses, zero new legal infrastructure beyond what CERN-OHL-S already
provides, zero ongoing maintenance burden beyond what already exists for the
docs tree.

### 2.2 Why drop D

The Prusa OCL research is unambiguous: the license is unsuitable on legal
drafting grounds, has zero ecosystem outside Prusa, fails the accessibility
shield narrative test (institutional recognition matters — OSI/OSHWA-approved
licenses signal seriousness; an "openwashing"-flagged license signals the
opposite), and **does not solve the problem the prompt thought it solved**
because CERN-OHL-S 2.0's reciprocity triggers on conveyance, not internal
modification.

If a care facility friction case actually emerges later, the right response
is a one-page interpretation note in `docs/IP-STRATEGY.md` clarifying the
conveyance distinction. Not a license change.

**Revisit horizon:** None planned. If OCL gains adoption beyond Prusa AND
gets meaningful institutional recognition (OSI review, OSHWA endorsement),
re-scope. Neither has happened in five months and neither looks likely. The
original "drop and revisit in 6-12 months" suggestion in the prompt is
generous; my recommendation is "drop and revisit if and only if specific
external triggers fire."

### 2.3 The new finding folded into layer A

`LICENSE-DOCS` is a missing artifact. The prompt didn't anticipate this; the
OSHWA research surfaced it. It belongs in layer A because it is part of the
naming-and-compatibility-and-licensing-clarity story for casual readers, and
because it is the gating blocker for OSHWA certification — which the
revision is partly motivated by. Recommended choice: **CC-BY-SA 4.0** (matches
the share-alike spirit of CERN-OHL-S 2.0). CC-BY 4.0 is the more permissive
alternative; see open question §4.1.

### 2.4 Mission alignment check

The four mission anchors:

| Anchor | Layer A | Layer B | Layer C | Layer D (dropped) |
|---|---|---|---|---|
| Accessibility-first | Reinforces — OSHWA cert visibility, accessibility tagging | Reinforces — accessibility validation as a conformance category | Neutral | Neutral or negative — opens openwashing questions about an accessibility project |
| $9 BOM / encourage competition | Strongly positive — derivative makers know how to claim compatibility honestly | Strongly positive — published criteria reduce ambiguity | Strongly positive — removes the "which factory" footgun | Negative — adds licensing complexity that derivative makers must reason through |
| Accessibility shield narrative | Strongly positive — OSHWA cert + named compatibility = stronger narrative | Strongly positive — accessibility validation criterion is a quotable artifact | Neutral | Negative — institutional recognition matters; OCL's openwashing flag would weaken the narrative |
| No-trademark posture (decided) | Required — layer A is the load-bearing doc that makes this explicit | Compatible — registry-as-PR works without trademark | Compatible | Compatible |

Layers A+B+C are net-positive on every anchor. Layer D is neutral or
negative on three of four. The mission alignment math agrees with the legal
analysis.

---

## 3. File-Level Diff Plan

This is what would actually change in the repo if A+B+C ship. **Nothing in
this section has been written yet.** Each row is one line per file with a
one-line rationale.

### 3.1 New files

| File | Layer | Rationale |
|---|---|---|
| `LICENSE-DOCS` | A | CC-BY-SA 4.0 license text. Required for OSHWA certification; closes the documentation licensing gap. |
| `docs/NAMING-AND-COMPATIBILITY.md` | A | Plain-language guidance on the descriptive-name posture, allowed phrasings ("compatible with," "based on," "for"), suggested practice that derivatives use their own product name, OSHWA Certified as recommended quality signal. |
| `docs/CONFORMANCE.md` | B | Four-category conformance checklist (BLE HID profile, hub composition protocol, companion app handshake, accessibility validation). Explicit self-attestation framing, no certifying body, no logo. Placeholder for future `compatible-devices/` registry that activates only when a third party ships. |
| `docs/REFERENCE-MANUFACTURERS.md` | C | Tiered manufacturer registry: 1-unit DIY / 25–500 small-batch / 500–10k mid / 10k+ large. Pre-populated structure with TODO marker for Seeed Studio Fusion+Propagate. Explicit "documentation of public knowledge, not endorsement, no referral fees" disclaimer. |
| `docs/scoping/LICENSE-REVISION-SCOPE.md` | (this doc) | Already created as part of this scoping pass. |

### 3.2 New directories

| Path | Layer | Rationale |
|---|---|---|
| `docs/scoping/` | meta | Holds this file. |
| `conformance/` | B (deferred) | **Do NOT create yet.** Reserve as the future home for self-attestation test scripts and the `compatible-devices/` registry. Activation criteria documented in `docs/CONFORMANCE.md`. Decision recorded in §4.2 below as an open question; my recommendation is `conformance/` at repo root with MIT license, separate from `trial-scripts/` (KiCad utilities) and `scripts/` (dev helpers). |

### 3.3 Modified files

| File | Layer | Change | Rationale |
|---|---|---|---|
| `README.md` | A | Add a short paragraph in the License section: name is descriptive, no trademark asserted, OSHWA Certified is the recommended third-party quality signal, point at `docs/NAMING-AND-COMPATIBILITY.md`. Add doc license to the three-license declaration. | Surfaces the descriptive-name posture for casual readers; closes the doc-license declaration gap visibly. |
| `LICENSE-HARDWARE` | A | Add a brief one-paragraph header above the existing CERN-OHL-S 2.0 boilerplate noting that "PowerFinger" is used descriptively and pointing at `docs/NAMING-AND-COMPATIBILITY.md`. Keep CERN-OHL-S text untouched. | Casual readers reading the LICENSE file shouldn't infer a trademark from the project name. |
| `LICENSE-SOFTWARE` | A | Same as `LICENSE-HARDWARE`. | Same rationale. |
| `CONTRIBUTORS.md` | A | Add a sentence about contributors agreeing the documentation license is CC-BY-SA 4.0, alongside the existing CERN-OHL-S/MIT declarations. | The CLA-style language at the bottom of the file currently covers hardware and software but not docs. |
| `docs/IP-STRATEGY.md` | A, C | Add cross-references to the three new docs. Add a one-paragraph "Conveyance vs. Internal Modification" interpretation note (the OCL research surfaced this and it deserves a permanent home regardless of OCL outcome). Update the Action Items table: OSHWA cert blockers list now includes `LICENSE-DOCS`. | Keeps IP-STRATEGY as the canonical IP doc; integrates the new layers without duplicating their content. |
| `docs/DEFENSIVE-PUBLICATION.md` | meta | Update §3 OSHWA gap analysis to reflect the corrected blocker list (working hardware NOT required; `LICENSE-DOCS` IS required). | The current §3 overstates the hardware-completeness blocker and understates the doc-license blocker. |

### 3.4 Files explicitly NOT touched

| File | Why not |
|---|---|
| `LICENSE-HARDWARE` (CERN-OHL-S 2.0 body) | Out of scope per BDFL decision. Header annotation only. |
| `LICENSE-SOFTWARE` (MIT body) | Out of scope per BDFL decision. Header annotation only. |
| `CLAUDE.md` / `AGENTS.md` | Hard rules section already says "All hardware files: CERN-OHL-S 2.0. All firmware/software: MIT." This stays correct. The doc license addition is below the threshold of agent-instruction relevance. |
| `firmware/**`, `hardware/**`, `companion/**` | Layer changes are documentation-only. No code/CAD touched. |

### 3.5 Phasing

If approved, this becomes three separate commits, each independently
revertible:

1. **Phase 1 — A.** `LICENSE-DOCS` + `docs/NAMING-AND-COMPATIBILITY.md` +
   README/LICENSE-HARDWARE/LICENSE-SOFTWARE/CONTRIBUTORS.md/IP-STRATEGY.md
   header updates + DEFENSIVE-PUBLICATION.md correction.
2. **Phase 2 — B.** `docs/CONFORMANCE.md` + IP-STRATEGY.md cross-reference.
   No `conformance/` directory yet.
3. **Phase 3 — C.** `docs/REFERENCE-MANUFACTURERS.md` + IP-STRATEGY.md
   cross-reference.

Phasing is intentional: phase 1 unlocks OSHWA certification work; phases 2
and 3 are independent of cert and of each other.

---

## 4. Open Questions for the BDFL

These are the decisions that need explicit BDFL approval before any file is
written. Trademark and certification mark questions are NOT here — those are
decided.

### 4.1 CC-BY 4.0 vs. CC-BY-SA 4.0 for `LICENSE-DOCS`

**Recommendation: CC-BY-SA 4.0.**

| Option | For | Against |
|---|---|---|
| CC-BY 4.0 | Most permissive for downstream documentation reuse; matches Adafruit's choice; simpler for academic citation | Allows commercial repackagers to remix docs without sharing back |
| CC-BY-SA 4.0 | Matches share-alike spirit of CERN-OHL-S 2.0; reciprocal docs ecosystem; consistent project-wide reciprocal posture | One-way GPLv3 compatibility for mixed software-docs cases is fine but slightly more friction for commercial reuse |

The mission anchor is reciprocity-protective. CC-BY-SA 4.0 is consistent.
But this is a real fork.

### 4.2 Future location of `conformance/` directory

**Recommendation: `conformance/` at repo root, MIT licensed, deferred until
activation triggers fire.**

Three viable locations:

- `conformance/` at repo root — sibling to `firmware/`, `hardware/`,
  `companion/`, `trial-scripts/`. Most discoverable.
- `tests/conformance/` — under a not-yet-existing `tests/` umbrella. Cleaner
  but requires creating a tests umbrella that doesn't exist.
- `firmware/conformance/` or `companion/conformance/` — bad fit; conformance
  spans firmware + hub + companion + hardware.

Repo-root `conformance/` is the right answer when scripts arrive. The
question for the BDFL is whether to commit to this in `docs/CONFORMANCE.md`
now (so future authors don't relitigate) or leave it open.

### 4.3 Conformance scope — does accessibility validation belong?

**Recommendation: Yes, with explicit gap acknowledgment.**

The four conformance categories the prompt suggests:

- BLE HID mouse profile compliance — reference upstream Bluetooth SIG
  validation; we don't replicate
- Hub composition protocol compatibility — our protocol, our test script
- Companion app handshake — our protocol, our test script
- Accessibility validation protocol — **this doc doesn't yet exist**

`docs/SURFACE-TEST-PROTOCOL.md` is the closest analog and is genuinely
rigorous (16 surfaces × 5 tests, pass/fail criteria, per-sensor expected
results). It is *surface* compatibility, not *accessibility scenario*
compatibility — different axis. An accessibility scenario protocol would
test something like "operable with one finger flexion of < 5mm travel,
operable on wheelchair-armrest at 30° tilt without surface, operable with
tremor up to 7Hz." We don't have that doc.

Three options:

| Option | Pros | Cons |
|---|---|---|
| Include accessibility validation as a category and flag it as "doc gap" | Honest; encourages someone to write it | The doc gap weakens layer B at ship |
| Write the accessibility scenario protocol as part of layer B | Strongest layer B | Significantly increases effort; pushes layer B from M to L |
| Drop accessibility validation from layer B | Smallest scope, fastest ship | Misses the strongest mission-aligned conformance category |

My recommendation is option 1 (include + flag gap). The flag becomes a
follow-up scoping job, separately authored, by someone with accessibility
testing expertise.

### 4.4 OSHWA cert timing — file now (schematic-only) or wait for routed PCB?

**Recommendation: Wait for first routed PCB, then file ring + hub
together.**

| Option | Pros | Cons |
|---|---|---|
| File schematic-only now | Earliest UID = earliest prior-art search hit | Likely clarification request from OSHWA; weaker reproduction package; `LICENSE-DOCS` still has to land first |
| File when ring + hub have routed PCBs | Stronger reproduction package; cleaner cert listing | Delays UID by however long routing takes |

The prior-art clock is mostly already running via the GitHub repo's commit
history. A few weeks/months delay on the OSHWA UID matters less than the
quality of the cert listing, which becomes the public face of the project's
prior art for patent examiners.

### 4.5 Open Healthware Certification — pursue or park?

**Recommendation: Park.** Revisit when standard cert is filed and the
Healthware program has matured past wireframe stage.

Announced 2026-01-09, requirements still in community feedback as of the
announcement. Targets "band-aids to implantable devices." Assistive HID
devices are not explicitly named as in-scope. Could be framed as a
mobility-impairment medical aid, but that framing has regulatory implications
(FDA in the US, EU MDR in Europe) that the project explicitly does not want
to inherit by accident.

### 4.6 Disclaimer placement in `docs/CONFORMANCE.md`

**Recommendation: triple placement.**

The OSHWA / USB-IF pattern uses a triple-disclaimer (header of the doc, top
of any registry README, in the attestation YAML schema itself) precisely
because the repetition weakens implied-endorsement arguments. Even though we
have no registry today, we should write the disclaimer once and have it
follow any future listing into every document.

---

## 5. Risk Register

What this revision does NOT solve, and what it might introduce.

### 5.1 Risks the revision does NOT solve

| Risk | Why this revision doesn't address it |
|---|---|
| Patent litigation by an existing patent holder | CERN-OHL-S 2.0 patent retaliation + defensive publication is the existing posture. Layers A+B+C don't change it. |
| Funding for FTO opinion | $2-5K still needed before Phase 2 investment. Outside license-revision scope. |
| BLE HID security (device authentication, manufacturer-specific adv data) | Tracked in HANDOFF.md M12. Outside license-revision scope. |
| Accessibility scenario validation rigor | See §4.3. Layer B flags the gap; doesn't fill it. |

### 5.2 New risks introduced

**Risk 1 — Bad-faith dangerous-product use of the project name.**

*Without a trademark, what happens?*

A bad-faith actor ships a counterfeit "PowerFinger" with an unsafe LiPo cell,
no charge protection, hot-glued together, and someone gets a thermal burn.
The project name appears in news coverage. We have no legal mechanism to
force the product off the market.

**Project response posture (new, to be written into
`docs/NAMING-AND-COMPATIBILITY.md`):**

1. **Public statement framework.** Pre-drafted public statement that names
   the bad-faith product, cites specific safety violations against the
   reference design's `docs/BATTERY-SAFETY.md` and CERN-OHL-S 2.0 source
   disclosure obligations, and points users at the OSHWA-certified reference
   variants.
2. **OSHWA Certified as the affirmative quality signal.** Users evaluating
   "PowerFinger-named" products are directed at the OSHWA registry to
   distinguish certified from non-certified. The OSHWA UID is the
   distinguishing mark we don't have to defend ourselves.
3. **CERN-OHL-S 2.0 source disclosure as the legal lever.** A counterfeiter
   distributing the design under the project name without source
   disclosure is in license violation. This is enforceable by any
   contributor who holds copyright on a portion of the design — including
   via cease-and-desist letters and (rarely) litigation. The license itself
   is the only legal lever, and it is a real one.
4. **Community / press mobilization.** The accessibility shield narrative
   in `docs/IP-STRATEGY.md` already covers this scenario for patent
   litigation. Extend it to cover bad-faith product impersonation.

**Honest assessment:** This posture is weaker than a trademark would be. It
is what we have given the no-trademark decision. The trade is: no annual
trademark maintenance + no policing burden + no aggressive C&D appearance,
in exchange for slower response to bad-faith actors. The BDFL has accepted
that trade.

**Risk 2 — OSHWA Certified gap for accessibility-specific concerns.**

*How reliably does OSHWA Certified fill the user-protection gap?*

Not very, for accessibility-specific concerns. OSHWA does not audit
accessibility claims. Their certification confirms openness, not safety,
not accessibility, not fitness for any specific user population. A product
with OSHWA Certified status could still be dangerous for users with limited
grip strength, tremor, or skin sensitivity.

**Mitigation:** Layer B's accessibility validation conformance category
(when written, see §4.3) becomes the project-specific signal that
complements OSHWA Certified. The project's own conformance attestation +
OSHWA Certified together are stronger than either alone. Neither replaces
the missing accessibility regulatory framework — there is no "FDA for
assistive HID devices" and we should not overclaim.

**Risk 3 — Conformance doc creates implicit legal liability if a
self-attestation is false.**

*If a third party falsely claims compatibility and a user is harmed, is the
project on the hook?*

This is the standard implied-endorsement question. The OSHWA / USB-IF /
Adafruit pattern is well-established and has not produced notable liability
exposure to those organizations in 6+ years (OSHWA), 24+ years (USB-IF),
or 8+ years (Adafruit CircuitPython compatibility list). The triple
disclaimer pattern (§4.6) is the standard response.

**Specific belt-and-braces mitigations:**

1. The triple disclaimer in `docs/CONFORMANCE.md`.
2. No registry exists today; it activates only when a third party ships
   AND opens a PR. If the activation criteria turn out to be insufficient,
   we can decline to merge the PR — there is no obligation to operate a
   registry.
3. Self-attestation framing places legal posture on the attesting third
   party, not on the project. "The vendor self-attested" not "we certified."
4. The project is an unincorporated open-source community, not a legal
   entity. Plaintiffs would have to sue contributors individually — a
   poorly-positioned legal theory given the explicit no-warranty language
   of CERN-OHL-S 2.0 §6 and MIT.

**Honest assessment:** This is a real but low-probability risk. The
mitigation pattern is well-established. The project's accessibility-focused
mission and pre-revenue status make it a poor litigation target.

**Risk 4 — Reference manufacturer doc creates supply-chain misinformation
risk.**

*New risk the prompt didn't name but that I think matters.*

`docs/REFERENCE-MANUFACTURERS.md` is a living doc of factories and services.
Factories change. MOQs change. Lead times change. Yields drift. A doc
that's accurate in 2026 may mislead a downstream entrant in 2028.

**Mitigation:**
1. Every entry includes a "Last verified" date.
2. The doc is structured so stale entries are visually obvious (date +
   visual marker for entries > 12 months unverified).
3. Explicit disclaimer at the top: "This is documentation of public
   knowledge at specific timestamps. Verify before placing orders. No
   relationship, no endorsement, no referral fee exists for any listed
   vendor."
4. The Seeed Studio Fusion+Propagate placeholder includes an explicit "no
   batch run yet, no realized COGS" annotation so the absence of data is
   not mistaken for an endorsement.

---

## 6. Effort Estimate

In project-native effort tiers (Small / Medium / Large). Per the project's
no-time-estimates rule, no calendar projections are given.

### 6.1 Per-file effort

| File | Effort | Notes |
|---|---|---|
| `LICENSE-DOCS` | **S** | CC-BY-SA 4.0 boilerplate + copyright line. Trivial. |
| `docs/NAMING-AND-COMPATIBILITY.md` | **M** | Plain-language doc, ~600-1000 words. Includes phrasing playbook, OSHWA Certified pointer, bad-faith response posture. Wants careful tone. |
| `docs/CONFORMANCE.md` | **M-L** | 4 categories + accessibility-gap flag + triple disclaimer + future-registry placeholder. Borderline L if accessibility scenario protocol is in scope (§4.3 option 2). M if not. |
| `docs/REFERENCE-MANUFACTURERS.md` | **M** | Tier structure + Seeed placeholder + disclaimer + verification-date scaffolding. ~500-800 words structural; data fills in over time. |
| `README.md` edit | **S** | One paragraph in License section + three-license declaration. |
| `LICENSE-HARDWARE` header | **S** | One-paragraph annotation. CERN-OHL-S body untouched. |
| `LICENSE-SOFTWARE` header | **S** | Same. |
| `CONTRIBUTORS.md` edit | **S** | One sentence about doc license. |
| `docs/IP-STRATEGY.md` edits | **M** | Cross-references + conveyance vs. internal-modification interpretation note + Action Items update. |
| `docs/DEFENSIVE-PUBLICATION.md` §3 correction | **S-M** | Fix gap analysis to reflect OSHWA's actual blocker list. |

### 6.2 Total per phase

- **Phase 1 (Layer A):** ~5 S + 1 M = **M total.** The longest piece is
  `docs/NAMING-AND-COMPATIBILITY.md`.
- **Phase 2 (Layer B):** ~1 M-L = **M-L total.** Hinges on §4.3 decision.
- **Phase 3 (Layer C):** ~1 M = **M total.**

**Bundled effort if all three ship: M-L total.** Comparable to writing
`docs/SURFACE-TEST-PROTOCOL.md` or `docs/IP-STRATEGY.md` from scratch.

### 6.3 Effort NOT included in this scope

| Activity | Effort | When |
|---|---|---|
| Actual OSHWA certification submission | S (mostly form-filling) | After phase 1 + first routed PCB + complete BOM with part numbers |
| Writing accessibility scenario validation protocol | L (separate scoping) | Independent of this revision |
| Writing the four conformance test scripts | M-L (firmware-side) | Activation-triggered; not now |
| Building the `compatible-devices/` registry | S (instructions) + M (review process) | Activation-triggered; not now |
| Filling `docs/REFERENCE-MANUFACTURERS.md` with real entries | Ongoing | Each batch run adds an entry |

---

## 7. Recommendation Summary (Re-Stated)

| Layer | Recommendation | Effort | Phase |
|---|---|---|---|
| A — Naming and compatibility guidance + `LICENSE-DOCS` | **Ship.** | M | 1 |
| B — Conformance criteria (doc only, no scripts/registry yet) | **Ship.** | M-L | 2 |
| C — Reference manufacturer graph | **Ship.** | M | 3 |
| D — Prusa OCL dual-licensing | **Drop. No revisit horizon.** | — | — |

**Pushback summary** (where I think the prompt was wrong-shaped):

1. The prompt framed OCL as plausibly worth evaluating. The research is
   unambiguous: it isn't, and the problem it was supposed to solve is
   already solved by CERN-OHL-S 2.0's conveyance scope.
2. The prompt didn't anticipate the documentation-license blocker, which is
   the actual primary blocker for OSHWA certification today. Layer A grew to
   absorb it.
3. The prompt suggested the conformance suite go in `trial-scripts/`. That
   directory holds KiCad utility scripts. Conformance belongs in a separate
   `conformance/` directory at repo root — but not yet, because no third
   party exists to attest.
4. The prompt's wand-and-puck framing in §1.3 (multi-variant cert) implied
   waiting for an umbrella cert. OSHWA does not offer one. Each variant gets
   its own UID; the citation chain is the family.

**The current CERN-OHL-S 2.0 + MIT stack is not optimal *as it stands*.** It
needs a documentation license declared, the descriptive-name posture made
explicit, and a published criterion for compatibility claims. Layers A+B+C
fill those gaps without disturbing the load-bearing licenses, without
adding a custom mark, without filing a trademark, and without committing to
infrastructure (registry, test scripts, filled manufacturer table) that the
project doesn't yet need.

---

*End of scoping. No code or files have been changed beyond the creation of
this document. Awaiting BDFL approval for phased implementation.*
