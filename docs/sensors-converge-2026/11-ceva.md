<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Ceva — Tier B

**What they make:** DSP IP licensing — sensor fusion (Hillcrest Labs / MotionEngine,
acquired by Ceva), audio DSP, Bluetooth and Wi-Fi IP cores. Their business is
licensing IP to silicon vendors who fab their own chips; not to end-product
designers.

## Why they matter for PowerFinger

Almost not at all, directly. Ceva is silicon-IP-vendor-to-silicon-vendors. The
chips they enable (e.g. Bosch BHI fusion runs on a Ceva DSP core under the hood)
are the parts PowerFinger would actually use. Engaging Ceva at the booth is
mostly a courtesy / context-gathering visit.

Two narrow reasons to stop by:

1. **MotionEngine context.** Hillcrest's MotionEngine fusion is licensable as
   software for host-side execution if Bosch's BSX path doesn't work. Worth
   understanding the licensing terms in case the IMU variant needs a host-side
   fusion library that isn't tied to a specific silicon vendor.
2. **Background knowledge.** Their booth conversation gives industry context
   on which silicon vendors are licensing what fusion / DSP IP — useful for
   reading the Bosch / TDK / ams / ADI conversations.

## The ask

1. **Minimum viable:** Pointer to MotionEngine licensing terms and whether a
   non-commercial / open-source license tier exists (it currently does not,
   per public sources, but ask).
2. **Stretch:** None realistic. Ceva does not sell to projects at PowerFinger's
   scale.

## What you bring

- Almost nothing of business value to Ceva. Be efficient with their time.

## Conversation cues

- If the booth foregrounds **smartphone / hearable audio DSP**, that's their
  primary business; the motion-fusion / MotionEngine path is secondary at the
  booth.
- Look for **Hillcrest Labs / MotionEngine** signage specifically — the
  acquisition signage may not be prominent.

## Risks / why they might say no

- They will absolutely say no to anything resembling a partnership.
  Reconnaissance only.

## Minimum viable outcome from a 5-minute booth visit

- Quick read on whether MotionEngine has any open-source / non-commercial
  licensing tier.
- Confirmation of which silicon vendors at the show license Ceva fusion IP
  (helps decode Bosch / TDK / ams positioning).

## Post-show follow-up path

1. None planned.

## Cross-references

- [`./03-bosch-sensortec.md`](03-bosch-sensortec.md) — Bosch BSX is the
  primary host-fusion conversation; Ceva is fallback / context only.
