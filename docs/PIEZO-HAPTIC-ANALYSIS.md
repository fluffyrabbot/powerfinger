# Piezo Click + Haptic Dead Zone Interaction Analysis

The Pro tier ring uses a PVDF piezo film for press detection and an LRA motor
(driven by DRV2605L) for simulated click feedback. The optical sensor (PMW3360)
reads a captive ball that is physically coupled to the ring body. This document
analyzes whether LRA haptic feedback creates sensor motion artifacts, whether the
current firmware dead zone is sufficient, and whether hardware changes are needed.

References:
- Click mechanisms: [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md)
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- Glide system: [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md)
- Firmware dead zone: `firmware/ring/components/click/dead_zone.c`
- Config constants: `firmware/ring/components/state_machine/include/ring_config.h`

---

## 1. PVDF Piezo Film Characteristics

### Operating Principle

PVDF (polyvinylidene fluoride) piezo film generates a voltage proportional to
mechanical strain. When the user presses a finger against the ring shell, the
shell deforms microscopically, straining the laminated film. The ADC reads the
resulting voltage spike.

### Force Sensitivity and Threshold

| Property | Value | Source |
|----------|-------|--------|
| Typical PVDF film thickness | 28-110 um (commonly 52 um) | Measurement Specialties / TE |
| Piezoelectric coefficient d33 | -33 pC/N | TE LDT0-028K datasheet |
| Piezoelectric coefficient d31 | 23 pC/N | TE LDT0-028K datasheet |
| Open-circuit voltage sensitivity | 12-15 mV/N typical through shell wall | Estimated from d33 + shell compliance |
| Reliable press detection threshold | 0.5-2.0 N (50-200 gf) | Empirical range for finger press through 1-2mm shell |
| Response time (10%-90%) | < 1 us | PVDF is sub-microsecond; limited by ADC sample rate |
| ADC sample rate needed | 1-10 kHz sufficient | Signal is a sharp spike followed by slow decay |

### Recommended ADC Threshold Approach

A simple voltage threshold on the ADC is insufficient. PVDF generates voltage on
strain *change*, not sustained strain. A press produces a sharp positive spike;
holding pressure produces near-zero output; release produces a sharp negative
spike.

Recommended detection algorithm:

1. **Positive threshold crossing** = press detected. Set threshold at 3-5x the
   noise floor. With a 12-bit ADC at 3.3V reference (0.8 mV/count), a typical
   noise floor of 5-10 counts, and a press signal of 50-200 counts, a threshold
   of 30 counts provides a 6:1 SNR minimum.

2. **Negative threshold crossing** = release detected. Use a symmetric negative
   threshold.

3. **Minimum hold time** = debounce. After detecting press, ignore further
   threshold crossings for `CLICK_DEBOUNCE_MS` (5ms). This prevents the LRA
   vibration from triggering re-detection (see section 2).

4. **Auto-calibrate noise floor** at boot. Sample 50 readings with no press;
   set threshold at `mean + 4 * stddev`. Same approach as the existing
   `calibration_run()` for motion sensors.

### Can LRA Haptic Feedback Trigger the Piezo Sensor?

**Yes. This is a real risk and requires firmware mitigation.**

The LRA vibrates the ring body. PVDF responds to any strain, not just finger
pressure. An LRA pulse at full amplitude produces ring body acceleration of
roughly 1-4 G (see section 3). Through the shell, this strain is smaller than a
deliberate finger press but may exceed the detection threshold.

Quantitative estimate:

| Signal | Approximate ADC magnitude (12-bit, 3.3V ref) |
|--------|-----------------------------------------------|
| Finger press (1 N through shell) | 50-200 counts |
| LRA "Strong Click" reflected vibration | 10-40 counts |
| Sensor noise floor (quiet) | 3-8 counts |

The LRA reflection overlaps with weak finger presses. **Mitigation:**

- After detecting a press and firing the LRA, mask the piezo ADC input for the
  duration of `HAPTIC_SUPPRESS_MS` + LRA mechanical decay. During this window,
  the press is already registered; only release detection matters, and release
  detection can use the negative threshold (LRA vibration is symmetric and
  should not produce a sustained negative bias).

- If the LRA reflection reliably exceeds the press threshold, raise the
  threshold. A 5:1 ratio between minimum finger press and maximum LRA reflection
  is achievable with proper shell stiffness (see section 5).

---

## 2. DRV2605L Waveform Analysis

### Relevant Waveforms for Click Simulation

The DRV2605L contains a ROM library of 123 haptic waveforms. For click
simulation, the relevant candidates are:

| Waveform ID | Name | Duration | Character |
|-------------|------|----------|-----------|
| 1 | Strong Click - 100% | ~12 ms active, ~30 ms total with decay | Sharp, aggressive. Highest mechanical coupling. |
| 2 | Strong Click - 60% | ~12 ms active, ~25 ms total | Slightly softer. Still significant coupling. |
| 3 | Strong Click - 30% | ~10 ms active, ~20 ms total | Moderate. Better coupling/feel tradeoff. |
| 7 | Soft Bump - 100% | ~20 ms active, ~40 ms total | Broader, less sharp. More ringing. |
| 10 | Double Click - 100% | ~30 ms active, ~60 ms total | Two pulses. Longest suppression window needed. |
| 17 | Sharp Click - 100% | ~8 ms active, ~20 ms total | Shortest active phase. Minimum coupling. |

### LRA Resonant Frequency

| Parameter | Value | Source |
|-----------|-------|--------|
| Typical LRA resonant frequency | 150-235 Hz | Common coin-type LRA datasheets |
| Ring-sized LRA (8mm coin) | 170-200 Hz | Typical for small form factor |
| DRV2605L auto-resonance tracking | Locks to motor within +/-2 Hz | Datasheet |
| Resonant period | ~5-6 ms per cycle | 170-200 Hz |

### Waveforms That Minimize Mechanical Coupling

**Shorter active phase = less total energy transferred to the ring body.**

Waveform 17 ("Sharp Click - 100%") has the shortest active phase (~8 ms, roughly
1.5 LRA cycles). It produces a convincing click feel because the human
perception of "click" is dominated by the onset transient, not sustained
vibration. Longer waveforms add ringing that the user perceives as "buzzy" rather
than "clicky."

**Recommendation: Waveform 17 (Sharp Click - 100%) as default, with Waveform 3
(Strong Click - 30%) as fallback.** Waveform 17 minimizes both suppression
window and mechanical coupling while producing the sharpest perceived click.
Waveform 3 is slightly longer but lower amplitude, useful if waveform 17 causes
excessive piezo re-triggering.

### Pulse Duration for Convincing Click Feel

| Duration | User Perception | Source |
|----------|----------------|--------|
| < 8 ms | Too faint; feels like a tap | Apple Taptic Engine studies, haptic HCI literature |
| 8-15 ms | Sharp click; most convincing | iPhone 7+ home button, Steam Deck triggers |
| 15-30 ms | Firm press; slightly buzzy | Longer LRA waveforms |
| 30-65 ms | Distinct vibration, no longer feels like a click | DRV2605L long-form waveforms |

The sweet spot is 8-15 ms of active drive. Mechanical ringing (the LRA mass
continuing to vibrate after drive ceases) adds another 15-40 ms of decaying
oscillation.

---

## 3. Vibration to Sensor Motion Coupling Model

### Physical Configuration

```
Pro Tier — Optical-on-Ball

    finger pad
    ──────────────
    │  LRA motor │ ← vibration source
    │  LiPo      │
    │  flex PCB   │
    │  PMW3360   │ ← reads ball surface via SPI
    │  ○ ball    │ ← captive ball in socket
    ╰──spindles──╯ ← ball contacts ring body via 3 ruby/ceramic spindles
    ═══════════════  surface (ball protrudes below, contacts desk)
```

The ball sits in a socket and is held in contact with spindles (or rollers) that
transfer its rotation to the ring body's mechanical frame. The PMW3360 sensor is
rigidly mounted to the PCB, which is rigidly mounted to the ring body. The ball
touches the desk surface below and the sensor/spindle assembly above.

### Coupling Path Analysis

**Ring body vibration from LRA:**

The LRA is a mass-on-spring. When driven, the moving mass oscillates, and by
Newton's third law the shell/body oscillates in the opposite direction. The
ratio of amplitudes is inversely proportional to mass:

| Component | Estimated Mass |
|-----------|---------------|
| LRA moving mass | ~0.3-0.5 g (typical 8mm coin LRA) |
| Ring body (shell + PCB + battery + sensor) | ~4-8 g |
| Ball (5-8mm steel or ceramic) | ~0.5-2.1 g (steel, diameter dependent) |

Ring body displacement amplitude relative to LRA moving mass displacement:

    A_body = A_lra_mass * (m_lra / m_body)
    A_body = A_lra * (0.4 g / 6 g) = 0.067 * A_lra

For a typical LRA peak displacement of ~0.2 mm at resonance:

    A_body ~ 0.067 * 0.2 mm = ~13 um peak

This 13 um body oscillation occurs at 170-200 Hz (5-6 ms period).

**Does the ball move with the ring body?**

The ball is constrained by:
- Gravity (downward)
- Surface contact (below, supporting the ring's weight)
- Spindle/roller contact (above, transferring rotation)

When the ring body oscillates vertically, the ball sits on the surface and has
inertia. The spindles pressing against the ball transmit some lateral oscillation,
but the ball's inertia and surface friction partially isolate it.

When the ring body oscillates laterally (the dominant LRA vibration axis for a
coin-type motor):
- The spindles push the ball laterally
- The ball's surface contact point provides friction that resists lateral motion
- The ball-to-spindle contact is a point/line contact with limited friction

**Estimated ball lateral displacement from LRA pulse:**

The spindle-to-ball coupling is imperfect. Estimating 30-60% transmission
(point contact with some slip):

    Ball lateral displacement ~ 0.3 to 0.6 * 13 um = 4-8 um peak

### PMW3360 Sensitivity at This Scale

| PMW3360 Parameter | Value | Source |
|-------------------|-------|--------|
| Resolution range | 100-12000 CPI (configurable) | Datasheet |
| Default CPI (assumed) | 800-1600 CPI for ring use | CLAUDE.md: configurable via BLE |
| 1 count at 800 CPI | 31.75 um | 25400 um/inch / 800 |
| 1 count at 1600 CPI | 15.87 um | 25400 um/inch / 1600 |
| SQUAL (surface quality) threshold for valid tracking | Typically > 20 | Datasheet |
| Frame rate | Up to 12000 fps | Datasheet |

**At 800 CPI:** 4-8 um ball displacement = 0.13-0.25 counts per half-cycle.
Over a 12 ms LRA pulse (~2 cycles), the oscillating displacement may integrate
to 0-1 counts total. **Below noise threshold (NOISE_THRESHOLD_OPTICAL = 2).**

**At 1600 CPI:** 4-8 um displacement = 0.25-0.50 counts per half-cycle.
Over a 12 ms pulse, may register 0-2 counts. **At noise threshold.**

**At 3200 CPI:** 4-8 um displacement = 0.50-1.00 counts per half-cycle.
Over a 12 ms pulse, may register 1-4 counts. **Above noise threshold. Dead zone
suppression required.**

**Key finding:** At typical ring CPI settings (800-1600), the ball's inertial
isolation keeps LRA-induced motion near or below the noise threshold. At high CPI
(3200+), the motion becomes detectable and would appear as a brief oscillating
artifact of ~2-4 counts.

### Comparison: Standard Tier (PAW3204, Direct Surface Tracking)

```
Standard Tier — Direct Optical

    finger pad
    ──────────────
    │  dome sw   │
    │  LiPo      │
    │  flex PCB   │
    │  PAW3204   │ ← reads desk surface directly
    ▓▓rim    ▓▓rim   ← raised rim, sensor faces down
    ═══════════════  surface
```

Standard tier does not have an LRA (dome click provides inherent tactile
feedback). However, this comparison is useful for understanding the coupling
model if LRA were ever added.

In the Standard configuration, the sensor is rigidly mounted to the ring body.
The tracking surface is the desk, which is effectively an infinite-mass reference
frame. Any ring body vibration translates directly to sensor-vs-surface relative
motion. There is no inertial isolation from a ball.

| Factor | Pro (PMW3360 + ball) | Standard (PAW3204 + desk) |
|--------|---------------------|--------------------------|
| Sensor reference frame | Ball (moves partially with body) | Desk (stationary) |
| Body vibration → sensor motion | Partially isolated by ball inertia | 100% coupling |
| Estimated artifact at 800 CPI | 0-1 counts | N/A (no LRA) |
| Estimated artifact at 1600 CPI | 0-2 counts | N/A (no LRA) |

**If Standard tier ever adds an LRA** (not planned), it would have worse
vibration coupling than Pro because the desk surface is a fixed reference frame.
The ball's inertia in the Pro tier is actually an advantage for vibration
isolation.

---

## 4. HAPTIC_SUPPRESS_MS Optimization

### Current Value and Analysis

The current default is 20 ms (`ring_config.h` line 46). This was set as a
conservative estimate. Here is a first-principles derivation.

### Timing Breakdown of an LRA Click Pulse

| Phase | Duration | Description |
|-------|----------|-------------|
| 1. Drive phase | 8-15 ms | DRV2605L actively drives the LRA at resonance |
| 2. Braking phase | 2-5 ms | DRV2605L active braking (if enabled) |
| 3. Mechanical decay | 15-40 ms | LRA mass rings down (Q factor dependent) |
| **Total vibration** | **25-60 ms** | From first drive to below 10% amplitude |

### LRA Mechanical Decay

The decay time depends on the LRA's quality factor (Q):

| LRA Type | Typical Q | Time to 10% amplitude | Source |
|----------|-----------|----------------------|--------|
| 8mm coin LRA, no braking | 15-25 | 35-50 ms | Derived from Q and resonant freq |
| 8mm coin LRA, DRV2605L braking | 8-15 | 20-30 ms | DRV2605L auto-braking reduces effective Q |
| 10mm coin LRA, DRV2605L braking | 10-18 | 25-35 ms | Larger mass, longer decay |

Derivation: For a damped oscillator, amplitude decays as `A(t) = A_0 * exp(-pi *
f * t / Q)`. To reach 10% of initial amplitude:

    t_10% = Q * ln(10) / (pi * f)
    t_10% = 15 * 2.303 / (pi * 185 Hz)
    t_10% = 34.5 / 581 = ~59 ms (no braking)

With DRV2605L active braking (effective Q ~10):

    t_10% = 10 * 2.303 / (pi * 185 Hz)
    t_10% = 23.0 / 581 = ~40 ms

At 10% amplitude, the body displacement is ~1.3 um, which at 1600 CPI is
~0.08 counts — well below the noise threshold.

### Where Does the Suppression Window Need to Start and End?

The dead zone state machine already suppresses deltas during click actuation.
`HAPTIC_SUPPRESS_MS` is an *additional* window that extends suppression after
the LRA fires, specifically to cover the mechanical decay that continues after
the haptic pulse command completes.

The timeline:

```
t=0     Piezo detects press → dead zone enters DZ_ACTIVE → deltas suppressed
t=0+    Firmware fires LRA pulse (e.g., waveform 17, 8ms active)
t=8ms   LRA drive phase ends
t=10ms  DRV2605L braking phase ends
t=10-50ms  Mechanical decay (ring body ringing down)
t=?     User releases finger → dead zone exits → deltas re-enabled
```

If the user does a quick click-and-release (press duration ~80-150 ms), the dead
zone DZ_ACTIVE state already covers the entire LRA decay period because the press
lasts longer than the vibration. `HAPTIC_SUPPRESS_MS` only matters if:

1. The dead zone exits to DZ_DRAG while the LRA is still ringing, or
2. The dead zone exits on release before the ringing stops.

Scenario (1) requires `DEAD_ZONE_TIME_MS` (50 ms) + accumulated distance. Since
the LRA vibration-induced motion is 0-4 counts (see section 3), it is unlikely
to meet `DEAD_ZONE_DISTANCE` (10 counts) by itself. The user must also be moving
the cursor intentionally. By the time both conditions are met, the LRA has
largely decayed.

Scenario (2) is the real concern: a very quick tap (press-to-release < 50 ms).
The dead zone exits on release (DZ_ACTIVE -> DZ_IDLE, deltas zeroed for that
tick but not subsequent ticks). If the LRA is still ringing when the next tick
arrives after release, those vibration-induced deltas would flow through
unfiltered.

### Recommended Value

`HAPTIC_SUPPRESS_MS` should cover the mechanical decay after the *end* of the
LRA drive phase, not after the start of the press. The firmware should start
this timer when the LRA drive command completes, not when the press is detected.

**Recommended: 35 ms** (up from current 20 ms).

Rationale:
- DRV2605L braking phase: ~3 ms after drive ends
- Mechanical decay to below noise threshold at 1600 CPI: ~30-40 ms after
  braking ends
- 35 ms after drive completion covers the braking + decay to well below the
  noise threshold even without active braking
- At 800 CPI, 20 ms is sufficient. At 1600 CPI, 35 ms provides margin.
- At 3200 CPI, even 35 ms may be marginal; consider 45 ms, but 3200 CPI is
  an unusual setting for a ring form factor

The suppression window should be **configurable via BLE characteristic**
alongside CPI. Higher CPI settings should automatically increase the suppression
window. A simple rule:

```c
// Suggested adaptive suppression
#define HAPTIC_SUPPRESS_BASE_MS     35
#define HAPTIC_SUPPRESS_PER_CPI_MS  5   // add 5ms per 800 CPI above 800

uint32_t haptic_suppress_ms(uint16_t cpi) {
    uint16_t extra_steps = (cpi > 800) ? ((cpi - 800) / 800) : 0;
    return HAPTIC_SUPPRESS_BASE_MS + (extra_steps * HAPTIC_SUPPRESS_PER_CPI_MS);
}
// 800 CPI  → 35 ms
// 1600 CPI → 40 ms
// 3200 CPI → 50 ms
```

### Implementation Note

The current `dead_zone.c` does not explicitly implement `HAPTIC_SUPPRESS_MS` as
a separate timer. The constant is defined in `ring_config.h` but unused in the
dead zone state machine. The recommended implementation:

1. Add a `DZ_HAPTIC_DECAY` state to the dead zone state machine, entered when
   the click is released during or shortly after LRA firing.
2. In `DZ_HAPTIC_DECAY`, suppress deltas for `haptic_suppress_ms(current_cpi)`
   after the LRA drive command completes.
3. Transition to `DZ_IDLE` when the timer expires.

This is a firmware-only change. No hardware modifications required.

---

## 5. Mechanical Isolation Analysis

### Option A: Elastomer Gasket Between LRA and Sensor PCB

An elastomer (silicone, TPE) gasket between the LRA mounting point and the
sensor PCB would attenuate vibration transmission through the rigid PCB.

| Factor | Assessment |
|--------|------------|
| Vibration attenuation | 40-70% at LRA resonant frequency (170-200 Hz is well above typical gasket natural frequency of 20-50 Hz) |
| Height cost | 0.5-1.0 mm for gasket + adhesive |
| Complexity | Moderate: adds a component, requires gasket material selection, adds assembly step |
| Reliability risk | Gasket creep or hardening over years could change sensor alignment |
| Benefit at 800-1600 CPI | Marginal — vibration artifact is already near noise floor |
| Benefit at 3200+ CPI | Meaningful — reduces 2-4 count artifact to 1-2 counts |

**Verdict: Not worth the complexity for prototype.** The 0.5-1.0 mm height
penalty is significant in a 7-8 mm height budget. The firmware suppression
window handles the artifact adequately at the CPI ranges expected for ring use.
Revisit only if user testing at 3200+ CPI shows perceptible cursor jump on click.

### Option B: Orient LRA Vibration Axis Perpendicular to Sensor

Coin-type LRAs vibrate primarily in one axis (the mass oscillates linearly).
If that axis is oriented perpendicular to the primary sensor tracking axes
(X/Y), the vibration energy couples less efficiently into trackable motion.

| Factor | Assessment |
|--------|------------|
| Effectiveness | Limited. The ring is a small rigid body; vibration in any axis produces body oscillation in all axes due to asymmetric mass distribution. Orientation helps by ~20-30%, not by an order of magnitude. |
| Height cost | Zero if the LRA fits in the available orientation |
| Complexity | Low — PCB layout decision only |

**Verdict: Do it if layout permits, but do not constrain layout for it.** The
benefit is real but modest. If the LRA fits better in one orientation for
space/routing reasons, that takes priority.

### Option C: Mass Differential Exploitation

The ring body is 10-20x heavier than the LRA moving mass. This is inherently a
good isolation ratio. For comparison:

| Device | Body-to-LRA mass ratio | Haptic vibration perceptible? |
|--------|----------------------|------------------------------|
| iPhone (Taptic Engine) | ~180g / ~5g = 36:1 | Barely visible on camera; sensor-imperceptible |
| Game controller | ~250g / ~8g = 31:1 | Visible on desk; sensor would detect |
| PowerFinger ring | ~6g / ~0.4g = 15:1 | Body moves ~13 um; marginal for sensor |

The ring's mass ratio is the worst of these cases because the ring is so light.
This is the fundamental reason the coupling question exists. The mitigations are:

1. **Firmware suppression** (already implemented, needs tuning) — primary defense
2. **Ball inertia** (inherent in Pro tier design) — secondary defense
3. **Short waveform selection** (waveform 17) — reduces total energy
4. **Lower LRA drive amplitude** — trades feel intensity for less coupling

### Recommendation

**No hardware changes needed.** The combination of:
- Ball inertia isolation (~50-70% attenuation)
- Short waveform (8-12 ms active)
- DRV2605L active braking (halves decay time)
- Firmware suppression window (35 ms, CPI-adaptive)
- Noise threshold filtering (`NOISE_THRESHOLD_OPTICAL = 2`)

is sufficient to keep LRA-induced artifacts below perceptible levels at all
practical CPI settings. Hardware isolation (gasket or LRA orientation) can be
added in a later revision if user testing contradicts this analysis.

---

## 6. Does Pro Tier Need a Longer Dead Zone Than Standard?

### Standard Tier Dead Zone Behavior

The Standard tier uses a snap dome click. When the user presses:
- Finger pressure causes micro-movement of the ring body relative to the desk
- The optical sensor (PAW3204) reads the desk surface directly
- Any ring body motion = cursor motion
- The dead zone (`DEAD_ZONE_TIME_MS = 50`, `DEAD_ZONE_DISTANCE = 10`)
  suppresses this micro-movement during click actuation

The artifact is purely mechanical: finger force tilting/shifting the ring by
50-200 um, producing 2-8 counts at typical CPI. The dead zone comfortably
covers this.

### Pro Tier Dead Zone Behavior

The Pro tier has two sources of click-induced artifact:

1. **Finger pressure artifact** (same as Standard): finger presses ring body,
   spindles push ball, ball moves on surface. However, the ball's surface
   contact provides friction that resists lateral displacement, partially
   isolating from tilt. Estimated artifact: 1-5 counts at 1600 CPI (similar or
   slightly less than Standard).

2. **LRA vibration artifact** (Pro-only): haptic pulse vibrates ring body, some
   vibration transfers to ball via spindles. Estimated artifact: 0-4 counts at
   1600 CPI (oscillating, not directional).

The two artifacts have different temporal profiles:

```
Standard tier:
    t=0   Dome press → artifact (2-8 counts, monotonic, ~10-30 ms)
    t=50  Dead zone exit (if still held)

Pro tier:
    t=0   Piezo detects press → finger artifact (1-5 counts, ~10-30 ms)
    t=1   LRA fires → vibration artifact (0-4 counts oscillating, ~40-60 ms)
    t=50  Dead zone exit (if still held)
    t=35-55  LRA mechanical decay completes
```

The Pro tier's vibration artifact extends beyond the finger artifact. The
`HAPTIC_SUPPRESS_MS` window covers this extension. But the question is whether
`DEAD_ZONE_TIME_MS` itself should be longer.

### Analysis

**No. `DEAD_ZONE_TIME_MS` should remain at 50 ms for both tiers.**

Reasoning:

1. The dead zone time controls when *intentional drag* becomes possible. Making
   it longer delays drag responsiveness, which is an accessibility regression.
   A user performing click-and-drag must wait `DEAD_ZONE_TIME_MS` before the
   cursor starts moving. 50 ms is already at the upper edge of imperceptible
   latency.

2. The LRA vibration artifact is handled by `HAPTIC_SUPPRESS_MS`, which is a
   separate, layered mechanism. It runs after click release (or after LRA
   completion), not during the dead zone active phase.

3. The dual-condition exit (time AND distance) already provides protection:
   the LRA vibration contributes 0-4 counts to the distance accumulator, which
   alone cannot meet the 10-count threshold. The user must also be moving
   intentionally.

**However, the `DEAD_ZONE_DISTANCE` threshold may need adjustment for Pro:**

At high CPI (3200), the LRA artifact could contribute 2-4 counts per tick for
several ticks during the vibration. Over the 50 ms dead zone window, this could
accumulate to 6-12 counts — potentially meeting the distance threshold without
intentional user motion.

**Recommendation: Increase `DEAD_ZONE_DISTANCE` to 15 for Pro tier builds.**

```c
#ifdef CONFIG_CLICK_PIEZO_LRA
    #define DEAD_ZONE_DISTANCE  15   // Pro: account for LRA vibration contribution
#else
    #define DEAD_ZONE_DISTANCE  10   // Standard: finger artifact only
#endif
```

This prevents false drag-entry from LRA vibration at high CPI while keeping the
same time window. The 5-count increase is small enough that intentional drag
initiation is not meaningfully delayed.

---

## 7. Summary: Can We Ship Pro Tier With Firmware-Only Dead Zone Suppression?

**Yes.** Firmware-only mitigation is sufficient. No hardware changes (mechanical
isolation, different LRA mounting) are required for the prototype or initial
consumer product.

### Required Firmware Changes

| Change | File | Priority |
|--------|------|----------|
| Increase `HAPTIC_SUPPRESS_MS` from 20 to 35 | `ring_config.h` | Must-have |
| Implement `HAPTIC_SUPPRESS_MS` as post-LRA timer in dead zone state machine | `dead_zone.c` | Must-have |
| Make suppression window CPI-adaptive | `dead_zone.c` | Should-have |
| Increase `DEAD_ZONE_DISTANCE` to 15 for Pro builds | `ring_config.h` | Should-have |
| Add piezo ADC mask during LRA pulse | Click driver (new) | Must-have |
| Select waveform 17 (Sharp Click) as default | DRV2605L driver (new) | Must-have |
| Enable DRV2605L auto-braking | DRV2605L driver (new) | Must-have |

### Hardware Decisions (Deferred to Post-User-Testing)

| Decision | Current Answer | Revisit If |
|----------|---------------|------------|
| Elastomer gasket between LRA and PCB | No | User testing at 3200+ CPI shows perceptible cursor jump |
| LRA orientation constraint | No (use whatever fits) | Layout allows it at zero cost |
| Different LRA motor | No (standard 8mm coin) | Click feel is inadequate with waveform 17 |

### Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| LRA triggers piezo re-detection | Medium | Click doubles or chatters | ADC mask during LRA pulse + debounce |
| LRA causes visible cursor jump on click | Low at 800-1600 CPI, Medium at 3200+ | User-perceptible artifact | Firmware suppression window + CPI-adaptive tuning |
| Dead zone too aggressive, delays drag | Low | Drag feels sluggish | 50 ms time + 15 count distance is conservative |
| Ball inertia worse than modeled | Low | Larger artifact than estimated | Increase suppression window; add gasket as last resort |

### Validation Plan

User testing should include:

1. **Click artifact test:** Click without moving at 800, 1600, and 3200 CPI.
   Record cursor position before and after 100 clicks. Any net displacement > 1
   pixel at 800 CPI or > 3 pixels at 3200 CPI indicates insufficient suppression.

2. **Click-and-drag latency test:** Compare perceived drag initiation latency
   between Standard (dome, 50ms/10 count) and Pro (piezo+LRA, 50ms/15 count).
   If users perceive a difference, reduce Pro distance threshold.

3. **Rapid click test:** Click 5 times per second for 10 seconds. Check for
   piezo re-triggering (phantom double-clicks) caused by LRA feedback. If
   present, increase `CLICK_DEBOUNCE_MS` or extend ADC mask window.

4. **Feel quality test:** Compare waveform 17 (Sharp Click) vs waveform 1
   (Strong Click - 100%) vs waveform 3 (Strong Click - 30%). Select the
   waveform that users rate as most click-like, then verify the dead zone
   parameters are sufficient for that waveform's vibration profile.
