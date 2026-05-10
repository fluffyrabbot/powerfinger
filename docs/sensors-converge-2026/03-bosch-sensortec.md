<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Bosch Sensortec — Tier S

**What they make:** BMI series IMUs (BMI270, BMI323), BHI series smart sensor
hubs (BHI260, BHI360 — with on-chip Fuser2 fusion DSP and self-learning
classifier), BME environmental sensors, BMP barometric. The IMU vendor of
record for serious wearables — used in Pixel Watch, Whoop, Oura, etc.

## Why they matter for PowerFinger

The IMU-variant ring is in the deferred capabilities list, but it's the only
variant that needs **no surface at all** — and the BHI smart-sensor-hub family
makes it dramatically easier than a raw IMU + DSP firmware approach.

- **BHI360 specifically** runs sensor fusion on-chip and exposes a high-level
  motion API to the host MCU, which means:
  - The MCU stays in deep sleep most of the time (BHI360 wakes it on a
    classified event, e.g. "finger pointing gesture detected").
  - PowerFinger firmware does not need to implement quaternion fusion, drift
    correction, or gesture classification — Bosch ships it.
  - Battery life on the IMU variant could plausibly exceed the surface-tracking
    variants because the host MCU is asleep more of the time.
- **BMI270 alone** is the lower-cost option for a "raw IMU + simple firmware
  fusion" path. Used in countless wearable design wins.

## The ask

1. **Minimum viable:** BHI360 / BMI270 dev kits + datasheet beyond NDA-walled
   sections. Bosch has historically gated some BHI fusion docs behind NDA,
   which is incompatible with CERN-OHL-S — surface this constraint immediately.
2. **Stretch:** Featured "wearable design win" placement in their reference
   design portfolio. Possibly a Bosch Sensortec Community blog post once a
   working IMU-variant demo exists.

## What you bring

- A wearable design that legitimately stresses the BHI360's "low-power motion
  classifier" value proposition. Most consumer-watch design wins underuse the
  on-chip fusion features; PowerFinger's IMU variant could showcase them.
- A public, open-source reference firmware implementation for BHI360 BLE HID —
  something Bosch's developer audience would benefit from.

## Conversation cues

- If the booth foregrounds **BHI360 / BHI260AP**, you're in the right
  conversation. Ask about the BSX fusion library licensing terms specifically —
  this is the load-bearing question for CERN-OHL-S compatibility.
- If the booth shows **BMI323** more prominently, that's the newer general-purpose
  IMU — useful for the cheaper IMU-variant path.
- Look for **demo wearables on display** (smartwatches, fitness bands) — Bosch
  routinely showcases customer products. Use them as conversation hooks.

## Risks / why they might say no

- Bosch's BSX fusion library is partially closed-source. If their fusion code
  cannot be redistributed under MIT/Apache-compatible terms, the IMU variant
  may need raw-sensor mode and host-side fusion (which loses the BHI360's
  battery-life advantage). **This is the single most important question to
  resolve at the booth.**
- Bosch's design-win threshold is volume-oriented like ST and Microchip. Lead
  with the open-source / accessibility / reference-design framing, not with a
  unit forecast.
- The IMU variant is currently deferred (Gate 6+). Be honest that this is a
  forward-looking conversation, not an immediate design-in.

## Minimum viable outcome from a 5-minute booth visit

- Clear answer on BSX fusion library licensing — can it be shipped in an
  MIT-licensed firmware package? If not, can the host-side fusion path be
  documented?
- BHI360 or BMI270 dev kit sample commitment.
- Named contact in their developer-relations or wearables-vertical team.

## Post-show follow-up path

1. Email contact within 48 hours; attach one-pager; specifically ask the BSX
   licensing question in writing for a paper trail.
2. If BSX is incompatible with the project's licensing, document the gap in
   `../IP-STRATEGY.md` and update the IMU-variant plan in
   `../COMBINATORICS.md` accordingly.
3. If BSX is compatible, the IMU variant moves up the priority order in
   `../GO-NO-GO-RUBRIC.md`.

## Cross-references

- [`../COMBINATORICS.md`](../COMBINATORICS.md) — IMU variant in the design
  matrix.
- [`../POWER-BUDGET.md`](../POWER-BUDGET.md) — BHI360's smart-sensor-hub
  architecture changes the power budget shape for the IMU variant.
- [`../IP-STRATEGY.md`](../IP-STRATEGY.md) — closed-source fusion libraries
  conflict with the project's reciprocal-license posture; resolve before
  committing to a Bosch path.
