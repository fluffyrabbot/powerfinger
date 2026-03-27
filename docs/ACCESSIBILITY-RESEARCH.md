# Accessibility User Research Synthesis

This document synthesizes published research on motor impairment, tremor
characteristics, grip/pinch force profiles, and assistive pointing device design
to inform PowerFinger's ring shell geometry, click mechanism, and firmware filter
parameters. Findings are mapped to specific firmware constants defined in
`firmware/ring/components/state_machine/include/ring_config.h`.

The project's first hard rule is accessibility-first. This document exists to
make that rule concrete and testable.

---

## 1. Target User Populations

### 1.1 Spinal Cord Injury (Cervical, C5-C7)

**Functional profile:**

| Level | Preserved function | Lost function | Pinch capability |
|-------|-------------------|---------------|------------------|
| C5 | Shoulder, elbow flexion | Wrist extension, all hand intrinsics | No active pinch; tenodesis only with gravity assist |
| C6 | + Wrist extension | Finger flexors, intrinsics | Tenodesis pinch (passive, gravity-dependent), ~1-5 N |
| C7 | + Triceps, some finger extension | Fine finger control, intrinsics | Weak voluntary pinch, ~5-16 N post-reconstruction |

**Key data points:**

- Smaby et al. (2004, Journal of Rehabilitation Research & Development)
  identified pinch force requirements for 12 ADL tasks ranging from 1.4 N
  (pressing a remote button) to 31.4 N (inserting an electrical plug). Surgical
  tendon transfer in C5-C7 patients restored key pinch to at least 16 N.
  Pre-surgical tenodesis pinch achieved only a 6.7% success rate on pure key
  pinch tasks.

- Mateo et al. (2013, Journal of NeuroEngineering and Rehabilitation) reviewed
  upper limb kinematics in cervical SCI, documenting significantly reduced
  movement speed, reach envelope, and grip aperture control compared to
  uninjured controls.

- Wangdell & Friden (2012, Journal of Hand Surgery) measured functional
  tenodesis pinch in C6 tetraplegics 1-16 years post-injury. Those with
  tenodesis pinch to the proximal or middle phalanx of the index finger had
  significantly higher functional independence scores (43.7 and 34.2
  respectively) vs. 17.8 for absent tenodesis.

**Ring implications:**

- C5 patients: ring mouse is likely **not usable** without external assistance
  to don the ring. No active pinch means the user cannot generate the 150 gf
  (1.47 N) minimum click force of a standard snap dome.
- C6 patients: **marginal candidates.** Tenodesis pinch may generate enough
  force for a low-force dome (~100 gf), but only with the wrist in extension.
  Finger sizing is unstable due to edema. Piezo click with a very low ADC
  threshold (~50 gf equivalent) is the better option.
- C7 patients: **viable candidates** with low-force click mechanisms. Active
  finger extension enables donning/doffing. Weak but present finger flexion can
  generate 5-16 N, well above the 1.47-2.45 N range of standard snap domes.

### 1.2 Essential Tremor

**Tremor characteristics (Elble, 2009, Tremor and Other Hyperkinetic Movements;
Deuschl et al., 1998, Movement Disorders):**

| Parameter | Value |
|-----------|-------|
| Frequency | 4-12 Hz (most common 5-7 Hz, decreases with age and severity) |
| Type | Action/postural tremor (present during voluntary movement) |
| Amplitude | Sub-millimeter in mild cases; up to 2-4 cm in severe cases |
| Finger involvement | Yes, especially during precision tasks |
| Worsens with | Anxiety, fatigue, caffeine, precision demand |

- Haubenberger et al. (2011, Tremor and Other Hyperkinetic Movements) showed
  that tremor severity does not correlate strongly with functional task
  performance, implying that simple amplitude measurements are insufficient for
  predicting device usability.

- Essential tremor is an **action tremor** — it is present during movement and
  absent at rest. This is the opposite of Parkinsonian tremor. For a ring mouse,
  this means tremor is maximal during the exact moments the user is trying to
  point, and minimal when the finger is stationary on the surface.

**Ring implications:**

- Essential tremor is the **primary target population** where firmware filtering
  provides the most value.
- Tremor at 4-12 Hz with sub-cm amplitude is within the range that a
  combination of dead zone, noise threshold, and low-pass filtering can suppress
  without destroying intentional movement.
- Severe essential tremor (>2 cm amplitude) may overwhelm sensor-level
  filtering. Companion app software filtering (SteadyMouse-style) provides an
  additional layer.

### 1.3 Parkinson's Disease

**Tremor characteristics (Jankovic, 2008, Journal of Neurology, Neurosurgery &
Psychiatry):**

| Parameter | Value |
|-----------|-------|
| Rest tremor frequency | 3-6 Hz (classic "pill-rolling") |
| Action tremor frequency | 5-8 Hz |
| Re-emergent tremor | Postural tremor that appears after a latency of several seconds |
| Rigidity | Cogwheel or lead-pipe rigidity in limbs |
| Bradykinesia | Slowness of movement initiation and execution |

- Distinguishing PD from ET: PD tremor is maximal at rest and suppressed during
  voluntary movement. Zhang et al. (2017, Parkinson's Disease) used EMG
  frequency analysis to distinguish PD tremor (<6 Hz, alternating agonist/
  antagonist) from ET (>5 Hz, co-contraction pattern).

**Ring implications:**

- Rest tremor is **not a significant problem** for ring mouse use — the tremor
  suppresses when the user begins an intentional pointing movement.
- **Rigidity and bradykinesia are the real barriers.** Donning a ring requires
  finger dexterity that rigidity impairs. Movement initiation delay
  (bradykinesia) means the firmware's idle-to-active transition must be patient
  — a premature sleep timeout punishes slow starters.
- Re-emergent tremor (appearing seconds after assuming a posture) could cause
  cursor drift when the user pauses mid-pointing. The firmware should detect
  this pattern: stable cursor -> sudden oscillation at 3-6 Hz -> suppress.
- **Early-to-mid PD patients are viable candidates.** Advanced PD with severe
  rigidity is contraindicated (see Section 6).

### 1.4 Cerebral Palsy (Spastic and Dyskinetic)

**Hand function classification (Eliasson et al., 2006, Developmental Medicine &
Child Neurology):**

The Manual Ability Classification System (MACS) classifies hand function in CP
on a five-level scale:

| Level | Description | Ring viability |
|-------|-------------|----------------|
| I | Handles objects easily and successfully | Viable |
| II | Handles most objects, reduced quality/speed | Viable with adaptations |
| III | Handles objects with difficulty, needs help to modify activities | Marginal — requires low-force click, loose ring fit |
| IV | Handles limited selection of easily managed objects in adapted situations | Likely contraindicated |
| V | Does not handle objects | Contraindicated |

**Grip force data:**

- Smits-Engelsman et al. (2023, Developmental Medicine & Child Neurology)
  measured grip strength in adolescents and adults with CP. Affected hands in
  hemiplegic CP showed mean grip strength of 2.7 kg (SD 3.4) vs. 16.3 kg (SD
  12.3) for contralateral hands (Arnould et al., 2014, Physical & Occupational
  Therapy in Pediatrics).

- 2.7 kg grip = ~26.5 N. Pinch force is typically 15-30% of grip force,
  implying ~4-8 N tip pinch for affected hands. This exceeds snap dome
  requirements (1.47-2.45 N) but leaves minimal margin.

**Ring implications:**

- Spasticity creates unpredictable grip force with sudden involuntary
  contractions. The click mechanism must handle force spikes without registering
  false clicks. A higher debounce time (10-15 ms vs. default 5 ms) is
  appropriate.
- Dyskinetic CP (MACS IV-V) is **contraindicated** — involuntary movements are
  continuous and unfiltered by dead zone approaches.
- Spastic CP (MACS I-III) is viable with firmware parameter adjustments.

### 1.5 Muscular Dystrophy (Duchenne, Myotonic)

**Progression pattern (Seferian et al., 2015, Neuromuscular Disorders; Decostre
et al., 2024, Institut de Myologie):**

- Hand function in DMD is initially preserved despite progressive loss of grip
  and pinch strength. Function declines only when grip force falls below 41% of
  age/gender-predicted values. For pinch force, the threshold is 42% of
  predicted.
- In practical terms: a healthy young male has ~11-12 kg key pinch (~108-118 N).
  The 42% threshold is ~45-50 N. Tip pinch is lower — the functional threshold
  for ring use might be ~15-20 N tip pinch.
- Normal strength is not essential for functional hand use. Individuals with DMD
  and SMA retain hand function longer than limb-level strength measurements
  would predict, because daily tasks require far less than maximum voluntary
  contraction.

**Ring implications:**

- DMD/SMA patients are **viable early in disease progression.** A ring mouse
  that requires only 1.5-2.5 N click force and minimal grip force to retain on
  the finger is well within capability until late-stage disease.
- The ring must be lightweight. The current BOM targets ~8-12g total ring weight.
  This is critical — a heavy ring accelerates fatigue in weakened muscles.
- **Progressive loss demands adjustable sensitivity.** DPI multiplier, dead zone
  size, and click threshold must all be adjustable via BLE characteristic as the
  disease progresses.

### 1.6 ALS / Motor Neuron Disease

**Progression pattern (Hobson & McDermott, 2016, Amyotrophic Lateral Sclerosis
and Frontotemporal Degeneration):**

- ALS progression is highly variable. Limb-onset ALS typically begins with
  distal weakness in one limb, progressing to adjacent segments over months.
- Fine finger-hand motor skills degrade early — precision grip before power
  grip, distal before proximal.
- ALSFRS-R (ALS Functional Rating Scale - Revised) subscores for handwriting
  and cutting food decline earliest among upper limb items.

**Ring implications:**

- ALS patients are **candidates during a time window** — after diagnosis but
  before finger dexterity drops below the ring-use threshold. This window varies
  from months to years depending on progression rate and onset site.
- The companion app should support a smooth transition pathway: ring mouse ->
  wand (requiring less finger dexterity) -> head mouse / eye tracking (hands-
  free). PowerFinger's multi-device architecture already supports this
  progression.
- **Do not market as a long-term solution for ALS.** Document the transition
  pathway honestly.

---

## 2. Tremor Filtering Algorithms: State of the Art

### 2.1 Approaches in Existing Assistive Devices

| Device / Software | Filtering approach | Configurable parameters |
|-------------------|-------------------|------------------------|
| **SteadyMouse** | Autoregressive model with stretching window; predicts intended target from recent samples; second-order system smooths cursor to predicted target | Tremor intensity (1-10 scale), click stabilization on/off |
| **AMAneo USB** | Hardware analog tremor filter with adjustable intensity; sits between any USB mouse and the computer | Filter intensity dial (continuous) |
| **SmoothPoint (Vortant)** | Linear and nonlinear physics equations; attempts to distinguish wanted from unwanted motion | Multiple parameters (sensitivity, smoothing, dead zone) |
| **FLipMouse** | Configurable dead zone per axis; speed multiplier; sip/puff pressure threshold | Deadzone (per axis), speed, sensitivity, threshold pressure |
| **LipSync** | Hall-effect joystick with 50 gf full deflection; configurable dead zone and speed profiles | Dead zone, speed profiles, sip/puff thresholds |

### 2.2 Algorithm Classes Relevant to PowerFinger

**Low-pass filter (simple moving average or exponential):**

- Suppresses high-frequency tremor (>4 Hz) while passing intentional movement
  (<2 Hz).
- Introduces latency proportional to filter order. At 15 ms connection interval,
  a 3-sample moving average adds 45 ms latency — perceptible but tolerable.
- Appropriate for essential tremor (4-12 Hz) and physiological tremor (8-12 Hz).
- **Not appropriate alone** — cannot distinguish slow tremor from intentional
  movement.

**Dead zone (threshold filter):**

- Suppresses all movement below a distance/velocity threshold.
- PowerFinger's current implementation (`dead_zone.c`) applies dead zone only
  during click actuation. A separate motion dead zone (noise threshold) handles
  idle-state jitter.
- Dead zone size is the critical parameter: too small = tremor passes through;
  too large = fine cursor control is impossible.
- FLipMouse uses per-axis dead zones with configurable values, which is
  appropriate for asymmetric tremor (common in ET and PD).

**Adaptive band-reject filter:**

- Estimates tremor frequency in real-time, then notch-filters at that frequency.
- Riviere et al. (1998, IEEE Transactions on Biomedical Engineering) pioneered
  the Weighted Fourier Linear Combiner (WFLC) for surgical tremor suppression.
  The WFLC adaptively tracks tremor frequency and removes it from the signal.
- Computationally heavier than low-pass or dead zone. Feasible on ESP32-C3 but
  must be profiled for real-time operation at 7.5 ms connection interval.

**Autoregressive model + Kalman filter:**

- Veluvolu & Ang (2011, IEEE Transactions on Instrumentation and Measurement)
  demonstrated AR-KF tremor estimation with 88 +/- 2% accuracy in real-time.
- Models tremor as an AR process and uses Kalman filtering to separate tremor
  from voluntary motion.
- **Too computationally expensive** for the ESP32-C3 at 7.5 ms tick rate.
  Suitable for companion app post-processing.

### 2.3 Recommended Firmware Filter Architecture

The firmware should implement a three-stage filter pipeline:

```
Raw sensor deltas
  |
  v
[Stage 1: Noise threshold]     -- Suppress sensor jitter
  |                                NOISE_THRESHOLD_OPTICAL = 2 counts
  v
[Stage 2: Tremor suppression]  -- Low-pass or adaptive filter
  |                                Configurable via BLE characteristic
  v
[Stage 3: Click dead zone]     -- Suppress click-induced micro-movement
  |                                DEAD_ZONE_TIME_MS, DEAD_ZONE_DISTANCE
  v
HID report
```

Stage 2 does not currently exist in firmware. It should be implemented as a
configurable exponential moving average (EMA) with adjustable alpha, selectable
per user profile:

- **No filter** (alpha = 1.0): For users without tremor. Zero added latency.
- **Light** (alpha = 0.6): ~1 sample latency. Mild essential tremor, physiological tremor.
- **Medium** (alpha = 0.3): ~2-3 sample latency. Moderate essential tremor, early PD re-emergent tremor.
- **Heavy** (alpha = 0.15): ~5 sample latency (~75 ms at 15 ms interval). Severe essential tremor. Noticeable cursor lag.

The alpha value must be exposed as a BLE GATT characteristic (uint8, 0-255
mapped to 0.0-1.0) so the companion app can adjust it without reflashing.

---

## 3. Firmware Parameter Recommendations by Population

### 3.1 Parameter Defaults (Current)

| Parameter | Current default | Defined in |
|-----------|----------------|------------|
| `NOISE_THRESHOLD_OPTICAL` | 2 counts | `ring_config.h` |
| `NOISE_THRESHOLD_HALL` | 5 counts | `ring_config.h` |
| `DEAD_ZONE_TIME_MS` | 50 ms | `ring_config.h` |
| `DEAD_ZONE_DISTANCE` | 10 counts | `ring_config.h` |
| `CLICK_DEBOUNCE_MS` | 5 ms | `ring_config.h` |
| `IDLE_TRANSITION_MS` | 250 ms | `ring_config.h` |
| `SLEEP_TIMEOUT_MS` | 45000 ms | `ring_config.h` |
| Click force (snap dome) | 150-250 gf | Hardware (dome selection) |
| DPI multiplier | 1-255 via BLE | Companion app / BLE GATT |
| Tremor filter (Stage 2) | Not yet implemented | -- |

### 3.2 Recommended Profiles

These profiles should be selectable via the companion app and stored in the
ring's NVS as user presets.

#### Profile: Default (no motor impairment)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 2 counts | Standard sensor jitter floor |
| Dead zone time | 50 ms | Sufficient for dome micro-movement |
| Dead zone distance | 10 counts | Standard |
| Click debounce | 5 ms | Standard dome bounce |
| Tremor filter | Off (alpha=1.0) | No tremor to suppress |
| DPI multiplier | 128 (mid-range) | Default sensitivity |
| Sleep timeout | 45 s | Standard |
| Click force | 150-200 gf dome | Standard |

#### Profile: Essential Tremor (Mild)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 4 counts | Higher floor catches mild tremor at rest |
| Dead zone time | 80 ms | Longer suppression during click (tremor amplitude increases during precision tasks) |
| Dead zone distance | 15 counts | Wider dead zone for tremor-induced movement during click |
| Click debounce | 8 ms | Tremor may cause re-contact during dome release |
| Tremor filter | Light (alpha=0.6) | Suppresses 6-12 Hz with minimal latency |
| DPI multiplier | 96 (lower) | Reduces tremor amplification — same physical displacement moves cursor less |
| Sleep timeout | 45 s | Standard |
| Click force | 200 gf dome | Slightly higher force reduces accidental clicks from tremor |

#### Profile: Essential Tremor (Severe)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 8 counts | Aggressive floor for large-amplitude tremor at rest |
| Dead zone time | 120 ms | Extended suppression — severe tremor generates large click-time movement |
| Dead zone distance | 25 counts | Very wide dead zone |
| Click debounce | 12 ms | Tremor-induced dome re-contact |
| Tremor filter | Heavy (alpha=0.15) | Maximum suppression; ~75 ms cursor latency is acceptable tradeoff |
| DPI multiplier | 64 (low) | Significant amplification reduction |
| Sleep timeout | 60 s | Allow longer pauses |
| Click force | 250 gf dome | Highest standard dome force |

#### Profile: Parkinson's Disease (Early-Mid)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 3 counts | Rest tremor is suppressed during voluntary motion; noise threshold mostly handles sensor jitter |
| Dead zone time | 60 ms | Slightly wider for re-emergent tremor during click hold |
| Dead zone distance | 12 counts | Slightly wider |
| Click debounce | 8 ms | Rigidity may cause uneven dome release |
| Tremor filter | Medium (alpha=0.3) | Catches re-emergent tremor (3-6 Hz) without excessive latency |
| DPI multiplier | 96 | Moderate reduction — bradykinesia means large movements are hard, so don't over-reduce |
| Sleep timeout | 90 s | Bradykinesia causes long pauses; premature sleep is a usability catastrophe |
| Click force | 150 gf dome (low) | Rigidity reduces available finger force |

#### Profile: Cerebral Palsy (Spastic, MACS I-II)

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 5 counts | Spasticity causes involuntary micro-movements at rest |
| Dead zone time | 100 ms | Spastic co-contraction during click causes prolonged sensor disturbance |
| Dead zone distance | 20 counts | Wide dead zone for spasticity-induced movement |
| Click debounce | 15 ms | Spastic muscles may cause multiple dome contacts |
| Tremor filter | Medium (alpha=0.3) | Suppresses spastic tremor component (typically 5-8 Hz) |
| DPI multiplier | 80 | Reduce amplification of involuntary movements |
| Sleep timeout | 90 s | Movement initiation may be slow |
| Click force | 150 gf dome (low) | Affected hand grip strength may be limited (~26 N grip, ~4-8 N pinch) |

#### Profile: Muscular Dystrophy / Neuromuscular

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 2 counts | No tremor — weakness is the issue, not involuntary movement |
| Dead zone time | 40 ms | Can be shorter — no tremor component, just dome actuation artifact |
| Dead zone distance | 8 counts | Narrow — maximize drag sensitivity for weak users |
| Click debounce | 5 ms | Standard |
| Tremor filter | Off (alpha=1.0) | No tremor to filter; any latency penalizes already-slow movements |
| DPI multiplier | 160 (higher) | Amplify small movements from weakened muscles |
| Sleep timeout | 120 s | Fatigue causes long rest breaks; don't punish them with reconnection |
| Click force | Piezo preferred, threshold minimum | Dome force must be as low as achievable. Piezo with configurable ADC threshold is ideal — can be set to <50 gf equivalent |

#### Profile: C6-C7 Spinal Cord Injury

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Noise threshold | 2 counts | No tremor |
| Dead zone time | 60 ms | Tenodesis grip mechanics cause movement during click |
| Dead zone distance | 15 counts | Wider — whole-hand movement during tenodesis actuation |
| Click debounce | 10 ms | Uneven force application |
| Tremor filter | Off | No tremor |
| DPI multiplier | 160 (higher) | Limited range of motion — amplify small movements |
| Sleep timeout | 120 s | Donning/doffing is effortful; avoid unnecessary reconnection |
| Click force | Piezo, minimum threshold | Tenodesis pinch generates ~1-5 N (C6) to ~5-16 N (C7). Standard dome at 150 gf (1.47 N) is at the floor of C6 capability. Piezo at ~50 gf equivalent required for reliable C6 use |

---

## 4. Ring Shell Geometry Parameters

### 4.1 Ring Diameter and Fit

**Problem:** Fixed ring diameter fails multiple populations.

- Essential tremor patients have normal finger dimensions but may need a
  slightly looser fit to reduce the sensation of constriction (anxiety worsens
  tremor).
- Rheumatoid arthritis patients experience **diurnal finger swelling** — a ring
  that fits in the morning may constrict blood flow by evening (blog.brilliance
  .com, 2024). Swan neck and boutonniere deformities alter DIP and PIP joint
  geometry (NCBI Bookshelf, NBK525970). Actively inflamed joints are
  contraindicated for any rigid ring.
- Edema from SCI, heart failure, or medication causes unpredictable finger size
  changes.
- Muscular dystrophy may cause muscle wasting, reducing finger circumference
  over time.

**Recommendation:** The ring shell must support **adjustable or multi-size
fitting.** Options:

1. **Silicone inner sleeve in multiple sizes** — 3D-printed shell with standard
   bore, silicone inserts in 1mm increments. Cheapest, easiest to prototype.
2. **Hinged or split-band design** — similar to arthritis ring splints (Arthritis
   Foundation, 2024). Opens to slide over an enlarged PIP joint, then closes on
   the smaller distal phalanx.
3. **Elastic retention band** — for users who cannot generate enough grip to
   retain a rigid ring.

### 4.2 Click Force

The snap dome force rating is a hardware selection decision, not a firmware
parameter. However, the choice of dome directly determines who can and cannot use
the device.

| Dome force | Population | Notes |
|------------|-----------|-------|
| 100 gf (0.98 N) | Low-force requirement: MD, SCI C6 | Below standard catalog; may require custom order from Snaptron |
| 150 gf (1.47 N) | Standard low: CP MACS I-II, PD, ET | Lowest standard catalog (Snaptron SQ-series) |
| 200 gf (1.96 N) | Default: able-bodied, mild ET | Good balance of feel and accidental-click prevention |
| 250 gf (2.45 N) | Severe ET, involuntary movement conditions | Higher force reduces false clicks from tremor |

The piezo click mechanism sidesteps this problem entirely — the ADC threshold is
a firmware parameter, adjustable per user via BLE. **Piezo is the
accessibility-optimal click mechanism.** The snap dome is the
reliability-optimal mechanism. For populations with force limitations (MD, SCI),
piezo should be the default.

### 4.3 Sensor Angle

The standard 30-degree sensor angle (R30) assumes a natural resting hand
position. This assumption holds for most populations but fails for:

- **Spastic CP:** Wrist and finger flexor spasticity may hold the finger at a
  steeper angle. R45 may be more appropriate for users with moderate flexor
  spasticity.
- **SCI C6-C7:** Tenodesis grip requires wrist extension, which flattens the
  finger. R15 or R00 may be more natural during tenodesis-assisted pointing.
- **Essential tremor:** No angle-specific concern. R30 is appropriate.

---

## 5. Comparison: PowerFinger vs. Existing Assistive Pointing Devices

| Feature | PowerFinger (ring) | LipSync | FLipMouse | GlassOuse | SmartNav 4 | HeadMouse Extreme |
|---------|-------------------|---------|-----------|-----------|------------|-------------------|
| **Input modality** | Fingertip drag on surface | Mouth joystick + sip/puff | Mouth/finger joystick + sip/puff | Head tilt (gyroscope) + bite switch | Head movement (IR camera) + dwell click | Head movement (IR camera) + dwell/switch |
| **Requires** | Finger dexterity, pinch force | Mouth/lip control, breath control | Mouth or finger control, breath | Head control, jaw control | Head control | Head control |
| **Connection** | BLE HID | USB / BLE | USB | BLE | USB | USB |
| **Latency** | 7.5-15 ms (BLE connection interval) | ~8 ms (USB HID) | ~10 ms (USB) | ~15 ms (BLE) | 10 ms (100 fps camera) | ~10 ms |
| **Tremor filtering** | Firmware (planned Stage 2 EMA filter) | Dead zone on joystick | Configurable per-axis dead zone | Gyroscope smoothing | Software smoothing + speed control | Software smoothing |
| **Click mechanism** | Snap dome / piezo (on-device) | Sip/puff (air pressure) | Sip/puff + external switches | Bite switch | Dwell click (software timer) | Dwell click / external switch |
| **Cost** | ~$9-18 (ring pair, BOM) | ~$175-325 (assembled) | ~$100-150 (kit) | ~$400-500 | ~$500 | ~$1000+ |
| **Open source** | Yes (CERN-OHL-S + MIT) | Yes (GPL-3.0, OSHWA certified) | Yes (MIT + CC) | No | No | No |
| **Hands-free** | No (requires finger) | Yes | Partially (lip mode yes, finger mode no) | Yes | Yes | Yes |
| **Surface required** | Yes (desk, pad, any flat surface) | No | No | No | No | No |
| **Portability** | Worn on finger, ~8-12g | Mounted on desk/wheelchair | Desktop/mounted | Worn like glasses, ~45g | Desk-mounted camera | Desk-mounted camera |
| **Best for** | Users with some finger function who want natural pointing | Quadriplegia, C1-C5 SCI | Severe upper limb restriction | Upper limb restriction with head control | Upper limb restriction with head control | Upper limb restriction with head control |

### 5.1 Key Differentiators

**PowerFinger's advantage:** Natural pointing gesture. Dragging a fingertip is
the most intuitive pointing metaphor short of a touchscreen. Users who retain
finger function but cannot grip or lift a mouse benefit from a device that
requires only lateral drag force, not grip force or wrist movement.

**PowerFinger's limitation:** Requires some finger function. Users with complete
upper limb paralysis (C1-C4 SCI, advanced ALS, MACS V CP) cannot use a ring
mouse. For these users, LipSync, FLipMouse, GlassOuse, or eye tracking is
appropriate.

**PowerFinger's cost advantage:** At $9-18 BOM, PowerFinger is 10-100x cheaper
than commercial assistive pointing devices. This is significant for users in
low-resource settings where $500+ devices are inaccessible.

---

## 6. Contraindications

Per the project's "document honestly" rule, the following populations and
conditions are contraindicated for ring mouse use. This is not a failure of the
device — it is a boundary condition that must be communicated clearly.

### 6.1 Absolute Contraindications

| Condition | Why | Alternative |
|-----------|-----|-------------|
| **Complete upper limb paralysis (C1-C4 SCI)** | No finger movement, no ability to don ring | LipSync, eye tracking |
| **Advanced ALS (ALSFRS-R hand subscale 0-1)** | Insufficient finger strength to move ring on surface | Eye tracking, brain-computer interface |
| **Cerebral palsy MACS IV-V** | Cannot handle objects; involuntary movements overwhelm any filter | Head mouse, eye tracking, switch scanning |
| **Severe dystonia with continuous involuntary finger movement** | Unpredictable, high-amplitude movement; filtering would suppress intentional movement too | Head mouse, eye tracking |
| **Locked-in syndrome** | No voluntary limb movement | Eye tracking, brain-computer interface |
| **Active finger inflammation (RA flare, gout)** | Ring constricts swollen tissue, restricts blood flow, causes pain | Wait for flare resolution; use wand variant |
| **Severe peripheral neuropathy (no finger sensation)** | Cannot feel click feedback or ring position; risk of skin breakdown without awareness | Devices with alternative sensory feedback channel |

### 6.2 Relative Contraindications (Use with Caution)

| Condition | Concern | Mitigation |
|-----------|---------|------------|
| **Advanced Parkinson's disease** | Rigidity prevents donning; freezing of movement disrupts pointing | Caregiver assistance for donning; extended sleep timeout; lower DPI |
| **Significant finger edema** | Ring fit is unstable; risk of vascular compromise | Adjustable fit system; elastic retention; check fit multiple times daily |
| **Raynaud's phenomenon** | Cold-triggered vasospasm in fingers; ring may worsen restricted circulation | Avoid tight fit; use during warm conditions only |
| **Essential tremor (severe, >2cm amplitude)** | Firmware filters insufficient; cursor remains unusable | Pair with SteadyMouse or AMAneo for additional host-side filtering |
| **Dupuytren's contracture** | Finger flexion contracture changes ring geometry and surface contact angle | Custom angle (R45+); may need wand variant if finger cannot straighten enough to contact surface |
| **Hemiparesis (post-stroke)** | Affected hand may have sufficient residual function but poor proprioception | Low DPI, extended dead zones, contralateral hand use if possible |

### 6.3 Transitional Use Cases

For progressive conditions (ALS, DMD, SMA), the ring mouse occupies a window in
the disease trajectory. The companion app should:

1. Track usage metrics (session length, click accuracy, pointing speed) over
   time.
2. Detect declining function (increasing tremor, decreasing session length,
   increasing error rate).
3. Suggest alternative devices when metrics indicate the ring is no longer
   effective.
4. Support a smooth handoff to the PowerFinger wand (requires less finger
   dexterity) and then to third-party head/eye tracking.

---

## 7. Open Questions for Hardware Prototyping

These questions cannot be answered from published literature alone. They require
user testing with physical prototypes.

1. **Minimum dome force that provides acceptable tactile feedback.** Published
   data gives force generation capability; it does not tell us whether a 100 gf
   dome "feels like a click" to users with reduced sensation.

2. **Piezo click threshold calibration procedure.** The ADC threshold must be
   calibrated per-user. What is the calibration UX? Can it be automated
   (press N times, firmware finds the threshold)?

3. **Tremor filter latency tolerance.** Published literature on cursor smoothing
   latency focuses on able-bodied users. What latency is acceptable to a user
   with essential tremor, given that the cursor is already shaky without
   filtering?

4. **Ring retention force on weakened fingers.** How loose can the ring be before
   it falls off during lateral drag? No published data exists for this specific
   question.

5. **Surface drag force required for optical tracking.** The user must press
   hard enough to maintain sensor focal distance but soft enough to avoid
   fatigue. What is this force range on different surfaces, and how does it
   compare to the force budget of target populations?

6. **Two-ring coordination with motor impairment.** The default setup uses two
   rings (nav + click). Does splitting click to a separate finger help or hurt
   users with impaired bimanual/bilateral coordination?

---

## 8. References

### Motor Impairment Profiles

- Smaby N, Johanson ME, Baker B, Kenney DE, Murray WM, Hentz VR. (2004)
  Identification of key pinch forces required to complete functional tasks.
  *Journal of Rehabilitation Research & Development*, 41(2), 215-224.

- Mateo S, Revol P, Fourtassi M, Rossetti Y, Collet C, Rode G. (2013) Kinematic
  characteristics of tenodesis grasp in C6 quadriplegia. *Spinal Cord*, 51,
  144-149.

- Wangdell J, Friden J. (2012) Hand function of C6 and C7 tetraplegics 1-16
  years following injury. *Journal of Hand Surgery*, 37(5), 961-967.

- Eliasson AC, Krumlinde-Sundholm L, Rosblad B, et al. (2006) The Manual
  Ability Classification System (MACS) for children with cerebral palsy.
  *Developmental Medicine & Child Neurology*, 48(7), 549-554.

- Smits-Engelsman B, Rameckers E, Duysens J. (2023) Measuring grip strength in
  adolescents and adults with cerebral palsy in a clinic setting.
  *Developmental Medicine & Child Neurology*, 66(4), 475-482.

- Seferian AM, Berry A, Estournet B, et al. (2015) Relationship between hand
  strength and function in Duchenne muscular dystrophy. *Neuromuscular
  Disorders*, 25(Suppl 2), S267.

- Decostre V. (2024) Normal strength is not essential for a functional hand.
  *Institut de Myologie Research Report*.

- Hobson EV, McDermott CJ. (2016) Supportive and symptomatic management of
  amyotrophic lateral sclerosis. *Nature Reviews Neurology*, 12, 526-538.

### Tremor Characteristics

- Elble RJ. (2009) Tremor: clinical features, pathophysiology, and treatment.
  *Neurologic Clinics*, 27(3), 679-695.

- Deuschl G, Bain P, Brin M. (1998) Consensus statement of the Movement
  Disorder Society on tremor. *Movement Disorders*, 13(S3), 2-23.

- Jankovic J. (2008) Parkinson's disease: clinical features and diagnosis.
  *Journal of Neurology, Neurosurgery & Psychiatry*, 79(4), 368-376.

- Zhang D, et al. (2017) Differential Diagnosis of Parkinson Disease, Essential
  Tremor, and Enhanced Physiological Tremor with the Tremor Analysis of EMG.
  *Parkinson's Disease*, Article 1597907.

- Haubenberger D, et al. (2011) Role of individual tremor features in the
  clinical differentiation of essential tremor. *Tremor and Other Hyperkinetic
  Movements*, 1, tre-01-48.

### Tremor Filtering Algorithms

- Riviere CN, Rader RS, Thakor NV. (1998) Adaptive canceling of physiological
  tremor for improved precision in microsurgery. *IEEE Transactions on
  Biomedical Engineering*, 45(7), 839-846.

- Veluvolu KC, Ang WT. (2011) Estimation of physiological tremor from
  accelerometers for real-time applications. *Sensors*, 11(3), 3020-3036.

- Gonzalez JG, Heredia EA, et al. (2000) Optimal digital filtering for tremor
  suppression. *IEEE Transactions on Biomedical Engineering*, 47(5), 664-668.

- Bani Hashem SK, Naghsh Nilchi AR. (2013) Adaptive path smoothing technique via
  B-spline for tremor suppression in Parkinson's disease patients. *Computers
  in Biology and Medicine*, 43(3), 264-270.

### Assistive Pointing Devices

- Makers Making Change. (2024) LipSync v4.1. Open Assistive Technology.
  https://github.com/makersmakingchange/LipSync

- AsTeRICS Foundation. (2023) FLipMouse: Flexible Alternative Input Solution.
  https://github.com/asterics/FLipMouse

- GlassOuse. (2024) GlassOuse Pro wireless head mouse.
  https://glassouse.com/

- NaturalPoint / TrackIR. SmartNav 4 AT head tracker.

### Snap Dome Engineering

- Snaptron Inc. (2024) Design Guide: Engineering Handbook for Metal Dome
  Switches. https://www.snaptron.com/

- Metal-domes.com. (2025) How to choose metal dome strength (gf)?
  Wearable electronics typical range: 150-220 gf.

### Joint Deformity and Ring Fit

- NCBI Bookshelf. Swan-Neck Deformity. StatPearls, NBK525970.

- Arthritis Foundation. (2024) Using Ring Splints to Support Finger Joints.
