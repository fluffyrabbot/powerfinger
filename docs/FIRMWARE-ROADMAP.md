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

**Bond asymmetry recovery:** If a ring attempts to reconnect using a stored bond
and the central (host or hub) rejects with an authentication error (bond deleted
on the host side), the ring must: (a) delete the stale bond from NVS, (b) re-enter
advertising with pairing capability, (c) accept a new bond. This is the most
common BLE pairing failure mode in consumer devices — do not rely on NimBLE
defaults, test explicitly.

**Done when:** Dev board pairs with macOS/Windows/Linux/iOS, moves cursor in a
circle, registers click events, reconnects after sleep without re-pairing, AND
recovers cleanly when the bond is deleted on the host side.

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

**Done when:** Cursor tracks finger movement on a desk surface. Latency below
perceptible threshold (~20ms end-to-end). Drift at rest < 1 pixel per 10 seconds
(sensor noise filtered by dead zone or averaging).

**Hardware needed:** Optical sensor breakout (~$5–8), jumper wires.

---

## Phase 2 — Click + Dead Zone + Calibration

**What:** Wire a dome switch to a GPIO. Add click event to HID report. Implement:

- **Dead zone during click.** Pressing the finger down causes micro-movement of
  the sensor. Suppress X/Y deltas while the dome is actuated to prevent
  click-drag artifacts. Dead zone exit requires BOTH conditions:
  (a) at least `DEAD_ZONE_TIME_MS` (default 50ms) since click actuation, AND
  (b) accumulated sensor deltas exceed `DEAD_ZONE_DISTANCE` (default 10 counts)
  since click actuation. For Pro tier piezo+LRA click, add an additional
  `HAPTIC_SUPPRESS_MS` (default 20ms) of delta suppression during the haptic
  pulse to prevent cursor jump from motor vibration. All thresholds are named
  constants, not magic numbers.
- **Calibration on boot.** Capture zero-offset from sensor at startup so drift
  from mounting angle doesn't bias the cursor. If standard deviation of sensor
  readings during calibration exceeds `CALIBRATION_MOTION_THRESHOLD`, re-run
  calibration after 500ms (device was moving during boot). Max 3 retries; on
  failure, use zero offset and log a warning.
- **Click-and-drag.** When dome is held and finger moves beyond dead zone exit
  conditions above, re-enable delta reporting for intentional drag.

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

- **Adaptive connection interval.** Default 15ms (allows inter-event light
  sleep). Switch to 7.5ms when sensor reports motion, revert to 15ms after
  ~250ms of no motion. This gives low-latency tracking during movement and
  ~2x power savings during idle. Measure average mA at both intervals. If the
  BLE central rejects the 7.5ms request, operate at whatever interval the
  central accepted — do not retry more than once per connection (avoid wasting
  BLE bandwidth on repeated rejections).
- **Light sleep** between BLE connection events. CPU halts, BLE radio wakes it
  at each connection interval. Verify that light sleep actually engages at 15ms
  intervals (community measurements suggest 7.5ms is too fast for the ESP32-C3
  to sleep between events).
- **Sensor noise threshold.** Sensor deltas below `NOISE_THRESHOLD` (default:
  2 counts for optical, TBD for Hall after calibration) do not count as motion
  and do not reset the idle timer. Without this, sensor jitter prevents the
  device from ever entering idle or deep sleep.
- **Deep sleep** after inactivity timeout (configurable, ~30–60 seconds of no
  motion above noise threshold and no click). Full power-down, wake on GPIO
  interrupt (dome press or sensor motion detect pin). With RT9080 LDO (0.5µA
  Iq), system deep sleep should be ~16–21µA total. **Pro tier wake:** the piezo
  click uses ADC threshold detection, not GPIO — it cannot wake from deep sleep
  directly. Pro tier must wake on the sensor motion detect pin only (user moves
  finger to wake, then clicks). Alternatively, use the ESP32-C3 ULP coprocessor
  to poll the piezo ADC during deep sleep (~100–150µA — significantly more than
  GPIO wake, evaluate whether this is acceptable).
- **Battery low-voltage cutoff.** Monitor VBAT via ADC every 60 seconds during
  connected states. If VBAT drops below `LOW_VOLTAGE_CUTOFF` (default 3.2V —
  above the LiPo knee at ~3.0V), immediately enter deep sleep and do not wake
  until USB charging voltage is detected. LiPo cells suffer permanent damage
  below ~3.0V.
- **Runtime fault policy.** Repeated sensor read failures should degrade to a
  click-only mode until the sensor recovers instead of forcing deep sleep
  during normal use. Repeated hard BLE HID notify failures should trigger a
  restart after a bounded window of continuous failure so the ring does not
  stay connected-but-dead. `HAL_ERR_BUSY` is treated as transient and does not
  count toward that restart window.
- **Hall sensor power gating** (ball variants only). The DRV5053 has no sleep
  mode — four sensors draw ~12mA continuously. Add a MOSFET or load switch on
  the Hall sensor VCC rail, controlled by a GPIO. Gate off in idle/deep sleep.
- **Current measurement.** Measure actual mA in each state with a USB power
  monitor or INA219 breakout. Compare against pre-hardware estimates in
  [POWER-BUDGET.md](POWER-BUDGET.md).

**Why:** Pre-hardware estimates (see [POWER-BUDGET.md](POWER-BUDGET.md)) predict
3–6 days on 80–100mAh with the ESP32-C3 — not the originally hoped 1–2 weeks.
The ESP32-C3 can't enter light sleep between BLE events at 7.5ms intervals (the
next event arrives before sleep completes). At 15ms intervals, inter-event sleep
works and roughly doubles battery life vs 7.5ms. This phase produces real
measurements to confirm or correct those estimates.

**Validate:** Log average mA over a simulated use session (30 min active, then
idle). Calculate battery life against 80mAh. This produces a real number — not
an estimate — for the ring prototype's battery life.

**Done when:** You have measured mA for active use (at both 7.5ms and 15ms
intervals), idle (with adaptive interval at 15ms), and deep sleep. You know
whether the 3–6 day estimate from POWER-BUDGET.md is correct and whether
adaptive connection intervals deliver the predicted ~2x improvement. This data
also informs whether the consumer product should migrate to nRF52840 (predicted
3–5x improvement over ESP32-C3 for BLE HID).

**Hardware needed:** USB power monitor or INA219 breakout (~$3).

---

## Phase 4 — USB Hub Dongle

**What:** An ESP32-S3 dev board acting as a BLE central + USB HID device. It
pairs with multiple rings over BLE, assigns roles, composes their events into
a single unified HID mouse report, and presents to the host OS as one standard
USB HID mouse.

```
Ring 1 (ESP32-C3) ──BLE──┐
                          ├──► USB Hub Dongle ──USB HID──► OS sees one mouse
Ring 2 (ESP32-C3) ──BLE──┘    (ESP32-S3)
                               - BLE central: pairs with N rings
                               - role engine: MAC → role mapping
                               - event composer: N inputs → 1 HID report
                               - USB HID device: one mouse to the OS
```

**Why the hub instead of a companion app:**

The companion app approach requires per-OS input interception to remap Ring 2's
cursor events into scroll events. This means different implementations for
Linux (`libevdev`/`uinput`), macOS (`IOHIDManager`/`CGEventTap`), Windows
(`RawInput`/`SendInput`), and is impossible on iOS and most Android without
root or accessibility hacks. The hub eliminates all of this:

- **OS sees one mouse.** No interception, no remapping, no permissions, no
  per-OS code. USB HID works on every OS including iOS (via Lightning/USB-C
  adapter), ChromeOS, game consoles, smart TVs.
- **No companion app required for multi-ring.** The hub does the composition.
  Two rings just work as cursor + scroll out of the box. Satisfies the "no app
  required" hard rule for the full two-ring setup, not just single-ring.
- **No host BLE radio dependency.** The hub's BLE radio manages ring
  connections. Works on hosts with no BLE at all (desktops, older laptops).
- **Latency control.** Hub runs BLE at 7.5ms intervals per ring and emits USB
  HID reports at 1ms (USB full-speed polling). Absorbs BLE jitter, outputs
  smooth reports. No latency budget splitting on the host's BLE radio.
- **N-ring scaling.** Add a third ring, a fourth — the hub manages all
  connections and composes one HID report. The OS never knows.

**Hub firmware architecture:**

1. **BLE central** — scans, pairs, bonds with PowerFinger peripherals. Stores
   bonding info in NVS. Reconnects automatically after power cycle. Maximum
   simultaneous connections: 4 rings (limited by ESP32-S3 NimBLE memory budget).
   Connection attempts beyond the limit are rejected.
2. **Role engine** — maps each ring's BLE MAC address to a role. Default
   assignment: first ring paired = cursor + left click, second = scroll + right
   click. Configurable via USB serial or companion app. Roles are a first-class
   concept:

   | Role | X deltas → | Y deltas → | Click → |
   |------|-----------|-----------|---------|
   | Cursor | cursor X | cursor Y | left click |
   | Scroll | horizontal scroll | vertical scroll | right click |
   | Modifier | (ignored) | (ignored) | middle click |

   Each connected ring has exactly one role. Role reassignment must be atomic:
   pause event composition, swap mappings, resume. No window where two rings
   share a role or a role is unassigned.

3. **Event composer** — maintains per-ring state: `{buttons, delta_x, delta_y,
   connected}`. On each USB HID poll (1ms): composed report buttons = OR of
   all connected rings' buttons per role mapping; cursor deltas = SUM of all
   cursor-role ring deltas since last poll; scroll = SUM of all scroll-role ring
   deltas since last poll.

   **Disconnection handling (critical for accessibility):** on BLE disconnection
   event for ring R, immediately: (a) set all buttons contributed by R to
   released, (b) zero R's delta contribution, (c) emit a USB HID report
   reflecting the change. A ring disconnecting mid-click must NOT leave a stuck
   button on the host — accessibility users may not be able to resolve this
   without assistance.

   **USB suspend:** on host USB suspend (laptop lid close, sleep), request each
   connected ring to drop to idle connection interval (30ms). On USB resume,
   do not change ring intervals — rings adapt via their own motion detection.

4. **USB HID device** — ESP32-S3 native USB OTG presents as a standard USB HID
   mouse. One device, one report descriptor, one cursor.

**Validate:** Two ESP32-C3 dev boards running ring firmware (from Phases 0–2)
pair with an ESP32-S3 dev board over BLE. The S3 plugs into a laptop via USB.
Move one C3 → cursor moves. Move the other C3 → scroll happens. Click either →
correct button. The laptop sees one mouse.

**Done when:** Two-ring mousing works through the hub on any OS. Plug in the
hub, rings auto-connect, cursor and scroll work. No software installed on the
host.

**Hardware needed:** ESP32-S3 dev board (~$4), USB cable. Plus two ESP32-C3
rigs from Phase 2.

---

## Phase 5 — Companion App (Configuration UI)

**What:** A lightweight app that talks to the hub over USB serial (or BLE for
hubless single-ring use). Provides:

- Role reassignment (drag-and-drop: which ring is cursor, which is scroll)
- Sensitivity / DPI adjustment per ring
- Gesture configuration (what does simultaneous two-ring click do?)
- Hub firmware update
- Ring firmware OTA update (hub relays to rings over BLE)

**Why:** The hub handles multi-ring composition without an app. The companion
app is the configuration UI — a luxury, not a requirement. It's how you
customize, not how you use.

**Platform priority:**
1. **Web Serial** (Chrome/Edge) — talks to hub over USB serial from a webpage.
   Zero install, works on any desktop OS with a modern browser. MVP path.
2. **Desktop app** (Tauri or Electron) — for users who want a native app.
3. **Mobile app** (Flutter) — talks to hub over BLE for mobile configuration.

**Done when:** User can reassign ring roles, adjust sensitivity, and update
firmware from a web page connected to the hub via USB.

**Hardware needed:** Same hub + ring setup from Phase 4.

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

**What:** Driver for a 6-axis IMU (BMI270 recommended — 685µA full mode vs
MPU-6050's 3.6mA, see [POWER-BUDGET.md](POWER-BUDGET.md)) over I2C or SPI.
Two operating modes:

- **IMU-only (S-IMU variant).** Angular velocity from gyroscope maps to cursor
  X/Y deltas. Accelerometer provides gravity reference for drift correction.
  High-pass filter rejects slow gyro drift, passes intentional hand movement.
  Auto-center cursor after inactivity timeout.
- **Hybrid mode (I-IMU variants).** The motion source module exports a
  `surface_confidence` value (0–255). For PixArt optical sensors this maps to
  the SQUAL register; for Hall arrays it maps to baseline magnetic field
  deviation; for IMU-only it is always 0. `surface_confidence` > threshold →
  track via surface sensor. `surface_confidence` → 0 → crossfade to IMU.
  Transition must be seamless with no cursor jump at the switch point. Use
  `surface_confidence` in the interface, not vendor-specific register names.

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
Valid range: 1–255 (0 is reserved, writes outside range rejected with
ATT_ERR_VALUE_NOT_ALLOWED). Persisted to NVS (ESP32 non-volatile storage) so
it survives power cycles. NVS writes should be deferred to idle periods (flash
erase takes 20–200ms and can interfere with BLE timing).

**Why:** Hard-coded DPI is a project hard rule violation. Different users, different
surfaces, and different use contexts need different sensitivity. This must be
configurable without reflashing.

**Done when:** Companion app can adjust cursor speed on a paired ring in real
time. Setting persists across reboots.

---

## Phase 9 — Firmware Update Architecture

**What:** Field-updateable firmware for both the hub and the rings. Three
components:

### Hub Update Path (USB)

The hub (ESP32-S3) is always plugged into USB. Two update mechanisms:

1. **USB DFU bootloader** in a protected flash partition. The ESP32-S3's
   built-in USB DFU mode (hold boot button on plug-in) accepts a raw `.bin`
   file. This is the failsafe — if application firmware is bricked, DFU always
   works. No application code involved.

2. **Web Serial update** from the companion app. The same browser-based UI
   (Phase 5) that configures roles also flashes firmware. User clicks "update,"
   selects a `.bin` file (downloaded from GitHub releases or loaded locally),
   companion app writes it over USB serial to the hub's OTA partition. Hub
   reboots into new firmware.

```
Flash partition layout (hub):

┌──────────────────────┐
│  DFU bootloader      │  ← protected, never overwritten
├──────────────────────┤
│  OTA partition A     │  ← active application firmware
├──────────────────────┤
│  OTA partition B     │  ← receives update, becomes active on reboot
├──────────────────────┤
│  NVS                 │  ← role assignments, bonding info (preserved)
└──────────────────────┘
```

ESP-IDF's `esp_ota_ops` handles the A/B partition swap. **Boot confirmation is
mandatory:** new firmware must call `esp_ota_mark_app_valid_cancel_rollback()`
within 30 seconds of boot. A watchdog timer (30s) forces a reset if this call
is not made (crash, hang, assertion failure), and the bootloader then rolls back
to the previous partition. Without this explicit confirmation, the A/B rollback
mechanism is inert — ESP-IDF does not roll back automatically. NVS (bonding
info, role assignments) is in a separate partition and survives firmware updates.

**BLE OTA link failure:** if the BLE link drops mid-transfer to a ring, the
partial image on the ring's inactive OTA partition is harmless (the active
partition is unaffected). The hub must detect the incomplete transfer and offer
to retry. Transfers are not resumable — restart from the beginning on failure.

### Ring Update Path (BLE OTA via Hub)

Rings (ESP32-C3) have no USB connection during normal use. The hub acts as a
firmware update gateway:

```
GitHub Releases ──download──► Browser (Web Serial)
                                    │
                                    ├──USB──► Hub: flash hub firmware directly
                                    │
                                    └──USB──► Hub ──BLE OTA──► Ring 1
                                                  ──BLE OTA──► Ring 2
                                                  ──BLE OTA──► Ring N
```

1. User downloads ring firmware `.bin` from GitHub releases (or loads local
   file).
2. Companion app (Web Serial) sends the binary to the hub over USB serial.
3. Hub stores the binary in a staging area (SPI flash or PSRAM on the S3).
4. Hub initiates BLE OTA to each connected ring using ESP-IDF's `esp_ota`
   over a custom GATT service (not the HID profile — a separate OTA service
   UUID).
5. Each ring receives the binary, writes to its OTA partition, validates
   checksum, reboots.
6. If the new ring firmware fails to boot, the ring's bootloader rolls back
   and the hub reports the failure to the companion app.

Ring flash partition layout mirrors the hub: DFU bootloader (protected) + A/B
OTA partitions + NVS (calibration, bonding info preserved across updates).

### Direct Ring Update (Fallback)

If no hub is available, a ring can be updated by plugging it into USB directly
(USB-C charging port doubles as serial) and flashing via `esptool.py` or
Web Serial. This is the development workflow and the fallback for users without
a hub.

**Why field-updateable firmware matters:**

- The BLE stack will need tuning per OS (connection interval negotiation
  behaves differently on macOS, Windows, Android).
- The hub's role composition logic will change as user testing reveals which
  gesture mappings work and which don't.
- Sensor calibration algorithms will improve over time.
- Security patches for BLE vulnerabilities.
- A device that can't be updated becomes e-waste when a bug is found. This
  violates the no-planned-obsolescence rule.

**Done when:** Hub can be updated from the companion app (Web Serial) without
any special tools. A ring can be OTA-updated through the hub. Both devices
roll back on failed update. NVS (bonding, calibration, role assignments) is
preserved across updates.

**Hardware needed:** Same hub + ring rigs. No additional hardware — the update
path uses existing USB and BLE connections.

---

## Ring State Machine

Every ring device has exactly one active state at any time:

```
                         gpio_interrupt
  ┌────────────┐      (dome or motion)      ┌──────────┐
  │ DEEP_SLEEP │ ──────────────────────────► │ BOOTING  │
  └────────────┘                             └────┬─────┘
       ▲                                          │ calibration_complete
       │ sleep_timeout                            ▼
       │ (or low battery)                  ┌──────────────┐
       │                    ble_timeout    │ ADVERTISING  │◄──── ble_disconnected
       │◄──────────────────────────────────└──────┬───────┘     (from any connected)
       │                                          │ ble_connected
       │                                          ▼
       │                    no activity     ┌─────────────────┐
       │      ┌──────────── for 250ms ──────│CONNECTED_ACTIVE │
       │      ▼                             └────────▲────────┘
  ┌────────────────────┐                             │
  │ CONNECTED_IDLE     │──── motion or click ─────────┘
  └────────────────────┘
```

**Key transitions:**
- `DEEP_SLEEP → BOOTING`: GPIO interrupt only (dome press or sensor motion
  detect pin). Pro tier: sensor motion detect pin is the primary wake source,
  since piezo cannot generate GPIO interrupt.
- `BOOTING → ADVERTISING`: after calibration completes and passes sanity check.
  HID reports are not enabled until calibration is complete.
- `ADVERTISING → CONNECTED_IDLE`: BLE connection established.
- `ADVERTISING → DEEP_SLEEP`: if no connection within `RECONNECT_TIMEOUT`
  (default 60s). Prevents battery drain from indefinite advertising.
- `CONNECTED_IDLE → CONNECTED_ACTIVE`: sensor delta exceeds `NOISE_THRESHOLD`
  or a click occurs while stationary. This keeps press/release usable even
  when the sensor sees no motion.
- `CONNECTED_ACTIVE → CONNECTED_IDLE`: no activity for 250ms.
- `CONNECTED_IDLE → DEEP_SLEEP`: no activity for `SLEEP_TIMEOUT` (30–60s).
- `CONNECTED_* → ADVERTISING`: BLE disconnected. Advertise for
  `RECONNECT_TIMEOUT`, then deep sleep.
- `* → DEEP_SLEEP`: VBAT < `LOW_VOLTAGE_CUTOFF`. Forced, non-negotiable.

**Invariants:**
- HID reports are never sent before calibration completes.
- `power_manager` owns active/idle timeout generation and BLE connection
  parameter requests so those policy decisions stay out of the pure state
  machine.
- Advertising never runs indefinitely (bounded by `RECONNECT_TIMEOUT`).
- VBAT < `LOW_VOLTAGE_CUTOFF` always results in deep sleep, from any state.

---

## Platform HAL — Define Before Writing Firmware

The nRF52840 migration is already recommended for the consumer product (see
[CONSUMER-TIERS.md](CONSUMER-TIERS.md) and [POWER-BUDGET.md](POWER-BUDGET.md)).
Every hour of ESP-IDF-specific firmware written without a hardware abstraction
layer is an hour of code that must be ported later.

Define a thin HAL as the first firmware artifact — before Phase 0 code:

| HAL interface | Hides | ESP-IDF impl | Zephyr impl |
|---|---|---|---|
| `hal_gpio_*` | Pin numbering, interrupt config | `gpio_*` | `gpio_*` (different API) |
| `hal_spi_*` | SPI bus init, transfer | `spi_master_*` | `spi_*` |
| `hal_adc_*` | ADC channel, calibration | `adc_oneshot_*` | `adc_*` |
| `hal_timer_*` | Hardware timers, software timers | `esp_timer_*` | `k_timer_*` |
| `hal_sleep(mode)` | Sleep mode semantics | `esp_light_sleep_start` / `esp_deep_sleep_start` | `k_cpu_idle` / `sys_poweroff` |
| `hal_storage_get/set` | NVS vs settings subsystem | `nvs_*` | `settings_*` |
| `hal_ota_*` | OTA mechanism | `esp_ota_ops` | MCUboot |
| `hal_ble_*` | BLE stack | NimBLE | Zephyr BLE |

This is not a heavy abstraction — one header file, one `.c` per target platform.
The sensor drivers, click logic, dead zone state machine, report composer, and
power policy all call HAL functions instead of ESP-IDF functions directly.

Cost: ~1 day of work at project start. Payoff: MCU migration touches one module
instead of every module.

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
