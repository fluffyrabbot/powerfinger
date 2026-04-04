# PowerPen Specification — WSTD-BALL-NONE-NONE (Core PowerPen)

This is the build specification for the core PowerPen prototype: a pen-shaped BLE
HID mouse with a ball+Hall sensor at the tip. It is the pen equivalent of the
ring's P0 optical prototype — the simplest variant that proves the form factor.

**Variant ID:** WSTD-BALL-NONE-NONE
**Sensing:** Ball+Hall (4x DRV5053, 5mm steel ball, 4 magnetic spindle rollers)
**Click:** Kailh GM8.0 micro switch (barrel, thumb) + metal snap dome (tip,
press-down)
**MCU:** ESP32-C3 SuperMini (same as ring)
**BOM target:** ~$14 at prototype quantity
**Accessibility use case:** Surface-agnostic pen pointer for users who cannot use
a flat mouse or trackpad. Weighted metal body stabilizes tremor. Works on glass,
fabric, skin, mirror — any surface.

---

## Physical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| Body | Aluminum or stainless steel tube | Rigid, no flex |
| Outer diameter | ~8mm | Must fit Kailh GM8.0 (5.8mm wide) plus wall |
| Length | ~120mm | Pen-length, comfortable for extended grip |
| Weight | TBD (measure prototype) | Heavier than 17g Genius pen mouse; weight is a feature for tremor damping |
| Tip | 3D-printed ball socket assembly | Houses 5mm ball, 4 roller spindles, 4 Hall sensors, DRV5032 wake sensor, snap dome |
| End cap | 3D-printed | Houses USB-C connector |
| Pen angle (use) | 30–70 degrees from horizontal | User-determined by grip; ball+Hall is angle-independent (hypothesis — validate) |
| Ball protrusion | ~1–2mm beyond tip | Ball contacts surface directly; no glide rim needed |

### Internal Layout (Tip to End)

```
USB-C ◄── End Cap ── Battery ── PCB Strip ── Barrel Switch ── Tip Assembly ──► Ball
       ◄─────────────────── ~120mm ──────────────────────────────────────────►
```

1. **Tip assembly** (~15mm): Ball socket, 4 roller spindles with neodymium
   magnets, 4x DRV5053 Hall sensors, DRV5032 wake sensor near one roller,
   snap dome under ball.
2. **Barrel switch zone** (~10mm): Kailh GM8.0 micro switch mounted on PCB,
   actuator through barrel cutout for thumb access.
3. **PCB strip** (~40mm): ESP32-C3-MINI-1-N4, LDO, charge controller, MOSFET
   for Hall power gating, passives. Rigid PCB, ~8mm wide.
4. **Battery** (~30mm): LiPo 100–150mAh, cylindrical or prismatic.
5. **End cap** (~10mm): USB-C connector for charging and fallback serial flash.

---

## Electrical Architecture

### MCU and Power

| Component | Part | Purpose |
|-----------|------|---------|
| MCU | ESP32-C3-MINI-1-N4 (or -N4X successor) | BLE HID, sensor polling, power management |
| LDO | RT9080-33GJ5 (0.5µA Iq) | 3.3V regulation; ultra-low quiescent for deep sleep |
| Charge controller | TP4054 | USB-C LiPo charging |
| NTC | 10kΩ NTC thermistor | Charge temperature monitoring |
| Charge gate | SI2301 P-ch MOSFET | Charge path control |
| Hall power gate | SI2301 P-ch MOSFET (or equivalent) | GPIO-controlled VCC rail for 4x DRV5053 |
| VBAT divider | 10MΩ / 10MΩ resistor divider | Battery voltage monitor; 0.165µA draw (see note) |
| Battery | LiPo 100–150mAh | Target 2–5 days; see power budget below |

**VBAT divider note:** Standard 100kΩ/100kΩ dividers draw 16.5µA — nearly
doubling the deep sleep budget. Use 10MΩ/10MΩ (0.165µA). The tradeoff is slower
ADC settling time (~100ms vs ~1ms) due to higher source impedance, but VBAT is
sampled only once per 60 seconds during connected states. Alternatively, gate the
divider high side with a MOSFET on the Hall power gate GPIO — then divider draw
during sleep is zero, but VBAT cannot be sampled without powering the Hall array.

### Sensor Subsystem

| Component | Part | Qty | Purpose |
|-----------|------|-----|---------|
| Steel ball | 5mm G25 chrome steel | 1 | Contact element |
| Roller spindles | Custom (magnet-tipped) | 4 | Translate ball rotation to magnetic field changes |
| Magnets | 1x1mm N52 neodymium | 4 | On roller spindle tips |
| Hall sensors | DRV5053 (analog, ratiometric) | 4 | Read magnetic field from each roller |
| Wake sensor | DRV5032FB (digital Hall switch, omnipolar) | 1 | Motion-based deep sleep wake; see wake circuit below |

**Sensor read path:** 4x ADC channels → differential rotation computation →
X/Y deltas. This is the Phase 6 ball+Hall driver from the firmware roadmap, shared
identically with the ball ring variant.

**Critical: Hall power gating.** DRV5053 has no sleep mode. 4 sensors draw ~12mA
total. Without MOSFET gating, the Hall array drains 100mAh in ~8 hours. The
power manager must gate the Hall VCC rail via GPIO in idle and deep sleep states.

### Wake Sensor Circuit (DRV5032)

The DRV5053 is analog-only — it cannot generate a GPIO interrupt for deep sleep
wake. Rather than powering an analog Hall sensor plus comparator during sleep
(~3mA — would destroy the deep sleep budget), the PowerPen uses a dedicated
ultra-low-power digital Hall switch for motion wake.

**Part:** TI DRV5032FB (omnipolar, push-pull output, SOT-23-3)
- Quiescent current: 0.54µA typ
- Supply: 1.65–5.5V (runs directly from LDO 3.3V output)
- Output: push-pull, active-low on magnetic field detect
- Magnetic operate point: ±4.9mT typ (FB variant — most sensitive omnipolar)
- Package: SOT-23-3 (2.9 x 1.6mm) or X2SON (1.0 x 1.0mm)
- Cost: ~$0.30

**Placement:** Adjacent to one roller spindle magnet, positioned so the magnet's
sweep during ball rotation crosses the DRV5032 operate/release thresholds. The
omnipolar (FB) variant triggers on either magnetic pole, so spindle rotation in
either direction generates a toggle.

**Circuit:**

```
LDO 3.3V (always on) ──┬── VDD (DRV5032 pin 1)
                        │
                        ├── 100nF bypass cap ── GND
                        │
                        └── OUT (DRV5032 pin 3) ── GPIO8 (wake-capable, internal pull-up)
                                                     │
                        GND (DRV5032 pin 2) ── GND   └── ESP32-C3 deep sleep wake source
```

The DRV5032 is powered from the LDO output, which stays live during deep sleep
(the RT9080 draws 0.5µA Iq regardless). Adding 0.54µA for the DRV5032 is
negligible — total deep sleep current increases from ~16–21µA to ~17–22µA.

**Why DRV5032 instead of a comparator circuit:**
- A comparator approach requires keeping one DRV5053 powered during sleep (~3mA)
  plus a comparator IC. 3mA on a 100mAh battery = ~33 hours to dead in sleep.
- The DRV5032 draws 0.54µA — 5,500x less power than the comparator approach.
- Fewer components (one IC vs two ICs + reference divider).
- Direct digital output — no analog threshold tuning needed.
- Purpose-built for exactly this use case (magnetic wake/detect).

**Placement constraint (critical):** The DRV5032FB has hysteresis of 1.7mT
(operate ±4.9mT, release ±3.2mT). If the roller magnet's rest-state field falls
within this hysteresis band, thermal noise or ambient field variations can cause
the output to oscillate, triggering spurious wakes. The magnet-to-sensor distance
must be set so that the rest-state field is clearly below the release threshold
(3.2mT) for all four roller orientations. Bench test with the actual 1x1mm N52
magnets at distances from 0.5mm to 5mm, recording output toggles over 60 seconds
with no ball motion, to identify the safe placement zone.

**Spurious wake debounce (firmware, required):** Even with correct placement,
external magnetic interference or component variation can cause occasional false
wakes. The power manager must implement a debounce policy:
1. After DRV5032-triggered wake (GPIO8 low, GPIO4+GPIO5 high), if calibration
   fails or no motion is detected within `WAKE_VALIDATION_MS` (default 200ms),
   increment a spurious wake counter in RTC memory (survives deep sleep).
2. After `SPURIOUS_WAKE_LIMIT` (default 3) spurious wakes within
   `SPURIOUS_WAKE_WINDOW_MS` (default 30,000ms), disable GPIO8 as a wake source.
   The pen falls back to barrel-switch-only and tip-dome-only wake.
3. Reset the counter on: successful tracking session (motion above noise
   threshold while connected), barrel switch press, or USB charging voltage
   detected.
4. A DRV5032-disabled state must be visible in the diagnostics snapshot
   (`drv5032_wake_enabled`, `spurious_wake_count`).

**Failure scenario without debounce:** Magnet at threshold → DRV5032 oscillates →
wake → Hall gate-on (12mA) → calibration fail → sleep → repeat. This duty-cycles
at ~5–10mA average. A 100mAh battery dies in 10–20 hours instead of weeks.

**DRV5032 permanent failure:** If the DRV5032 dies (open circuit, stuck high), the
pen degrades to barrel-switch and tip-dome wake only. This is acceptable — the
DRV5032 is a convenience for motion wake, not a safety-critical component.
Document in user-facing material that if the pen stops waking on ball roll,
click-to-wake is the fallback.

**Firmware integration:** Configure all three wake GPIOs (GPIO4, GPIO5, GPIO8) via
`esp_deep_sleep_enable_gpio_wakeup()` bitmask. The current ring implementation
(`power_manager_enter_sleep()`) configures only a single dome pin — the pen build
must extend this to a Kconfig-driven wake GPIO bitmask
(`CONFIG_POWERFINGER_WAKE_GPIO_MASK`). On wake, the power manager gates on the 4x
DRV5053 array via the MOSFET, waits `HALL_STABILIZATION_MS` (default 1ms, covers
DRV5053 30µs power-on + MOSFET + 400nF bypass RC) for Hall sensors to stabilize,
then runs calibration. The DRV5032 output is not used during active tracking —
only the 4x DRV5053 analog array provides motion data.

### Click Subsystem

| Location | Part | Actuation | GPIO | Purpose |
|----------|------|-----------|------|---------|
| Barrel (thumb) | Kailh GM8.0 micro switch | ~60gf, 0.5mm travel | Primary click GPIO | Left click (default) |
| Tip (press-down) | 5mm 200gf Snaptron SQ snap dome | 200gf press-down on ball | Secondary click GPIO | Right click (two-button pen) |

The barrel switch is the primary click — thumb-operated, same hand that holds the
pen. The tip dome is secondary — press the pen down into the surface to click.
Both map to standard HID mouse buttons; role assignment is configurable via hub or
companion app.

**Dead zone behavior:** Unlike the ring (where dome press causes micro-movement of
the optical sensor), the PowerPen barrel switch is physically separated from the
tip sensor. Dead zone during barrel click may be unnecessary or need a shorter
window. The tip dome press WILL cause ball displacement — dead zone required, same
logic as ring Phase 2.

### Pin Assignment (Preliminary)

| GPIO | Function | Notes |
|------|----------|-------|
| GPIO0 | Hall sensor A (ADC) | ADC1_CH0 |
| GPIO1 | Hall sensor B (ADC) | ADC1_CH1 |
| GPIO2 | Hall sensor C (ADC) | ADC1_CH2 |
| GPIO3 | Hall sensor D (ADC) | ADC1_CH3 |
| GPIO4 | Barrel switch (Kailh GM8.0) | Internal pull-up, active-low, wake-capable |
| GPIO5 | Tip dome (Snaptron SQ) | Internal pull-up, active-low, wake-capable |
| GPIO6 | Hall VCC gate (MOSFET) | Active-low for P-ch MOSFET |
| GPIO7 | VBAT ADC | Battery voltage monitor via divider |
| GPIO8 | DRV5032 wake sensor output | Internal pull-up, active-low, wake-capable |
| GPIO10 | USB D- | USB-C (charging + serial) |
| GPIO18 | SDA (I2C, future IMU) | Reserved for Phase 7 IMU hybrid |
| GPIO19 | SCL (I2C, future IMU) | Reserved for Phase 7 IMU hybrid |

**Note:** Pin assignments are preliminary. Final mapping depends on PCB layout
routing convenience. All pins must be named constants in firmware (Kconfig or
header), never hard-coded numbers.

---

## Power Budget (Pre-Hardware Estimate)

Based on ring power budget analysis, adapted for PowerPen components.

| State | Components Active | Estimated mA | Duration Model |
|-------|-------------------|-------------|----------------|
| Active tracking | MCU + BLE (15ms) + 4x Hall + click poll | ~18–22mA | 3 hrs/day |
| Active tracking (low-latency) | MCU + BLE (7.5ms) + 4x Hall + click poll | ~25–30mA | Burst during motion |
| Connected idle | MCU + BLE (15ms) + Hall gated | ~3–5mA | 5 hrs/day |
| Deep sleep | LDO Iq + DRV5032 + GPIO wake | ~17–22µA | 16 hrs/day |

**Estimated battery life (100mAh, 3 hrs active / 5 hrs idle / 16 hrs sleep):**
~2–4 days. Matches CLAUDE.md target of 2–5 days on 100–150mAh.

**Key difference from ring:** The PowerPen has a 100–150mAh battery (vs 80–100mAh
for ring), giving ~25–50% more capacity. But the 4x Hall sensors draw ~12mA
where the PAW3204 optical draws ~3mA, so active power is significantly higher.
Net battery life is similar or slightly worse than the optical ring.

---

## Firmware Architecture — Sharing Strategy

The PowerPen shares the vast majority of its firmware with the ring. The goal is
**one firmware codebase** with build-time configuration, not a fork.

### Shared with Ring (No Changes)

| Component | Ring Location | Notes |
|-----------|---------------|-------|
| HAL layer | `ring/components/hal/` | GPIO, SPI, ADC, timer, sleep, storage, OTA, BLE — all platform-agnostic |
| BLE HID | `ring/components/ble_hid/` | Same HID report descriptor, same GAP handler |
| Sensor interface | `ring/components/sensors/sensor_interface.h` | Ball+Hall driver implements same `sensor_init/read/power_down/wake` |
| Calibration | `ring/components/click/calibration.c` | Sensor zero-offset calibration (misplaced in `click/` — should move to `sensors/` or own component) |
| Test suite | `firmware/test/` | Same host-side test harness |

### Shared with Ring (Interface Extension Required)

These components are conceptually shared but their current interfaces assume a
single-button ring. Extending them is prerequisite work for the pen build.

| Component | Ring Location | What Changes |
|-----------|---------------|-------------|
| Click interface | `ring/components/click/click_interface.h` | `click_is_pressed()` returns one `bool`. Pen needs N sources → add `click_source_t` enum and `click_is_pressed(click_source_t)` or return bitmask |
| Dead zone | `ring/components/click/dead_zone.c` | Module-level singleton state (`s_state`). Pen needs two independent instances → refactor to `dead_zone_ctx_t *` per instance |
| Power manager | `ring/components/power/` | `power_manager_enter_sleep()` configures one wake GPIO. Pen needs three → Kconfig-driven `CONFIG_POWERFINGER_WAKE_GPIO_MASK` bitmask. Also: Hall gate-off on calibration failure path |
| Diagnostics | `ring/components/diagnostics/` | Hard-imports `ring_state.h`, snapshot contains `ring_state_t`. Pen needs generic `uint8_t device_state` + state-name function pointer |
| Runtime health | `ring/components/runtime_health/` | All types `ring_`-prefixed. Functionally form-factor-agnostic — rename to `pf_` when extracting shared components |

### PowerPen-Specific (Configuration Only)

| Concern | What Changes | Approach |
|---------|-------------|----------|
| Pin assignments | Different GPIO map (+ DRV5032 wake pin) | Kconfig / `sdkconfig.defaults` per target |
| Wake GPIO bitmask | Three wake sources vs ring's one | `CONFIG_POWERFINGER_WAKE_GPIO_MASK` in Kconfig |
| Wake sensor debounce | Spurious wake counter + DRV5032 disable | See wake sensor circuit section |
| Click topology | Two independent switches (barrel + tip) | Requires click interface extension (see table above) |
| Dead zone tuning | Barrel: disabled. Tip: enabled | Requires dead zone instanceability (see table above) |
| Hall power gating | GPIO for MOSFET gate | Already in power manager; pen just sets Kconfig pin |
| DRV5032 circuit | Always-on LDO rail, GPIO8 | See wake sensor circuit section |
| Device identity | BLE device name, appearance | Kconfig: "PowerPen" vs "PowerFinger Ring" |
| VBAT divider | 10MΩ/10MΩ for low sleep current | See electrical architecture section |

### Build Configuration

```
firmware/
  ring/         ← existing ring build target
  pen/          ← PowerPen build target
    CMakeLists.txt       ← pulls shared components from ring/, adds pen config
    sdkconfig.defaults   ← pen-specific pin map, device name, click topology
    main/
      main.c             ← pen entry point (may be identical to ring main.c)
    Kconfig              ← POWERFINGER_FORM_FACTOR=PEN
```

The pen `CMakeLists.txt` uses `EXTRA_COMPONENT_DIRS` to include shared components
from `ring/components/`. No code duplication — just different configuration.

---

## State Machine

The PowerPen reuses the ring state machine (`ring_state_t`, `ring_event_t`,
`ring_actions_t`) unchanged. The states, events, and transitions are structurally
identical — the pen's differences (wake sources, Hall gating, click topology) are
all handled by the power manager and click driver, not the state machine.

If genuine pen-specific states or events emerge during development (e.g., a
DRV5032-specific wake event for the spurious wake debounce), fork the state
machine at that point. The cost of forking later is near-zero (copy a header,
rename enums). The cost of maintaining two identical state machines is nonzero
forever.

**Hall power gating is the power manager's responsibility, not the state
machine's.** The power manager already owns Hall gating via `hall_power_set()`,
triggered by connect/disconnect/sleep lifecycle events. The state machine emits
lifecycle actions only (`start_advertising`, `stop_advertising`,
`enter_deep_sleep`).

```
                         gpio_interrupt
  ┌────────────┐      (barrel switch, DRV5032    ┌──────────┐
  │ DEEP_SLEEP │ ──── wake sensor, or tip dome) ► │ BOOTING  │
  └────────────┘                                  └────┬─────┘
       ▲                                               │ calibration_complete
       │ sleep_timeout                                 ▼
       │ (or low battery)                       ┌──────────────┐
       │                    ble_timeout         │ ADVERTISING  │◄── ble_disconnected
       │◄───────────────────────────────────────└──────┬───────┘
       │                                               │ ble_connected
       │                                               ▼
       │                    no activity          ┌─────────────────┐
       │      ┌──────────── for 250ms ───────────│CONNECTED_ACTIVE │
       │      ▼                                  └────────▲────────┘
  ┌────────────────────┐                                  │
  │ CONNECTED_IDLE     │──── motion or click ──────────────┘
  └────────────────────┘
```

**Wake sources for deep sleep (all three are GPIO interrupt sources):**
- Barrel switch (GPIO4) — user picks up pen and clicks
- DRV5032 wake sensor (GPIO8) — user rolls pen on surface, magnet triggers
  digital Hall switch (subject to spurious wake debounce — see wake sensor
  section)
- Tip dome (GPIO5) — user presses pen tip into surface

**Wake sequence:** GPIO interrupt fires → `RING_EVT_GPIO_WAKE` → transition to
`RING_STATE_BOOTING` → power manager gates on Hall VCC (MOSFET) → wait
`HALL_STABILIZATION_MS` for DRV5053 stabilization → run calibration →
`RING_EVT_CALIBRATION_DONE` → transition to `RING_STATE_ADVERTISING`.

**Calibration failure path (critical for battery):** If calibration fails, the
power manager must gate off the Hall sensor array before entering advertising.
Without this, the 4x DRV5053 draw 12mA during up to 60 seconds of advertising —
0.2mAh wasted per failed wake cycle.

**Wake-to-first-report latency targets:**
- **Bonded peer reconnection:** < 500ms (achievable: ~310ms best case)
- **Cold pairing (first connection):** 3–5 seconds (BLE pairing dominates)

The 500ms target assumes a bonded host actively scanning. Calibration alone takes
~100ms best case (50 samples × 2ms); worst case with retries is ~1.9 seconds.
BLE cold connection adds 1–3 seconds. The spec does not claim 500ms on first
pairing.

---

## PowerPen-Specific Firmware Tasks (Incremental Over Ring)

These assume the ring firmware through Phase 3 (BLE HID + optical sensor +
click + power management) is complete, and Phase 6 (ball+Hall driver) is complete
or in progress. Tasks prefixed "PP" (PowerPen) to avoid numbering collision with
ring Phases 0–9.

### PP1 — Shared Interface Extensions

Prerequisite work in shared components before the pen build target can function.

**Click interface:** Add `click_source_t` enum (`CLICK_SOURCE_PRIMARY`,
`CLICK_SOURCE_SECONDARY`). Change `click_is_pressed()` to accept a source
parameter or return a bitmask. Ring callers pass `CLICK_SOURCE_PRIMARY` — no
behavioral change for the ring.

**Dead zone:** Convert `dead_zone.c` from module-level singleton state to
instanceable `dead_zone_ctx_t`. `dead_zone_update()` takes a context pointer.
Ring allocates one instance; pen allocates two (barrel: dead zone disabled,
tip: dead zone enabled). Existing `test_dead_zone.c` updated.

**Power manager wake:** Replace single-pin wake GPIO config in
`power_manager_enter_sleep()` with Kconfig-driven
`CONFIG_POWERFINGER_WAKE_GPIO_MASK` bitmask. Ring sets one bit (dome pin); pen
sets three (barrel, tip dome, DRV5032). Add Hall gate-off on the calibration
failure path.

**Done when:** Ring firmware still passes all existing tests with the extended
interfaces. `test_dead_zone.c` covers multi-instance operation.

### PP2 — PowerPen Build Target

Stand up `firmware/pen/` with CMakeLists.txt that pulls shared components.
Kconfig sets `POWERFINGER_FORM_FACTOR=PEN`, device name "PowerPen", pen pin map,
three-GPIO wake mask. Verify `idf.py build` produces a binary. Flash to ESP32-C3
dev board. Confirm BLE HID pairing works.

**Done when:** `cd firmware/pen && idf.py build` succeeds and the binary pairs
as "PowerPen" on macOS/Windows/Linux.

### PP3 — Dual-Click Integration

Wire two switches to the pen build (barrel + tip dome GPIOs). Map barrel →
left click (dead zone disabled), tip dome → right click (dead zone enabled).
Verify independent operation including simultaneous press.

**Done when:** Two switches on breadboard produce independent left/right click
HID events. Barrel click is immediate; tip click has dead zone. Simultaneous
press produces both buttons in HID report.

### PP4 — Ball+Hall Integration on PowerPen

Wire Phase 6 ball+Hall sensor rig to the pen build. Confirm cursor tracking
works through the PowerPen firmware path. Test at 30, 50, and 70 degree angles
per surface test protocol. Document per-axis sensitivity ratio at each angle —
if ratio exceeds 3:1 between axis pairs, flag for dynamic gain compensation in
the Phase 6 driver.

**Done when:** Ball+Hall cursor tracking works on glass, wood, and fabric at all
three test angles. Per-axis sensitivity data documented.

### PP5 — Deep Sleep Wake and Hall Power Gating

Combined DRV5032 wake sensor integration and Hall power gating validation — these
are the same bench session with the same hardware rig.

Wire DRV5032 breakout near one roller spindle magnet. Configure three-GPIO wake
bitmask. Verify:
- DRV5032 triggers on ball rotation (magnet sweep past sensor)
- ESP32-C3 wakes from deep sleep on each of the three wake sources independently
- MOSFET gating of Hall VCC rail works (gate on → sensors stabilize → calibrate)
- Spurious wake debounce engages after 3 false wakes in 30 seconds
- Deep sleep current with Hall sensors gated: measure and confirm < 25µA
- Wake-to-first-report latency with bonded peer: measure with logic analyzer

**DRV5032 placement test:** Position 1x1mm N52 magnet at distances from 0.5mm to
5mm from DRV5032FB. At each distance, record output toggles over 60 seconds with
no ball motion. Identify the oscillation zone and the safe placement zone.

**Done when:** All three wake sources work. Deep sleep current < 25µA. Spurious
wake debounce verified. Bonded-peer wake-to-first-report latency documented.

### PP6 — PowerPen Ergonomic Tuning

Breadboard pen rig: ESP32-C3 + 4 Hall breakouts + DRV5032 + ball socket +
barrel switch + tip dome, strapped to a pen or dowel. Use it for real tasks.
Tune:
- Dead zone thresholds for tip press
- Sensitivity scaling for ball+Hall resolution (~15–60 DPI)
- Idle/sleep timeouts (pen devices may have longer idle periods than rings)
- DRV5032 placement relative to roller (sensitivity vs false-wake tradeoff)

**Done when:** You can browse the web and do basic OS navigation using the
breadboard pen rig.

---

## Hub Compatibility

The PowerPen is a PowerFinger BLE peripheral, identical to a ring from the hub's
perspective. The hub's BLE central, role engine, and event composer work
unchanged. A PowerPen can be assigned any role (cursor, scroll, modifier).

**Multi-device composition:** Ring (cursor) + PowerPen (precision/drawing) is a
supported combination. The hub composes their events into one HID report.

---

## Surface Test Matrix (Pre-Hardware Hypotheses)

Ball+Hall is hypothesized to be surface-agnostic. Validate on prototype.

| Surface | Expected | Confidence | Notes |
|---------|----------|------------|-------|
| Wood desk | Works | High | Ball rolls on any hard surface |
| Glass | Works | High | Primary advantage over optical |
| Fabric (mouse pad) | Works | Medium | Ball may sink slightly; test |
| Paper | Works | High | |
| Glossy magazine | Works | Medium | Low friction may cause slip |
| Matte plastic | Works | High | |
| Skin (arm/leg) | Works | Medium | Soft surface — ball may not roll cleanly |
| Mirror | Works | High | Same as glass |
| Rough concrete | Works (degraded) | Low | May damage ball surface over time |

Test at 30, 50, and 70 degrees from horizontal per surface test protocol.

---

## Open Questions

1. **Ball socket manufacturing.** 3D-printed tip assembly for prototype — what
   tolerance is needed for the ball to roll freely without wobble? FDM vs SLA?
2. **Roller spindle sourcing.** Custom part with magnet press-fit. Is there an
   off-the-shelf alternative (e.g., modified clock gears)?
3. **DRV5032 placement tuning.** How close to the roller magnet? Must be outside
   the hysteresis band at rest (< 3.2mT) for all four roller orientations.
   Bench test required — see wake sensor section and PP5.
4. **Barrel switch ergonomics.** Where exactly on the barrel? Thumb rest position
   varies by grip angle. Should the cutout be elongated (slot) or round (button)?
5. **Weight distribution.** Battery at the end (away from tip) acts as
   counterweight. Is this desirable for tremor stabilization or does it make the
   pen feel unbalanced?
6. **ESP32-C3-MINI-1-N4 NRND status.** Should we spec the -N4X successor (chip
   rev v1.1, same pinout) for the prototype BOM?
7. **VBAT divider strategy.** 10MΩ/10MΩ (slow ADC settling, always-on) vs
   MOSFET-gated standard divider (zero sleep draw, but couples VBAT sampling to
   Hall gate). See electrical architecture section.
8. **Shared component extraction.** When to move shared components from
   `ring/components/` to `firmware/shared/`? Before or after pen build target
   ships? Doing it before is cleaner; doing it after risks divergent copies.

---

## Naming Glossary

The PowerPen is referred to by three names across the repository:

| Context | Name | Example |
|---------|------|---------|
| Product / user-facing | **PowerPen** | BLE device name, docs title |
| Firmware / code | **pen** | `firmware/pen/`, Kconfig `FORM_FACTOR=PEN` |
| Design-space matrix | **wand** | COMBINATORICS.md variant prefix `W`, `WSTD-BALL-NONE-NONE` |

Existing docs (COMBINATORICS.md, WAND-COMPETITIVE.md, PROTOTYPE-SPEC.md) use
"wand" — these have not been rebranded and should not be, as "wand" is the form
factor class (which may eventually include non-PowerPen variants). "PowerPen" is
the first product in the wand form factor family.

---

## References

- [COMBINATORICS.md](COMBINATORICS.md) — Full variant matrix (32 wand combos)
- [WAND-COMPETITIVE.md](WAND-COMPETITIVE.md) — Competitive landscape and patent analysis
- [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md) — EE/ME build spec (wand = Prototype 3)
- [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md) — Phase 6 (ball+Hall) and Phase 7 (IMU hybrid)
- [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md) — Kailh GM8.0 and dome analysis
- [POWER-BUDGET.md](POWER-BUDGET.md) — Pre-hardware power estimates
- [SURFACE-TEST-PROTOCOL.md](SURFACE-TEST-PROTOCOL.md) — Wand test angles: 30, 50, 70 degrees
- [IP-STRATEGY.md](IP-STRATEGY.md) — Wand-specific patent section
