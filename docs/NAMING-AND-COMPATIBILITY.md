<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Naming and Compatibility Guidance

**Plain language. No legal advice. The project's posture on its own name and
on derivative-maker compatibility claims.**

This document is meant to be read in five minutes by someone considering
building, selling, modifying, or referencing PowerFinger. If you are a lawyer
or downstream productizer reading this and want the underlying license text,
see [`LICENSE-HARDWARE`](../LICENSE-HARDWARE) (CERN-OHL-S 2.0) and
[`LICENSE-SOFTWARE`](../LICENSE-SOFTWARE) (MIT). Documentation is under
CC-BY-SA 4.0 — see [`LICENSE-DOCS`](../LICENSE-DOCS).

---

## 1. The Name Is Descriptive, Not a Trademark

"PowerFinger" — and the variant names "PowerPen" and "PowerPuck" — are used
descriptively throughout this project. **No trademark is asserted by the
maintainers.** The project name has not been registered with the USPTO, EUIPO,
or any other trademark office, and the maintainers have no plans to register
it.

This is a deliberate choice. The project is an open-source accessibility
project that aims to make assistive input devices as cheap and as widely
adopted as possible. A trademark would create a policing burden, an
appearance of corporate posture inconsistent with the project's mission, and
ongoing maintenance cost in exchange for protections we don't think we need.

The reciprocal license (CERN-OHL-S 2.0), the patent retaliation clause
embedded in it, the OSHWA Certified path for derivative makers (when
applicable), and the project's accessibility-shield narrative do the
load-bearing protection work.

---

## 2. What This Means for Builders, Sellers, and Modifiers

Anyone may build, sell, modify, and distribute the open hardware design
under CERN-OHL-S 2.0. Anyone may use, modify, redistribute, and incorporate
the open firmware and software under MIT. The reciprocal hardware license
requires that anyone manufacturing or distributing products based on these
designs publishes their complete modified source. The MIT license has its
usual attribution and no-warranty terms.

That is the entire legal framework. There is no separate "permission" you
need from the maintainers to build, sell, or modify a PowerFinger device.
There is no royalty, no licensing fee, no signed agreement.

---

## 3. Encouraged Practice for Derivative Makers

Three sentences captures the encouraged practice — none of it is legally
required, just culturally encouraged:

1. **Use your own product name.** It helps end users — especially users
   evaluating accessibility devices, where provenance matters — distinguish
   your product from the reference design and from other derivatives.
2. **Make honest compatibility claims.** Phrasings like "compatible with
   PowerFinger," "based on the PowerFinger reference design," or "for
   PowerFinger users" are welcomed and culturally encouraged.
3. **Pursue OSHWA Certified status** at https://certification.oshwa.org/ if
   you want a third-party-recognizable quality signal. OSHWA Certified is
   what users will look for to distinguish ecosystem products from one-off
   counterfeits.

### Suggested phrasings for compatibility claims

| Honest | Discouraged |
|---|---|
| "Compatible with PowerFinger" | "PowerFinger Pro" (implies official variant) |
| "Based on the PowerFinger reference design" | "Authorized PowerFinger" (no authorization scheme exists) |
| "For PowerFinger" | "PowerFinger Certified" (we don't operate a cert) |
| "PowerFinger-compatible accessibility ring" | "Official PowerFinger" (no official designation exists) |

The discouraged phrasings imply an authorization, certification, or official
status that does not exist. There is no enforcement mechanism behind this
table — it's documentation of social expectations.

---

## 4. The "What If Someone Misuses the Name" Question

The honest answer: without a trademark, the maintainers cannot legally
prevent a bad-faith actor from selling an unsafe product under the
PowerFinger name. The project has decided this trade-off is worth it for the
reasons in §1.

The project's response posture if a bad-faith actor ships a dangerous
product using the PowerFinger name:

1. **Public statement.** Pre-drafted public statement that names the
   bad-faith product, cites the specific safety violations against the
   reference design's published safety analysis (`docs/BATTERY-SAFETY.md`,
   `docs/REGULATORY-PRESCAN.md`), and points users at the OSHWA-certified
   reference variants (when those exist).
2. **OSHWA Certified as the affirmative quality signal.** Users evaluating
   "PowerFinger-named" products will be directed at the OSHWA registry to
   distinguish certified reference variants from non-certified products.
   The OSHWA UID is the distinguishing mark we don't have to defend
   ourselves.
3. **CERN-OHL-S 2.0 source disclosure as the legal lever.** A counterfeiter
   distributing the design under the project name without source disclosure
   is in license violation. This is enforceable by any contributor who
   holds copyright on a portion of the design — including via cease-and-
   desist letters.
4. **Community and press mobilization.** The accessibility-shield narrative
   in `docs/IP-STRATEGY.md` is the project's strongest defense. Frame the
   incident accurately: the bad-faith actor is harming users of an
   open-source accessibility project. Engage EFF, accessibility advocacy
   organizations, and the open-hardware press.

This posture is weaker than a trademark would be in absolute terms. It is
what the project has by deliberate choice, and what works.

---

## 5. The Three-License Stack at a Glance

| Artifact | License | Reason |
|---|---|---|
| Hardware designs (schematics, PCB layouts, 3D models, BOMs, assembly docs) | [CERN-OHL-S 2.0](../LICENSE-HARDWARE) | Strongly reciprocal, with patent retaliation. The load-bearing protection. |
| Firmware and software (ESP-IDF code, companion app) | [MIT](../LICENSE-SOFTWARE) | Maximum adoption, since firmware-without-hardware is not closure-attractive. |
| Documentation (`docs/*.md`, READMEs, assembly guides) | [CC-BY-SA 4.0](../LICENSE-DOCS) | Share-alike on docs matches the share-alike posture on hardware. |

If you are mixing PowerFinger artifacts into your own work, apply the
appropriate license per artifact type. A derivative product manual that
quotes from `docs/SURFACE-TEST-PROTOCOL.md` is under CC-BY-SA 4.0 by
inheritance; a derivative ring shell CAD file is under CERN-OHL-S 2.0; a
derivative firmware fork is under MIT.

---

## 6. Internal Modification vs. Conveyance (CERN-OHL-S 2.0 Clarification)

A common question from print farms, care facilities, and other internal
production users: **Do I have to publish my changes if I'm only using the
modified design internally?**

**No.** CERN-OHL-S 2.0's source-disclosure obligation triggers on
**conveyance** — distribution outside the organization. A care facility
that 3D-prints a custom shell variant for one of its residents and uses it
internally has not conveyed anything and has no disclosure obligation. The
disclosure obligation activates only if the facility distributes the
modified design to a third party.

This is a built-in feature of the license, not a special arrangement for
this project. See CERN-OHL-S 2.0 §3.1 for the precise definition of
conveyance.

If you are uncertain whether your specific use case qualifies as
"internal," err on the side of consulting your organization's counsel.
This document is not legal advice.

---

## 7. Cross-References

- [`LICENSE-HARDWARE`](../LICENSE-HARDWARE) — CERN-OHL-S 2.0 license text
- [`LICENSE-SOFTWARE`](../LICENSE-SOFTWARE) — MIT license text
- [`LICENSE-DOCS`](../LICENSE-DOCS) — CC-BY-SA 4.0 license text
- [`docs/IP-STRATEGY.md`](IP-STRATEGY.md) — full IP defense strategy,
  patent landscape, accessibility-shield narrative
- [`docs/CONFORMANCE.md`](CONFORMANCE.md) — published criteria for
  derivative makers claiming PowerFinger compatibility
- [`docs/REFERENCE-MANUFACTURERS.md`](REFERENCE-MANUFACTURERS.md) — public
  knowledge of factories and services that have run open-hardware projects
  at various volumes
- OSHWA certification: https://certification.oshwa.org/

---

*This document is part of the PowerFinger documentation set, licensed under
CC-BY-SA 4.0. It is not legal advice. The project consists of unincorporated
contributors, not a legal entity.*
