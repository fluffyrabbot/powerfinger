<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Microchip — Tier S

**What they make:** PIC32, AVR, SAM, dsPIC MCU families. PIC32CX-BZ /
WBZ351-WBZ451 wireless families with integrated BLE 5.x. Wide sensor-conditioning
analog portfolio. MPLAB toolchain. Active maker / educational outreach.
Headline sponsor of Sensors Converge 2026.

## Why they matter for PowerFinger

Microchip is the second-leg silicon platform that could supply MCU + BLE for
PowerFinger end-to-end, and they are the most maker-friendly of the headline
sponsors. Specific angles:

- **MCU + BLE:** WBZ351 / WBZ451 are integrated BLE 5.x SoCs in the same
  category as nRF52840 and STM32WBA. Their idle current and HID stack are
  acceptable for the Standard tier; less polished than Nordic's, more accessible
  than ST's.
- **AVR / SAM at the low end:** for a hub or a future stripped-down secondary
  device, Microchip's portfolio reaches lower than ST's or Nordic's.
- **Maker reach:** Microchip acquired Atmel, which means the Arduino
  ecosystem's roots lead back to them. They actively cultivate hobbyist /
  educational projects and have a history of supporting accessibility-flavored
  reference designs.

## The ask

1. **Minimum viable:** WBZ451 Curiosity board + harmony BLE stack guidance from
   an FAE.
2. **Stretch:** Microchip Design Partner Network listing + featured project on
   their developer blog. Possibly a sponsored kit for the Makers Making Change
   submission described in `../OUTREACH.md`.

## What you bring

- A small but real open-source design win that lives in the maker / accessibility
  / education segments Microchip already targets — exactly the kind of project
  their developer blog routinely features.
- A second-platform BOM evaluation alongside ST and Nordic, giving Microchip
  visibility into a comparison they don't always get.

## Conversation cues

- If the booth has **Curiosity Nano** boards on display, you're in maker
  territory and the conversation will go well.
- If the booth foregrounds **functional safety / automotive**, ask for a
  different contact and leave a one-pager.
- Look for **MPLAB Harmony** demos — Harmony is their BLE HID stack story, and
  if it's running, you can ask pointed questions about HID descriptor support.

## Risks / why they might say no

- Microchip's BLE stack story is less mature than Nordic's NimBLE or Zephyr
  BLE. If the project's BLE work is already substantial on ESP-IDF / NimBLE,
  porting cost is real and worth flagging up front.
- Their wireless silicon volume targets are similar to ST's (10k+). At
  pre-prototype scale, the inside-sales floor may not engage. Bias the
  conversation toward developer-relations or maker-program contacts, not sales.

## Minimum viable outcome from a 5-minute booth visit

- WBZ Curiosity board sample commitment.
- A developer-relations or maker-program contact email.
- Pointer to MPLAB Harmony BLE HID example code (if it exists) or
  acknowledgment that there isn't one (which is itself useful information).

## Post-show follow-up path

1. Email the named contact within 48 hours; attach project one-pager; ask the
   one specific BLE-HID-stack question.
2. If engaged, propose a parallel evaluation alongside the ST and nRF52840
   builds. Microchip's developer blog audience is the right shape for a
   "low-power BLE HID on WBZ" post.
3. Coordinate with Makers Making Change submission: Microchip-sponsored kits
   for MMC-matched builds is a high-fit ask once a working demo exists.

## Cross-references

- [`../OUTREACH.md`](../OUTREACH.md) §"Makers Making Change" — Microchip
  sponsorship is highly compatible with the MMC pipeline.
- [`../NRF52840-MIGRATION.md`](../NRF52840-MIGRATION.md) — context for any
  MCU swap conversation.
- [`./01-stmicroelectronics.md`](01-stmicroelectronics.md) — visit ST and
  Microchip back-to-back to compare design-win postures.
