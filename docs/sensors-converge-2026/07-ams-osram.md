<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# ams OSRAM — Tier A

**What they make:** Optical sensing IP — ambient light, proximity, color, ToF
(TMF series), VCSEL emitters. Spectral / NIR sensors. LED drivers. Strong in
automotive interior sensing, smartphone front-facing sensors, and smartphone
ToF.

## Why they matter for PowerFinger

ams OSRAM's optical-sensing portfolio is mostly orthogonal to PowerFinger's
current direct-optical mouse sensor stack (PixArt's domain), but they have
two adjacent angles:

1. **Tip-presence / proximity detection on the wand.** A small ambient/
   proximity sensor on the wand tip could detect "is the wand near a surface,
   and at what distance?" — useful for adaptive sensitivity and for waking the
   ball+Hall sensor only when the wand is in use. TMD-series proximity sensors
   are the right category.
2. **VCSEL guidance laser.** The deferred "guidance laser" capability in
   `../README.md` ("red dot on the surface shows exactly where you're
   pointing/scanning") — if pursued, ams OSRAM is one of two or three
   single-vendor sources for low-power VCSEL emitters and integrated dot
   projectors.

Neither is in the active validation lane. This is forward-looking
reconnaissance.

## The ask

1. **Minimum viable:** Datasheet pointers for their lowest-power proximity
   sensor and their lowest-power VCSEL emitter (sub-mA average preferred).
2. **Stretch:** A sample TMD-series proximity sensor and a 5-minute
   conversation about VCSEL eye-safety class compatibility with a body-worn
   guidance laser.

## What you bring

- Honest framing: this is a forward-looking conversation, not a current
  design-in. The wand-tip proximity and guidance-laser capabilities are
  Gate-4-or-later. ams OSRAM is unlikely to be excited.
- An open-hardware reference design that, if it eventually adopts their
  parts, would be public — visibility in the open-hardware accessibility
  segment is genuinely novel for ams OSRAM.

## Conversation cues

- If the booth foregrounds **smartphone proximity / ambient light**, you're
  near the right team — ask about wearable-grade proximity sensors.
- If the booth foregrounds **automotive in-cabin sensing**, it's the wrong
  team but the underlying tech is the same; ask for a consumer-vertical
  contact.
- If they have **VCSEL** demos visible — that's the guidance-laser story.

## Risks / why they might say no

- ams OSRAM's design wins are heavily skewed toward major smartphone /
  automotive OEMs. PowerFinger is far below their engagement threshold.
- VCSEL eye safety (Class 1 vs higher) is a regulatory gating question that
  could disqualify the guidance-laser concept entirely — better to discover
  this at the booth than later.

## Minimum viable outcome from a 5-minute booth visit

- Datasheet pointer for one suitable proximity sensor.
- Confirmation (or denial) that a Class-1 eye-safe VCSEL guidance laser is
  feasible in their portfolio.

## Post-show follow-up path

1. No high-priority follow-up unless the guidance-laser feature becomes active.
2. Re-engage if and when the deferred capabilities in `../README.md` move
   into the validation lane.

## Cross-references

- [`../README.md`](../README.md) §"Deferred Capabilities" — guidance laser
  context.
- [`../COMBINATORICS.md`](../COMBINATORICS.md) — wand variants where
  proximity sensing could be relevant.
