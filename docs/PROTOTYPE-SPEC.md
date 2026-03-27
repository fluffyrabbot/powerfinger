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
| Sensor | 5mm steel ball in socket, 4 roller spindles with magnets, 4 SMD Hall sensors (DRV5053, power-gated) |
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

Pen grip, drag on any surface. The ball at the tip must accommodate 30–70
degree pen angles from horizontal. The ball+Hall sensor is angle-independent —
unlike optical pen mice that fail past ~20 degrees, this works at any natural
pen grip angle. No commercial pen mouse offers surface agnosticism + angle
independence + BLE HID (no dongle). See [WAND-COMPETITIVE.md](WAND-COMPETITIVE.md)
for the full competitive analysis.

---

## USB Hub Dongle (x1)

| Component | Spec |
|-----------|------|
| MCU | ESP32-S3 (BLE 5.0 + native USB OTG) |
| Role | BLE central — pairs with all rings/wands over BLE |
| Output | USB HID mouse — single unified mouse to the host OS |
| Connector | USB-A or USB-C |
| Enclosure | 3D-printed, small USB-stick form factor |
| BOM target | ~$5-6 |

The hub pairs with multiple PowerFinger devices over BLE, assigns roles
(first ring = cursor + left click, second = scroll + right click), composes
their events into a single USB HID mouse report, and presents to the OS as
one mouse. No companion app required. No drivers. Works on any OS with USB.

A single ring also works without the hub — pairs directly with the host as a
standard BLE HID mouse for basic cursor + click.

---

## Fixed Across All Devices

- **Ring/wand MCU:** ESP32-C3 (BLE 5.0, supports 7.5ms connection intervals,
  ESP-IDF framework)
- **Hub MCU:** ESP32-S3 (BLE 5.0 central + USB OTG device)
- **Protocol:** Each ring is a BLE HID mouse peripheral. The hub is a BLE
  central + USB HID device. Single ring works standalone (BLE HID to host).
  Multi-ring works through the hub (BLE to hub, USB to host).
- **License:** MIT firmware, CERN-OHL-S 2.0 hardware (open source, your name on
  it if you want it)
- **No cloud dependency.** No app required for single-ring or multi-ring
  function. Companion app is optional configuration UI.
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

- **LDO: RT9080-33GJ5** (not AP2112K). 0.5µA quiescent vs 55µA — the LDO
  dominates deep sleep power. See [POWER-BUDGET.md](POWER-BUDGET.md).
- **Charge resistor: 20kΩ** (not 2kΩ or 10kΩ). Sets 50mA charge rate (0.625C on
  80mAh). 10kΩ (100mA) produces unsafe temperatures in the sealed ring enclosure
  — see [BATTERY-SAFETY.md](BATTERY-SAFETY.md) §5 thermal analysis.
- **Hall sensor: DRV5053** (not SS49E). ~3mA vs 6mA per sensor. Must be
  power-gated via MOSFET in sleep.
- **BOM ceiling:** No single variant may exceed $25. The cheapest ring must stay
  under $10.
- **BLE HID latency < 15ms.** Default 15ms connection interval with adaptive
  switch to 7.5ms during active tracking.
- **Ring height budget: ~10mm** finger-to-surface. Sensor + PCB + battery must
  fit in ~7-8mm above surface with 2.5-3mm air gap below.
- **DPI/sensitivity must be configurable** via BLE characteristic, never
  hard-coded.
- **Ring geometry must be parametric** for finger size — never assume a specific
  hand.
- **All parts must be sourceable from multiple vendors.** No single-source
  components.

---

## Battery Safety

Full analysis: [BATTERY-SAFETY.md](BATTERY-SAFETY.md)

### Cell specification

All LiPo cells **must have an integrated protection circuit module (PCM)**
providing overcharge (4.25-4.30V), over-discharge (2.4-2.5V), and short-circuit
protection. Bare/unprotected cells are rejected regardless of cost savings. Cells
must have UN 38.3 test documentation from the manufacturer.

### Charge rate constraint

**RPROG must be 20 kohm (not 10 kohm).** This sets 50 mA charge current (0.5C
on 100mAh, 0.625C on 80mAh). The ring is a sealed enclosure worn on skin with
no ventilation. At 100 mA (RPROG = 10 kohm), the TP4054 dissipates up to 200 mW,
which in the ring's thermal environment produces an estimated 50 deg C
temperature rise above body temperature — reaching ~87 deg C internally.
This exceeds both the cell's 45 deg C charge limit and IEC 62368-1's 48 deg C
skin contact limit (Clause 9.1.2, Table 42, non-metal continuous contact).

At 50 mA the worst-case temperature rise drops to ~25 deg C. Charge time
increases from ~1 hour to ~2 hours — acceptable for a device charged overnight.

### Thermal limits

| Limit | Value | Source |
|-------|-------|--------|
| Max skin-contact surface temperature | 48 deg C | IEC 62368-1 Table 42 (non-metal, continuous) |
| Max cell charging temperature | 45 deg C | Cell manufacturer spec (typical LiPo) |
| Min cell charging temperature | 0 deg C | Below this, lithium plating occurs |

### Hardware additions for safety

| Component | Purpose | BOM impact |
|-----------|---------|-----------|
| NTC 10k B3950 (0402) + divider resistor | Cell temperature monitoring via ADC | +$0.03 |
| SI2301 P-ch MOSFET (SOT-23) + pull-up | Firmware charge enable/disable on VBUS | +$0.04 |
| RPROG change: 10 kohm to 20 kohm | Reduce charge current from 100 mA to 50 mA | $0.00 |

The TP4054 has **no NTC input and no temperature monitoring**. The firmware must
fill this gap using the NTC thermistor and VBUS MOSFET.

### Firmware requirements

- **Temperature monitoring during charging:** Read NTC via ADC. Disable charging
  (cut VBUS via MOSFET) at T >= 45 deg C. Re-enable at T < 40 deg C (5 deg C
  hysteresis). Disable charging at T < 0 deg C.
- **Overvoltage detection:** If VBAT > 4.25V, disable charging. This indicates
  a TP4054 fault or cell PCM failure.
- **Critical low voltage:** If VBAT < 3.0V, enter deep sleep immediately
  (existing 3.2V cutoff handles the normal case).
- **Charge state tracking:** Detect USB via VBUS GPIO; monitor TP4054 CHRG pin.
  Stay awake during charging to maintain temperature monitoring.
- **Cell health logging:** Track charge cycles, max temperature, and fault
  events in NVS. Expose via BLE diagnostic characteristic.

### Shell requirements

- **Non-hermetic closure.** Friction-fit or snap-fit that separates under
  internal pressure. A sealed shell could build pressure during a cell vent
  event.
- **Minimum 1.0mm wall thickness** over the cell for mechanical puncture
  protection.
- **Swelling detection.** Cell swelling must be tactilely or visually detectable
  through the shell.

---

## Build Order

| Priority | What | Quantity | Why |
|----------|------|----------|-----|
| **P0** | Optical ring pair (Prototype 1) | x2 | Validates core concept: two identical rings = mouse |
| **P0** | USB hub dongle | x1 | Validates multi-ring composition — two rings = one USB mouse |
| **P1** | Ball+Hall ring pair (Prototype 2) | x2 | Validates surface-agnostic sensing in ring form factor |
| **P1** | Ball+Hall wand (Prototype 3) | x1 | Validates pen-on-any-surface concept |

P0 is the gate. If the optical ring pair works as a mouse through the hub,
everything else follows. The hub is P0 because multi-ring composition is the
core product concept — without it, you just have two independent mice.
