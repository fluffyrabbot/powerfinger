# PowerPuck Specification — Desktop Fingertip Mouse

Two small disc-shaped BLE HID mice that rest on a desk surface, operated by
placing fingertips on top and sliding. The PowerPuck resolves the ring form
factor's fundamental keyboard-coexistence problem: the user lifts their fingers
to type, places them back to point. No donning, no doffing, no mode switch.

**Form factor prefix:** P (design-space matrix)
**Product name:** PowerPuck
**Firmware path:** `firmware/puck/`
**Kconfig:** `POWERFINGER_FORM_FACTOR=PUCK`

---

## Why This Form Factor Exists

The fingertip ring (R-00 through R-45) requires the user to either remove the
ring or lift the finger away from the keyboard to type. The nail mount (N-30,
N-45) is the only keyboard-compatible ring variant, and it remains at concept
stage with extreme height constraints (≤3–4mm above nail). Every other ring
variant imposes a mode switch penalty on every typing↔pointing transition.

The PowerPuck eliminates this by moving the device off the finger and onto the
desk. The user's fingers are unencumbered — they type normally, then rest
fingertips on the pucks to point. The interaction model after contact is
identical to the ring: slide to move, press to click, two devices compose into
a full mouse via the hub.

**What the puck gives up vs. the ring:**

- **Passive retention.** The ring stays on the finger regardless of hand
  position. The puck must be found and touched. Users who point while their hand
  is in the air (presenting, gesturing) need the ring or wand, not the puck.
- **Portability.** The ring travels with the user. The puck lives on a desk.
- **Off-surface use.** The ring with IMU hybrid can track in air. The puck is
  inherently surface-bound.

**What the puck gains:**

- **Zero keyboard interference.** Lift fingers, type, put them back.
- **No sizing.** No parametric finger circumference. One device fits all hands.
- **Guaranteed focal distance.** The device rests on the surface — the sensor is
  always at the correct height. The ring's entire glide system exists to solve a
  problem the puck doesn't have.
- **Rigid PCB.** No flex or rigid-flex circuits. Dramatically cheaper and more
  reliable PCB fabrication.
- **Relaxed weight budget.** Not worn — 12–20g is fine. Enables larger battery
  (150–200mAh) without ergonomic penalty.
- **Simpler click.** Press the whole device down. Natural, intuitive, no
  side-pinch or dome-under-PCB contortion.
- **Simpler manufacturing.** Rigid PCB in a simple disc shell. No parametric
  geometry, no flex circuits, no raised rim focal distance tuning.

---

## Variants

The puck sits flat on a surface. There is no sensor angle variation (unlike the
ring's R-00 through R-45). Variants differ only in sensing mechanism.

| Variant ID | Sensing | Click | BOM Target | Notes |
|------------|---------|-------|------------|-------|
| **P-OLED-NONE-NONE** | PAW3204 optical | Snap dome (whole-puck press) | ~$7 | P0 prototype — cheapest proof of concept |
| **P-BALL-NONE-NONE** | Ball+Hall (5mm, 4x DRV5053) | Snap dome (whole-puck press) | ~$11 | Surface-agnostic variant |
| **P-OPTB-NONE-NONE** | PMW3360 optical-on-ball | Snap dome or piezo sealed | ~$15 | Pro variant — all surfaces, high CPI |
| **P-IMU-NONE-NONE** | 6-axis IMU only | Snap dome | ~$5 | Air mouse puck (niche — puck implies surface use) |

All optional capabilities from the ring matrix (IMU hybrid, camera, laser)
apply identically. The form factor prefix `P` replaces `R30` in the naming
convention.

**P0 prototype recommendation:** Build `P-OLED-NONE-NONE` × 2. This is the
cheapest path to validating the form factor concept. If the two-puck mouse
workflow works on a desk, the ball and optical-on-ball variants follow.

---

## Physical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| Shape | Circular disc, domed or concave top | Concave top cups the fingertip for proprioceptive feedback |
| Diameter | 22–28mm | Must be wide enough for lateral stability under finger force; must be small enough to fit between keyboard and monitor edge |
| Height | 10–15mm | Low enough for comfortable extended fingertip rest; high enough for battery + PCB + sensor + click mechanism |
| Weight | 12–18g | Heavy enough to stay put when not in use; light enough to slide without fatigue. Heavier than a ring (8–12g), lighter than any mouse |
| Top surface | Concave with textured silicone or TPU grip | Prevents finger slipping off during lateral movement |
| Bottom surface | Flat with UHMWPE glide pads | Same pad material as ring glide system; 3–4 discrete pads around sensor aperture |
| Shell material | 3D-printed (prototype), injection-molded ABS/PC (production) | No parametric sizing needed — one shell design |
| Closure | Non-hermetic snap-fit | Same battery safety requirement as ring: must separate under internal pressure |
| Minimum wall thickness | 1.0mm over cell | Mechanical puncture protection per BATTERY-SAFETY.md |

### Profile

```
CROSS-SECTION — PowerPuck on desk surface

         ┌──────────────────┐
        ╱   concave top      ╲   ← fingertip rests here
       │  ┌──────────────┐    │
       │  │  LiPo 150mAh │    │
       │  ├──────────────┤    │
       │  │  rigid PCB    │    │
       │  │  ESP32-C3     │    │
       │  │  sensor ◉     │    │
       │  └──────────────┘    │
       ▓▓                    ▓▓  ← shell wall
       ░░░░░░░░░░░░░░░░░░░░░░  ← UHMWPE glide pads
       ════════════════════════  desk surface
          ↕ 2.4–3.2mm
          focal distance (optical variants)
```

### Bottom View

```
BOTTOM VIEW — PowerPuck underside

           ░░░░░░
        ░░        ░░
      ░░    ◉       ░░    ◉ = sensor aperture
      ░░  sensor    ░░    ░ = UHMWPE glide pads
        ░░        ░░          (4 discrete pads)
           ░░░░░░

      Diameter: 22–28mm
```

### Retention and Findability

The puck has no passive retention — it doesn't stay on the user's finger. This
creates two design concerns:

1. **Drift during non-use.** The puck can be knocked away or drift on a sloped
   surface. Mitigations:
   - Weight (12–18g) provides inertia against casual bumps
   - UHMWPE pad friction provides static resistance when no finger is applied
   - A magnetic dock/cradle (optional accessory) holds pucks in a fixed position
     near the keyboard when not in use
   - A silicone base ring (optional) increases static friction on smooth surfaces

2. **Findability after typing.** The user must locate the puck by touch after
   looking away. Mitigations:
   - Fixed desk position (muscle memory develops quickly — same as reaching for
     a mouse)
   - Concave top provides tactile landmark
   - Magnetic dock establishes a consistent home position
   - Two pucks should be positioned symmetrically relative to the keyboard (one
     per side, or both on the dominant side) — user testing will determine
     preferred placement

**Honest limitation:** For users who frequently alternate between typing and
pointing in rapid succession (e.g., coding with frequent cursor repositioning),
the findability overhead may be annoying compared to a trackpad or ThinkPad
nub, which are always under the fingers. The puck optimizes for sustained
pointing sessions with discrete typing breaks, not interleaved character-by-
character editing. Document this honestly in user-facing materials.

---

## Electrical Architecture

### MCU and Power

Identical to the ring and wand. The puck is electrically a ring with a different
shell and a rigid PCB.

| Component | Part | Purpose |
|-----------|------|---------|
| MCU | ESP32-C3-MINI-1-N4 (or -N4X successor) | BLE HID, sensor polling, power management |
| LDO | RT9080-33GJ5 (0.5µA Iq) | 3.3V regulation; ultra-low quiescent for deep sleep |
| Charge controller | TP4054 | USB-C LiPo charging |
| NTC | 10kΩ NTC thermistor | Charge temperature monitoring |
| Charge gate | SI2301 P-ch MOSFET | Charge path control |
| VBAT divider | 10MΩ / 10MΩ resistor divider | Battery voltage monitor (0.165µA draw) |
| Battery | LiPo 150–200mAh | Larger than ring (80–100mAh) — weight budget allows it |

**Battery note:** The ring's 80–100mAh constraint comes from the 10mm height
budget. The puck's 10–15mm height and 22–28mm diameter accommodate cells up to
200mAh without difficulty. The extra capacity partially compensates for the
ESP32-C3's power consumption, extending battery life to a usable range without
requiring the nRF52840 migration for the prototype phase.

### Sensor Subsystem (P-OLED Variant — P0)

| Component | Part | Qty | Purpose |
|-----------|------|-----|---------|
| Optical sensor | PAW3204 / ADNS-2080 class | 1 | Surface tracking, 800–2000 CPI |
| LED | Integrated in sensor package | 1 | Surface illumination |

**Sensor position:** Bottom-center of the PCB, facing down through an aperture
in the shell floor. The shell floor sets the focal distance (2.4–3.2mm) — same
principle as the ring's raised rim, but simpler because the puck sits flat on
the surface with its full weight ensuring consistent contact.

**Focal distance is self-maintaining.** Unlike the ring (where finger pressure
and tilt vary the rim-to-surface distance), the puck rests on the surface under
gravity. The UHMWPE pads define a fixed standoff height. Tilt is minimal because
the puck's diameter (22–28mm) is wide relative to any finger-induced torque.

### Sensor Subsystem (P-BALL Variant)

Identical to ring Prototype 2 and PowerPen tip assembly:

| Component | Part | Qty | Purpose |
|-----------|------|-----|---------|
| Steel ball | 5mm G25 chrome steel | 1 | Contact element, protrudes through bottom |
| Roller spindles | Custom (magnet-tipped) | 4 | Translate ball rotation to magnetic field changes |
| Magnets | 1x1mm N52 neodymium | 4 | On roller spindle tips |
| Hall sensors | DRV5053 (analog, ratiometric) | 4 | Read magnetic field from each roller |
| Hall power gate | SI2301 P-ch MOSFET | 1 | GPIO-controlled VCC rail for 4x DRV5053 |

**Ball position:** Bottom-center, protruding ~1–2mm below the shell. No UHMWPE
pads needed for the ball variant — the ball itself is the contact point. Three
small standoff feet at 120° prevent the puck from rocking when the ball is not
under load.

**Wake sensor (ball variant only):** DRV5032FB ultra-low-power digital Hall
switch, same circuit as PowerPen. Positioned adjacent to one roller spindle
magnet. Provides motion-based deep sleep wake. Same spurious wake debounce
firmware as the PowerPen (see POWERPEN-SPEC.md §Wake Sensor Circuit).

The optical variant does not need a DRV5032 — the PAW3204 has a built-in motion
detect pin that can wake the MCU from deep sleep directly.

### Click Subsystem

The puck's click mechanism is its simplest advantage over the ring. The entire
device is a button.

**Primary mechanism: whole-puck press-down.**

The PCB is mounted on a snap dome (or pair of domes) inside the shell. When the
user presses down on the puck, the shell compresses against the dome(s),
registering a click. The entire top surface is the actuator.

| Component | Part | Actuation | GPIO | Purpose |
|-----------|------|-----------|------|---------|
| Primary click | Metal snap dome (Snaptron SQ, 8–10mm) | 100–170gf press-down | Click GPIO | Left click (default) |

**Actuation force:** 100–170gf. Lower than the ring's dome (which competes with
finger curl force) because the puck press is a simple downward push with
gravity assist. Must be high enough to prevent accidental clicks during lateral
sliding — the dome should not actuate from normal tracking force.

**Click-during-tracking interaction:** Pressing down on the puck will cause
slight lateral displacement (finger slides forward as it pushes down). Dead zone
is required during click actuation, same logic as the ring. The dead zone window
may be shorter than the ring's because the puck's mass provides more inertia
against lateral displacement than the ring's finger-mounted mass.

**Two-puck click topology:** Same as two-ring. Each puck has one click. The hub
assigns roles:
- Puck 1 (dominant hand): cursor + left click
- Puck 2 (non-dominant hand or adjacent finger): scroll + right click

Roles are software-assigned and reconfigurable via hub or companion app.

**Alternative click: side squeeze.** A secondary click can be added via a force-
sensitive resistor (FSR) or second dome on the puck's sidewall, activated by
pinching. This is optional and not part of the P0 prototype — the single press-
down click is sufficient for concept validation. If users need right-click on a
single puck, side squeeze is the candidate mechanism.

### Pin Assignment (Preliminary — P-OLED Variant)

| GPIO | Function | Notes |
|------|----------|-------|
| GPIO0 | Sensor SPI CS | PAW3204 chip select |
| GPIO1 | SPI MOSI | Shared SPI bus |
| GPIO2 | SPI MISO | |
| GPIO3 | SPI CLK | |
| GPIO4 | Click dome | Internal pull-up, active-low, wake-capable |
| GPIO5 | Sensor motion detect | PAW3204 MOTION pin, active-low, wake-capable |
| GPIO6 | Reserved | Future: side squeeze FSR ADC |
| GPIO7 | VBAT ADC | Battery voltage monitor via divider |
| GPIO10 | USB D- | USB-C (charging + serial flash) |
| GPIO18 | SDA (I2C, future IMU) | Reserved for IMU hybrid |
| GPIO19 | SCL (I2C, future IMU) | Reserved for IMU hybrid |

**Pin assignment for P-BALL variant** follows the PowerPen layout (GPIO0–3 for
4x Hall ADC, GPIO6 for Hall VCC gate, GPIO8 for DRV5032 wake). See
POWERPEN-SPEC.md §Pin Assignment.

---

## Power Budget (Pre-Hardware Estimate)

The puck's larger battery (150–200mAh vs ring's 80–100mAh) is its primary power
advantage. Active power draw is identical to the ring — same MCU, same sensors,
same BLE stack.

### P-OLED Variant (PAW3204)

| State | Components Active | Estimated mA | Duration Model |
|-------|-------------------|-------------|----------------|
| Active tracking | MCU + BLE (15ms) + PAW3204 | ~8–12mA | 3 hrs/day |
| Active tracking (low-latency) | MCU + BLE (7.5ms) + PAW3204 | ~15–20mA | Burst during motion |
| Connected idle | MCU + BLE (15ms) + sensor sleep | ~3–5mA | 5 hrs/day |
| Deep sleep | LDO Iq + motion detect GPIO | ~6–10µA | 16 hrs/day |

**Estimated battery life (150mAh, 3 hrs active / 5 hrs idle / 16 hrs sleep):**

| Usage | 150mAh | 200mAh |
|-------|--------|--------|
| Heavy (4 hrs/day active) | ~2.5 days | ~3.5 days |
| Moderate (2 hrs/day active) | ~5 days | ~7 days |
| Light (1 hr/day active) | ~9 days | ~12 days |

This is 50–100% better than the ring's optical variant on 80mAh. The puck's
larger battery makes ESP32-C3 battery life adequate for the prototype phase.

### P-BALL Variant (4x DRV5053)

| State | Components Active | Estimated mA | Duration Model |
|-------|-------------------|-------------|----------------|
| Active tracking | MCU + BLE (15ms) + 4x Hall | ~18–22mA | 3 hrs/day |
| Active tracking (low-latency) | MCU + BLE (7.5ms) + 4x Hall | ~25–30mA | Burst during motion |
| Connected idle | MCU + BLE (15ms) + Hall gated | ~3–5mA | 5 hrs/day |
| Deep sleep | LDO Iq + DRV5032 wake | ~17–22µA | 16 hrs/day |

**Estimated battery life (150mAh):** ~3–5 days moderate use. Comparable to the
PowerPen on 100–150mAh, with the larger battery compensating for higher active
draw.

---

## Firmware Architecture — Sharing Strategy

The PowerPuck shares the vast majority of its firmware with the ring and
PowerPen. Same one-codebase, build-time-configuration approach.

### Shared with Ring (No Changes)

| Component | Location | Notes |
|-----------|----------|-------|
| HAL layer | `ring/components/hal/` | GPIO, SPI, ADC, timer, sleep, storage, OTA, BLE |
| BLE HID | `ring/components/ble_hid/` | Same HID report descriptor, same GAP handler |
| Sensor interface | `ring/components/sensors/sensor_interface.h` | Optical and ball+Hall drivers implement same interface |
| Calibration | `ring/components/click/calibration.c` | Sensor zero-offset calibration |
| Dead zone | `ring/components/click/dead_zone.c` | Click-during-tracking suppression (after PP1 instanceability refactor) |
| Test suite | `firmware/test/` | Same host-side test harness |

### Shared with Ring (Already Extended for PowerPen)

The PowerPen's PP1 task (shared interface extensions) benefits the puck directly:

| Component | Extension | Puck Benefits |
|-----------|-----------|---------------|
| Click interface | `click_source_t` enum + multi-source support | Puck uses `CLICK_SOURCE_PRIMARY` (press-down dome). Future side-squeeze uses `CLICK_SOURCE_SECONDARY` |
| Dead zone | Instanceable `dead_zone_ctx_t` | Puck allocates one instance (press-down: dead zone enabled) |
| Power manager wake | Kconfig-driven `CONFIG_POWERFINGER_WAKE_GPIO_MASK` | Optical puck: dome + motion detect (2 GPIOs). Ball puck: dome + DRV5032 (2 GPIOs) |
| Diagnostics | Generic `device_state` | Puck reports as `FORM_FACTOR=PUCK` |

### PowerPuck-Specific (Configuration Only)

| Concern | What Changes | Approach |
|---------|-------------|----------|
| Pin assignments | Different GPIO map (simpler than pen — no barrel switch, no tip dome) | Kconfig / `sdkconfig.defaults` per target |
| Wake GPIO bitmask | Two wake sources (dome + sensor motion or DRV5032) | `CONFIG_POWERFINGER_WAKE_GPIO_MASK` in Kconfig |
| Click topology | Single press-down dome | Simplest click config — one source, dead zone enabled |
| Device identity | BLE device name, appearance | Kconfig: "PowerPuck" |
| Battery capacity | 150–200mAh (affects low-battery thresholds) | Kconfig: `CONFIG_POWERFINGER_BATTERY_MAH` |

### Build Configuration

```
firmware/
  ring/         ← existing ring build target
  pen/          ← PowerPen build target
  puck/         ← PowerPuck build target
    CMakeLists.txt       ← pulls shared components from ring/, adds puck config
    sdkconfig.defaults   ← puck-specific pin map, device name, click topology
    main/
      main.c             ← puck entry point (likely identical to ring main.c)
    Kconfig              ← POWERFINGER_FORM_FACTOR=PUCK
```

The puck `CMakeLists.txt` uses `EXTRA_COMPONENT_DIRS` to include shared
components from `ring/components/`. No code duplication.

**Note:** The puck firmware is the simplest of the three form factors. The ring
has parametric geometry concerns in its sensor driver (angle compensation). The
pen has multi-button click topology and DRV5032 wake debounce. The puck is a
flat device with one button and a sensor — it's the ring firmware with fewer
features, not more.

---

## State Machine

The PowerPuck reuses the ring state machine unchanged. The puck's simplicity
means it exercises fewer features of the state machine, not more.

```
                         gpio_interrupt
  ┌────────────┐      (dome press or sensor    ┌──────────┐
  │ DEEP_SLEEP │ ────  motion detect)         ►│ BOOTING  │
  └────────────┘                               └────┬─────┘
       ▲                                            │ calibration_complete
       │ sleep_timeout                              ▼
       │ (or low battery)                    ┌──────────────┐
       │                    ble_timeout      │ ADVERTISING  │◄── ble_disconnected
       │◄────────────────────────────────────└──────┬───────┘
       │                                            │ ble_connected
       │                                            ▼
       │                    no activity       ┌─────────────────┐
       │      ┌──────────── for 250ms ────────│CONNECTED_ACTIVE │
       │      ▼                               └────────▲────────┘
  ┌────────────────────┐                               │
  │ CONNECTED_IDLE     │──── motion or click ──────────┘
  └────────────────────┘
```

**Wake sources for deep sleep:**
- Click dome (GPIO4) — user presses puck down
- Sensor motion detect (GPIO5, optical variant) — user slides puck
- DRV5032 wake sensor (GPIO8, ball variant) — ball rotation triggers Hall switch

**Idle/sleep timeout tuning:** The puck may need longer idle-to-sleep timeouts
than the ring. A ring on a finger is clearly "in use" or "not in use." A puck
sitting on a desk might be temporarily idle while the user types a sentence.
Default: 60 seconds connected idle → deep sleep (vs ring's 30 seconds).
Configurable via BLE characteristic.

---

## PowerPuck-Specific Firmware Tasks

These assume the ring firmware through Phase 3 and the PowerPen's PP1 (shared
interface extensions) are complete. The puck benefits from all shared work
already done for the pen. Tasks prefixed "PK" (PowerPuck).

### PK1 — PowerPuck Build Target

Stand up `firmware/puck/` with CMakeLists.txt that pulls shared components.
Kconfig sets `POWERFINGER_FORM_FACTOR=PUCK`, device name "PowerPuck", puck pin
map, two-GPIO wake mask. Verify `idf.py build` produces a binary. Flash to
ESP32-C3 dev board. Confirm BLE HID pairing works.

**Done when:** `cd firmware/puck && idf.py build` succeeds and the binary pairs
as "PowerPuck" on macOS/Windows/Linux.

### PK2 — Optical Sensor Integration

Wire PAW3204 breakout to the puck build. Confirm cursor tracking works. The
sensor faces straight down (0° angle — no angle compensation needed). Test on
standard surface matrix.

**Done when:** Cursor tracks on wood, paper, mouse pad, and matte plastic.

### PK3 — Press-Down Click Integration

Wire snap dome between PCB and a test shell floor. Verify that pressing the
whole assembly down registers a click. Tune dead zone threshold — the puck's
mass means lateral displacement during click should be smaller than the ring's.

**Done when:** Press-down click produces left-click HID event. Dead zone
prevents cursor jump during click. Lateral slide does not accidentally trigger
click.

### PK4 — Idle/Sleep Timeout Tuning

Test the puck in a realistic typing↔pointing workflow. Measure how long the
user's fingers are off the puck during typical typing bursts. Set the idle-to-
sleep timeout so the puck doesn't deep-sleep during normal typing pauses.

**Done when:** Typing for up to 60 seconds does not trigger deep sleep. Puck
reconnects within 500ms (bonded peer) after typing pause.

### PK5 — Two-Puck Composition via Hub

Connect two PowerPuck devices to the hub. Verify role assignment (cursor +
left click on puck 1, scroll + right click on puck 2) and event composition
into single USB HID report.

**Done when:** Two pucks function as a complete mouse through the hub. Cursor
movement, left click, right click, and scroll all work.

### PK6 — Shell Prototyping and Ergonomic Tuning

3D-print test shells at 22mm, 25mm, and 28mm diameter. Test concave top
profiles (shallow, medium, deep). Evaluate:
- Fingertip comfort during sustained use (30+ minutes)
- Lateral stability under fast movement
- Accidental click rate during tracking (dome actuation force tuning)
- Findability after typing (eyes-off return to puck position)
- Acoustic noise on wood, glass, and mouse pad surfaces

**Done when:** A preferred diameter and top profile are selected with documented
rationale.

---

## Hub Compatibility

The PowerPuck is a standard PowerFinger BLE peripheral, identical to a ring or
PowerPen from the hub's perspective. The hub's BLE central, role engine, and
event composer work unchanged.

**Multi-device composition:** Any combination of PowerFinger devices works
through the hub:
- 2 pucks (standard desktop mouse replacement)
- 1 puck (cursor) + 1 ring (scroll/modifier)
- 1 puck (cursor) + 1 PowerPen (precision/drawing)
- 1 puck + 1 ring + 1 PowerPen (three-role composition)

The hub does not need to know the form factor of connected devices. It sees BLE
HID peripherals reporting X/Y deltas and button states.

---

## Surface Test Matrix (P-OLED Variant)

The optical puck has the same surface compatibility as the optical ring. The
puck's advantage is not surface range — it's ergonomics and manufacturing.

| Surface | Expected | Confidence | Notes |
|---------|----------|------------|-------|
| Wood desk | Works | High | Primary use case |
| Matte plastic | Works | High | |
| Paper | Works | High | |
| Mouse pad | Works | High | Optimal surface |
| Glossy magazine | Marginal | Medium | Specular reflection may confuse sensor |
| Glass | No | High | Direct optical fails on transparent surfaces |
| Mirror | No | High | |
| Fabric / clothing | Marginal | Medium | Not a primary puck use case (desk device) |
| Skin | Not applicable | — | Puck is a desk device, not a body device |

For surface-agnostic puck use, build the P-BALL or P-OPTB variant.

---

## Accessibility Analysis

The PowerPuck serves a different accessibility population than the ring.

### Who the puck helps (vs. ring)

- **Keyboard-heavy users with RSI/carpal tunnel.** The puck eliminates the
  ring's donning/doffing overhead. Users who alternate frequently between
  typing and pointing benefit most.
- **Users who cannot wear rings.** Edema, joint swelling, skin sensitivity,
  bandaged fingers, prosthetic fingers — any condition that prevents wearing a
  ring on the fingertip.
- **Users with mild tremor.** The puck's weight (12–18g) and surface contact
  provide natural stabilization. The desk surface acts as a damper.
- **Users who need a low-learning-curve alternative to a standard mouse.** The
  puck interaction model (slide a thing on a desk) is familiar. The ring
  interaction model (drag your fingertip) is novel.

### Who should use the ring instead

- **Users who cannot maintain hand position over a desk.** Severe tremor,
  involuntary movement, or weakness that prevents reliably placing and keeping
  a fingertip on a small disc.
- **Users who need off-desk pointing.** Wheelchair users without a tray table,
  bedbound users, presenters. The ring stays on the finger; the puck stays on
  the desk.
- **Users who need continuous cursor control through typing.** The ring allows
  moving the cursor while the other hand types. The puck requires a discrete
  switch between typing and pointing.

### Accessibility-specific design requirements

- **Dome actuation force must be configurable.** Default 100–170gf is too high
  for some users with weakness. A lower-force dome option (50–80gf) should be
  available, with firmware dead zone widened to compensate for increased
  accidental click risk.
- **Puck diameter affects usability for users with impaired proprioception.**
  Larger diameter (28mm) is easier to locate by touch. Smaller diameter (22mm)
  is more discreet and portable. Offer both sizes.
- **Idle-to-sleep timeout matters more for users with slow transitions.** Users
  with Parkinson's bradykinesia or spasticity may take longer to transition
  from typing back to pointing. Default 60-second timeout accommodates this;
  make it configurable up to 300 seconds via BLE characteristic.
- **Audible feedback option.** For users who cannot feel the dome click (numb
  fingertips, neuropathy), a short buzzer or speaker beep on click confirms
  actuation. This is a firmware feature, not a hardware change — any piezo
  element or LRA can produce an audible tick.

---

## BOM Estimate — P-OLED-NONE-NONE (P0 Prototype)

| Component | Part | Cost |
|-----------|------|------|
| MCU | ESP32-C3-MINI-1-N4 | $2.01 |
| Sensor | PAW3204 / ADNS-2080 class optical | $0.60–1.80 |
| Click | Metal snap dome (Snaptron SQ, 8–10mm) | $0.05–0.15 |
| Battery | LiPo 150–200mAh | $1.00–2.00 |
| Charging | TP4054 + USB-C connector | $0.15–0.35 |
| LDO | RT9080-33GJ5 | $0.15 |
| Glide | UHMWPE pad set (4 pads) | $0.05–0.10 |
| Shell | 3D-printed disc (non-parametric, one design) | $0.30–0.80 |
| PCB | **Rigid** (not flex — circular, ~20mm diameter) | $0.50–1.50 |
| NTC + MOSFET | Battery safety per BATTERY-SAFETY.md | $0.07 |
| VBAT divider | 10MΩ / 10MΩ | $0.02 |
| **Total** | | **~$5.00–9.00 (~$7 target)** |

**Two-puck system BOM:** ~$14 + hub ($6) = ~$20.

The puck is the cheapest PowerFinger variant. The rigid PCB ($0.50–1.50 vs
$2–5 for flex/rigid-flex) and non-parametric shell ($0.30–0.80 vs $0.50–1.00)
are the primary savings. Under the $10 BOM floor for the cheapest variant.

### BOM Estimate — P-BALL-NONE-NONE

| Component | Part | Cost |
|-----------|------|------|
| MCU | ESP32-C3-MINI-1-N4 | $2.01 |
| Ball + rollers + magnets | 5mm G25 + 4 custom spindles + 4x 1x1mm N52 | $1.00–1.50 |
| Hall sensors | 4x DRV5053 | $0.80–1.20 |
| Wake sensor | DRV5032FB | $0.30 |
| Hall power gate | SI2301 P-ch MOSFET | $0.04 |
| Click | Metal snap dome | $0.05–0.15 |
| Battery | LiPo 150–200mAh | $1.00–2.00 |
| Charging | TP4054 + USB-C | $0.15–0.35 |
| LDO | RT9080-33GJ5 | $0.15 |
| Shell | 3D-printed disc + ball socket | $0.40–1.00 |
| PCB | Rigid, ~20mm diameter | $0.50–1.50 |
| NTC + MOSFET | Battery safety | $0.07 |
| VBAT divider | 10MΩ / 10MΩ | $0.02 |
| **Total** | | **~$6.50–12.00 (~$9 target)** |

**Two-puck system BOM (ball):** ~$18 + hub ($6) = ~$24. Same as the optical ring
pair system.

---

## Consumer Tier Mapping

The puck maps naturally onto the same Standard/Pro tier structure as the ring
(see CONSUMER-TIERS.md). The same four points of divergence apply:

| Tier | Variant | Sensor | Click | BOM | Retail (est.) |
|------|---------|--------|-------|-----|---------------|
| **Standard** | P-OLED-NONE-NONE | PAW3204 optical | Snap dome | ~$7 | ~$22–28 |
| **Pro** | P-OPTB-NONE-NONE | PMW3360 optical-on-ball | Piezo sealed | ~$15 | ~$45–58 |

**Two-puck system retail:**
- Standard: ~$14 + hub = ~$59–75
- Pro: ~$30 + hub = ~$99–129

The puck system is cheaper than the ring system at both tiers due to rigid PCB
and non-parametric shell. It's the most affordable entry point into the
PowerFinger ecosystem.

---

## Open Questions

1. **Optimal diameter.** 22mm vs 25mm vs 28mm — fingertip comfort, lateral
   stability, and desk footprint are in tension. Requires user testing with
   printed shells.

2. **Top surface profile.** Concave (cups fingertip), flat (simple), or convex
   (domed, like a trackball inverted). Concave is the hypothesis for best
   proprioceptive feedback. Needs validation.

3. **Click force threshold.** The press-down dome must be strong enough to not
   actuate during lateral sliding but weak enough for comfortable clicking.
   The ring's dome force (100–200gf) may need adjustment — the puck's
   mass changes the force dynamics. Bench test required.

4. **Magnetic dock.** Is a dock/cradle necessary for findability, or does muscle
   memory suffice? If a dock is needed, should it be a passive magnet in a
   3D-printed holder, or an active charging cradle (wireless charging adds
   $2–3 BOM)?

5. **Single-puck viability.** Can one puck with side-squeeze for right-click
   serve as a complete mouse without the hub? This would be the simplest, most
   affordable PowerFinger product — a single $7 BOM device that replaces a
   mouse for basic use.

6. **Puck-on-glass.** The optical puck fails on glass (same as the optical
   ring). For users with glass desks, the answer is the P-BALL or P-OPTB
   variant — or a mouse pad. Is this acceptable, or should the P0 prototype
   include a ball variant?

7. **Two-puck vs one-puck-plus-ring.** The canonical system is two identical
   devices. But a puck (cursor) + ring (modifier/scroll, worn on another
   finger) might be a better combination for some users — the puck handles
   the bulk pointing on the desk while the ring provides the secondary input.
   This is a usage pattern to explore, not a hardware design question.

8. **Shared component extraction timing.** The puck build target is the third
   firmware target (after ring and pen). Three targets sharing components from
   `ring/components/` makes the extraction to `firmware/shared/` more urgent.
   Consider extracting before standing up the puck build target.

9. **Naming collision.** Apple's 2000 "Hockey Puck" mouse (officially the
   Apple USB Mouse) was widely considered one of the worst mice ever designed.
   The PowerPuck is a fundamentally different device (fingertip-scale, BLE,
   two-device composition), but the "puck" association may carry negative
   connotations for users old enough to remember. Monitor user testing feedback
   for this. The name is functional and descriptive — don't change it
   preemptively, but be prepared to rebrand if it proves to be a liability.

---

## IP Position

The desktop fingertip mouse concept has sparse patent coverage. Key prior art:

- **Conventional mice** are desk-resting pointing devices but are palm-scale,
  not fingertip-scale. The PowerPuck's defining novelty is the fingertip scale
  and two-device composition paradigm.
- **Trackballs** rest on a desk but are stationary — the user moves the ball,
  not the device. The puck is a slider, not a trackball.
- **The Ploopy Nano** (~30mm) is the closest commercial product in scale, but
  it's a standalone trackball, not a sliding fingertip mouse, and not designed
  for two-device composition.
- **Fingertip ring mice (US8648805B2)** are worn, not desk-resting. Different
  form factor class.

The two-device composition via a hub dongle — where two independent fingertip-
scale pointing devices are software-composed into a single HID mouse — is the
most defensible novel element. This applies to all PowerFinger form factors
(ring, pen, puck) and is covered by the defensive publication strategy in
IP-STRATEGY.md.

**Defensive publication:** This document constitutes a defensive publication for
the desktop fingertip puck mouse concept: a disc-shaped BLE HID pointing device,
22–28mm diameter, operated by fingertip contact and lateral sliding on a desk
surface, designed for two-device composition into a single mouse input via a USB
hub dongle. Sensing variants include direct optical (LED/VCSEL), mechanical ball
with Hall effect array, and optical-on-captive-ball. Target use case: accessible
input for users who need keyboard coexistence without a ring form factor.

---

## Naming Glossary

The PowerPuck is referred to by three names across the repository:

| Context | Name | Example |
|---------|------|---------|
| Product / user-facing | **PowerPuck** | BLE device name, docs title |
| Firmware / code | **puck** | `firmware/puck/`, Kconfig `FORM_FACTOR=PUCK` |
| Design-space matrix | **puck** | COMBINATORICS.md variant prefix `P`, `P-OLED-NONE-NONE` |

Unlike the wand→PowerPen relationship (where "wand" is the design-space class
and "PowerPen" is the first product), the puck has no pre-existing design-space
name. "Puck" is both the form factor class and the product name's root.

---

## References

- [COMBINATORICS.md](COMBINATORICS.md) — Full variant matrix (to be updated with puck variants)
- [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md) — EE/ME build spec (to be updated with puck prototypes)
- [CONSUMER-TIERS.md](CONSUMER-TIERS.md) — Standard/Pro tier structure
- [POWERPEN-SPEC.md](POWERPEN-SPEC.md) — PowerPen spec (parallel form factor, shared firmware)
- [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md) — Shared firmware phases
- [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md) — Snap dome and piezo analysis
- [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md) — UHMWPE pad material (reused for puck base)
- [POWER-BUDGET.md](POWER-BUDGET.md) — Pre-hardware power estimates
- [BATTERY-SAFETY.md](BATTERY-SAFETY.md) — LiPo safety requirements (all form factors)
- [IP-STRATEGY.md](IP-STRATEGY.md) — Patent landscape and defensive publication strategy
