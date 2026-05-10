<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Analog Devices — Tier A

**What they make:** Massive analog / mixed-signal portfolio: ADCs, DACs,
op-amps, instrumentation amps, low-power MCUs (ADuCM family, including
ADuCM4050 with integrated BLE), IMU and motion sensors (ADXL series), power
management, RF. The depth-of-analog-knowledge vendor; their app notes are
considered industry references.

## Why they matter for PowerFinger

ADI's portfolio touches three meaningful BOM lines for PowerFinger, but their
consumer-wearable engagement is weaker than ST's, Microchip's, or Bosch's.
The realistic value is reference material and one-off precision components.

- **ADC for the Pro click.** The piezo-film click path needs ADC sampling of
  a millivolt-scale piezo signal with low quiescent current. ADI makes the best
  precision low-power ADCs in the industry. The application-note depth here is
  unmatched.
- **ADXL accelerometers.** ADXL362 in particular has best-in-class
  ultra-low-power motion-wake current. Could replace a Bosch BMI for a "wake
  on tap" use case in the wand or a no-IMU ring variant.
- **ADuCM4050 BLE MCU.** Theoretical alternative to nRF52840 / STM32WBA / WBZ.
  Less mainstream — fewer reference designs, smaller community — but the
  silicon is competent.

## The ask

1. **Minimum viable:** Pointer to the best ADI precision-ADC app note for
   piezo-film signal conditioning, and an ADXL362 sample.
2. **Stretch:** A named applications engineer for one specific question on
   piezo-film ADC threshold detection at sub-100µA current budget. ADI's
   FAEs are worth more than most vendors' for analog questions specifically.

## What you bring

- Honest answer: ADI is unlikely to get meaningful design-in volume from
  PowerFinger, and they probably know it. Lead with curiosity / reference-
  material asks rather than a partnership pitch. The relationship value is
  asymmetric; do not waste their time pretending otherwise.
- If the partnership conversation does open, the angle is the same as ST /
  Microchip: open-source assistive-input reference design.

## Conversation cues

- If the booth shows **healthcare / medical wearables**, that's their
  consumer-wearable story and the right entry point.
- If the booth foregrounds **industrial / aerospace / instrumentation** — the
  default ADI vibe — keep the conversation short and focused on app-note
  pointers.
- Look for **ADXL362** demos. It's the part most directly relevant to a
  PowerFinger conversation.

## Risks / why they might say no

- ADI's consumer-wearable design wins are dominated by major OEMs (Apple,
  Samsung, etc.). PowerFinger is far below the volume threshold their sales
  floor engages.
- Their BLE MCU (ADuCM4050) is a niche product; betting on it would be a poor
  decision relative to STM32WBA / nRF52840 / WBZ.
- App-note depth is the win here — not silicon swap.

## Minimum viable outcome from a 5-minute booth visit

- App-note pointers (URLs or doc numbers) for piezo-film signal conditioning
  and low-power ADC sampling.
- ADXL362 sample if available.
- Acknowledgment from the booth that PowerFinger is below their direct-sales
  threshold but their distributor network can support sampling.

## Post-show follow-up path

1. No high-priority follow-up. ADI is a reference-material partner, not a
   silicon partner.
2. If a specific app note unlocks the Pro click ADC path, cite it in
   `../CLICK-MECHANISMS.md` and move on.
3. Re-evaluate ADI only if Bosch + TDK both fail for IMU and a third option
   becomes load-bearing.

## Cross-references

- [`../CLICK-MECHANISMS.md`](../CLICK-MECHANISMS.md) — piezo ADC threshold
  design.
- [`./03-bosch-sensortec.md`](03-bosch-sensortec.md) /
  [`./05-tdk-invensense.md`](05-tdk-invensense.md) — primary IMU candidates
  before ADI is reached.
