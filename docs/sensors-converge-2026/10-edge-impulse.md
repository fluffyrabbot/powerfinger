<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Edge Impulse — Tier B

**What they make:** Cloud-based and on-device TinyML platform — data ingestion,
labeling, model training, deployment to MCUs. Free tier exists; paid tiers for
enterprise. Headline sponsor of Sensors Converge 2026.

## Why they matter for PowerFinger

Edge Impulse is only relevant to PowerFinger if **gesture or motion
classification** ever becomes part of the input model. Today it isn't. The
project's primitives are cursor + click + scroll; everything else is
software-defined combinations of those four primitives (per `../README.md`).

Forward-looking scenarios where Edge Impulse becomes interesting:

- **IMU-variant gesture recognition.** "Quick double-flick of the wrist =
  back button," "twist motion = scroll wheel substitute," etc. Useful for
  users whose mobility limits available primitives. This is the strongest
  case for a TinyML model on PowerFinger.
- **Adaptive sensitivity / tremor compensation.** A model that learns the
  individual user's tremor profile and filters it. This is genuinely
  accessibility-relevant — but it's also the kind of thing that should be
  deeply scrutinized for whether classical signal processing (Kalman / IMM
  filtering) does the job better than ML.

Edge Impulse's free tier is good enough for prototyping; their no-cloud /
on-device deployment story means a TinyML feature would not violate the
project's "no cloud dependency" rule (per `CLAUDE.md`).

## The ask

1. **Minimum viable:** Confirmation that their free tier supports the project's
   target MCU (whichever it ends up being — ESP32-C3, nRF52840, STM32WBA).
2. **Stretch:** Editorial / blog feature once a TinyML accessibility-flavored
   demo exists. Edge Impulse's blog actively features accessibility projects.

## What you bring

- A future demo of TinyML for accessibility-specific use case (tremor
  compensation, gesture recognition for low-mobility users) — Edge Impulse's
  marketing audience would value this content.
- An open-source reference design that uses their platform without locking
  users in (the project's open-data, no-cloud posture pairs cleanly with their
  on-device deployment story).

## Conversation cues

- If the booth has **EON Tuner** demos visible, ask about model-size /
  inference-time numbers for IMU-fed classification on Cortex-M4-class targets.
- If the booth foregrounds **enterprise / industrial customers**, this is the
  wrong day; ask for a community / developer-relations contact.

## Risks / why they might say no

- Their commercial model depends on enterprise customers; an open-source
  project at PowerFinger's scale is editorial value at best, revenue zero.
- The TinyML feature itself is speculative for PowerFinger. Be honest at the
  booth: this is reconnaissance, not a commitment.

## Minimum viable outcome from a 5-minute booth visit

- Pointer to their best example project for IMU classification on a low-power
  MCU.
- Developer-relations contact email.

## Post-show follow-up path

1. No high-priority follow-up.
2. Re-engage if and when an IMU-variant gesture-recognition feature moves
   into an active validation lane.

## Cross-references

- [`../COMBINATORICS.md`](../COMBINATORICS.md) — IMU variant in the design
  matrix.
- [`../README.md`](../README.md) §"What Exists Today" — current scope is
  cursor / click / scroll; gesture classification is not in it.
