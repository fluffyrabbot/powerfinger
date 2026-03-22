# Click Mechanisms — Analysis and Recommendations

Every PowerFinger configuration needs a click input. This document evaluates
every viable mechanism for compactness, durability, tactile feel, and
compatibility with each sensor type and form factor.

---

## The Constraint

A click mechanism for the ring must:
- Fit within the ~7–8mm height budget (finger pad to surface)
- Co-locate with any sensor type (optical, ball, laser, optical-on-ball)
- Survive years of daily use (target: 3+ years at 5,000 clicks/day = ~5M cycles)
- Provide unambiguous tactile feedback (the user must *feel* the click)
- Not interfere with cursor tracking during actuation

---

## Mechanisms Evaluated

### 1. Metal Snap Dome — Recommended Default

A thin stainless steel disc that buckles with a crisp snap when pressed.

| Property | Value |
|----------|-------|
| Size | 2.5–5mm diameter, **0.15–0.2mm tall** |
| Lifecycle | **5–10 million cycles** (Snaptron SQ-series) |
| Feel | Excellent — sharp, consistent snap point. Inherent tactile feedback. |
| Cost | $0.05–0.15 |
| Moving parts | Yes (dome flexes) |
| Sealable | No — needs air gap for dome travel |

**Co-location with sensors:** Fits alongside any sensor at any angle. At 0.2mm
tall, it adds essentially zero height. Can sit beside an optical sensor
aperture, under a ball, on the ring body adjacent to the sensor, or beneath
the sensor PCB (activated by pressing the whole ring down).

**Durability math:** 5M cycles ÷ 5,000 clicks/day = **2.7 years.** 10M cycles
= **5.5 years.** Acceptable, and the dome is a $0.10 replaceable part.

**Verdict:** Default click mechanism for both the click ring and the nav ring's
built-in click. Best ratio of feel, size, cost, and durability.

### 2. Piezoelectric Film + Haptic LRA — Premium Option

A thin polymer film (150 microns) that generates a signal when deformed by
pressure. Paired with the haptic LRA motor (already in the BOM) that creates
simulated "click feel."

| Property | Value |
|----------|-------|
| Size | Film: **<0.15mm thick**, any shape. LRA: already in BOM. |
| Lifecycle | **Effectively infinite** (no moving parts in the sensor). LRA: ~10M+ actuations. |
| Feel | Simulated — depends on haptic response quality. iPhone 7+ home button territory. Convincing for most users, uncanny valley for some. |
| Cost | ~$1–2 for piezo film. LRA shared with other haptic uses. |
| Moving parts | None in the sensor. LRA vibrates for feedback. |
| Sealable | **Yes — enables fully sealed ring** (moisture-proof, debris-proof) |

**Co-location with sensors:** Perfect. Film laminates onto the inner ring shell
above the finger pad. Zero additional height. Works alongside every sensor type.
Detects press force through the ring body itself.

**Why this matters for accessibility:** A fully sealed ring with no mechanical
openings is easier to clean, resistant to moisture (sweat, rain), and has no
debris ingress path. For users who wear the ring all day, this durability
advantage is significant.

**Tradeoff:** The "click feel" is simulated, not mechanical. Requires firmware
calibration of the press threshold (ADC reading from piezo → click event) and
tuning of the haptic response (LRA pulse duration, intensity). User testing
must determine whether the simulated feel is acceptable.

**Verdict:** Premium option. Prototype alongside the dome switch. If user testing
validates the feel, this becomes the preferred mechanism due to infinite sensor
life and full sealability.

### 3. Kailh/Omron Micro Switch — Wand Only

The actual switches inside mice. Metal leaf spring with snapping contact.

| Property | Value |
|----------|-------|
| Size | **12.8 × 5.8 × 6.5mm** (Kailh GM8.0) — too large for any ring |
| Lifecycle | **80 million cycles** — 44 years at heavy use |
| Feel | Best in class. This is what "mouse click" feels like. |
| Cost | $0.20–0.50 |
| Moving parts | Yes (leaf spring) |
| Sealable | No |

**Co-location with sensors:** Does not fit in the ring (~15mm wide cavity).
Fits in the wand body (8mm diameter tube has room for a 5.8mm-wide switch
along the barrel). Perfect for wand thumb/barrel buttons.

**Verdict:** Use for wand barrel buttons only. Overkill durability, best feel,
but physically cannot fit in a fingertip ring.

### 4. Force-Sensitive Resistor (FSR) — Avoid

Thin polymer film (<0.5mm) whose resistance changes under pressure.

| Property | Value |
|----------|-------|
| Size | <0.5mm, any shape |
| Lifecycle | **Degrades over time.** Sensitivity drift under repeated loading. ~1–5M useful cycles before calibration drift causes phantom or missed clicks. |
| Feel | None. Pure pressure sensing with zero tactile feedback. |
| Cost | $0.50–1.50 |

**Verdict:** Violates the "no planned obsolescence" rule. A device that gets
worse over time is not acceptable for an assistive device that someone depends
on. Potentially useful as a *secondary* pressure sensor (grip force for scroll
speed modulation), never as the primary click trigger.

### 5. Conductive Rubber / Carbon Pill — Prototype Only

Silicone dome with carbon-coated contact underneath. The technology in TV
remotes and game controllers.

| Property | Value |
|----------|-------|
| Size | Dome ~1–2mm tall, 4–8mm diameter |
| Lifecycle | **1–5 million cycles.** Carbon contact wears, silicone hardens and deforms. |
| Feel | Soft, mushy, less crisp than metal dome. "Cheap remote button" feel. |
| Cost | $0.02–0.10 |

**Verdict:** Acceptable for breadboard prototyping (cheap, widely available).
Not for production — inferior feel and shorter life than the metal dome at
nearly the same size, for a negligible cost saving.

### 6. Capacitive Touch — Modifier Only

Detects finger proximity via capacitance change on a PCB trace.

| Property | Value |
|----------|-------|
| Size | Zero additional height. A PCB trace is the sensor. |
| Lifecycle | Infinite. No moving parts, no wear. |
| Feel | **None.** No physical threshold between "hovering" and "touching." |
| Cost | ~$0 (PCB trace) |

**Verdict:** Terrible for primary click — users will constantly accidentally
click or miss clicks without a physical threshold. Useful as a *secondary*
input: a capacitive zone on the ring body for "touch to enter scroll mode" or
similar context switches. Keep out of the primary click path.

### 7. Hall Effect + Spring — Over-Engineered for Ring

Magnet approaches a Hall sensor. Contactless, analog, no wear on the
electrical side. Needs a spring or flexure for return.

| Property | Value |
|----------|-------|
| Size | Sensor: tiny (SOT-23). Mechanism with spring: ~3–5mm tall. |
| Lifecycle | Electrically infinite. Spring fatigue: 1M–100M depending on design. |
| Feel | Depends on spring design. Can range from mushy to snappy. |
| Cost | $1–3 |

**Verdict:** The spring design adds mechanical complexity without clear benefit
over a metal dome. Interesting for the wand (room for the mechanism), but the
Kailh micro switch is simpler and better-feeling in that form factor. Not
recommended for rings.

---

## Compatibility Matrix

| Mechanism | Nav ring (built-in click) | Click ring (dedicated) | Wand barrel button |
|-----------|:---:|:---:|:---:|
| **Metal snap dome** | **Recommended** | **Recommended** | Good |
| **Piezo film + haptic** | **Premium option** | **Premium option** | Good |
| Kailh micro switch | Too tall | Too tall | **Best** |
| FSR | Avoid (degrades) | Avoid (degrades) | Avoid (degrades) |
| Conductive rubber | Prototype only | Prototype only | Avoid |
| Capacitive touch | Modifier only | Avoid | Modifier only |
| Hall effect + spring | Marginal | Marginal | Acceptable |

---

## Integration Patterns

### Pattern A: Dome Under Sensor PCB (Nav Ring)

The dome sits directly beneath the sensor PCB. Pressing the ring down toward
the surface compresses the dome. The same downward motion that brings the
finger to the surface also clicks.

```
  finger pad
  ──────────────
  │  LiPo      │
  │  flex PCB   │
  │  sensor     │
  │  [dome]     │  ← dome between sensor and standoff base
  ──────────────
  ○foot    ○foot   ← standoff feet, with dome travel gap
  ═══════════════  surface
```

**Works with:** S-OLED, S-VCSL, S-OPTB (any sensor with a rigid PCB mount).
**Does not work with:** S-BALL (ball protrusion conflicts with dome travel).
**Feel:** Whole-finger press. Less precise than a dedicated click ring.

### Pattern B: Dome on Ring Side (Nav Ring or Click Ring)

The dome sits on the lateral surface of the ring body. Click by pressing thumb
against the ring, or pressing the ring against an adjacent finger.

```
  finger pad
  ──────────────
  │  sensor     │
  │         [D] │  ← dome on outer wall
  ──────────────
```

**Works with:** All sensor types.
**Feel:** Thumb pinch motion. More deliberate than Pattern A, less accidental.

### Pattern C: Dome as Entire Contact Surface (Click Ring)

The dedicated click ring is essentially a dome switch in a ring shell. The
entire bottom of the ring is the dome. Press fingertip down = click.

```
  finger pad
  ──────────────
  │  ESP32-C3  │
  │  LiPo      │
  │  [DOME]    │  ← dome is the bottom of the ring
  ──────────────
  ═══════════════  surface
```

**No sensor in this ring.** It's a click-only device. Simplest possible build.
The dome is large enough (8–10mm) to be easy to actuate and very tactile.

### Pattern D: Piezo Film on Inner Shell (Any Ring)

Piezo film laminated to the inside of the ring shell, between the finger pad
and the electronics. Detects finger pressure through shell deformation.

```
  finger pad
  ─[piezo film]─
  │  LiPo      │
  │  flex PCB   │
  │  sensor     │
  ──────────────
  ○foot    ○foot
  ═══════════════  surface
```

**Works with:** All sensor types. Adds zero height. Ring can be fully sealed.
**Requires:** ADC threshold calibration, haptic LRA driver for feedback.

### Pattern E: Ball Press-Down (Ball Variants Only)

The ball sits on a dome switch. Pressing the ball down (increasing finger
pressure against the surface) actuates the dome beneath it. This is the
BlackBerry trackball click mechanism.

```
  finger pad
  ──────────────
  │  Hall ICs  │
  │  rollers   │
  │    ○ ball  │
  │  [dome]    │  ← dome under ball socket
  ──────────────
  ═══════════════  surface
```

**Works with:** S-BALL only.
**Feel:** Pressing the ball down gives a distinct click while the ball is still
free to roll. Proven mechanism (hundreds of millions of BlackBerry devices).
**Risk:** The same finger pressure used for clicking could cause unintended
cursor movement during the press. Firmware must apply a dead zone during click
detection.

---

## Recommended Pairings

### Canonical Two-Ring Setup

| Device | Mechanism | Pattern | Why |
|--------|-----------|---------|-----|
| Nav ring (R30-OLED) | Metal dome | A (under sensor) or D (piezo) | Built-in fallback click for single-ring use |
| Click ring | Metal dome | C (dome is the floor) | Dedicated, maximally tactile, simplest build |

### Wand

| Device | Mechanism | Location | Why |
|--------|-----------|----------|-----|
| Wand barrel | Kailh GM8.0 micro switch | Side of tube, thumb-operated | 80M cycles, best feel, fits in 8mm tube |
| Wand tip (ball variants) | Metal dome | E (under ball) | Secondary click via press-down |

### Premium (Fully Sealed) Ring Pair

| Device | Mechanism | Pattern | Why |
|--------|-----------|---------|-----|
| Nav ring | Piezo film + haptic | D (inner shell) | Sealed, infinite sensor life |
| Click ring | Piezo film + haptic | D (inner shell) | Sealed, infinite sensor life |

This configuration has zero mechanical openings, zero debris ingress, and zero
wear surfaces. The tradeoff is simulated click feel and firmware complexity.

---

## Prototyping Order

1. **Metal dome in click ring (Pattern C)** — simplest possible build, validates
   the two-ring interaction model
2. **Metal dome in nav ring (Pattern A)** — validates built-in click as fallback
3. **Piezo film in click ring (Pattern D)** — tests whether simulated feedback
   is acceptable to users
4. **Kailh switch in wand barrel** — validates wand click ergonomics
5. **Ball press-down dome (Pattern E)** — only if ball variants proceed past
   Phase 1

User testing should directly compare dome vs. piezo with the same participants
to determine whether the simulated click feel is a dealbreaker.
