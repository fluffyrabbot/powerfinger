# Glide System — Raised Rim and Contact Pads

Optical sensor variants (S-OLED, S-VCSL) require a consistent focal distance of
2.4–3.2mm between the sensor aperture and the tracking surface. This document
defines how the ring maintains that distance: a structural raised rim with
replaceable UHMWPE glide pads.

---

## Design

The ring's lower shell extends into a raised rim that surrounds the sensor
aperture. The rim holds the sensor at the correct focal height above any surface.
The rim's contact points use replaceable UHMWPE (ultra-high molecular weight
polyethylene) pads as the actual glide surface.

```
CROSS-SECTION — Ring on desk surface

    finger pad
    ──────────────
    │  LiPo      │
    │  flex PCB   │
    │  sensor     │
    │  LED ◉      │  ← illumination + sensor aperture
    ▓▓            ▓▓  ← structural rim (POM or nylon)
    ░░            ░░  ← UHMWPE glide pads (replaceable)
    ══════════════════  surface
       ↕ 2.4–3.2mm
       focal distance
```

### Two-Layer Construction

| Layer | Material | Purpose | Replaced? |
|-------|----------|---------|-----------|
| **Structural rim** | POM (Delrin/acetal) or nylon | Sets geometry, holds sensor at correct height, provides rigidity | No — part of ring shell |
| **Glide pads** | UHMWPE | Contact surface against desk/table, low friction, quiet, wear-tolerant | Yes — user-replaceable when worn |

The structural rim is integral to the 3D-printed or injection-molded ring shell.
The UHMWPE pads press-fit or adhesive-mount onto the rim's contact points. When
pads wear down (changing focal distance), the user replaces the pads, not the
ring. The wear part is the cheapest, most replaceable part.

---

## Material Selection

### UHMWPE Glide Pads

UHMWPE is the primary glide surface material.

| Property | Value |
|----------|-------|
| Coefficient of friction | ~0.05–0.10 (on steel, comparable to PTFE) |
| Abrasion resistance | Excellent — used in hip replacements, synthetic ice panels |
| Noise | Quiet — softer than POM, absorbs micro-vibrations rather than transmitting them as audible resonance |
| Chemical concerns | None — no PFAS, no fluoropolymers, biocompatible |
| Sourcing | Commodity — rod/sheet stock from McMaster-Carr, AliExpress, etc. Pennies per pad |
| Machinability | Easy to cut, punch, or CNC from sheet stock |

### Why Not PTFE

PTFE (Teflon) is the traditional mouse skate material but is avoided here:

- **PFAS regulation risk.** The EU universal PFAS restriction proposal is
  advancing. EPA regulations are tightening. PTFE is a fluoropolymer and falls
  under PFAS restrictions in some regulatory frameworks. Even if solid PTFE is
  lower-risk than fluoropolymer coatings, supply disruption is a real
  possibility mid-product-life.
- **Optics.** An open-source accessibility project should not depend on a
  material class under increasing regulatory scrutiny, regardless of whether
  the specific form factor poses health risk.
- **UHMWPE is comparable.** Slightly higher friction than PTFE in controlled
  tests, but not meaningfully different at the force levels of a finger ring
  gliding under its own weight. The difference is imperceptible in use.

### Structural Rim: POM vs. Nylon

| Property | POM (Delrin) | Nylon (PA6/PA12) |
|----------|-------------|-----------------|
| Dimensional stability | Excellent — low moisture absorption | Good — absorbs moisture, can swell |
| Surface hardness | Higher — better for maintaining geometry | Lower — more flexible |
| Friction (if pad falls off) | Low (~0.15) — acceptable emergency glide | Higher (~0.2–0.3) |
| 3D printability | Poor (warps in FDM) — better injection molded | Good (SLS/MJF nylon is standard) |
| Cost | Low | Low |

**Recommendation:** Nylon for 3D-printed prototypes (SLS/MJF prints well). POM
for injection-molded production runs (better dimensional stability for
maintaining precise focal distance over time).

---

## Known Failure Modes and Mitigations

### Tilt

**Problem:** A ring on a finger naturally rocks during movement. The circular rim
becomes a pivot point — the ring tilts, one side lifts, and the sensor's viewing
angle to the surface changes. Optical sensors tolerate ~5–10 degrees of tilt
before tracking degrades.

**Mitigation:** A wider rim diameter reduces angular change per mm of finger
movement (longer lever arm). The rim diameter should be as wide as the ring
shell allows. Prototype testing must measure actual tilt range during natural
use — it may be less than expected if the rim diameter is adequate relative to
the sensor aperture. Firmware can also detect tracking degradation and signal the
user (haptic pulse) when tilt exceeds the sensor's reliable range.

### Debris Ingress

**Problem:** The rim creates an enclosed cavity around the sensor and LED. Hair,
dust, and fiber migrate into the cavity during use. Unlike flat mouse skates,
a ring rim can act as a scoop relative to direction of travel.

**Mitigation:** The cavity should be open enough to self-clear — avoid creating a
fully enclosed pocket. A slight chamfer on the inner rim edge prevents debris
from catching. The rim geometry should allow debris to pass through rather than
accumulate. Periodic cleaning with compressed air is acceptable for maintenance.

### Surface Scratching

**Problem:** Hard contact surfaces can leave micro-scratches on glass or glossy
lacquered surfaces.

**Mitigation:** UHMWPE is softer than glass and most desk finishes. It will not
scratch glass (Mohs hardness of UHMWPE is well below glass). This is a solved
problem — UHMWPE is used as a bearing surface against metals and ceramics in
industrial applications without damaging the harder surface.

### Illumination Reflections

**Problem:** The LED illuminates the surface at an angle and the sensor reads the
reflected pattern. If the inner wall of the rim is close to the sensor aperture
and light-colored, stray LED light bounces off the rim walls into the sensor,
washing out the tracking image.

**Mitigation:** The inner surface of the rim cavity must be matte black. If the
ring shell is printed in a light color, the sensor cavity interior needs a matte
black coating or insert. Standard optical mouse design practice.

### Surface Transitions

**Problem:** When the ring slides from one surface onto another (desk to paper,
desk to mouse pad), the rim catches on the surface edge or discontinuity.

**Mitigation:** A slight chamfer or radius on the rim's leading edge reduces
catching. Since the rim is circular, there is no single "leading edge" — the
entire rim perimeter should have a chamfered or radiused outer edge. This is a
minor annoyance, not a showstopper — conventional mice have the same issue with
their skate pads at surface boundaries.

### Pad Wear Changing Focal Distance

**Problem:** As UHMWPE pads wear, the sensor-to-surface distance decreases. If
pads wear unevenly, tilt bias is introduced.

**Mitigation:** UHMWPE wear rate is extremely low at the forces involved (ring
weight + light finger pressure). Pad replacement interval is likely measured in
years. When replacement is needed, new pads restore original focal distance. Pad
thickness should be standardized (e.g., 0.5mm) so replacements are consistent.

### Acoustic Noise

**Problem:** Any rigid contact surface on a hard desk produces some audible
scraping/sliding sound.

**Mitigation:** UHMWPE is quieter than POM, nylon, or ABS due to its lower
hardness and better vibration damping. It will not be silent — but it will be
quieter than a conventional mouse on the same surface. The dominant noise factor
is surface conformity, not material friction. A smooth, slightly compliant
contact surface (which UHMWPE provides) is quieter than a perfectly rigid one.

---

## Applicability

| Sensor Type | Needs Glide System? | Notes |
|-------------|:-------------------:|-------|
| **S-OLED** (optical LED) | **Yes** | Focal distance 2.4–3.2mm is critical for tracking |
| **S-VCSL** (laser VCSEL) | **Yes** | Same focal distance requirement as S-OLED |
| **S-BALL** (ball + Hall) | No | Ball protrudes below ring, contacts surface directly |
| **S-OPTB** (optical on ball) | No | Sensor reads ball surface, not desk surface — focal distance is internal |

For S-BALL and S-OPTB variants, the rim is not needed for focal distance but may
still be present as a structural element of the ring shell. In those variants,
the rim does not require precision height control or UHMWPE pads.

---

## BOM Impact

| Component | Cost | Source |
|-----------|------|--------|
| UHMWPE pad set (4 pads per ring) | ~$0.05–0.10 | Punched from UHMWPE sheet stock |
| Replacement pad set (user-purchased) | ~$0.50–1.00 | Includes packaging/shipping overhead |

The glide system adds negligible cost to the ring BOM. The structural rim is
part of the shell (no additional cost). The UHMWPE pads are the only added
component.

---

## Pad Geometry

The glide pads should be arranged symmetrically around the sensor aperture to
provide stable, tilt-resistant contact:

```
BOTTOM VIEW — Ring underside

        ░░░░
      ░░    ░░
    ░░   ◉    ░░    ◉ = sensor aperture
    ░░  sensor ░░   ░ = UHMWPE pad ring
      ░░    ░░          (continuous or 3–4 discrete pads)
        ░░░░

    Option A: Continuous ring pad
    - Maximum tilt resistance
    - Harder to replace (one piece)

        ░░
    ░░      ░░
       ◉
    ░░      ░░
        ░░

    Option B: 4 discrete pads at cardinal points
    - Easy to replace individually
    - Slightly less tilt resistance
    - Allows debris to exit between pads
```

**Recommendation:** Option B (discrete pads) for prototyping — easier to
fabricate, allows debris clearance, individual pad replacement. Option A
(continuous ring) may be better for production if tilt proves problematic.
