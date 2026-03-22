# Prototype Spec — 3 Builds for EE/ME

Open-source assistive ring mouse. BLE HID. Full design space and rationale at
the links below — this document is the concise build spec.

- Design matrix: [COMBINATORICS.md](COMBINATORICS.md)
- Click mechanisms: [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md)
- Glide system: [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md)
- IP landscape: [IP-STRATEGY.md](IP-STRATEGY.md)

---

## Prototype 1 — Optical Ring Pair (x2 identical units)

| Component | Spec |
|-----------|------|
| MCU | ESP32-C3 SuperMini |
| Sensor | Optical mouse sensor (PAW3204 / ADNS-2080 class) |
| Click | Metal snap dome (Snaptron SQ-series or equivalent, 5-10M cycles) |
| Battery | LiPo 80-100mAh |
| Charging | USB-C |
| Protocol | BLE HID mouse — reports X/Y deltas + click press/release |
| Shell | 3D-printed ring, 30 degree sensor angle, parametric for finger circumference |
| Glide | Raised structural rim (POM or nylon) with replaceable UHMWPE glide pads |
| Focal distance | 2.4-3.2mm, maintained by rim height |
| Sensor cavity | Matte black interior (prevent LED wash-out) |
| BOM target | ~$9 each |

Two identical units. Companion app assigns roles (cursor + left click on one,
scroll + right click on the other). Together they are a complete mouse for ~$18.

This is the P0 build. Everything else depends on this working.

---

## Prototype 2 — Ball+Hall Ring Pair (x2 identical units)

| Component | Spec |
|-----------|------|
| MCU | ESP32-C3 SuperMini |
| Sensor | 5mm steel ball in socket, 4 roller spindles with magnets, 4 SMD Hall sensors |
| Click | Metal snap dome |
| Battery | LiPo 80-100mAh |
| Charging | USB-C |
| Protocol | BLE HID mouse |
| Shell | 3D-printed ring, 30 degree sensor angle, parametric for finger circumference |
| Glide | No raised rim needed — ball protrudes below shell, contacts surface directly |
| BOM target | ~$11 each |

Surface-agnostic: works on glass, fabric, skin, mirror, anything. Validates
whether the Hall sensor array can resolve usable cursor deltas from a 5mm ball
at finger-pressure force levels.

---

## Prototype 3 — Ball+Hall Wand (x1)

| Component | Spec |
|-----------|------|
| MCU | ESP32-C3 SuperMini |
| Sensor | Ball+Hall at tip (same mechanism as Prototype 2) |
| Click | Kailh GM8.0 micro switch on barrel, thumb-operated |
| Body | Aluminum or stainless steel tube, ~8mm diameter x 120mm length |
| Battery | LiPo 100-150mAh |
| Charging | USB-C |
| Protocol | BLE HID mouse |
| BOM target | ~$14 |

Pen grip, drag on any surface. The ball at the tip must accommodate 30-70
degree pen angles from horizontal.

---

## Fixed Across All Three

- **MCU:** ESP32-C3 (BLE 5.0, supports 7.5ms connection intervals, ESP-IDF
  framework)
- **Protocol:** BLE HID mouse profile — no custom drivers, no USB dongle, pairs
  like any Bluetooth mouse on any OS
- **License:** MIT firmware, CERN-OHL-S 2.0 hardware (open source, your name on
  it if you want it)
- **No cloud dependency.** No app required for basic single-ring mouse function.
  Companion app (built separately) adds multi-ring role assignment and
  configuration
- **No planned obsolescence.** Every component must be replaceable. Commodity
  parts with multiple source vendors

---

## What We Need From You

- **Schematic + PCB layout** — rigid for wand, flex or rigid-flex for ring
- **Ring shell CAD** — parametric for finger circumference and sensor angle.
  OpenSCAD, FreeCAD, or Fusion 360. Should accept at minimum: `finger_circumference`
  (mm) and `angle` (degrees)
- **Wand body CAD** — tube with internal component mounting
- **Assembled prototypes** — even ugly is fine. We are validating sensing
  geometry, BLE connectivity, and ergonomics, not aesthetics
- **Firmware is handled separately.** You just need to verify the ESP-IDF BLE
  HID mouse example flashes and pairs successfully on the assembled hardware

### Scope

Three small PCBs. Two shell designs (ring + wand). Assembly. No application
software beyond verifying the stock ESP-IDF BLE HID example runs and pairs.

---

## Constraints That Matter

These are non-negotiable — see CLAUDE.md and the design docs for full rationale.

- **BOM ceiling:** No single variant may exceed $25. The cheapest ring must stay
  under $10.
- **BLE HID latency < 15ms.** ESP32-C3 supports 7.5ms connection intervals.
- **Ring height budget: ~10mm** finger-to-surface. Sensor + PCB + battery must
  fit in ~7-8mm above surface with 2.5-3mm air gap below.
- **DPI/sensitivity must be configurable** via BLE characteristic, never
  hard-coded.
- **Ring geometry must be parametric** for finger size — never assume a specific
  hand.
- **All parts must be sourceable from multiple vendors.** No single-source
  components.

---

## Build Order

| Priority | What | Quantity | Why |
|----------|------|----------|-----|
| **P0** | Optical ring pair (Prototype 1) | x2 | Validates core concept: two identical rings = mouse |
| **P1** | Ball+Hall ring pair (Prototype 2) | x2 | Validates surface-agnostic sensing in ring form factor |
| **P1** | Ball+Hall wand (Prototype 3) | x1 | Validates pen-on-any-surface concept |

P0 is the gate. If the optical ring pair works as a mouse, everything else
follows. If it doesn't, we learn why before spending more.
