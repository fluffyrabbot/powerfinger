# Surface Compatibility Test Protocol

A complete, reproducible test protocol for evaluating PowerFinger tracking
performance across all target surfaces. Designed to be executable on day 1 of
hardware arrival by someone who has never tested a mouse sensor before.

References:
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Design matrix: [COMBINATORICS.md](COMBINATORICS.md)
- Glide system: [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)

---

## 1. Equipment List

### Required

| Item | Purpose | Approximate Cost | Source |
|------|---------|-----------------|--------|
| Steel ruler, 300mm | Linear guide for controlled-speed drag | $5 | Hardware store |
| Digital calipers (150mm, 0.01mm resolution) | Measure physical displacement | $15 | Amazon, hardware store |
| Smartphone with 240fps+ slow-motion video | Latency measurement | Already owned | iPhone 12+ or Galaxy S21+ |
| Tripod or phone clamp | Steady camera for latency and jitter recording | $10 | Amazon |
| Kitchen scale (0.1g resolution) | Calibrate finger pressure weights | $10 | Amazon |
| Small weight set: 50g, 100g, 200g | Simulate controlled finger pressure (0.5N, 1N, 2N) | $8 | Fishing weights or lab weights |
| USB mouse jiggle detection software | Log raw HID reports with timestamps | Free | `evtest` (Linux), MouseTester (Windows) |
| Laptop/desktop with BLE and screen recording | Host for BLE HID connection and data logging | Already owned | Any OS |
| Masking tape | Mark start/end positions on surfaces | $3 | Hardware store |
| Fine-tip marker | Mark reference lines on tape (not on test surfaces) | $2 | Office supply |
| Rubber band or silicone strap (optional) | Secure weight to ring during pressure-controlled tests | $1 | Office supply |

### Test Surfaces (acquire before hardware arrives)

| # | Surface | Specimen | Where to Get |
|---|---------|----------|-------------|
| S01 | Wood desk (unfinished) | 300x300mm plywood or hardwood scrap | Hardware store |
| S02 | Wood desk (lacquered) | 300x300mm varnished wood panel or actual desk surface | Desk or hardware store |
| S03 | Glass | 300x300mm clear glass pane, 3-4mm thick | Hardware store, picture frame glass |
| S04 | Fabric (cotton) | 300x300mm cotton cloth, flat weave (e.g., plain T-shirt material) | Fabric store or old shirt |
| S05 | Paper (printer paper) | Standard A4/Letter, taped flat to a hard surface | Office supply |
| S06 | Glossy magazine | Magazine cover or glossy photo paper, taped flat | Newsstand |
| S07 | Matte plastic | 300x300mm ABS or HDPE sheet, matte finish | Plastics supplier, cutting board |
| S08 | Wheelchair tray (vinyl-coated) | Vinyl-coated plywood or actual wheelchair tray surface sample | Medical supply, or vinyl wrap on plywood |
| S09 | Wheelchair tray (powder-coated metal) | Powder-coated aluminum or steel panel | Metal supplier, or actual tray sample |
| S10 | Hospital bed rail (chrome) | Chrome-plated steel tube or flat chrome sample | Plumbing supply, or actual rail segment |
| S11 | Hospital bed rail (painted steel) | Painted steel tube or flat painted steel sample | Hardware store (painted shelf bracket) |
| S12 | Skin (forearm) | Tester's own forearm, positioned flat on table | N/A |
| S13 | Skin (thigh) | Tester's own thigh, seated position | N/A |
| S14 | Clothing (cotton, worn) | Cotton pants or sleeve, on the body in normal position | N/A |
| S15 | Clothing (denim, worn) | Jeans, on the body in normal position | N/A |
| S16 | Clothing (fleece, worn) | Fleece jacket sleeve or lap blanket, on the body | N/A |

**Accessibility note:** Surfaces S08-S16 are the actual use environment for
target users. A ring that scores perfectly on S01-S07 but fails on S08-S16 has
failed its purpose. Prioritize these surfaces in reporting.

---

## 2. Software Setup

### HID Report Logger

The logger must capture raw BLE HID mouse reports (X delta, Y delta, button
state) with host-side timestamps at report-level granularity.

**Linux (recommended):**
```bash
# Find the ring's event device after pairing
sudo evtest /dev/input/eventN
# Or for structured logging:
sudo evemu-record /dev/input/eventN > test_log.txt
```

**Windows:**
- MouseTester by microe (GitHub). Run it, pair the ring via BLE, start recording.
  Export as CSV.

**macOS:**
- IOHIDEventSystemClient sample code, or use Karabiner-EventViewer for quick
  visual verification then switch to a Linux machine for structured logging.

### Screen Recording for Latency

Use the smartphone at 240fps (or higher). Frame the screen and the ring in the
same shot. The phone must see both the physical ring movement and the on-screen
cursor movement simultaneously.

---

## 3. Test Fixture Setup

### 3.1 Linear Guide (Controlled-Speed Drag)

```
TOP VIEW — Linear drag fixture

  ┌─────────── 300mm ruler (steel) ──────────┐
  │                                            │
  ══════════════════════════════════════════════
  ↑ START mark                    STOP mark ↑
  (masking tape)                 (masking tape)
  │←────────── 100mm ───────────→│
```

1. Place a strip of masking tape at the 0mm and 100mm marks on the ruler.
2. Draw a fine line across each tape strip as a visual start/stop cue.
3. Tape the ruler firmly to the test surface so it does not slide.
4. The ring slides along the ruler edge. The ruler acts as a guide rail, not a
   drag surface — the ring's glide pads (or ball) contact the test surface
   beneath it, and the ruler provides lateral constraint so the path is straight.

**For ball-sensor variants (no glide pads):** The ball contacts the test surface
directly. The ruler edge guides the ring shell.

**For skin/clothing tests (S12-S16):** The ruler cannot be taped to skin. Instead,
draw two dots 100mm apart on masking tape applied to the skin/clothing. Move the
ring between the dots freehand, using the tape marks as start/stop references.
Accept that these tests will have higher variance; run 5 trials instead of 3.

### 3.2 Controlled Pressure

Normal finger pressure during ring use is 0.5-2N (50-200g force). The ring's own
weight (~8-15g) is far below this range. During testing, simulate finger pressure
using dead weights:

| Pressure Level | Weight on Ring | Force | Represents |
|----------------|---------------|-------|------------|
| Light | 50g | ~0.5N | Very light touch, minimal contact |
| Normal | 100g | ~1.0N | Typical relaxed finger resting on surface |
| Firm | 200g | ~2.0N | Deliberate press, e.g., during a drag operation |

**How to apply weight:** Place the weight directly on top of the ring body,
centered over the sensor. For small weights, use a coin stack or fishing weights
in a small bag. Secure with a rubber band or silicone strap if the weight tends
to slide off during drag tests. The weight must not block the sensor aperture or
interfere with the glide pads / ball.

**For skin/clothing tests:** Use finger pressure only (no dead weights on the
body). Record subjective pressure as "light / normal / firm" based on the
tester's self-assessment. These tests assess whether the sensor tracks on the
surface at all, not precision at controlled pressure.

### 3.3 Controlled Angle

All ring tests default to 30 degrees (matching the R-30 shell design). The ring
shell geometry enforces the angle — when the glide pads sit flat on the surface,
the sensor is at 30 degrees. No external angle fixture is needed for ring tests.

For wand tests, test at three angles from horizontal: 30, 50, and 70 degrees.
Use a protractor or angle block to verify. For ball+Hall PowerPen bring-up, run
the tracking-accuracy drag both horizontally and vertically at each angle and
record the per-axis sensitivity ratio
`max(horizontal counts_total, vertical counts_total) / min(...)` in
`POWERPEN-SPEC.md`. Ratios above 3:1 flag a follow-up for dynamic gain
compensation; they are not an automatic test failure.

---

## 4. Test Procedures

Run all five tests on every surface. For each surface, start fresh: re-pair BLE
if needed, verify cursor is responding, then proceed through tests in order.

### 4.1 Tracking Accuracy (Linearity)

**What this measures:** When the ring moves 100mm on the surface, how far does
the cursor actually move? And is the relationship linear (consistent across the
stroke)?

**Procedure:**

1. Open a drawing program (MS Paint, GIMP, or any canvas app). Set brush to 1px.
2. Start the HID logger.
3. Position the ring at the START mark.
4. Draw a straight horizontal line on screen by dragging the ring from START to
   STOP (100mm) along the ruler guide at approximately 50mm/s (~2 seconds for
   the full stroke). Do not rush.
5. Stop the HID logger.
6. Measure the on-screen line length in pixels. Record as `px_total`.
7. From the HID log, sum all X-delta values during the stroke. This is the raw
   sensor output in counts. Record as `counts_total`.
8. Divide the stroke into 5 equal segments (20mm each) by examining the HID log
   timestamps. For each segment, compute counts for that segment. Record as
   `counts_seg[1..5]`.
9. Repeat 3 times (5 times for skin/clothing surfaces). Record all trials.

**Derived metrics:**

| Metric | Formula | Units |
|--------|---------|-------|
| Resolution | `counts_total / 100` | counts/mm |
| On-screen ratio | `px_total / 100` | px/mm |
| Linearity error | `max(counts_seg) - min(counts_seg)) / mean(counts_seg) * 100` | % deviation |

**Notes:**
- The px/mm ratio depends on the host OS sensitivity setting. Record the OS
  pointer speed setting and keep it constant across all tests.
- Counts/mm is the sensor-intrinsic metric and is more meaningful for
  cross-surface comparison. Use it as the primary metric.
- For ball+Hall PowerPen bring-up, repeat the stroke in both X and Y directions
  at each of the three test angles and compute the per-axis sensitivity ratio
  from the two `counts_total` values. Record that ratio alongside the surface
  results in `POWERPEN-SPEC.md`.

### 4.2 Jitter at Rest

**What this measures:** When the ring is sitting still on the surface, does the
cursor drift? This matters for accessibility users who may leave the ring resting
on a surface while doing other tasks.

**Procedure:**

1. Place the ring on the test surface with 100g weight (normal pressure).
2. Start the HID logger.
3. Do not touch the ring or the surface. Do not walk near the table (vibration).
4. Wait 60 seconds.
5. Stop the HID logger.
6. From the log, extract all X and Y deltas reported during the 60 seconds.

**Derived metrics:**

| Metric | Formula | Units |
|--------|---------|-------|
| Total displacement | `sqrt(sum(dx)^2 + sum(dy)^2)` | counts |
| RMS jitter | `sqrt(mean(dx^2 + dy^2))` per report | counts/report |
| Max single-report jump | `max(sqrt(dx_i^2 + dy_i^2))` | counts |
| Cumulative pixel drift | Measure cursor displacement on screen after 60s | px |

**Notes:**
- For skin/clothing, the tester must remain as still as possible. Breathing and
  micro-movements will produce some motion. Record whether the tester was seated
  (thigh tests) or had arm resting on table (forearm tests). Accept that
  biological surfaces have inherently higher jitter.
- If the sensor enters a low-power sleep mode during the 60 seconds (expected
  for PAW3204 after ~256ms of no motion), note the transition point.

### 4.3 Dropout Rate

**What this measures:** During a continuous drag, how often does the sensor lose
tracking and report zero movement? Dropouts cause cursor stutter and are
especially disorienting for users with motor control challenges.

**Procedure:**

1. Start the HID logger.
2. Drag the ring along the ruler guide at a steady ~50mm/s for 100mm.
3. Without lifting, drag back to START at the same speed.
4. Stop the HID logger.
5. From the log, count total HID reports during the drag (N_total) and reports
   where both dx=0 and dy=0 (N_zero).
6. Repeat 3 times (5 times for skin/clothing). Record all trials.

**Derived metrics:**

| Metric | Formula | Units |
|--------|---------|-------|
| Dropout rate | `N_zero / N_total * 100` | % |
| Max consecutive dropouts | Longest run of zero-delta reports | reports |
| Dropout duration | `Max consecutive dropouts * report_interval_ms` | ms |

**Notes:**
- A report with dx=0 and dy=0 during movement is a dropout. But at very low
  speeds, some zero reports are expected because sub-count movements round to
  zero. To distinguish: zero reports at 50mm/s with a sensor rated for >400
  counts/inch are true dropouts.
- On glass (S03), expect high dropout rates from optical sensors. This is a
  known limitation. The purpose of testing glass is to quantify how bad it is,
  not to expect it to pass.

### 4.4 Latency (End-to-End)

**What this measures:** The total delay from when the ring physically moves to
when the cursor visibly moves on screen. This includes sensor capture time, MCU
processing, BLE transmission, host processing, and display pipeline.

**Procedure:**

1. Set up the smartphone in slow-motion mode (240fps minimum). Mount on tripod.
2. Frame the shot so both the ring on the surface AND a portion of the monitor
   showing the cursor are visible simultaneously.
3. Place the ring at the START mark with 100g weight.
4. Start video recording.
5. Make a quick, sharp movement of the ring (~30mm in ~0.2 seconds). The
   movement must be abrupt enough that the exact start frame is unambiguous in
   slow-motion footage.
6. Stop video recording.
7. Review the slow-motion footage frame by frame:
   - **Frame A:** The first frame where the ring visibly begins to move.
   - **Frame B:** The first frame where the cursor visibly begins to move.
   - **Latency = (B - A) / fps** in seconds. Convert to ms.
8. Repeat 10 times. Discard the highest and lowest. Average the remaining 8.

**Notes:**
- The monitor's own refresh rate and pixel response time add to measured latency.
  Record the monitor model and its rated response time. Use a 144Hz+ monitor if
  available; a 60Hz monitor adds up to 16.7ms of display latency.
- BLE connection interval is a major contributor. Record the active connection
  interval (should be 15ms default, 7.5ms during active tracking if adaptive
  interval is implemented).
- This test does not isolate sensor latency from BLE latency. It measures what
  the user actually experiences, which is the relevant metric.

### 4.5 Click Stability

**What this measures:** When the user clicks (presses the dome switch), does the
ring move and produce an unintended cursor displacement? For accessibility users,
stable clicks are critical — an unintended 5px jump during a click can miss a
small UI target.

**Procedure:**

1. Open a click-test webpage or program that displays cursor position at each
   click event (e.g., https://clickspeedtest.com or a simple HTML page that logs
   `event.clientX, event.clientY` on mousedown).
2. Place the ring on the test surface with 100g weight (normal pressure).
3. Position the cursor at a known screen location.
4. Click the dome switch 50 times. Pause ~1 second between clicks. Try to press
   straight down without lateral force.
5. For each click, record the cursor position at the mousedown event.
6. Compute displacement from the initial position for each click.

**Derived metrics:**

| Metric | Formula | Units |
|--------|---------|-------|
| Max click displacement | `max(sqrt((x_i - x_0)^2 + (y_i - y_0)^2))` | px |
| Mean click displacement | `mean(sqrt((x_i - x_0)^2 + (y_i - y_0)^2))` | px |
| Cumulative drift | `sqrt((x_50 - x_0)^2 + (y_50 - y_0)^2)` | px |
| Failure count | Clicks where displacement > dead zone threshold | count out of 50 |

**Notes:**
- The "dead zone threshold" is the firmware's configured dead zone radius. If
  the firmware suppresses movement below N counts during click events, use that
  value as the threshold. If no click dead zone is implemented, use 3px as the
  threshold for this test and flag the firmware for dead zone implementation.
- On soft surfaces (fabric, fleece, skin), the surface itself deforms under click
  force, which may cause sensor displacement. Record whether displacement
  correlates with surface deformation.

---

## 5. Pass/Fail Criteria

### 5.1 Criteria Table

| Metric | Pass | Marginal | Fail |
|--------|------|----------|------|
| **Linearity error** | < 10% | 10-20% | > 20% |
| **Jitter RMS** | < 1.0 counts/report | 1.0-3.0 counts/report | > 3.0 counts/report |
| **Cumulative jitter (60s)** | < 5px drift | 5-15px drift | > 15px drift |
| **Dropout rate** | < 2% | 2-10% | > 10% |
| **Max consecutive dropouts** | < 3 reports | 3-10 reports | > 10 reports |
| **End-to-end latency** | < 20ms | 20-40ms | > 40ms |
| **Click displacement (mean)** | < 2px | 2-5px | > 5px |
| **Click failure count** | 0 out of 50 | 1-3 out of 50 | > 3 out of 50 |

**Interpretation:**
- **Pass on all metrics:** Surface is fully supported. Advertise compatibility.
- **Marginal on any metric:** Surface works but with caveats. Document the
  limitation honestly.
- **Fail on any metric:** Surface is not supported by this sensor variant.
  Document it. If the surface is accessibility-critical (S08-S16), escalate:
  either fix the issue or recommend a different sensor variant for that use case.

### 5.2 Relaxed Criteria for Skin/Clothing (S12-S16)

Skin and clothing are inherently mobile, deformable surfaces. Apply these
adjusted thresholds:

| Metric | Pass | Marginal | Fail |
|--------|------|----------|------|
| **Linearity error** | < 20% | 20-35% | > 35% |
| **Jitter RMS** | < 3.0 counts/report | 3.0-6.0 counts/report | > 6.0 counts/report |
| **Dropout rate** | < 5% | 5-15% | > 15% |
| **Latency** | Same as rigid surfaces | Same | Same |
| **Click displacement** | < 5px | 5-10px | > 10px |

The rationale: a user operating on their own thigh or forearm accepts lower
precision in exchange for the ability to use the device at all without a desk
surface. The criteria here define "usable" rather than "precise."

---

## 6. Expected Results by Sensor Type

These are pre-hardware predictions based on sensor datasheets and known physics.
After testing, fill in the "Measured" column and compare.

### 6.1 PAW3204 (Optical LED — Standard Ring)

The PAW3204 uses an LED illuminator and optical flow on surface texture. It
requires a surface with visible texture at the sensor's imaging resolution and a
consistent focal distance of 2.4-3.2mm maintained by the glide system.

| Surface | Tracking | Linearity | Jitter | Dropouts | Notes |
|---------|----------|-----------|--------|----------|-------|
| S01 Wood (unfinished) | Good | < 5% | Low | < 1% | Strong grain texture, ideal for optical |
| S02 Wood (lacquered) | Good | < 8% | Low | < 2% | Finish may reduce contrast slightly |
| S03 Glass | Non-functional | N/A | N/A | > 90% | Transparent + featureless = no optical tracking. Expected fail. |
| S04 Fabric (cotton) | Marginal | 10-20% | Medium | 5-15% | Texture exists but focal distance varies with weave deformation |
| S05 Paper | Good | < 5% | Low | < 1% | Fiber texture is ideal for optical sensors |
| S06 Glossy magazine | Poor-Marginal | 15-25% | Medium | 10-30% | Specular reflection confuses optical flow |
| S07 Matte plastic | Good | < 8% | Low | < 3% | Depends on surface micro-texture; some plastics are too smooth |
| S08 Wheelchair tray (vinyl) | Good-Marginal | 5-15% | Low-Med | 2-10% | Vinyl grain provides texture; depends on specific vinyl |
| S09 Wheelchair tray (powder-coat) | Good | < 8% | Low | < 3% | Powder coat has good micro-texture |
| S10 Bed rail (chrome) | Poor | 20-30% | High | 20-50% | Highly reflective, low texture contrast |
| S11 Bed rail (painted steel) | Good-Marginal | 8-15% | Low-Med | 2-8% | Paint texture helps; depends on paint type |
| S12 Skin (forearm) | Non-functional | N/A | N/A | > 80% | Focal distance impossible to maintain; skin deforms under ring |
| S13 Skin (thigh) | Non-functional | N/A | N/A | > 80% | Same as forearm |
| S14 Clothing (cotton) | Poor-Marginal | 15-30% | High | 10-30% | Fabric moves and deforms, focal distance unstable |
| S15 Clothing (denim) | Marginal | 10-20% | Medium | 5-15% | Stiffer fabric, better focal distance stability |
| S16 Clothing (fleece) | Non-functional | N/A | N/A | > 50% | Pile fabric, no stable focal plane |

**Summary:** PAW3204 works well on rigid, textured, opaque surfaces (wood, paper,
matte plastic, powder-coated metal). Fails on glass, skin, and soft fabrics.
Marginal on some accessibility surfaces. This is the $9 baseline — its
limitations are understood and acceptable if paired with the ball+Hall variant
for surface-agnostic use.

### 6.2 Ball+Hall (Steel Ball + Hall Effect Sensors — Surface-Agnostic Ring)

The ball+Hall sensor uses a 5mm steel ball in a socket with 4 Hall-effect sensors
detecting ball rotation via embedded magnets on roller spindles. Tracking depends
on the ball making frictional contact with the surface and rotating. It is
independent of surface opacity, reflectivity, or texture.

| Surface | Tracking | Linearity | Jitter | Dropouts | Notes |
|---------|----------|-----------|--------|----------|-------|
| S01 Wood (unfinished) | Good | < 10% | Low | < 2% | Good friction, ball rolls cleanly |
| S02 Wood (lacquered) | Good | < 10% | Low | < 2% | Smooth but sufficient friction |
| S03 Glass | Good | < 10% | Low | < 3% | Ball-on-glass friction is adequate; steel on glass is not frictionless |
| S04 Fabric (cotton) | Marginal | 15-25% | Medium | 5-15% | Fabric may bunch or drag rather than letting ball roll cleanly |
| S05 Paper | Good | < 8% | Low | < 2% | Good friction surface for steel ball |
| S06 Glossy magazine | Good | < 10% | Low | < 3% | Ball rolls on any rigid surface regardless of gloss |
| S07 Matte plastic | Good | < 10% | Low | < 2% | Good friction |
| S08 Wheelchair tray (vinyl) | Good | < 10% | Low | < 3% | Vinyl provides good ball friction |
| S09 Wheelchair tray (powder-coat) | Good | < 8% | Low | < 2% | Slightly textured, excellent for ball |
| S10 Bed rail (chrome) | Good-Marginal | 10-15% | Low-Med | 3-8% | Chrome is slippery; ball may slip at low pressure |
| S11 Bed rail (painted steel) | Good | < 10% | Low | < 3% | Paint adds friction |
| S12 Skin (forearm) | Good-Marginal | 15-25% | Medium | 5-10% | Skin has high friction but deforms; ball may skip |
| S13 Skin (thigh) | Marginal | 15-25% | Medium | 5-15% | More deformation than forearm, more skipping expected |
| S14 Clothing (cotton) | Marginal | 15-25% | Medium | 5-15% | Fabric movement absorbs ball rotation |
| S15 Clothing (denim) | Good-Marginal | 10-20% | Medium | 3-10% | Stiff fabric, decent ball traction |
| S16 Clothing (fleece) | Poor | 20-35% | High | 10-25% | Ball sinks into pile, inconsistent contact |

**Summary:** Ball+Hall works on nearly everything, including glass, skin, and
most accessibility surfaces. Its weakness is soft, deformable surfaces where the
ball sinks in or the surface absorbs rotation rather than transmitting it. It
trades precision for universality — linearity on rigid surfaces may not match the
optical sensor, but it never fully fails.

### 6.3 PMW3360 (Laser Optical — Pro Ring / Future Variant)

The PMW3360 uses a VCSEL laser illuminator with much higher resolution
(12,000 CPI max) and a more sophisticated optical flow engine. Expected behavior
is similar to PAW3204 but better on marginal surfaces due to higher illumination
power and imaging resolution. Still fails on transparent/featureless surfaces.

| Surface | vs. PAW3204 | Notes |
|---------|-------------|-------|
| S01-S02 Wood | Similar or better | Higher resolution may improve linearity |
| S03 Glass | Still fails | Laser helps slightly on frosted glass, not on clear glass |
| S04 Fabric | Better | Higher illumination may track weave pattern more reliably |
| S05-S07 Paper/magazine/plastic | Better | Fewer dropouts on glossy surfaces |
| S08-S09 Wheelchair tray | Better | Higher illumination power benefits low-contrast surfaces |
| S10 Chrome rail | Marginal (improved) | Laser tracks specular surfaces slightly better than LED |
| S11 Painted rail | Similar or better | Already adequate with PAW3204 |
| S12-S13 Skin | Still fails | Focal distance problem is mechanical, not sensor-related |
| S14-S16 Clothing | Slightly better | Same mechanical limitations, slightly better image processing |

**Summary:** PMW3360 expands the marginal zone into passing on several surfaces
but does not solve the fundamental limitations of optical sensing: transparent
surfaces and surfaces where focal distance cannot be maintained. The $18-20 BOM
Pro variant using PMW3360 makes sense for desk/tray users who want maximum
precision but does not replace ball+Hall for universal surface agnosticism.

---

## 7. Data Recording Templates

### 7.1 Test Session Header

Fill in once per test session.

```
Test date:          _______________
Tester name:        _______________
Ring variant:       [ ] R30-OLED (PAW3204 optical)
                    [ ] R30-BALL (ball+Hall)
                    [ ] WSTD-BALL (wand, ball+Hall)
                    [ ] Other: _______________
Firmware version:   _______________
BLE conn interval:  _____ ms
Host OS:            _______________
Pointer speed:      _______________ (OS setting, exact value or slider position)
Monitor:            _______________ (model, refresh rate, resolution)
HID logger:         _______________ (software name, version)
Ambient temp:       _____ C (relevant for skin moisture/friction)
Notes:              _______________
```

### 7.2 Per-Surface Tracking Accuracy

One table per surface. Copy and fill in.

```
Surface: S__ - _______________
Pressure: ___g (___N)

| Trial | Physical dist (mm) | counts_total | px_total | counts/mm | px/mm | Seg1 | Seg2 | Seg3 | Seg4 | Seg5 | Linearity error (%) |
|-------|-------------------|-------------|---------|-----------|-------|------|------|------|------|------|-------------------|
| 1     | 100               |             |         |           |       |      |      |      |      |      |                   |
| 2     | 100               |             |         |           |       |      |      |      |      |      |                   |
| 3     | 100               |             |         |           |       |      |      |      |      |      |                   |
| Mean  |                   |             |         |           |       |      |      |      |      |      |                   |

Result: [ ] PASS  [ ] MARGINAL  [ ] FAIL
Notes:
```

### 7.3 Per-Surface Jitter at Rest

```
Surface: S__ - _______________
Pressure: 100g (1.0N)
Duration: 60 seconds

| Metric                  | Value  |
|------------------------|--------|
| Total reports           |        |
| Reports with nonzero dX or dY |  |
| Sum(dX)                |        |
| Sum(dY)                |        |
| Total displacement (counts) |   |
| Cumulative pixel drift  |   px  |
| RMS jitter (counts/report) |    |
| Max single-report jump  |       |
| Sensor sleep observed?  | Y / N (at what timestamp?) |

Result: [ ] PASS  [ ] MARGINAL  [ ] FAIL
Notes:
```

### 7.4 Per-Surface Dropout Rate

```
Surface: S__ - _______________
Pressure: ___g (___N)
Speed: ~50mm/s

| Trial | N_total | N_zero | Dropout % | Max consecutive zeros | Max dropout duration (ms) |
|-------|---------|--------|-----------|----------------------|--------------------------|
| 1     |         |        |           |                      |                          |
| 2     |         |        |           |                      |                          |
| 3     |         |        |           |                      |                          |
| Mean  |         |        |           |                      |                          |

Result: [ ] PASS  [ ] MARGINAL  [ ] FAIL
Notes:
```

### 7.5 Per-Surface Latency

```
Surface: S__ - _______________
Camera fps: ___
Monitor refresh rate: ___ Hz

| Trial | Frame A | Frame B | Frame delta | Latency (ms) |
|-------|---------|---------|-------------|-------------|
| 1     |         |         |             |             |
| 2     |         |         |             |             |
| 3     |         |         |             |             |
| 4     |         |         |             |             |
| 5     |         |         |             |             |
| 6     |         |         |             |             |
| 7     |         |         |             |             |
| 8     |         |         |             |             |
| 9     |         |         |             |             |
| 10    |         |         |             |             |
| After discarding highest and lowest: |  |  |       |             |
| Mean of 8 |     |         |             |             |

Result: [ ] PASS  [ ] MARGINAL  [ ] FAIL
Notes:
```

### 7.6 Per-Surface Click Stability

```
Surface: S__ - _______________
Pressure: 100g (1.0N)
Dead zone threshold: ___ px (firmware setting, or 3px default)

| Click # | X position | Y position | Displacement from initial (px) |
|---------|-----------|-----------|-------------------------------|
| 0 (initial) |       |           | 0                             |
| 1       |           |           |                               |
| 2       |           |           |                               |
| ...     |           |           |                               |
| 50      |           |           |                               |

| Summary Metric         | Value  |
|------------------------|--------|
| Mean displacement       |   px  |
| Max displacement        |   px  |
| Cumulative drift (click 50 vs 0) | px |
| Failures (> threshold)  |  / 50 |

Result: [ ] PASS  [ ] MARGINAL  [ ] FAIL
Notes:
```

---

## 8. Test Execution Order

The full matrix is 16 surfaces x 5 tests x (up to 3 sensor variants) = up to
240 test runs. Prioritize as follows.

### Phase 1: Smoke Test (First Hour)

Run tracking accuracy only, on 3 surfaces, with the first ring that boots:

1. S05 Paper (expected best-case for optical)
2. S03 Glass (expected worst-case for optical; best-case differentiator for ball)
3. S07 Matte plastic (common desk surface)

**Purpose:** Confirm the sensor is working at all. If tracking accuracy fails on
paper, there is a hardware or firmware problem — stop and debug before running
the full matrix.

### Phase 2: Desk Surfaces (Full Protocol)

Run all 5 tests on S01-S07 with all available sensor variants.

### Phase 3: Accessibility Surfaces (Full Protocol)

Run all 5 tests on S08-S16 with all available sensor variants. This is the
highest-value phase for the project's mission.

### Phase 4: Cross-Pressure Sweep

For the 3 surfaces where the sensor performed best and the 3 where it performed
worst, repeat tracking accuracy and dropout rate at all 3 pressure levels (50g,
100g, 200g). This reveals whether marginal surfaces become usable at higher
pressure or whether passing surfaces degrade at light pressure.

---

## 9. Reporting

After completing the test matrix, produce:

1. **Surface compatibility table** — one row per surface, one column per sensor
   variant, cells contain PASS / MARGINAL / FAIL with the primary limiting metric.
   This table goes into `docs/COMBINATORICS.md` and `README.md`.

2. **Detailed results file** — all raw data tables from Section 7, stored as
   `docs/test-results/surface-YYYY-MM-DD-[sensor].md`.

3. **Surprise findings** — any surface that performed significantly better or
   worse than the predictions in Section 6. These inform firmware tuning (dead
   zones, polling rates, sleep thresholds) and hardware revision decisions (glide
   pad material, focal distance, ball diameter).

4. **Accessibility surface summary** — a dedicated section highlighting S08-S16
   results with recommendations for which sensor variant to use in each
   accessibility scenario. This is the most important output of the entire test
   protocol.
