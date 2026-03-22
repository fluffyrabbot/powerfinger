# PowerFinger Design Combinatorics

Every PowerFinger variant is a combination of:
1. **Form factor** (ring at various angles, or wand)
2. **Sensing mechanism** (how movement is tracked)
3. **Optional: OCR camera** (for text scanning)
4. **Optional: guidance laser** (for aim feedback)

This document enumerates all viable combinations, their properties, and their
BOM estimates.

---

## Axes of Variation

### Form Factors

#### Fingertip Ring

The ring sits on the distal phalanx (fingertip pad) of the middle finger. The
sensor angle determines how the ring sits when the finger contacts a surface.

When a relaxed hand rests on a desk, the middle finger is typically 25–35
degrees from horizontal. A ring angled to match means the sensor window is
flush with the desk surface in the natural resting position — no need to
flatten the finger unnaturally.

| Variant | Sensor Angle | Optimized For | Notes |
|---------|-------------|---------------|-------|
| **R-00** | 0° (flat) | Finger pressed flat on surface | Simplest geometry. Requires conscious finger flattening. Best for trackpad-like use. |
| **R-15** | 15° | Slight curl, very relaxed hand | Works well for large-surface desk dragging |
| **R-30** | 30° | **Natural resting hand position** | Sweet spot for most users. Sensor flush with desk when hand is relaxed. Recommended default. |
| **R-45** | 45° | More curled finger, typing-adjacent posture | Good for keyboard users who want mouse control without changing hand shape. Higher off surface when flat. |

```
SIDE VIEW — Ring angle variants on a desk surface

R-00 (0°):          R-15 (15°):         R-30 (30°):         R-45 (45°):

  ══╗                 ══╗                  ══╗                 ══╗
    ║ finger            ║                    ║                   ║
    ║ flat              ║ slight             ║ natural           ║ curled
    ║                    ╲ curl               ╲ rest              ╲
  [sensor]═══desk    [sensor]═══desk      [sensor]═══desk     [sensor]══desk
  flush              ~flush               flush at 30°        angled up
```

The R-30 variant is the recommended starting point for prototyping. It matches
the natural hand position without requiring the user to consciously adjust their
finger posture.

**Manufacturing note:** The angle is determined by the 3D-printed shell geometry
and sensor mount orientation, not by the sensor itself. All angles use identical
electronics. A parametric CAD model with angle as a variable produces all four
variants from one design.

#### Hefty Pen (Wand)

A rigid pen-shaped device, ~8mm diameter × 120mm length. Aluminum or stainless
steel tube. Sensing element at the tip, electronics and battery in the body.

| Variant | Tip Design | Notes |
|---------|-----------|-------|
| **W-STD** | Ball or sensor at tip, straight | Standard pen grip, drag on any surface |
| **W-RET** | Retractable tip | Ball/sensor retracts into body when not in use (debris protection) |

Wand angle is determined by the user's grip, not the device geometry. The ball
or sensor at the tip must accommodate 30–70° pen angles from horizontal.

---

### Sensing Mechanisms

| Code | Mechanism | How It Works | Surface Agnostic? | Moving Parts | Resolution | Sensor BOM |
|------|-----------|-------------|-------------------|-------------|-----------|-----------|
| **S-BALL** | Mechanical ball + Hall effect | 5–8mm ball in socket, 4 roller spindles with magnets, 4 SMD Hall sensors detect rotation | **Yes — any surface** | Yes (ball + rollers) | Low: ~9–36 ticks/rev (~15–60 DPI equiv.) | ~$2.50 |
| **S-OLED** | Direct optical (LED) | Optical mouse sensor (YS8205/ADNB-3532/ADNS-2080) looks down at surface through aperture, LED illumination. Requires raised rim for focal distance — see [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md) | Most opaque surfaces. Fails on glass, mirrors, uniform white | **None** | Good: 800–2000 CPI | ~$0.50–4 |
| **S-VCSL** | Direct laser (VCSEL) | Same as S-OLED but VCSEL laser illumination instead of LED. Same raised rim requirement — see [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md) | More surfaces than LED — handles gloss, lacquer. Still fails on glass | **None** | Good: 2000+ CPI | ~$3–6 |
| **S-OPTB** | Optical on captive ball | Ball in socket, optical sensor (PMW3360 or similar) aimed at ball underside, reads ball surface texture | **Yes — any surface** | Ball only (no rollers) | Excellent: 100–12,000 CPI | ~$6–9 |
| **S-IMU** | 6-axis IMU (gyro + accel) | MPU-6050/BMI270/LSM6DSO. Tracks angular velocity and acceleration in free space. No surface contact needed. Cursor movement from hand tilt/rotation. | **Yes — no surface needed** | **None** | Moderate: depends on fusion algorithm and filtering. Sufficient for cursor control, not precision drawing. | ~$0.50–2 |

#### S-IMU Notes

The IMU is fundamentally different from the other sensing mechanisms: it tracks
the ring's orientation in space, not its position on a surface. This means:

- **No surface required.** Works in bed, in a wheelchair without a tray table,
  standing, walking. The most accessible option for users who lack a usable
  surface.
- **Drift.** Gyroscope integration accumulates error over time. The cursor
  slowly wanders if the hand is "still." Mitigated by: high-pass filtering
  (ignore slow drift, pass fast movement), accelerometer gravity reference
  (corrects roll/pitch), and auto-centering after inactivity timeout.
- **No angle dependency.** Unlike optical variants, the IMU doesn't care about
  the ring's angle relative to a surface. The `angle` parameter in the ring
  shell becomes purely ergonomic, not functional.
- **Lower precision than optical.** Sufficient for cursor navigation, scrolling,
  and clicking. Not suitable for precision drawing or pixel-level work. For
  users who need precision, the hybrid variants (see below) provide optical
  tracking on surfaces with IMU fallback in air.

---

### Optional Capabilities

#### IMU Hybrid Mode

Any surface-tracking sensor (S-OLED, S-VCSL, S-BALL, S-OPTB) can be combined
with an IMU to create a hybrid that auto-switches between surface and air
tracking:

| Code | Capability | Hardware Required | BOM Add | Notes |
|------|-----------|-------------------|---------|-------|
| **I-NONE** | No IMU | — | $0 | Surface tracking only |
| **I-IMU** | IMU hybrid | 6-axis IMU (MPU-6050/BMI270/LSM6DSO) | +$0.50–2 | Auto-switches between surface and air tracking |

**How hybrid mode works:** The optical sensor's SQUAL register (surface quality
score) reports in real time whether a trackable surface is present. When SQUAL
is above threshold, the ring tracks via the surface sensor (high precision, no
drift). When SQUAL drops to zero (finger lifted or no surface), the ring
switches to IMU (air mouse, moderate precision, drift-compensated). The
transition is automatic — no button, no mode switch, no user action.

For S-BALL and S-OPTB variants, lift detection uses the Hall sensor baseline
shift (ball not rotating + no surface pressure = lifted) instead of SQUAL.

This is the most accessible configuration: surface precision when available,
air control when not. A user in a wheelchair can track on the armrest, then
lift their hand to point at a screen across the room.

| Code | Capability | Hardware Required | BOM Add | Notes |
|------|-----------|-------------------|---------|-------|
| **C-NONE** | No camera | — | $0 | Pure pointing device |
| **C-CAM** | OCR camera | OV2640 or endoscope camera module + ESP32-S3 (replaces ESP32-C3) | +$5–8 | Enables text scanning from books/documents. Requires companion app with Tesseract/PaddleOCR. Upgrades MCU to ESP32-S3 for camera interface |

| Code | Capability | Hardware Required | BOM Add | Notes |
|------|-----------|-------------------|---------|-------|
| **L-NONE** | No laser | — | $0 | Pure pointing device |
| **L-DOT** | Guidance laser | 4mm 650nm Class 1 laser dot module | +$2–3 | Red dot on surface shows scan/point target. Essential for OCR mode, helpful for presentation use. Eye-safety compliance required (Class 1 only). |

---

## The Full Matrix

### Naming Convention

Each variant is named: `{FormFactor}-{Sensing}-{IMU}-{Camera}-{Laser}`

Examples:
- `R30-OLED-NONE-NONE-NONE` = Ring at 30°, optical LED, no IMU, no camera, no laser.
- `R30-OLED-IMU-NONE-NONE` = Ring at 30°, optical LED + IMU hybrid, no camera, no laser.
- `R30-IMU-NONE-NONE` = Ring at 30°, IMU only (air mouse), no camera, no laser.

For brevity in tables below, IMU-NONE variants omit the IMU field (matching
the original naming). IMU variants are listed separately after each angle's
base table.

---

### Ring Variants

Base variants: 4 angles x 4 surface sensors x 2 camera x 2 laser = 64.
IMU-only variants: 4 angles x 2 camera x 2 laser = 16.
IMU hybrid variants: 4 angles x 4 surface sensors x 2 camera x 2 laser = 64.
Total ring sensing configurations: 144.

#### R-00 (Flat, 0°)

*IMU and IMU hybrid variants follow the same pattern as R-30 (see below) at
identical BOMs. Only the base 16 surface-tracking combinations are listed here.*

| Variant ID | Sensing | Camera | Laser | BOM Est. | Primary Use | Surface | Notes |
|-----------|---------|--------|-------|----------|-------------|---------|-------|
| R00-BALL-NONE-NONE | Ball+Hall | — | — | ~$11 | Cursor (any surface) | Any | Simplest surface-agnostic ring |
| R00-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$14 | Cursor + aim | Any | Laser aids precision pointing |
| R00-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$19 | Cursor + OCR | Any | Dual-mode: mouse + book scanner |
| R00-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$22 | Full suite | Any | Maximum capability variant |
| R00-OLED-NONE-NONE | Optical LED | — | — | **~$9** | Cursor (desk) | Most | **Cheapest viable ring** |
| R00-OLED-NONE-DOT | Optical LED | — | Laser | ~$12 | Cursor + aim | Most | Laser helps presentations |
| R00-OLED-CAM-NONE | Optical LED | OCR | — | ~$17 | Cursor + OCR | Most | OCR ring, budget sensor |
| R00-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$20 | Full suite | Most | Full capability, not glass-safe |
| R00-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$12 | Cursor (wide surface) | Most+ | Better surface range than LED |
| R00-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$15 | Cursor + aim | Most+ | Dual laser (tracking + guide) |
| R00-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$20 | Cursor + OCR | Most+ | High-end OCR ring |
| R00-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$23 | Full suite | Most+ | Premium ring |
| R00-OPTB-NONE-NONE | Optical/ball | — | — | ~$15 | Cursor (any surface) | Any | Premium, no moving rollers |
| R00-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$18 | Cursor + aim | Any | Surface-agnostic + precision |
| R00-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$23 | Cursor + OCR | Any | Premium dual-mode |
| R00-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$25 | Full suite | Any | Maximum everything (at BOM ceiling) |

#### R-15 (15°)

*Same 16 combinations as R-00 with identical BOMs.* Shell geometry changes only.
Optimized for slight-curl hand position. Same IMU and IMU hybrid variants as
R-30 apply at identical BOMs.

| Variant ID | Sensing | Camera | Laser | BOM Est. |
|-----------|---------|--------|-------|----------|
| R15-BALL-NONE-NONE | Ball+Hall | — | — | ~$11 |
| R15-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$14 |
| R15-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$19 |
| R15-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$22 |
| R15-OLED-NONE-NONE | Optical LED | — | — | **~$9** |
| R15-OLED-NONE-DOT | Optical LED | — | Laser | ~$12 |
| R15-OLED-CAM-NONE | Optical LED | OCR | — | ~$17 |
| R15-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$20 |
| R15-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$12 |
| R15-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$15 |
| R15-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$20 |
| R15-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$23 |
| R15-OPTB-NONE-NONE | Optical/ball | — | — | ~$15 |
| R15-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$18 |
| R15-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$23 |
| R15-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$25 |

#### R-30 (30° — Recommended Default)

*Same 16 combinations. This is the recommended prototyping starting angle.*

The 30° angle matches the natural resting position of the middle finger on a
desk surface. The sensor window is flush with the desk without conscious finger
adjustment.

| Variant ID | Sensing | Camera | Laser | BOM Est. | Priority |
|-----------|---------|--------|-------|----------|----------|
| **R30-OLED-NONE-NONE** | Optical LED | — | — | **~$9** | **P0 — Build first** |
| R30-BALL-NONE-NONE | Ball+Hall | — | — | ~$11 | P1 — Surface-agnostic baseline |
| R30-OLED-NONE-DOT | Optical LED | — | Laser | ~$12 | P2 — Presentation variant |
| R30-OPTB-NONE-NONE | Optical/ball | — | — | ~$15 | P2 — Premium cursor |
| R30-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$20 | P3 — Full OCR ring |
| R30-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$14 | P3 |
| R30-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$19 | P3 |
| R30-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$22 | P3 |
| R30-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$12 | P3 |
| R30-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$15 | P3 |
| R30-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$20 | P3 |
| R30-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$23 | P3 |
| R30-OLED-CAM-NONE | Optical LED | OCR | — | ~$17 | P3 |
| R30-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$18 | P3 |
| R30-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$23 | P4 |
| R30-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$25 | P4 — BOM ceiling |

#### R-30 IMU Variants

##### IMU Only (Air Mouse)

| Variant ID | Sensing | Camera | Laser | BOM Est. | Notes |
|-----------|---------|--------|-------|----------|-------|
| R30-IMU-NONE-NONE | IMU only | — | — | **~$9** | **Cheapest surface-free ring.** Air mouse, no surface needed. Same BOM as optical — IMU replaces optical sensor at similar cost. |
| R30-IMU-NONE-DOT | IMU only | — | Laser | ~$12 | Air pointer with laser dot — presentation use |
| R30-IMU-CAM-NONE | IMU only | OCR | — | ~$17 | Air mouse + OCR. Unusual combo but valid for book scanning without surface tracking. |
| R30-IMU-CAM-DOT | IMU only | OCR | Laser | ~$20 | Full air ring |

##### IMU Hybrids (Surface + Air Auto-Switch)

These add an IMU to a surface-tracking sensor. The ring tracks on surfaces via
the primary sensor and switches to IMU air tracking when lifted. Add ~$0.50–2
to the base variant BOM.

| Variant ID | Sensing | Camera | Laser | BOM Est. | Priority |
|-----------|---------|--------|-------|----------|----------|
| **R30-OLED-IMU-NONE-NONE** | Optical LED + IMU | — | — | **~$10–11** | **P2 — Most accessible optical ring** |
| R30-BALL-IMU-NONE-NONE | Ball+Hall + IMU | — | — | ~$13 | P2 — Surface-agnostic + air fallback |
| R30-OLED-IMU-NONE-DOT | Optical LED + IMU | — | Laser | ~$13–14 | Hybrid + presentation laser |
| R30-OPTB-IMU-NONE-NONE | Optical/ball + IMU | — | — | ~$17 | Premium hybrid: 12K CPI on surface, air fallback |
| R30-BALL-IMU-NONE-DOT | Ball+Hall + IMU | — | Laser | ~$16 | Surface-agnostic hybrid + aim |
| R30-OLED-IMU-CAM-DOT | Optical LED + IMU | OCR | Laser | ~$22 | Full-capability hybrid ring |

*All other IMU hybrid combinations (VCSL+IMU, OPTB+IMU with camera/laser)
follow the same pattern: base variant BOM + $0.50–2 for IMU. Only the
high-value combinations are listed above. The full combinatorial expansion
(4 surface sensors × 2 camera × 2 laser × IMU = 16 per angle) exists for
the design space and defensive publication.*

---

#### R-45 (45°)

*Same 16 base combinations with identical BOMs.* Shell geometry changes only.
Optimized for curled-finger, keyboard-adjacent hand position. Same IMU and
IMU hybrid variants as R-30 apply at identical BOMs.

| Variant ID | Sensing | Camera | Laser | BOM Est. |
|-----------|---------|--------|-------|----------|
| R45-BALL-NONE-NONE | Ball+Hall | — | — | ~$11 |
| R45-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$14 |
| R45-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$19 |
| R45-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$22 |
| R45-OLED-NONE-NONE | Optical LED | — | — | **~$9** |
| R45-OLED-NONE-DOT | Optical LED | — | Laser | ~$12 |
| R45-OLED-CAM-NONE | Optical LED | OCR | — | ~$17 |
| R45-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$20 |
| R45-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$12 |
| R45-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$15 |
| R45-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$20 |
| R45-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$23 |
| R45-OPTB-NONE-NONE | Optical/ball | — | — | ~$15 |
| R45-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$18 |
| R45-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$23 |
| R45-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$25 |

---

### Wand Variants (2 tip styles × 4 sensors × 2 camera × 2 laser = 32 combinations)

*IMU and IMU hybrid variants apply to wands identically to rings. An IMU-only
wand is an air pointer (presentation remote). An IMU hybrid wand tracks on a
surface via the tip sensor and switches to air pointing when lifted. Add
~$0.50–2 to base BOM. IMU wand variants are not enumerated separately below —
the combinatorial pattern is the same.*

#### W-STD (Standard Tip)

| Variant ID | Sensing | Camera | Laser | BOM Est. | Notes |
|-----------|---------|--------|-------|----------|-------|
| **WSTD-BALL-NONE-NONE** | Ball+Hall | — | — | **~$14** | **Core wand — pen mouse on any surface** |
| WSTD-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$17 | Pointer + aim dot for presentations |
| WSTD-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$22 | Pen mouse + book scanner |
| WSTD-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$25 | Full suite wand (BOM ceiling) |
| WSTD-OLED-NONE-NONE | Optical LED | — | — | ~$12 | Pen mouse, most surfaces |
| WSTD-OLED-NONE-DOT | Optical LED | — | Laser | ~$15 | With aim dot |
| WSTD-OLED-CAM-NONE | Optical LED | OCR | — | ~$20 | Optical pen + OCR |
| WSTD-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$23 | Full optical wand |
| WSTD-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$15 | Wide surface pen mouse |
| WSTD-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$18 | Dual laser wand |
| WSTD-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$23 | Premium OCR wand |
| WSTD-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$25 | Maximum wand (BOM ceiling) |
| WSTD-OPTB-NONE-NONE | Optical/ball | — | — | ~$18 | Premium: 12K CPI on any surface |
| WSTD-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$21 | Drawing-grade wand + aim |
| WSTD-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$25 | At BOM ceiling |
| WSTD-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$28 | **Exceeds $25 ceiling — not recommended** |

#### W-RET (Retractable Tip)

Same 16 combinations as W-STD, +$2 for retraction mechanism (spring + sleeve).
Protects ball/sensor from pocket debris when not in use.

| Variant ID | Sensing | Camera | Laser | BOM Est. | Notes |
|-----------|---------|--------|-------|----------|-------|
| WRET-BALL-NONE-NONE | Ball+Hall | — | — | ~$16 | Core retractable wand |
| WRET-BALL-NONE-DOT | Ball+Hall | — | Laser | ~$19 | With aim dot |
| WRET-BALL-CAM-NONE | Ball+Hall | OCR | — | ~$24 | Near ceiling |
| WRET-BALL-CAM-DOT | Ball+Hall | OCR | Laser | ~$27 | **Exceeds ceiling** |
| WRET-OLED-NONE-NONE | Optical LED | — | — | ~$14 | Budget retractable pen |
| WRET-OLED-NONE-DOT | Optical LED | — | Laser | ~$17 | With aim dot |
| WRET-OLED-CAM-NONE | Optical LED | OCR | — | ~$22 | Optical + OCR |
| WRET-OLED-CAM-DOT | Optical LED | OCR | Laser | ~$25 | At ceiling |
| WRET-VCSL-NONE-NONE | Laser VCSEL | — | — | ~$17 | Retractable laser pen |
| WRET-VCSL-NONE-DOT | Laser VCSEL | — | Laser | ~$20 | Dual laser |
| WRET-VCSL-CAM-NONE | Laser VCSEL | OCR | — | ~$25 | At ceiling |
| WRET-VCSL-CAM-DOT | Laser VCSEL | OCR | Laser | ~$27 | **Exceeds ceiling** |
| WRET-OPTB-NONE-NONE | Optical/ball | — | — | ~$20 | Premium retractable |
| WRET-OPTB-NONE-DOT | Optical/ball | — | Laser | ~$23 | With aim dot |
| WRET-OPTB-CAM-NONE | Optical/ball | OCR | — | ~$27 | **Exceeds ceiling** |
| WRET-OPTB-CAM-DOT | Optical/ball | OCR | Laser | ~$30 | **Well over ceiling** |

---

## Totals

| Category | Variants |
|----------|---------|
| Ring — surface sensors (4 angles × 4 sensors × 2 camera × 2 laser × 2 click) | 128 |
| Ring — IMU only (4 angles × 2 camera × 2 laser × 2 click) | 32 |
| Ring — IMU hybrids (4 angles × 4 sensors × 2 camera × 2 laser × 2 click) | 128 |
| Wand standard (16 combos) | 16 |
| Wand retractable (16 combos) | 16 |
| **Total unique variants** | **320** |

In practice, start with one: R30-OLED-NONE-NONE-NONE with dome click. Buy two.
That's a complete mouse for ~$18.

The 320 variants exist for the combinatorial design space and defensive
publication. The canonical two-ring setup (two identical rings, software-
assigned roles) is the opinionated default and the only configuration that
should be prototyped in Phase 1.

---

## Recommended Build Order

### Phase 1 — Two Identical Rings (P0)

| Priority | What to Build | Quantity | BOM Each | Why |
|----------|--------------|----------|----------|-----|
| **P0** | R30-OLED-NONE-NONE (dome click) | **×2** | ~$9 | Two identical rings = complete mouse |

**Total P0 BOM: ~$18 for a complete mouse replacement.**

Build two copies of the same ring. Flash identical firmware. Companion app
assigns one as "cursor + left click" (middle finger) and the other as "scroll +
right click" (index finger). This is the canonical setup.

### Phase 2 — Alternative Sensing + Wand (P1–P2)

| Priority | Variant | Quantity | BOM | Why |
|----------|---------|----------|-----|-----|
| **P1** | R30-BALL-NONE-NONE (dome click) | ×2 | ~$22 | Surface-agnostic pair, tests ball in inverted orientation |
| **P1** | WSTD-BALL-NONE-NONE | ×1 | ~$14 | Core wand concept, pen-on-any-surface |
| **P2** | R30-IMU-NONE-NONE (dome click) | ×2 | ~$18 | Air mouse pair: no surface needed, most accessible variant |
| **P2** | R30-OLED-IMU-NONE-NONE (dome click) | ×2 | ~$22 | Hybrid pair: surface + air auto-switch, validates IMU fallback |
| **P2** | R30-OPTB-NONE-NONE (dome click) | ×2 | ~$30 | Premium pair: any surface + high resolution |
| **P2** | R30-OLED-NONE-NONE (piezo click) | ×2 | ~$22 | Tests sealed piezo click vs. dome — same sensor, different click |

### Phase 3 — OCR + Extras (P3)

| Priority | Variant | BOM | Why |
|----------|---------|-----|-----|
| **P3** | R30-OLED-CAM-DOT (dome click) | ~$20 | OCR ring — add camera + laser to proven base |
| **P3** | WSTD-BALL-CAM-DOT | ~$25 | Full OCR wand: any-surface pen + scanner + aim |

### Phase 4 — Angle Validation

Build R00, R15, R45 variants of the P0 ring (OLED-NONE-NONE, dome click) in
pairs of two, to validate which angles work best for different hand sizes,
mobility ranges, and use postures (desk, lap, bed, wheelchair armrest).

---

## Surface Compatibility Matrix

| Surface | S-BALL | S-OLED | S-VCSL | S-OPTB | S-IMU |
|---------|--------|--------|--------|--------|-------|
| Wood desk | Yes | Yes | Yes | Yes | Yes |
| Glass | **Yes** | No | No | **Yes** | **Yes** |
| Glossy/lacquered | Yes | Marginal | Yes | Yes | Yes |
| Matte plastic | Yes | Yes | Yes | Yes | Yes |
| Paper/book page | Yes | Yes | Yes | Yes | Yes |
| Fabric (jeans, couch) | Yes | Marginal | Marginal | Yes | **Yes** |
| Metal (tray table) | Yes | Yes | Yes | Yes | Yes |
| Skin | Yes | Marginal | Marginal | Yes | **Yes** |
| Mirror | Yes | No | No | Yes | **Yes** |
| Uniform white | Yes | No | Marginal | Yes | **Yes** |
| Dark/black matte | Yes | Yes | Yes | Yes | Yes |
| **No surface (air)** | No | No | No | No | **Yes** |

S-IMU works on every surface because it doesn't use the surface. The "Yes"
entries for S-IMU mean the ring functions in that environment — the surface is
irrelevant. The IMU hybrid variants (I-IMU) inherit the surface column of their
primary sensor AND the S-IMU column as fallback.

---

## The Universal Ring

Every ring in the matrix above includes **both** a tracking sensor **and** a
click mechanism (metal snap dome or piezo film — see
[CLICK-MECHANISMS.md](CLICK-MECHANISMS.md) for full analysis). There is no
separate "click-only ring." Every ring is the same device. The companion app
assigns roles.

This is the core insight: **a mouse is just move + click, distributed across
fingers.** Two identical rings decompose a mouse into its atomic primitives:

```
  RING 1 (middle finger)          RING 2 (index finger)
  ┌──────────────┐                ┌──────────────┐
  │  sensor      │                │  sensor      │
  │  + click     │                │  + click     │
  │              │                │              │
  │  Role:       │                │  Role:       │
  │  CURSOR +    │                │  RIGHT CLICK │
  │  LEFT CLICK  │                │  + SCROLL    │
  └──────────────┘                └──────────────┘
    identical hardware              identical hardware
    software-assigned role          software-assigned role
```

### Why One Universal Ring

- **One BOM.** One PCB design, one firmware image, one shell. Half the variants
  to manufacture and maintain vs. separate nav/click rings.
- **One replacement part.** Lose or break a ring — buy the same part regardless
  of which finger it was on.
- **Role is software, not hardware.** The companion app says "you're left click"
  and "you're right click + scroll." Roles can be reassigned, swapped, or
  reconfigured per-app without touching hardware.
- **Scales to N rings.** A third identical ring on the ring finger becomes a
  modifier or scroll wheel. A fourth becomes whatever you want. Same device,
  different software role.
- **Scales to N body positions.** The electronics module is not a ring — it's a
  sensor + click + BLE + battery package. The ring shell is just one harness for
  it. A different silicone harness mounts the same module on a toe, a knuckle,
  a prosthetic, a headband, or any other body position where a user has
  controllable fine motor movement. No hardware redesign needed — same PCB, same
  firmware, different harness.

### Harness, Not Ring

The electronics module is the product. The ring shell is one harness — the
default, most common mounting for finger use. But the module can be mounted
anywhere:

| Harness | Mounting | Use Case |
|---------|----------|----------|
| **Fingertip ring** (default) | Silicone/printed ring on distal phalanx | Standard desk, lap, any-surface use |
| **Toe ring** | Silicone band on big toe or second toe | Users with limited hand/arm mobility |
| **Knuckle strap** | Elastic strap on proximal phalanx | Users who can't curl fingertips |
| **Prosthetic mount** | Clip or socket adapter | Mounts to existing prosthetic device |
| **Headband mount** | Forehead or temple bracket | Head-tracking for users with limited limb mobility |
| **Flat puck** | Adhesive base, no body attachment | Stationary surface use (desk widget) |

The harness is the cheapest, simplest part of the system — a piece of shaped
silicone or a 3D-printed bracket. Designing a new harness requires no electrical
engineering, no firmware changes, and no PCB redesign. Anyone with a 3D printer
or a silicone mold can create a harness for a body position that doesn't exist
yet.

This is the deepest form of the universal ring principle: **the hardware is
body-position-agnostic because the mounting is separate from the electronics.**
Software assigns the role. The harness assigns the body position. The module
itself is invariant.

### Click Mechanism in Every Ring

Every ring variant includes a click mechanism. Two options (choose at build
time, both fit alongside any sensor — see CLICK-MECHANISMS.md):

| Mechanism | Added to BOM | Lifecycle | Feel | Sealed? |
|-----------|-------------|-----------|------|---------|
| Metal snap dome (default) | +$0.10 | 5–10M cycles | Real mechanical snap | No |
| Piezo film + haptic LRA (premium) | +$1.50 | Infinite | Simulated (good, not perfect) | Yes — fully sealed ring |

BOM impact is negligible. The R30-OLED-NONE-NONE ring with dome click is
**~$9.** With piezo click it's **~$11.** Both are well under the $25 ceiling.

---

## Canonical Setup: Two Identical Rings

The opinionated default for PowerFinger is **two identical rings on two
fingers**, with roles assigned in software:

| Finger | Role | Move | Click |
|--------|------|------|-------|
| **Middle** (Ring 1) | Primary | Cursor X/Y | Left click |
| **Index** (Ring 2) | Secondary | Scroll (vertical drag) | Right click |

**Total BOM: ~$18** (two R30-OLED-NONE-NONE rings with dome click).

This is a **complete mouse replacement** from two copies of the same $9 device.

### Gesture Map

Every mouse action decomposes into combinations of move and click from two
fingers. The rings report raw deltas and press/release events. The companion
app composes them into OS-level HID events:

| Mouse Action | Ring 1 (Middle) | Ring 2 (Index) |
|---|---|---|
| Move cursor | Drag finger | — |
| Left click | Press down | — |
| Right click | — | Press down |
| Double click | Press twice | — |
| Drag | Hold + drag | — |
| Scroll | — | Drag finger (vertical) |
| Middle click | Press down | Press down (simultaneous) |
| Zoom | — | Drag finger (vertical, in zoom-capable apps) |
| Modifier (Ctrl-click, etc.) | Press down | Hold (acts as modifier) |

**The rings are dumb.** Each reports two things: (1) sensor X/Y deltas when
the finger moves, (2) click press/release events. All gesture interpretation
lives in the companion app, which is free to update, configure per-app, and
adapt to individual users' needs.

### Why This Works

A conventional mouse bundles move + left click + right click + scroll into one
device held by one hand in one posture. PowerFinger unbundles them across
fingers:

- **Move** = drag one finger (the most natural pointing gesture)
- **Click** = press one finger down (the most natural selection gesture)
- **Scroll** = drag a different finger (the most natural browsing gesture)
- **Right click** = press a different finger (unambiguous "other action")

Each atomic action is a single finger doing one thing. No overloaded gestures,
no mode confusion, no accidental clicks during cursor movement (the click is
on a different finger from the cursor).

---

## Single-Ring Fallback

For users who can only wear one ring (e.g., limited finger independence), a
single ring provides cursor + left click on one device. The built-in dome or
piezo click activates by pressing the finger down harder. This works but is
less precise — the same finger that moves the cursor also clicks, so accidental
clicks during movement are possible.

The companion app detects single-ring mode automatically and maps accordingly:
cursor + left click + long-press for right click.

---

## Multi-Ring Expansion

Beyond the canonical two-ring pair, additional identical rings add axes of
control. The companion app manages role assignment:

| Configuration | Body Position | Roles |
|--------------|---------------|-------|
| **Two rings (canonical)** | **Middle + Index fingers** | **Cursor+LClick, Scroll+RClick — complete mouse** |
| Single ring (fallback) | Middle finger | Cursor + click (compromised) |
| Three rings | Middle + Index + Ring fingers | Cursor+LClick, Scroll+RClick, Modifier |
| Four rings | Middle + Index + Ring + Pinky | Full configurable surface |
| Ring + Wand | Any finger + dominant hand | Cursor (ring) + precision/drawing (wand) |
| **Toe pair** | **Big toe + second toe** | **Cursor+LClick, Scroll+RClick — foot-operated mouse** |
| Toe + finger | Big toe + any finger | Cursor (toe), click (finger) — or vice versa |
| Prosthetic mount | Residual limb or prosthetic | Cursor + click via available motion |
| Headband | Forehead/temple | Head-tilt cursor control |

**Every module is the same hardware.** Adding a body position adds a channel.
The companion app assigns meaning. The harness determines where it mounts. This
is a modular, composable input system built from a single universal component.

---

## Parametric Design Notes

The system has two design layers: the **electronics module** (invariant) and the
**harness** (variable). The module is one PCB, one firmware image, one BOM. The
harness is the physical mounting that adapts it to a body position and use
context.

### Electronics Module

One PCB design fits every harness. The module exposes:
- Sensor aperture (bottom face)
- Click actuation surface (top or bottom, depending on mechanism)
- USB-C or pogo pin charging interface
- BLE antenna (keep-out zone in harness design)

### Harness Parameters

A parametric CAD model for the default ring harness should accept:

- `angle`: sensor tilt (0, 15, 30, 45 degrees)
- `finger_circumference`: sizing parameter (S/M/L or continuous)
- `sensor_type`: determines aperture geometry and rim height (optical variants need precise focal distance — see [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md))
- `click_type`: dome (needs dome cavity) or piezo (fully sealed shell)
- `has_camera`: adds camera mount and cable routing
- `has_laser`: adds laser module mount

Other harness types (toe, knuckle, prosthetic mount, headband) are separate
parametric models that accept the same module. The mechanical interface between
module and harness should be standardized early — snap-fit, friction fit, or
silicone overmold — so that new harnesses can be designed independently.

One module. One firmware. Many harnesses. The entire product line is a single
universal component in different mountings with software-assigned roles.
