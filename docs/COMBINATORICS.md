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
| **S-OLED** | Direct optical (LED) | Optical mouse sensor (YS8205/ADNB-3532/ADNS-2080) looks down at surface through aperture, LED illumination | Most opaque surfaces. Fails on glass, mirrors, uniform white | **None** | Good: 800–2000 CPI | ~$0.50–4 |
| **S-VCSL** | Direct laser (VCSEL) | Same as S-OLED but VCSEL laser illumination instead of LED | More surfaces than LED — handles gloss, lacquer. Still fails on glass | **None** | Good: 2000+ CPI | ~$3–6 |
| **S-OPTB** | Optical on captive ball | Ball in socket, optical sensor (PMW3360 or similar) aimed at ball underside, reads ball surface texture | **Yes — any surface** | Ball only (no rollers) | Excellent: 100–12,000 CPI | ~$6–9 |

---

### Optional Capabilities

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

Each variant is named: `{FormFactor}-{Sensing}-{Camera}-{Laser}`

Example: `R30-OLED-NONE-NONE` = Ring at 30°, LED optical sensor, no camera, no laser.

---

### Ring Variants (4 angles × 4 sensors × 2 camera × 2 laser = 64 combinations)

#### R-00 (Flat, 0°)

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
Optimized for slight-curl hand position.

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

#### R-45 (45°)

*Same 16 combinations with identical BOMs.* Shell geometry changes only.
Optimized for curled-finger, keyboard-adjacent hand position.

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
| Ring (4 angles × 16 combos) | 64 |
| Wand standard (16 combos) | 16 |
| Wand retractable (16 combos) | 16 |
| **Total unique variants** | **96** |
| Within $25 BOM ceiling | 85 |
| Exceed $25 BOM ceiling | 11 |

---

## Recommended Build Order

### Phase 1 — Minimum Viable Prototypes (3 builds)

| Priority | Variant | BOM | Why |
|----------|---------|-----|-----|
| **P0** | R30-OLED-NONE-NONE | ~$9 | Cheapest ring, natural hand angle, proves the core concept |
| **P1** | R30-BALL-NONE-NONE | ~$11 | Surface-agnostic ring, tests mechanical ball in inverted orientation |
| **P1** | WSTD-BALL-NONE-NONE | ~$14 | Core wand concept, pen-on-any-surface |

### Phase 2 — Capability Expansion (4 builds)

| Priority | Variant | BOM | Why |
|----------|---------|-----|-----|
| **P2** | R30-OPTB-NONE-NONE | ~$15 | Premium ring: surface-agnostic + high resolution, no debris |
| **P2** | R30-OLED-NONE-DOT | ~$12 | Laser-guided variant for presentation use |
| **P2** | WSTD-BALL-NONE-DOT | ~$17 | Wand + aim laser for OCR prep |
| **P2** | R15-OLED-NONE-NONE | ~$9 | Alternate angle, compare ergonomics against R30 |

### Phase 3 — OCR Integration (2 builds)

| Priority | Variant | BOM | Why |
|----------|---------|-----|-----|
| **P3** | R30-OLED-CAM-DOT | ~$20 | Full OCR ring: mouse + text scanner + aim laser |
| **P3** | WSTD-BALL-CAM-DOT | ~$25 | Full OCR wand: any-surface pen + scanner + aim |

### Phase 4 — Angle Validation

Build R00, R15, R45 variants of the P0 winner (OLED-NONE-NONE) to validate
which angles work best for different hand sizes, mobility ranges, and use
postures (desk, lap, bed, wheelchair armrest).

---

## Surface Compatibility Matrix

| Surface | S-BALL | S-OLED | S-VCSL | S-OPTB |
|---------|--------|--------|--------|--------|
| Wood desk | Yes | Yes | Yes | Yes |
| Glass | **Yes** | No | No | **Yes** |
| Glossy/lacquered | Yes | Marginal | Yes | Yes |
| Matte plastic | Yes | Yes | Yes | Yes |
| Paper/book page | Yes | Yes | Yes | Yes |
| Fabric (jeans, couch) | Yes | Marginal | Marginal | Yes |
| Metal (tray table) | Yes | Yes | Yes | Yes |
| Skin | Yes | Marginal | Marginal | Yes |
| Mirror | Yes | No | No | Yes |
| Uniform white | Yes | No | Marginal | Yes |
| Dark/black matte | Yes | Yes | Yes | Yes |

---

## Multi-Ring Composition

Any ring variant can be combined with other ring variants on different fingers.
The companion app manages role assignment:

| Configuration | Fingers | Roles |
|--------------|---------|-------|
| Single ring | Middle | Cursor X/Y + click |
| Dual ring | Middle + Index | Cursor + scroll/zoom |
| Triple ring | Middle + Index + Ring | Cursor + scroll + modifier |
| Ring + Wand | Any + dominant hand | Cursor (ring) + precision/drawing (wand) |

Rings do not need to be the same variant. A user might wear R30-OLED-NONE-NONE
on the middle finger (cursor) and R30-BALL-NONE-NONE on the index finger
(scroll, works on any surface including glass).

---

## Parametric Design Notes

All ring variants at a given angle share identical electronics. The shell is the
only variable. A parametric CAD model should accept:

- `angle`: sensor tilt (0, 15, 30, 45 degrees)
- `finger_circumference`: sizing parameter (S/M/L or continuous)
- `sensor_type`: determines aperture geometry and standoff height
- `has_camera`: adds camera mount and cable routing
- `has_laser`: adds laser module mount

This means one parametric model produces all 64 ring variants.
