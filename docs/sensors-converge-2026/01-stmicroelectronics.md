<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# STMicroelectronics — Tier S

**What they make:** STM32 MCU family (huge), STM32WB / STM32WBA series with
integrated BLE 5, LSM/ISM IMU and motion sensors, VL53 ToF sensors, MEMS
microphones, capacitive touch ICs. Headline sponsor of Sensors Converge 2026.

## Why they matter for PowerFinger

ST is the only single-vendor silicon partner that could plausibly supply the
**entire** PowerFinger BOM:

- **MCU + BLE:** STM32WB55 or STM32WBA52 replaces the documented ESP32-C3 →
  nRF52840 path with a third candidate. WBA in particular has best-in-class
  idle current (sub-µA in BLE sleep) and would beat the nRF52840 power numbers
  the project currently relies on. ST runs Zephyr support officially, so the
  toolchain migration cost is shared with the nRF52840 plan.
- **IMU:** LSM6DSOX / LSM6DSV with on-chip ML core could replace Bosch BHI for
  the IMU-variant ring.
- **Optical / ToF:** VL53L4CD could feed the wand's tip-presence detection or
  serve as an alternative ranging sensor for off-surface modes.
- **MEMS click:** ST piezo / haptic drivers could replace the LRA path.

## The ask

Two layered asks, depending on how the booth conversation goes:

1. **Minimum viable:** STM32WBA dev kit + a named FAE for low-power BLE HID
   guidance. This is what they hand out routinely; almost no risk of refusal.
2. **Stretch:** ST Partner Program inclusion as an open-source assistive-input
   reference design. Featured on the ST community blog (ST Blog) and listed in
   their solutions directory. ST has historically supported visible accessibility
   stories.

## What you bring

- A published, production-grade reference design that uses ST silicon end-to-end
  if a design-win conversation closes. ST gets a credible, technically rigorous,
  sub-25mW wearable BLE design to point at.
- Public BLE HID + sensor-fusion + low-power code under MIT — usable directly
  by every other ST customer asking the same questions.
- Accessibility narrative + Tolkien-adjacent PR upside (the One Ring pitch is
  not for the booth conversation, but it's in the back pocket if a marketing
  contact gets engaged).

## Conversation cues

- If the booth foregrounds **STM32WBA**, the BLE/wearable story is live and
  this is your conversation. Ask about the Partner Program directly.
- If the booth foregrounds **automotive / industrial**, this is the wrong day;
  ask for a wearable-focused contact and leave a one-pager.
- If they have an **ST Authorized Partner / "Partner Program"** sign visible,
  that's the program you want.

## Risks / why they might say no

- ST sells to design wins of 100k+ units. PowerFinger is pre-prototype with no
  volume forecast. Their inside-sales floor may not engage.
- The CERN-OHL-S reciprocal license means anyone manufacturing must publish.
  ST customers in regulated industries may see this as a contamination risk.
  Acknowledge it directly: PowerFinger does not impose CERN-OHL-S on customers
  using ST silicon for unrelated products.
- ST may want exclusivity ("you'll only feature ST in this design"). The
  project's defensive-publication posture and CERN-OHL-S forbid exclusive
  silicon lock-in commitments. Be explicit about this.

## Minimum viable outcome from a 5-minute booth visit

- Named FAE or partner-program contact email.
- Confirmation that STM32WBA dev kits can be sampled.
- A pointer to whichever ST blog / showcase you'd publish to.

## Post-show follow-up path

1. Email the named contact within 48 hours with the project one-pager and a
   single specific question (e.g. "what's the STM32WBA equivalent of nRF52840
   adaptive connection-interval power numbers?").
2. If engaged, propose a parallel BOM evaluation: build a Standard-tier
   prototype on STM32WBA alongside the nRF52840 plan, publish both power
   numbers, let the data choose.
3. If the parallel build closes, formally apply to ST Partner Program with a
   working demo.

## Cross-references

- [`../NRF52840-MIGRATION.md`](../NRF52840-MIGRATION.md) — the existing MCU
  migration plan that an STM32WBA conversation would fork or replace.
- [`../CONSUMER-TIERS.md`](../CONSUMER-TIERS.md) §"Power Reality and MCU
  Strategy" — context on why MCU choice is the project's largest open question.
- [`../IP-STRATEGY.md`](../IP-STRATEGY.md) — the CERN-OHL-S posture to
  surface in any silicon-partner conversation.
