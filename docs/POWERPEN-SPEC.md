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
| Battery | LiPo 100–150mAh | Target 2–5 days; see power budget below |

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

**Firmware integration:** Configure GPIO8 as a deep sleep wake source
(`esp_sleep_enable_gpio_wakeup`). On wake, the power manager gates on the 4x
DRV5053 array via the MOSFET, waits for Hall sensors to stabilize (~1ms), then
runs calibration. The DRV5032 output is not used during active tracking — only
the 4x DRV5053 analog array provides motion data.

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
| Power manager | `ring/components/power/` | Already decoupled from ring state types; uses its own event enum |
| Sensor interface | `ring/components/sensors/sensor_interface.h` | Ball+Hall driver implements same `sensor_init/read/power_down/wake` |
| Click interface | `ring/components/click/click_interface.h` | Barrel switch is just another GPIO click source |
| Calibration | `ring/components/click/calibration.c` | Same Hall calibration logic |
| Dead zone | `ring/components/click/dead_zone.c` | Same dead zone state machine |
| Diagnostics | `ring/components/diagnostics/` | Same snapshot structure |
| Runtime health | `ring/components/runtime_health/` | Same fault policy |
| Test suite | `firmware/test/` | Same host-side test harness |

### PowerPen-Specific (New or Configured)

| Concern | What Changes | Approach |
|---------|-------------|----------|
| State machine | Own `pen_state_t` / `pen_event_t` types | See "State Machine" section below |
| Pin assignments | Different GPIO map (+ DRV5032 wake pin) | Kconfig / `sdkconfig.defaults` per target |
| Click topology | Two independent switches (barrel + tip) instead of one dome | Click driver reads two GPIOs; HID report maps both |
| Dead zone tuning | Barrel click may not need dead zone; tip press-down does | Configurable per-click-source dead zone enable/disable |
| Hall power gating | GPIO for MOSFET gate | Already in power manager spec (Phase 3); pen just uses it |
| Wake sensor | DRV5032 on always-on LDO rail | See wake sensor circuit above |
| Device identity | BLE device name, appearance | Kconfig: "PowerPen" vs "PowerFinger Ring" |
| Partition table | Possibly different flash layout for larger battery | Likely identical; evaluate |

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

The PowerPen has its own state machine types (`pen_state_t`, `pen_event_t`,
`pen_actions_t`) with the same structure and transitions as the ring state machine.
Separate types rather than a shared typedef because:

- The ring and PowerPen are distinct products with distinct firmware binaries.
- As the products diverge, pen-specific states or events may emerge (e.g.,
  `PEN_EVT_WAKE_SENSOR_TRIGGERED` has no ring equivalent).
- Grep-ability: `pen_state_` finds pen code, `ring_state_` finds ring code.
- No runtime cost — these are compile-time enum types.

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

### States

```c
typedef enum {
    PEN_STATE_DEEP_SLEEP = 0,
    PEN_STATE_BOOTING,
    PEN_STATE_ADVERTISING,
    PEN_STATE_CONNECTED_ACTIVE,
    PEN_STATE_CONNECTED_IDLE,
    PEN_STATE_COUNT,
} pen_state_t;
```

### Events

```c
typedef enum {
    PEN_EVT_NONE = -1,
    PEN_EVT_GPIO_WAKE = 0,          // Barrel switch, tip dome, or DRV5032 wake sensor
    PEN_EVT_CALIBRATION_DONE,
    PEN_EVT_CALIBRATION_FAILED,
    PEN_EVT_BLE_CONNECTED,
    PEN_EVT_BLE_DISCONNECTED,
    PEN_EVT_BLE_ADV_TIMEOUT,
    PEN_EVT_MOTION_DETECTED,
    PEN_EVT_CLICK_ACTIVITY,
    PEN_EVT_IDLE_TIMEOUT,
    PEN_EVT_SLEEP_TIMEOUT,
    PEN_EVT_LOW_BATTERY,
    PEN_EVT_COUNT,
} pen_event_t;
```

### Actions

```c
typedef struct {
    bool start_advertising;
    bool stop_advertising;
    bool enter_deep_sleep;
    bool gate_hall_sensors_on;   // pen-specific: MOSFET gate control
    bool gate_hall_sensors_off;
} pen_actions_t;
```

**Wake sources for deep sleep (all three are GPIO interrupt sources):**
- Barrel switch (GPIO4) — user picks up pen and clicks
- DRV5032 wake sensor (GPIO8) — user rolls pen on surface, magnet triggers
  digital Hall switch
- Tip dome (GPIO5) — user presses pen tip into surface

**Wake sequence:** GPIO interrupt fires → `PEN_EVT_GPIO_WAKE` → transition to
`PEN_STATE_BOOTING` → power manager gates on Hall VCC (MOSFET) → wait ~1ms for
DRV5053 stabilization → run calibration → `PEN_EVT_CALIBRATION_DONE` → transition
to `PEN_STATE_ADVERTISING`. Target: < 500ms from wake to first valid HID report.

---

## PowerPen-Specific Firmware Tasks (Incremental Over Ring)

These assume the ring firmware through Phase 3 (BLE HID + optical sensor +
click + power management) is complete, and Phase 6 (ball+Hall driver) is complete
or in progress.

### P1 — PowerPen Build Target

Stand up `firmware/pen/` with CMakeLists.txt that pulls shared components.
Verify `idf.py build` produces a binary. Flash to ESP32-C3 dev board. Confirm
BLE HID pairing works (same as ring Phase 0, just different device name).

**Done when:** `cd firmware/pen && idf.py build` succeeds and the binary pairs
as "PowerPen" on macOS/Windows/Linux.

### P2 — Pen State Machine

Implement `pen_state.h` / `pen_state.c` with `pen_state_t`, `pen_event_t`,
`pen_actions_t`. Same transition logic as the ring state machine, plus
`gate_hall_sensors_on/off` actions. Host-testable (no hardware dependencies).

**Done when:** `test_pen_state.c` passes all transition tests on host, mirroring
`test_state_machine.c` coverage.

### P3 — Dual-Click Driver

Extend click driver to support two independent click sources on separate GPIOs.
Map barrel switch → left click, tip dome → right click (configurable). Each
click source has independent dead zone enable/disable.

**Done when:** Two switches on breadboard produce independent left/right click
HID events.

### P4 — Ball+Hall Integration on PowerPen

Wire Phase 6 ball+Hall sensor rig to the pen build. Confirm cursor tracking
works through the PowerPen firmware path. Test at 30, 50, and 70 degree angles
per surface test protocol.

**Done when:** Ball+Hall cursor tracking works on glass, wood, and fabric at all
three test angles.

### P5 — DRV5032 Wake Sensor Integration

Wire DRV5032 breakout near one roller spindle magnet. Configure GPIO8 as deep
sleep wake source. Verify:
- DRV5032 triggers on ball rotation (magnet sweep past sensor)
- ESP32-C3 wakes from deep sleep on DRV5032 output toggle
- Wake + Hall gate-on + calibration completes in < 500ms
- Deep sleep current with DRV5032 powered: measure and confirm < 25µA

**Done when:** Rolling the ball on the breadboard rig wakes the device from deep
sleep and cursor tracking begins within 500ms.

### P6 — Hall Power Gating Validation

Verify MOSFET gating of Hall VCC rail. Measure deep sleep current with Hall
sensors gated off and DRV5032 as sole active sensor. Confirm gate-on + Hall
stabilization + recalibration time.

**Done when:** Deep sleep current measured < 25µA with Hall sensors gated.
Wake-to-first-report latency measured and documented.

### P7 — PowerPen Ergonomic Tuning

Breadboard pen rig: ESP32-C3 + 4 Hall breakouts + DRV5032 + ball socket +
barrel switch, strapped to a pen or dowel. Use it for real tasks. Tune:
- Dead zone thresholds for barrel click vs tip press
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
3. **DRV5032 placement tuning.** How close to the roller magnet? Too close =
   always triggered (magnet never leaves operate threshold). Too far = ball must
   rotate significantly before wake fires. Needs bench testing with the actual
   1x1mm N52 magnets.
4. **Barrel switch ergonomics.** Where exactly on the barrel? Thumb rest position
   varies by grip angle. Should the cutout be elongated (slot) or round (button)?
5. **Weight distribution.** Battery at the end (away from tip) acts as
   counterweight. Is this desirable for tremor stabilization or does it make the
   pen feel unbalanced?
6. **ESP32-C3-MINI-1-N4 NRND status.** Should we spec the -N4X successor (chip
   rev v1.1, same pinout) for the prototype BOM?

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
