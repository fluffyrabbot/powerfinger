# Firmware Roadmap — Software Build Order

Every phase runs on a $3 ESP32-C3 SuperMini dev board. The firmware written on
the breadboard **is** the firmware that runs on the prototype ring — same MCU,
same SPI bus, same GPIO. When the EE/ME delivers assembled hardware, flash the
binary that's already been tested for weeks.

---

## Phase 0 — BLE HID on Bare Dev Board

No sensor, no hardware beyond the dev board itself.

**What:** ESP32-C3 SuperMini running as a BLE HID mouse that sends fake X/Y
deltas. Validates the entire BLE stack: pairing, bonding, reconnection, HID
report descriptors, connection interval negotiation (target 7.5ms).

**Why first:** BLE pairing across OSes (macOS, Windows, Linux, iOS, Android) is
where most projects burn weeks on edge cases. NimBLE stack configuration,
bonding persistence across reboots, reconnection without re-pairing — all of
this is pure software and needs no hardware.

**Done when:** Dev board pairs with macOS/Windows/Linux/iOS, moves cursor in a
circle, registers click events, and reconnects after sleep without re-pairing.

**Hardware needed:** ESP32-C3 SuperMini (~$3).

---

## Phase 1 — Optical Sensor Driver

**What:** PAW3204 or ADNS-2080 breakout board wired to the ESP32-C3 dev board
over SPI. Read X/Y deltas from the sensor, feed into the BLE HID report from
Phase 0.

**Why:** This is the P0 prototype's complete sensing path. The driver is
straightforward (SPI read of two delta registers), but polling cadence matters —
read fast enough that deltas don't overflow, not so fast that power is wasted.

**Validate:** Strap dev board + sensor breakout to a finger (tape, rubber band,
ugly cradle). Drag finger on desk. Cursor moves. This is the P0 ring's complete
firmware running on breadboard hardware.

**Done when:** Cursor tracks finger movement on a desk surface. No visible lag,
no drift at rest.

**Hardware needed:** Optical sensor breakout (~$5–8), jumper wires.

---

## Phase 2 — Click + Dead Zone + Calibration

**What:** Wire a dome switch to a GPIO. Add click event to HID report. Implement:

- **Dead zone during click.** Pressing the finger down causes micro-movement of
  the sensor. Suppress X/Y deltas while the dome is actuated to prevent
  click-drag artifacts.
- **Calibration on boot.** Capture zero-offset from sensor at startup so drift
  from mounting angle doesn't bias the cursor.
- **Click-and-drag.** When dome is held and finger moves beyond dead zone
  threshold, re-enable delta reporting for intentional drag.

**Why:** This is where "it moves a cursor" becomes "it's usable as a mouse."
Without the dead zone, every click drags the cursor a few pixels.

**Validate:** Click dome on breadboard — cursor doesn't jump. Click-and-drag
works (hold dome, move finger, release). Double-click works.

**Done when:** You can browse a website using the dev board taped to your finger.
Open links, close tabs, select text.

**Hardware needed:** Metal snap dome or tactile switch (~$0.10), one GPIO wire.

---

## Phase 3 — Power Management

**What:** Implement sleep states:

- **Light sleep** between BLE connection events. CPU halts, BLE radio wakes it
  at each connection interval.
- **Deep sleep** after inactivity timeout (configurable, ~30–60 seconds of no
  movement and no click). Full power-down, wake on GPIO interrupt (dome press
  or sensor motion detect pin).
- **Current measurement.** Measure actual mA in each state with a USB power
  monitor or INA219 breakout.

**Why:** The 1–2 week battery target on 80–100mAh means ~6–12mW average draw.
BLE at 7.5ms connection intervals alone eats ~3–5mW. Aggressive sleep between
connection events is mandatory. This phase tells you whether the power budget is
realistic or whether latency vs. battery life tradeoffs are needed.

**Validate:** Log average mA over a simulated use session (30 min active, then
idle). Calculate battery life against 80mAh. This produces a real number — not
an estimate — for the ring prototype's battery life.

**Done when:** You have measured mA for active use, idle, and deep sleep. You
know whether 80mAh yields a week or three days, and what connection interval
tradeoffs are available.

**Hardware needed:** USB power monitor or INA219 breakout (~$3).

---

## Phase 4 — Two-Ring Coordination

**What:** Flash two ESP32-C3 dev boards with identical firmware. Both pair with
the same host as separate BLE HID mice. A validation script (Python or similar)
listens to both devices and assigns roles: one as cursor + left click, the other
as scroll + right click.

**Why:** This is the core product concept. Each ring is stateless — reports raw
deltas and click events. Role assignment lives in the companion software. But
the BLE layer must support it: two ESP32-C3 devices paired simultaneously to
the same host, HID reports interleaving without conflict, both reconnecting
after sleep.

**Validate:** Two dev boards, two dome switches, two sensor breakouts (or one
sensor + one click-only board). Use the Python script as a minimal companion
app. Browse a website with two fingers: cursor on one, scroll on the other.

**Done when:** Two-finger browsing works. No pairing conflicts. Both devices
reconnect after sleep. Latency is acceptable with both active simultaneously.

**Hardware needed:** Second ESP32-C3 SuperMini (~$3), second dome switch, second
sensor breakout (optional — one sensor + one click-only is sufficient).

---

## Phase 5 — Companion App (MVP)

**What:** A lightweight daemon (system tray app or CLI) that discovers paired
PowerFinger devices, assigns roles, and remaps HID events. Translates "Ring 2
Y-axis delta" into OS scroll events. Handles:

- Device discovery and identification (which ring is which)
- Role assignment (cursor, scroll, modifier)
- Event remapping (raw HID → OS-level mouse events with role semantics)
- Single-ring fallback detection (auto-switch to cursor + click when only one
  ring is connected)

**Why:** Without this, Ring 2 is just a second cursor. The companion app is what
makes two cursors into cursor + scroll. For MVP, a Python script or Rust CLI is
sufficient. Flutter/PWA companion app with full UI is a later phase — prove the
concept before building the interface.

**Validate:** Full two-ring mouse experience. Move cursor with middle finger,
scroll with index finger, left click with middle finger dome, right click with
index finger dome. If this works on dev boards, it works on prototype hardware.

**Done when:** You have a demo video of two-finger mousing. That's the outreach
asset.

**Hardware needed:** Same two dev board rigs from Phase 4.

---

## Phase 6 — Ball+Hall Sensor Driver

**What:** Driver for the 4-Hall-sensor array reading magnetic spindle rollers.
This is harder than the optical driver:

- Analog reads from 4 Hall sensors via ADC
- Differential rotation computation across 4 axes → X/Y deltas
- Calibration for magnet strength variation across units
- Dead zone for ball-at-rest noise (Hall sensors pick up ambient magnetic fields)
- Temperature compensation (Hall sensitivity drifts with temperature)

**Why:** This is for Prototypes 2 and 3 (ball+Hall ring pair and wand). It's a
separate sensor driver that plugs into the same BLE HID + power management +
click stack from Phases 0–3. Everything else is identical.

**Validate:** 4 Hall breakouts, a ball in a 3D-printed socket, spindles with
magnets. Ugly but functional. When the P1 ring hardware arrives, flash and go.

**Done when:** Ball rotation produces usable cursor deltas. Resolution is low
(~15–60 DPI equivalent) but sufficient for cursor control. Works on glass (the
whole point of the ball variant).

**Hardware needed:** 4x Hall sensor breakouts (~$1 each), 5mm steel ball, small
magnets, 3D-printed socket (~$2 in filament).

---

## Phase 7 — IMU Driver + Hybrid Mode

**What:** Driver for a 6-axis IMU (MPU-6050/BMI270/LSM6DSO) over I2C or SPI.
Two operating modes:

- **IMU-only (S-IMU variant).** Angular velocity from gyroscope maps to cursor
  X/Y deltas. Accelerometer provides gravity reference for drift correction.
  High-pass filter rejects slow gyro drift, passes intentional hand movement.
  Auto-center cursor after inactivity timeout.
- **Hybrid mode (I-IMU variants).** Optical sensor's SQUAL register (or Hall
  baseline for ball variants) detects surface presence. SQUAL > threshold →
  track via surface sensor. SQUAL → 0 → crossfade to IMU. Transition must be
  seamless with no cursor jump at the switch point.

**Why:** The IMU-only ring is the most accessible variant — works anywhere with
no surface. The hybrid is the most capable — surface precision when available,
air control when not. Both use the same BLE HID + power + click stack from
Phases 0–3.

**Validate:** Tape an MPU-6050 breakout (~$2) to the existing dev board rig.
Test IMU-only mode: tilt hand, cursor moves. Test hybrid mode: track on desk
(optical), lift hand, cursor switches to air tracking without jumping.

**Done when:** IMU-only mode produces usable cursor control in air. Hybrid mode
switches cleanly between surface and air. Drift is imperceptible during normal
use (< 1 pixel/second at rest after high-pass filter).

**Hardware needed:** MPU-6050 or BMI270 breakout (~$2), jumper wires.

---

## Phase 8 — DPI/Sensitivity Configuration

**What:** Expose a BLE GATT characteristic for DPI/sensitivity adjustment. The
companion app (or any BLE tool) can read and write the sensitivity multiplier.
Persisted to NVS (ESP32 non-volatile storage) so it survives power cycles.

**Why:** Hard-coded DPI is a project hard rule violation. Different users, different
surfaces, and different use contexts need different sensitivity. This must be
configurable without reflashing.

**Done when:** Companion app can adjust cursor speed on a paired ring in real
time. Setting persists across reboots.

---

## What the Prototype Hardware Validates That Dev Boards Can't

When the EE/ME delivers assembled prototype rings and wands, the firmware is
already proven. The prototype hardware validates:

- **Thermal.** Does the ring get warm in continuous use?
- **Antenna.** Does BLE range degrade inside the ring shell? Does the shell
  material or hand proximity detune the antenna?
- **Ergonomics.** Does it feel right on a finger? Is the dome in the right
  position? Is the ring too heavy?
- **Glide system.** Does the raised rim maintain focal distance in practice?
  Does tilt during natural use degrade tracking?
- **Sensor geometry.** Is the optical sensor aperture positioned correctly
  relative to the LED and the surface?
- **Battery fit.** Does the LiPo physically fit in the shell alongside the PCB?
- **Charging.** Does USB-C connect and charge reliably given the ring form factor?

Everything else — the entire firmware stack — is proven before the hardware
arrives.
