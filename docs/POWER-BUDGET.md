# Power Budget — Component-Level Estimates

Pre-hardware estimates based on datasheet values, community measurements, and
commercial BLE mouse benchmarks. No numbers in this document are measured on
PowerFinger hardware. Confidence ratings: **HIGH** = datasheet spec under our
conditions, **MEDIUM** = datasheet spec under different conditions or community
measurement on similar hardware, **LOW** = extrapolated from incomplete data.

References:
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)

---

## Component Power Draw

### ESP32-C3-MINI-1-N4 (MCU + BLE Radio)

Source: Espressif datasheet v2.3, Rob Dobson bare-module measurements, Hubble
community guide for BLE connection interval scaling.

| State | Current | Confidence | Source |
|-------|---------|------------|--------|
| BLE connected, 7.5ms interval, light sleep | 15–30 mA | MEDIUM | Community measurements; chip can't sleep between events this fast |
| BLE connected, 15ms interval, light sleep | 8–15 mA | MEDIUM | Threshold where inter-event sleep begins working |
| BLE connected, 30ms interval, light sleep | 3–6 mA | MEDIUM | Community guide extrapolation |
| Modem sleep, CPU 80 MHz idle | 13 mA | HIGH | Datasheet Table 5-8 |
| Light sleep, no BLE | 130 µA (datasheet) / 350 µA (measured) | HIGH / HIGH | Datasheet vs Rob Dobson module-level |
| Deep sleep + GPIO wake | 5 µA (datasheet) / 8 µA (measured) | HIGH / HIGH | Datasheet vs ESP-IDF measurement guide |
| BLE TX/RX peak (per radio event) | 100–130 mA | MEDIUM | Multiple community sources |

**Critical finding:** At 7.5ms connection intervals the BLE stack effectively
stays awake — the next event comes too soon for the chip to enter light sleep.
15ms is the practical floor for duty-cycled power savings. This means our 7.5ms
latency target and our power target are in direct tension.

### PAW3204DB (Standard Tier — Optical LED Sensor)

Source: PixArt datasheet.

| State | Current | Confidence |
|-------|---------|------------|
| Active tracking (Normal1, full frame rate) | 2.3 mA | HIGH |
| Active tracking (Normal2, reduced frame rate) | 1.4 mA | HIGH |
| Active tracking (Normal3, lowest frame rate) | 1.2 mA | HIGH |
| Sleep1 (no motion, entered after ~256ms) | 75 µA | HIGH |
| Sleep2 (no motion, entered after ~61s in Sleep1) | 12 µA | HIGH |
| Power down | 10 µA | HIGH |

The PAW3204 has built-in automatic sleep state transitions. When the user stops
moving, current drops from 2.3 mA to 75 µA within 256ms, then to 12 µA after
~61 seconds. No firmware intervention needed — the sensor manages this itself.

### PMW3360DM (Pro Tier — Optical-on-Ball Sensor)

Source: PixArt datasheet PMS0058 Rev 1.50.

| State | Current | Confidence |
|-------|---------|------------|
| Run mode 1 (standard tracking) | 16.3 mA | HIGH |
| Run mode 2 | 18.6 mA | HIGH |
| Run mode 3 | 21.6 mA | HIGH |
| Run mode 4 (max performance) | 37.0 mA | HIGH |
| Rest1 (first idle tier) | 2.8 mA | HIGH |
| Rest2 (deeper idle) | 61 µA | HIGH |
| Rest3 (deepest rest) | 32 µA | HIGH |
| Power down | 10 µA | HIGH |

**The PMW3360 draws 7–16x more active current than the PAW3204.** This is the
single largest power impact of the Standard→Pro upgrade. Rest modes bring it
back into line, so the impact scales with active tracking duty cycle.

### Hall Effect Sensors (Standard P1 — Ball Tracking, 4x DRV5053)

Source: TI datasheet SLIS153, TI E2E forum.

| State | Current (per sensor) | Current (4 sensors) | Confidence |
|-------|---------------------|---------------------|------------|
| Active (continuous) | ~3 mA typ / 3.6 mA max | ~12 mA typ / 14.4 mA max | MEDIUM |
| Power-gated off | 0 | 0 | HIGH |

The DRV5053 has no built-in sleep mode. It draws ~3 mA continuously when
powered. The four sensors must be power-gated via a MOSFET or load switch when
the device enters sleep. Without power gating, four Hall sensors alone would
drain 80 mAh in ~6.5 hours.

**Alternative:** SS49E draws 6 mA typ per sensor (24 mA for four) — avoid.

### IMU (Optional — Air Tracking)

| Sensor | Full Mode | Low-Power Accel | Sleep | Confidence |
|--------|-----------|-----------------|-------|------------|
| MPU-6050 | 3.6 mA | 10–110 µA (1.25–40 Hz) | 5 µA | HIGH |
| BMI270 | 685 µA | 14 µA | <0.5 µA | HIGH |

The BMI270 is 5x more efficient in full mode and 7x more efficient in low-power
mode than the MPU-6050. For any variant that includes an IMU, the BMI270 is the
correct choice despite higher unit cost (~$3 vs ~$2 at prototype quantity).

### LRA Haptic Motor (Pro Tier — Click Feedback)

Source: Vybronics datasheets, TI DRV2605 datasheet.

| State | Current | Confidence |
|-------|---------|------------|
| Active pulse (click feedback) | 70–90 mA for 10–65ms | HIGH |
| Idle | 0 mA (motor) / 1 µA (DRV2605 driver standby) | HIGH |
| Average at 100 clicks/hour, 20ms pulses | ~0.04 mA | HIGH |

Haptic power is negligible. Even at 300 clicks/hour the average contribution is
~0.12 mA.

### LDO (AP2112K-3.3)

Source: Diodes Inc datasheet.

| State | Current | Confidence |
|-------|---------|------------|
| Quiescent (output enabled) | 55 µA | HIGH |
| Shutdown (EN low) | 0.01 µA | HIGH |

**Problem:** During deep sleep the ESP32-C3 draws ~5 µA but the LDO adds 55 µA —
the LDO dominates system deep sleep current by 11:1. Options:
- Replace with ultra-low-Iq LDO (RT9080: ~0.5 µA, XC6220: ~8 µA)
- Use the AP2112K EN pin to fully shut down in deep sleep (requires external
  wake circuit to re-enable)

### TP4054 Charge Controller

Source: TP4054 datasheet.

| State | Current from battery | Confidence |
|-------|---------------------|------------|
| USB disconnected (reverse-blocked) | <2 µA | HIGH |

Negligible during battery operation.

---

## System-Level Power Budgets

### Standard Tier (R30-OLED) — Optical Ring

#### Active Tracking

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C3 (BLE connected) | 8–30 mA | Range: 15ms interval (8–15) to 7.5ms interval (15–30) |
| PAW3204 (tracking) | 1.2–2.3 mA | Normal3 to Normal1 frame rate |
| AP2112K quiescent | 0.055 mA | |
| TP4054 standby | 0.002 mA | |
| **Total active** | **~9–33 mA** | |

#### Idle (BLE Connected, No Motion)

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C3 (light sleep, maintaining BLE) | 0.35–2 mA | Depends on connection interval |
| PAW3204 (Sleep1, then Sleep2) | 0.012–0.075 mA | Auto-transitions |
| AP2112K quiescent | 0.055 mA | |
| **Total idle** | **~0.4–2.1 mA** | |

#### Deep Sleep (Disconnected After Timeout)

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C3 (deep sleep) | 0.005–0.008 mA | GPIO wake enabled |
| PAW3204 (power down) | 0.010 mA | |
| AP2112K quiescent | 0.055 mA | **Dominant** |
| TP4054 standby | 0.002 mA | |
| **Total deep sleep** | **~0.07–0.08 mA (70–80 µA)** | |

With an ultra-low-Iq LDO (e.g., RT9080 at 0.5 µA), deep sleep drops to
~17–21 µA. This is the difference between 47 days and 6+ months shelf life on
80 mAh.

### Pro Tier (R30-OPTB + Piezo) — Optical-on-Ball Ring

#### Active Tracking

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C3 (BLE connected) | 8–30 mA | Same as Standard |
| PMW3360 (Run mode 1) | 16.3 mA | 7–16x more than PAW3204 |
| LRA haptic (amortized) | 0.04 mA | Negligible |
| DRV2605 driver standby | 0.001 mA | |
| AP2112K quiescent | 0.055 mA | |
| **Total active** | **~24–46 mA** | |

#### Idle (BLE Connected, No Motion)

| Component | Current | Notes |
|-----------|---------|-------|
| ESP32-C3 (light sleep) | 0.35–2 mA | |
| PMW3360 (Rest2/Rest3) | 0.032–0.061 mA | Auto-transitions, comparable to PAW3204 |
| AP2112K quiescent | 0.055 mA | |
| **Total idle** | **~0.4–2.1 mA** | Same as Standard when idle |

#### Deep Sleep

Same as Standard — sensor powered down, MCU in deep sleep. ~70–80 µA.

---

## Battery Life Estimates

Usage model assumptions:
- "Active hours" = finger is moving and the sensor is tracking
- "Connected idle" = BLE connected, no finger movement, sensor in auto-sleep
- "Deep sleep" = device timed out, full power-down, wake on button press
- A "day" = some mix of active + idle + sleep

### Standard Tier (R30-OLED, 80 mAh battery)

**15ms connection interval** (recommended — balances latency and power):

| Usage Pattern | Active | Idle | Deep Sleep | Avg mA | Battery Life | Confidence |
|---------------|--------|------|------------|--------|--------------|------------|
| Heavy (4h active, 4h idle, 16h sleep) | 4h @ 10 mA | 4h @ 0.7 mA | 16h @ 0.075 mA | 1.85 mA | **~43 hours / ~1.8 days** | MEDIUM |
| Moderate (2h active, 4h idle, 18h sleep) | 2h @ 10 mA | 4h @ 0.7 mA | 18h @ 0.075 mA | 0.96 mA | **~83 hours / ~3.5 days** | MEDIUM |
| Light (1h active, 2h idle, 21h sleep) | 1h @ 10 mA | 2h @ 0.7 mA | 21h @ 0.075 mA | 0.54 mA | **~150 hours / ~6 days** | LOW |

**7.5ms connection interval** (original target — gaming-grade latency):

| Usage Pattern | Active | Idle | Deep Sleep | Avg mA | Battery Life | Confidence |
|---------------|--------|------|------------|--------|--------------|------------|
| Heavy | 4h @ 20 mA | 4h @ 1.5 mA | 16h @ 0.075 mA | 3.64 mA | **~22 hours / ~0.9 days** | MEDIUM |
| Moderate | 2h @ 20 mA | 4h @ 1.5 mA | 18h @ 0.075 mA | 1.98 mA | **~40 hours / ~1.7 days** | MEDIUM |
| Light | 1h @ 20 mA | 2h @ 1.5 mA | 21h @ 0.075 mA | 1.03 mA | **~78 hours / ~3.2 days** | LOW |

### Pro Tier (R30-OPTB, 80 mAh battery)

**15ms connection interval:**

| Usage Pattern | Active | Idle | Deep Sleep | Avg mA | Battery Life | Confidence |
|---------------|--------|------|------------|--------|--------------|------------|
| Heavy | 4h @ 25 mA | 4h @ 0.7 mA | 16h @ 0.075 mA | 4.34 mA | **~18 hours / ~0.8 days** | MEDIUM |
| Moderate | 2h @ 25 mA | 4h @ 0.7 mA | 18h @ 0.075 mA | 2.21 mA | **~36 hours / ~1.5 days** | MEDIUM |
| Light | 1h @ 25 mA | 2h @ 0.7 mA | 21h @ 0.075 mA | 1.13 mA | **~71 hours / ~3 days** | LOW |

**7.5ms connection interval:**

| Usage Pattern | Active | Idle | Deep Sleep | Avg mA | Battery Life | Confidence |
|---------------|--------|------|------------|--------|--------------|------------|
| Heavy | 4h @ 35 mA | 4h @ 1.5 mA | 16h @ 0.075 mA | 6.14 mA | **~13 hours / ~0.5 days** | MEDIUM |
| Moderate | 2h @ 35 mA | 4h @ 1.5 mA | 18h @ 0.075 mA | 3.23 mA | **~25 hours / ~1 day** | MEDIUM |

---

## The 1–2 Week Target Is Not Achievable on ESP32-C3

The CLAUDE.md target of 1–2 weeks on 80–100 mAh requires an average system
current of ~0.24–0.60 mA (100 mAh ÷ 168–336 hours). Even the most optimistic
Standard tier scenario (light use, 15ms interval) averages ~0.54 mA — barely
scraping one week, and only with 1 hour of active use per day.

**For context — what commercial BLE mice achieve:**

| Mouse | Battery | Rated Life | Back-calculated Avg | MCU |
|-------|---------|-----------|---------------------|-----|
| Logitech Pebble M350 | 1x AA (~2500 mAh) | 18 months | ~0.12 mA | Nordic nRF52 |
| Logitech MX Anywhere 3S | 500 mAh Li-Po | 70 days | ~0.30 mA | Nordic nRF52 |

These mice achieve 0.1–0.3 mA average because Nordic's nRF52 draws **50–200 µA
average** in BLE HID connected mode, vs the ESP32-C3's **350–2000 µA**. The
ESP32-C3 is 5–10x worse for this specific use case.

### Options to Close the Gap

| Option | Impact | Tradeoff |
|--------|--------|----------|
| **Accept 3–6 day battery life** | None — adjust expectation | Honest but disappointing |
| **Increase battery to 150–200 mAh** | 1.5–2.5x life (5–12 days) | Ring gets bigger/heavier; may not fit 10mm height budget |
| **Relax connection interval to 15ms** | ~2x life vs 7.5ms | Latency goes from ~11ms to ~19ms; acceptable for accessibility, not for gaming |
| **Adaptive connection interval** | Best of both worlds | 15ms idle → 7.5ms on motion detection; firmware complexity |
| **Migrate to nRF52840** | 3–5x life (2–4 weeks realistic) | Platform migration; different toolchain, BLE stack, OTA; +$3–5 MCU cost |
| **Ultra-low-Iq LDO** | Adds ~1 week shelf life; minor active impact | Drop-in replacement, ~$0.10 cost delta |
| **Aggressive sensor duty cycling** | 10–30% active current reduction | Reduces effective CPI during transition; firmware complexity |

### Recommended Approach

1. **Prototype on ESP32-C3 as planned** — validate sensing, form factor,
   firmware architecture. The 3–6 day battery life is adequate for prototyping.
2. **Use 15ms connection interval as default**, with adaptive switch to 7.5ms
   during active tracking.
3. **Swap AP2112K for ultra-low-Iq LDO** (RT9080 or similar, ~$0.10 BOM delta).
4. **Evaluate nRF52840 migration for consumer product.** At Pro tier pricing
   (~$17 BOM), the +$3–5 MCU cost increase is absorbed. The power improvement
   is transformative — it's the difference between "charge twice a week" and
   "charge every two weeks."

---

## LDO Recommendation

The AP2112K is a poor fit. Its 55 µA quiescent current dominates deep sleep.

| LDO | Iq | Cost | Notes |
|-----|-----|------|-------|
| AP2112K-3.3 (current) | 55 µA | $0.08 | Good availability, poor sleep performance |
| RT9080-33GJ5 | 0.5 µA | ~$0.15 | 600 mA, SOT-23-5, best-in-class Iq |
| XC6220B331MR | 8 µA | ~$0.12 | 700 mA, SOT-25, widely available |
| ME6211C33M5G | 40 µA | ~$0.05 | 500 mA, cheaper but marginal improvement |

**Recommendation:** RT9080-33GJ5. Drops deep sleep from ~75 µA to ~16 µA. The
$0.07 cost delta pays for itself in user experience.

---

## nRF52840 vs ESP32-C3 — Power Comparison

For completeness, since the consumer tier RFC deferred this question.

| Parameter | ESP32-C3 | nRF52840 | Ratio |
|-----------|----------|----------|-------|
| Deep sleep + RTC | 5–8 µA | 1.5–2 µA | 3–4x |
| Light sleep (no radio) | 130–350 µA | 1.5–4.5 µA | 30–90x |
| BLE connected idle (15ms) | 350–2000 µA | 50–200 µA | 5–10x |
| BLE TX peak | 100–130 mA | 5–8 mA (@0 dBm) | 15–20x |
| Active CPU (80 MHz equiv) | 13–17 mA | 3–5 mA | 3–4x |
| BLE HID average (mixed use) | ~1–4 mA | ~0.1–0.5 mA | 5–10x |

The nRF52840's advantage compounds: lower peak current means less time at high
draw, faster return to sleep, and lower sleep floor. For a BLE HID peripheral on
an 80 mAh battery, the platform choice is the single largest determinant of
battery life — larger than sensor choice, connection interval, or firmware
optimization combined.

**The ESP32-C3 was chosen for cost ($2 vs $5–7) and ecosystem familiarity
(ESP-IDF + NimBLE).** These are valid prototyping reasons. But for a consumer
product where the Pro tier already costs $17 in components, the MCU cost delta
is noise and the power delta is transformative.

---

## Battery Degradation and Charge Cycles

Source: General LiPo industry data (not component-specific, since the ring uses
generic pouch cells).

| Parameter | Value | Confidence |
|-----------|-------|------------|
| Rated cycle life (to 80% capacity) | 300–500 cycles | HIGH |
| Calendar aging | ~2–5% capacity loss/year at room temp | MEDIUM |
| Charge rate impact | >1C shortens life; 0.5C or lower is ideal | HIGH |
| Storage voltage | 3.7–3.8V optimal for longevity; 4.2V accelerates aging | HIGH |

At the Standard tier's estimated 3–6 day battery life, charging frequency is
~1–2x per week = 52–104 cycles/year. Time to 80% capacity: **3–10 years**.

The TP4054 charges at a rate set by RPROG. The BOM specifies a 2kΩ resistor,
setting charge current to ~500 mA. On an 80 mAh cell, that's a **6.25C rate** —
this is far too aggressive and will significantly shorten battery life. At 6C,
many LiPo cells lose 20% capacity within 100–200 cycles.

**Recommendation:** Change RPROG to 20kΩ for ~50 mA charge current (0.625C on
80 mAh). Charge takes ~1.5–2 hours instead of ~12 minutes. The earlier
recommendation of 10kΩ (100mA) was superseded by
[BATTERY-SAFETY.md](BATTERY-SAFETY.md) §5 thermal analysis, which found 100mA
produces unsafe temperatures (62–87°C internal) in the sealed ring enclosure.

---

## Summary of Confidence Levels

| Claim | Confidence | Why |
|-------|------------|-----|
| ESP32-C3 deep sleep: 5–8 µA | HIGH | Datasheet + independent measurement |
| PAW3204 active: 1.2–2.3 mA | HIGH | Datasheet under our conditions |
| PMW3360 active: 16.3 mA | HIGH | Datasheet under our conditions |
| ESP32-C3 BLE at 7.5ms: 15–30 mA | MEDIUM | Community measurements, not on our hardware |
| ESP32-C3 BLE at 15ms: 8–15 mA | MEDIUM | Extrapolated from community data at other intervals |
| Standard tier battery life: 3–6 days | MEDIUM | Extrapolated from component data + usage model |
| Pro tier battery life: 1–3 days | MEDIUM | Extrapolated, dominated by PMW3360 uncertainty |
| nRF52 would give 3–5x improvement | MEDIUM | Based on Nordic profiler data, not measured in ring form factor |
| 1–2 week target not achievable on ESP32-C3 | HIGH | Math doesn't work at any realistic usage level |
| Charge rate too high (6.25C) | HIGH | Simple calculation from RPROG value and cell capacity |
| AP2112K LDO dominates deep sleep | HIGH | Datasheet Iq vs ESP32-C3 deep sleep current |
