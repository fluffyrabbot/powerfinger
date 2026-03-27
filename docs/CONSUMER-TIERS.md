# Consumer Product Tiers — RFC

PowerFinger prototypes validate sensing, firmware, and form factor. This document
asks: if this becomes a consumer product, where should the SKU lines be drawn?

The prototype BOM ceiling ($25/unit, $10 floor) optimizes for validation speed and
accessibility. A consumer product can afford to price up where the
ergonomic/reliability return justifies it. This RFC identifies the two sharpest
Pareto knees in the design space and recommends them as canonical tiers.

References:
- Design matrix: [COMBINATORICS.md](COMBINATORICS.md)
- Click mechanisms: [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md)
- Glide system: [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md)
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- IP landscape: [IP-STRATEGY.md](IP-STRATEGY.md)

---

## The Pareto Landscape

Six distinct price/capability knees exist above the prototype baseline. Each buys
a qualitatively different thing:

| BOM Range | What You Buy | Key Component Change |
|-----------|-------------|---------------------|
| $9–11 | It works (surface-limited, dome click, adequate) | Baseline: PAW3204 + snap dome |
| ~$15 | It lasts (sealed, no wear components) | + Piezo film + haptic LRA |
| ~$18–20 | It works everywhere (surface-agnostic + precision) | + PMW3360-class optical-on-ball |
| ~$22–25 | It works everywhere and lasts forever | + Both of the above |
| ~$25–30 | It disappears (sub-gram fingertip, wrist hub) | Tethered sensor architecture |
| ~$35–45 | It sees (camera, OCR, new input modality) | + ESP32-S3 + OV2640 + laser |

Each row is a genuine knee — the return on the next dollar drops sharply between
rows and rises sharply at the start of the next row. The question is which two
to standardize.

---

## Why Two Tiers

One tier leaves money on the table: users who would pay more for a device that
doesn't degrade or compromise on surfaces can't. Three or more tiers create SKU
sprawl, confuse buyers, split firmware validation effort, and fragment the
accessory ecosystem (chargers, glide pads, shells). Two tiers is the minimum
that covers meaningfully different user needs.

The two tiers should:
1. **Not overlap** — the upgrade must be obviously worth the price delta.
2. **Share firmware** — same MCU, same BLE stack, same companion app protocol.
3. **Share form factor** — same ring geometry, same parametric sizing.
4. **Differ in sensing + click only** — minimize mechanical design divergence.

---

## Recommendation: Standard and Pro

### Standard — R30-OLED-NONE-NONE (current P0 design)

| Component | Spec | BOM |
|-----------|------|-----|
| MCU | ESP32-C3-MINI-1-N4 | $2.01 |
| Sensor | PAW3204 / ADNS-2080 class optical | $0.60–1.80 |
| Click | Metal snap dome (Snaptron SQ-series) | $0.05–0.15 |
| Battery | LiPo 80–100mAh | $0.80–1.50 |
| Charging | TP4054 + USB-C | $0.15–0.35 |
| LDO | RT9080-33GJ5 (0.5µA Iq) | $0.15 |
| Glide | Raised structural rim + UHMWPE pads | $0.05–0.10 |
| Shell | 3D-printed parametric ring, 30° angle | $0.50–1.00 |
| PCB | Flex or rigid-flex | $2.00–5.00 |
| **Total** | | **$6.50–12.00 (~$9 target)** |

**Surface compatibility:**

| Surface | Works? |
|---------|--------|
| Wood desk | Yes |
| Matte plastic | Yes |
| Paper / magazine | Yes |
| Mouse pad | Yes |
| Glass | No |
| Mirror | No |
| Fabric / clothing | Marginal |
| Skin | Marginal |

**Click lifecycle:** 5–10M cycles (2.7–5.5 years at 5,000 clicks/day). Dome is
a $0.10 replaceable part.

**Who it's for:** Users with a desk surface. Budget-conscious buyers.
Accessibility users who need a basic pointing device. Developers and tinkerers
who want to hack on the firmware. Two-ring systems where total cost matters.

**Two-ring system BOM:** ~$18 + hub ($6) = ~$24.

---

### Pro — R30-OPTB-NONE-NONE + piezo sealed click

| Component | Spec | BOM |
|-----------|------|-----|
| MCU | ESP32-C3-MINI-1-N4 | $2.01 |
| Sensor | PMW3360-class optical reading captive ball | $6.00–9.00 |
| Click | Piezo film + haptic LRA (simulated click) | $1.00–2.00 |
| Battery | LiPo 80–100mAh | $0.80–1.50 |
| Charging | TP4054 + USB-C | $0.15–0.35 |
| LDO | RT9080-33GJ5 (0.5µA Iq) | $0.15 |
| Glide | Ball protrudes below shell, contacts surface directly | — |
| Shell | 3D-printed parametric ring, 30° angle, sealed | $0.60–1.20 |
| PCB | Flex or rigid-flex | $2.00–5.00 |
| **Total** | | **$13.00–21.00 (~$17 target)** |

**Surface compatibility:**

| Surface | Works? |
|---------|--------|
| Wood desk | Yes |
| Matte plastic | Yes |
| Paper / magazine | Yes |
| Mouse pad | Yes |
| Glass | **Yes** |
| Mirror | **Yes** |
| Fabric / clothing | **Yes** |
| Skin | **Yes** |

**Click lifecycle:** Piezo film effectively infinite. LRA haptic motor ~10M+
actuations. No mechanical wear path. Fully sealed — moisture-proof, debris-proof.

**Deep sleep wake:** Piezo click uses ADC, not GPIO — cannot wake from deep sleep
directly. Pro tier wakes on sensor motion detect pin (move finger to wake, then
click). See FIRMWARE-ROADMAP.md Phase 3 for details.

**Who it's for:** Users who need surface agnosticism — wheelchair tray surfaces,
bedside use on blankets, glass desks, variable environments. Users who want a
sealed device they don't maintain. Accessibility users for whom surface
limitation is a hard barrier, not a convenience issue.

**Two-ring system BOM:** ~$34 + hub ($6) = ~$40.

---

## Why These Two Knees

### Why not slot the tiers differently?

**"Why not Standard + OPTB without piezo (~$15–18)?"**
The piezo sealed click adds only ~$1.50 to a ring that already costs $15+. At
that price point, the dome click becomes the weakest component — the only part
with a finite lifecycle in a device whose sensor has none. Skipping the seal
saves $1.50 on a $17 ring and leaves a wear point. Poor tradeoff.

**"Why not make Standard the ball+Hall ring (~$11)?"**
The S-BALL sensor has ~15–60 DPI equivalent resolution. It requires heavy
firmware interpolation to produce usable cursor movement. The optical sensor
gives 800–2000 CPI natively. For the Standard tier, the user gets a device that
feels like a normal mouse on compatible surfaces. S-BALL is a surface-agnosticism
play at low cost — valid for prototyping, but the user experience gap versus
optical is too large for a consumer product.

**"Why not jump to tethered sensor (~$25–30) for Pro?"**
The tethered architecture is a different product category. It requires a flex
cable, a wrist hub enclosure, a different PCB design, different assembly
instructions, and different mechanical reliability analysis. It doesn't share the
ring form factor or the ring shell. Making it a "tier" of the ring product
creates more divergence than two products should have. It's a candidate for a
separate product line, not a tier.

**"Why not include nRF52840 MCU swap for Pro?"**
The nRF52840 would deliver 3–5x battery life improvement (see
[POWER-BUDGET.md](POWER-BUDGET.md)) and is worth the $3–5 MCU cost delta at Pro
pricing. However, it's a platform migration: different toolchain (Zephyr vs
ESP-IDF), different BLE stack (SoftDevice or Zephyr BLE vs NimBLE), different
build system, different OTA mechanism. Running two MCU platforms doubles firmware
validation effort. Splitting MCU platforms across tiers is the wrong kind of
complexity — but migrating both tiers to nRF52840 for the consumer product
generation is the recommended strategy (see MCU Strategy below).

---

## What the Tiers Share

Keeping this list long is the point. Divergence costs engineering effort on every
firmware release, every companion app update, every accessory design.

| Property | Standard | Pro | Shared? |
|----------|----------|-----|---------|
| MCU | ESP32-C3-MINI-1-N4 | ESP32-C3-MINI-1-N4 | **Yes** |
| BLE stack | NimBLE HID | NimBLE HID | **Yes** |
| Firmware image | Same base, sensor driver flag | Same base, sensor driver flag | **Yes** (build-time flag) |
| Companion app | Same | Same | **Yes** |
| USB hub dongle | Same | Same | **Yes** |
| Battery | 80–100mAh LiPo | 80–100mAh LiPo | **Yes** |
| Charging circuit | TP4054 + USB-C | TP4054 + USB-C | **Yes** |
| LDO | RT9080-33GJ5 | RT9080-33GJ5 | **Yes** |
| Form factor | R-30 ring | R-30 ring | **Yes** |
| Parametric sizing | Continuous finger circ. | Continuous finger circ. | **Yes** |
| Sensor angle | 30° | 30° | **Yes** |
| Glide system | Raised rim + UHMWPE | Ball protrudes below | **No** |
| Click mechanism | Snap dome | Piezo + LRA haptic | **No** |
| Sensor driver | PAW3204 SPI | PMW3360 SPI | **No** (same bus, different driver) |
| Shell sealing | Vented (dome needs air gap) | Sealed | **No** |

Four points of divergence: sensor, click, glide geometry, shell seal. Everything
else is shared. Both tiers use the same SPI bus for sensor communication — the
firmware difference is which driver is compiled in, selectable via a build-time
Kconfig flag.

---

## Firmware Implications

The two-tier model requires the firmware to support two sensor drivers and two
click input paths behind a build-time configuration flag.

```
Kconfig:
  POWERFINGER_SENSOR_DRIVER
    - SENSOR_PAW3204    (Standard: SPI optical, delta X/Y registers)
    - SENSOR_PMW3360    (Pro: SPI optical-on-ball, delta X/Y registers)

  POWERFINGER_CLICK_INPUT
    - CLICK_SNAP_DOME   (Standard: GPIO digital input, hardware debounce)
    - CLICK_PIEZO_LRA   (Pro: ADC threshold detection + LRA PWM haptic pulse)
```

The HID report structure is identical for both tiers. The companion app does not
need to know which tier it's connected to — the BLE characteristics are the same.

The piezo click path requires:
- ADC sampling of piezo film voltage (threshold-based press detection)
- Configurable press threshold (BLE characteristic, same as DPI sensitivity)
- LRA pulse timing profile (duration, amplitude) for convincing click feel
- Dead zone suppression during haptic pulse (prevent cursor jump from vibration)

These are firmware additions, not branches. The Standard firmware simply doesn't
compile the piezo/LRA code paths.

---

## Upgrade Path

A user who buys Standard and later wants Pro does not need to buy a new device.
The PCB should be designed so that:

1. The optical sensor footprint accommodates both PAW3204 and PMW3360 pinouts
   (or uses a common SPI header with a daughter board).
2. The click pad area has pads for both snap dome and piezo film.
3. The shell is the only part that must be replaced (sealed vs vented).

This makes the Standard-to-Pro upgrade a sensor swap + click swap + shell swap,
not a full device replacement. Consistent with the no-planned-obsolescence rule.

Whether this is economically rational for the user depends on pricing — but the
hardware design should not prevent it.

---

## Pricing Implications (Retail, Not BOM)

This RFC does not set retail prices. But for context, typical hardware markup for
open-source consumer electronics at low volume (100–1000 units):

| | BOM | Estimated Retail (3–4x markup) |
|--|-----|-------------------------------|
| Standard ring | ~$9 | ~$29–35 |
| Pro ring | ~$17 | ~$49–65 |
| USB hub dongle | ~$6 | ~$19–24 |
| Standard two-ring + hub | ~$24 | ~$79–95 |
| Pro two-ring + hub | ~$40 | ~$119–159 |

The Standard two-ring system competes with budget wireless mice ($15–30). The
Pro two-ring system competes with premium ergonomic mice ($80–150) and assistive
pointing devices ($100–300+). Both are dramatically cheaper than existing
assistive input devices, which often run $200–500+.

---

## Power Reality and MCU Strategy

Pre-hardware power estimates ([POWER-BUDGET.md](POWER-BUDGET.md)) reveal that
the ESP32-C3 cannot deliver competitive battery life for a consumer BLE HID
peripheral. The recommended strategy is: prototype on ESP32-C3, ship on nRF52840.

### Battery Life Estimates (ESP32-C3, 80 mAh, 15ms adaptive interval)

| Tier | Heavy (4h/day) | Moderate (2h/day) | Light (1h/day) |
|------|---------------|-------------------|----------------|
| Standard (PAW3204) | ~1.8 days | ~3.5 days | ~6 days |
| Pro (PMW3360) | ~0.8 days | ~1.5 days | ~3 days |

### Projected Battery Life (nRF52840, 80 mAh, 15ms adaptive interval)

| Tier | Heavy (4h/day) | Moderate (2h/day) | Light (1h/day) |
|------|---------------|-------------------|----------------|
| Standard (PAW3204) | ~5–9 days | ~10–18 days | ~18–30 days |
| Pro (PMW3360) | ~2–5 days | ~5–10 days | ~9–18 days |

### Recommended MCU Strategy

1. **Prototype phase: ESP32-C3.** Validates sensing, form factor, firmware
   architecture, and companion app. The 3–6 day battery life is adequate for
   user testing and design iteration. ESP-IDF ecosystem is familiar and
   well-documented. Cost is $2/module.

2. **Consumer product: nRF52840 (both tiers).** Migrate when firmware
   architecture is stable and proven on ESP32-C3. The nRF52840 is 5–10x more
   efficient for BLE HID idle current (50–200µA vs 350–2000µA). At Pro tier
   BOM ($17+), the +$3–5 MCU delta is noise. At Standard tier BOM (~$9), it
   pushes cost to ~$12–14 — still under the $25 ceiling, still cheaper than any
   competing assistive device.

3. **Migration scope:** Toolchain (ESP-IDF → Zephyr RTOS), BLE stack (NimBLE →
   Zephyr BLE or SoftDevice), build system (CMake/idf.py → CMake/west), OTA
   mechanism (esp_ota → MCUboot). The sensor drivers (SPI), click logic (GPIO/
   ADC), and HID report structure are portable. The hub (ESP32-S3) stays — it
   needs USB OTG which the nRF52840 supports via its built-in USB 2.0 controller,
   but migration can be deferred since the hub is USB-powered.

4. **Do not split MCU platforms across tiers.** Both Standard and Pro must use
   the same MCU at any given generation. The firmware validation cost of
   maintaining two platforms exceeds the BOM savings.

### Other Power Fixes (Applied to All BOMs)

These changes are already reflected in the hardware BOMs:

- **LDO: RT9080-33GJ5** replaces AP2112K-3.3. The AP2112K's 55µA quiescent
  current dominated deep sleep power 11:1 over the ESP32-C3's 5µA. The RT9080
  at 0.5µA Iq drops system deep sleep from ~75µA to ~16µA. Cost delta: +$0.07.

- **Charge resistor: 20kΩ** replaces 2kΩ. The 2kΩ set a 500mA charge rate
  (6.25C on 80mAh) — far too aggressive, degrading the cell in 100–200 cycles.
  20kΩ sets 50mA (0.625C on 80mAh). 10kΩ (100mA) was initially considered but
  [BATTERY-SAFETY.md](BATTERY-SAFETY.md) §5 thermal analysis found unsafe
  temperatures in the sealed ring. Charge takes ~1.5–2 hours.

- **Hall sensors: DRV5053** replaces SS49E as the recommended part. ~3mA per
  sensor vs 6mA (12mA vs 24mA for four sensors). Must be power-gated via MOSFET
  in sleep — the DRV5053 has no built-in sleep mode.

- **Connection interval: 15ms default** with adaptive switch to 7.5ms during
  active tracking. At 7.5ms the ESP32-C3 can't sleep between BLE events;
  at 15ms inter-event sleep engages and roughly doubles battery life.

---

## Open Questions

1. **Piezo click feel validation.** The Pro tier's sealed click is the highest
   UX risk. Simulated click feel is subjective. iPhone 7+ proved it works for
   buttons, but a cursor click has different expectations (sharper, less travel).
   Needs user testing before committing to Pro tier design.

2. **PMW3360 ball diameter.** The prototype S-BALL uses a 5mm steel ball. The
   PMW3360 reads surface texture on the ball — optimal ball diameter and surface
   finish for a ring form factor need validation. Ploopy uses 34mm+ balls. A
   ring might need 6–8mm with a textured ceramic or coated steel ball.

3. **Upgrade PCB feasibility.** Dual-footprint PCB for both sensor classes may
   require a larger board or a modular daughter board. The flex PCB cost is
   already the largest BOM uncertainty ($2–5). Adding dual footprints might push
   it higher.

4. **Pro tier battery life on ESP32-C3.** The PMW3360 draws 16.3mA active vs
   the PAW3204's 2.3mA. On the ESP32-C3 prototype, Pro tier battery life may be
   under 2 days at moderate use. This is tolerable for prototyping but strengthens
   the case for nRF52840 migration before consumer launch — the MCU power savings
   partially offset the sensor's higher draw.

5. **Haptic dead zone interaction with cursor tracking.** The LRA vibration
   during click feedback could introduce sensor noise on the PMW3360 (which is
   reading a ball that's physically coupled to the ring body). May need a
   brief sensor hold during haptic pulse, or mechanical isolation between
   sensor and LRA.

---

## Decision Requested

Adopt two-tier Standard/Pro model as the canonical consumer product structure,
with:
- **Standard** = R30-OLED (optical LED, snap dome, ~$9 BOM)
- **Pro** = R30-OPTB + piezo sealed click (optical-on-ball, piezo+LRA, ~$17 BOM)

Both tiers share MCU, firmware base, companion app, hub dongle, battery, charging,
form factor, and parametric sizing. Divergence is limited to sensor driver, click
input, glide geometry, and shell seal.
