<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# TDK InvenSense — Tier A

**What they make:** ICM-series IMUs (ICM-42688, ICM-45686 — among the lowest-
noise consumer IMUs on the market). MEMS microphones. Ultrasonic ToF (Chirp
sensors). PiezoPlanar / PiezoListen films. Battery-related passives.

## Why they matter for PowerFinger

Two mostly-independent reasons to talk to TDK:

1. **IMU alternative to Bosch.** ICM-42688-P and ICM-45686 are at or above
   Bosch BMI270 in noise / current / size. TDK's IMU stack does not generally
   include on-chip fusion (their SmartMotion line did, but TDK has been
   sunsetting it), so the comparison is "raw IMU, host-side fusion" — useful
   only if Bosch's BSX licensing turns out incompatible with the project's
   reciprocal license (see `./03-bosch-sensortec.md`).
2. **Piezo film for the Pro click.** TDK acquired the assets that produced
   PVDF piezo film products that are well-suited to the Pro tier's sealed
   click mechanism. `../CLICK-MECHANISMS.md` and `../CONSUMER-TIERS.md` describe
   the Pro click as "piezo film + LRA haptic" — TDK is one of the few
   single-vendor sources for both halves of that.

## The ask

1. **Minimum viable:** ICM-42688 dev kit; piezo film sample if their booth
   handles it.
2. **Stretch:** Engineering contact for both IMU and piezo lines (often
   different teams within TDK). A pointer to whichever app note covers
   piezo-film signal conditioning for the Pro click ADC threshold approach.

## What you bring

- A real-world piezo-film design-in target. PVDF piezo film is mostly used in
  industrial / NDT applications today; consumer wearable click is a
  not-yet-mainstream use case TDK could showcase.
- Comparative IMU data (TDK vs. Bosch) published openly — useful to TDK's sales
  team in design competitions where they don't always have public data.

## Conversation cues

- If the booth foregrounds **MEMS speakers (PiezoListen)** or **ultrasonic
  ToF**, that's their wearable-vertical story; piezo click is the closest
  adjacent use case to mention.
- If the booth foregrounds **automotive IMU**, ask for a different contact.
- Look for **smartwatch reference design displays** — their consumer-wearable
  team is the right target.

## Risks / why they might say no

- TDK consolidated InvenSense (IMU) and other acquisitions into multiple sales
  channels. The piezo-film team and the IMU team may not coordinate; you might
  need two booth conversations.
- Piezo-film samples for a sub-100-unit prototype run may not be possible from
  the booth — sometimes those go through a separate distributor process.
- TDK's design-win threshold is similar to Bosch's; lead with the open-source
  reference-design framing.

## Minimum viable outcome from a 5-minute booth visit

- ICM-42688 dev kit sample commitment OR pointer to where to buy one cheaply
  if no sampling is possible.
- Piezo-film team contact OR distributor pointer.
- Quick gut-check on whether their ICM-45686 is meaningfully better than the
  ICM-42688 for the wand's ball+Hall variant tilt detection.

## Post-show follow-up path

1. Email IMU-team contact within 48 hours; specific question about ICM-42688
   FIFO behavior in low-power motion-detect-wake mode.
2. Email piezo-film contact separately; specific question about minimum sample
   size for an evaluation run.
3. If both are positive, treat TDK as the **fallback** silicon for the IMU
   variant and the **primary candidate** for piezo-film sourcing in the Pro
   click design.

## Cross-references

- [`./03-bosch-sensortec.md`](03-bosch-sensortec.md) — visit Bosch first; TDK
  is the alternative if Bosch's BSX licensing fails.
- [`../CLICK-MECHANISMS.md`](../CLICK-MECHANISMS.md) — piezo click design
  context.
- [`../CONSUMER-TIERS.md`](../CONSUMER-TIERS.md) §"Pro" — piezo film + LRA
  haptic Pro click.
