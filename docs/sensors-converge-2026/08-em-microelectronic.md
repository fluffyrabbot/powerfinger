<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# EM Microelectronic — Tier B

**What they make:** EM9304 / EM9305 ultra-low-power BLE controllers, RFID /
NFC ICs, smart-card / metering ICs, energy-harvesting power-management ICs.
Swiss; part of the Swatch Group. Famous for the lowest idle-current BLE
silicon in the industry.

## Why they matter for PowerFinger

The EM9304 / EM9305 family targets **coin-cell wearables that must run for
months or years on a single CR2032**. That positioning is in direct line with
PowerFinger's "Light usage = 18–30 days" projection on nRF52840 in
`../CONSUMER-TIERS.md`.

- **EM9304** can be used as a co-processor BLE controller alongside a
  general-purpose MCU, isolating BLE radio current from MCU activity.
- **EM9305** is more integrated — application MCU + BLE in one die — and is
  the more interesting candidate for a single-chip ultra-low-power ring
  variant.

This is a **lateral exploration** rather than a primary path. nRF52840 +
optimization is already documented as the consumer target. EM Microelectronic
is the answer to the question "what if we wanted weeks-of-battery on a CR2032
in a smaller ring?"

## The ask

1. **Minimum viable:** EM9305 datasheet and idle-current numbers. Comparison
   with nRF52840 in HID-equivalent BLE workload.
2. **Stretch:** Sample dev kit. Honestly tell them you're evaluating against
   nRF52840 and want to compare in your own workload.

## What you bring

- An open-source reference design that publishes head-to-head idle-current
  numbers vs. nRF52840 in a real wearable workload — useful data EM doesn't
  always have public.
- The accessibility / open-hardware visibility is novel for them.

## Conversation cues

- If the booth foregrounds **smartwatch / fitness wearable** — right team.
- If the booth foregrounds **smart-card / RFID** — wrong team, ask for the
  wearable BLE contact.
- Look for **EM9305** specifically; the EM9304 is older and more limited.

## Risks / why they might say no

- EM Microelectronic is small and focused. PowerFinger may not register as
  a meaningful design-in opportunity.
- Toolchain ecosystem is thinner than Nordic / ST / Microchip. Porting cost
  from ESP-IDF / Zephyr is real and may not pay back.
- Their wearable-design wins are heavily NDA'd; they may not be able to share
  comparable customer-design data even if they have it.

## Minimum viable outcome from a 5-minute booth visit

- EM9305 datasheet + idle-current numbers.
- Yes/no on whether their toolchain has a Zephyr port (it currently does not,
  per public sources).

## Post-show follow-up path

1. Defer. EM Microelectronic is "interesting if Nordic and ST both
   underperform on power and PowerFinger needs a third escape hatch."
2. If the Standard tier ever needs sub-µA idle for a coin-cell variant, EM
   becomes the primary candidate.

## Cross-references

- [`../POWER-BUDGET.md`](../POWER-BUDGET.md) — current MCU power numbers.
- [`../NRF52840-MIGRATION.md`](../NRF52840-MIGRATION.md) — primary low-power
  MCU plan.
- [`./01-stmicroelectronics.md`](01-stmicroelectronics.md) — STM32WBA is the
  more probable third option before EM is reached.
