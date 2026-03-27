# Handoff Summary

## 1. Current State

**Branch:** `main`, 5 commits ahead of origin (not pushed). Working tree clean
except `build-test/` (host test artifacts) and `docs/AGENTIC_CHECKLIST.md` (new,
unstaged).

```
e667442 fix(firmware): finish remaining audit items — types, errors, supervision, tests
f6ead50 fix(firmware): round 2 audit fixes — supervision, type safety, error propagation
c627c51 test(firmware): add event composer tests, strengthen state machine coverage
0900d5d fix(firmware): medium-priority audit fixes — role engine, sensor, OTA, BLE
78c3c88 feat(firmware): implement ring + hub firmware through Phase 4
```

**What exists:** Complete ring + hub firmware through Phase 4 of
[FIRMWARE-ROADMAP.md](docs/FIRMWARE-ROADMAP.md). 46 tests, 123 assertions, 0
failures. Two rounds of audits (9 specialized auditors) with all findings fixed.
Ten design/analysis docs in `docs/`. BOMs for all 4 prototypes.

**What does NOT exist:** No physical hardware validation. No ESP-IDF build
(`idf.py build` has never been run). No schematics, PCB layouts, or CAD models.
No companion app. The hub USB HID output is a stub.

---

## 2. Crucial Context

### Architecture decisions (load-bearing)

- **BLE→main queue.** Ring uses a FreeRTOS queue for BLE→main task events. BLE
  callbacks never dispatch directly — eliminates state machine races.
- **State machine controls advertising.** HAL auto-advertising was removed. The
  state machine starts advertising via action flags on boot and after disconnect.
  If this path doesn't fire on first boot, the ring is invisible.
- **Event composer `connected` flag.** Managed only by `mark_connected` /
  `ring_disconnected`, NOT by `feed()`. Prevents stale BLE notifications from
  resurrecting disconnected rings.
- **Opaque typed handles.** `struct hal_spi_ctx *`, not `void *` — compile-time
  safety across HAL boundary.
- **Power manager decoupled.** Uses `power_event_t` enum, not `ring_state.h`
  types — reusable for wand firmware without pulling in ring state machine.

### Gotchas (discovered by auditors)

- **PAW3204 is NOT SPI.** Proprietary 2-wire serial, bit-banged via GPIO with
  direction caching. `T_RESYNC_US` corrected from 1 to 4 per datasheet.
- **Zero-report on transition.** Hub must send a zero-report when transitioning
  from non-zero (button release), otherwise host sees stuck click.
- **Deep sleep wake config.** Without GPIO wake + timer safety net, deep sleep =
  bricked device. Both are configured in `power_manager.c`.
- **`sensor_wake()` SDIO direction.** Must set SDIO direction explicitly before
  resync clocks, or the first read after wake returns garbage.
- **Bond recovery peer address.** Must use actual address from
  `ble_gap_conn_find`, not zeroed struct — zeroed address causes silent
  reconnection failure.
- **Battery level placeholder.** `hal_ble_esp.c:226` always reports 100%. Power
  manager reads real VBAT via ADC but doesn't wire it to the BLE characteristic.

### Test coverage gaps (known)

- Calibration: retry logic, variance computation, motion threshold — 0 tests
- Power manager: battery cutoff, adaptive interval switching — 0 tests
- PAW3204: product ID verification, register timing — 0 tests
- Mock HAL supports failure injection (`mock_hal_set_adc_mv`,
  `mock_hal_inject_ble_send_failure`) but no tests use it yet

---

## 3. Work Plan

Two tracks run in parallel. **Track A** is pure agentic work (no hardware).
**Track B** requires dev boards / prototypes. Items are numbered by recommended
execution order within each track.

### Track A — Agentic Research & Firmware (no hardware needed)

Full checklist with sub-tasks: [docs/AGENTIC_CHECKLIST.md](docs/AGENTIC_CHECKLIST.md)

#### A1. TinyUSB HID Integration (Hub) — `AGENTIC_CHECKLIST.md §1.1`

The only stub in the ring→hub→PC data path. Implement `usb_hid_mouse.c` and
`hid_report_descriptor.c` using ESP-IDF TinyUSB component. Wire into existing
`event_composer.c` output. Add host-side tests via mock HAL.

**Files:** `firmware/hub/components/usb_hid/usb_hid_mouse.c`,
`firmware/hub/components/usb_hid/hid_report_descriptor.c`

**Gotcha:** Report descriptor should be superset of ring's BLE descriptor (add
horizontal scroll). The hub main loop's zero-report stuck-button logic depends
on `usb_hid_mouse_send` actually transmitting.

#### A2. Dual-Footprint PCB Feasibility — `AGENTIC_CHECKLIST.md §1.2`

Determines whether the EE designs one ring PCB or two. Compare PAW3204 vs
PMW3360 pinouts, footprints, voltage domains. Produce a constraints document
for schematic capture.

#### A3. PMW3360 Ball Diameter Optimization — `AGENTIC_CHECKLIST.md §1.3`

Model focal distance geometry for 4–8mm balls against 10mm height budget. The
answer constrains Pro ring shell CAD and sensor cavity dimensions.

#### A4. Battery Safety Spec — `AGENTIC_CHECKLIST.md §2.1`

LiPo on a finger. Research IEC 62133-2, TP4054 charge behavior, protection
circuit requirements, and thermal constraints in a sealed ring. Write safety
spec section for PROTOTYPE-SPEC.md.

#### A5. Regulatory Pre-Scan — `AGENTIC_CHECKLIST.md §2.2`

Map FCC Part 15 and CE RED requirements. Determine what ESP32-C3-MINI-1-N4
module pre-certification covers vs. what still needs testing. Identify PCB
layout constraints (antenna keep-out, ground plane, shielding).

#### A6. Piezo/Haptic Dead Zone Analysis — `AGENTIC_CHECKLIST.md §2.3`

Model whether LRA vibration registers as sensor motion. Determine optimal
`HAPTIC_SUPPRESS_MS`. Decide if Pro tier needs mechanical isolation or a
different click mechanism.

#### A7. Patent Claim Mapping — `AGENTIC_CHECKLIST.md §2.4`

Pull full claims for US8648805B2 and 3 other ring patents. Map claim elements
to P0 design. Produce claim chart for patent attorney. FTO opinion is blocked
on funding — this cuts attorney hours when it happens.

#### A8. Multi-Source Vendor Verification — `AGENTIC_CHECKLIST.md §2.5`

Verify every BOM line item is available from 2+ distributors. Check that
"Alternatives" column entries are real (in stock, pin-compatible).

#### A9. nRF52840 Migration Spec — `AGENTIC_CHECKLIST.md §2.6`

Map all 9 HAL interfaces to Zephyr/nRF equivalents. Identify gaps that would
be cheaper to fix now while the HAL is young. POWER-BUDGET.md concludes
ESP32-C3 can't hit consumer battery targets — this migration is inevitable.

#### A10–A14. Tier 3 items — `AGENTIC_CHECKLIST.md §3.1–3.5`

Accessibility research synthesis, surface test protocol, companion app
architecture (GATT schema + Web Serial), multi-ring composition protocol
formalization, defensive publication drafts. Valuable, not blocking. Do when
Track B is waiting on hardware.

---

### Track B — Hardware Validation (requires dev boards)

#### B1. Phase 0: BLE HID on bare ESP32-C3

Flash ring firmware to ESP32-C3 SuperMini. Pair with macOS/Windows/Linux/iOS.
Verify cursor circles, click events, reconnect after reboot, bond recovery when
host deletes bond.

**Build:** `idf.py set-target esp32c3 && idf.py build && idf.py flash`

**Config:** `CONFIG_SENSOR_NONE=y`, `CONFIG_CLICK_NONE=y` (defaults).

**Watch for:** State machine advertising path must fire on first boot (HAL
auto-advertise was removed). May need `sdkconfig.defaults` tuning for NimBLE
memory. Bond asymmetry recovery: delete bond on phone, verify ring re-enters
pairing.

**Hardware:** ESP32-C3 SuperMini (~$3).

#### B2. Phase 1–2: PAW3204 sensor + snap dome click

Wire PAW3204 breakout + snap dome to C3 dev board. Set
`CONFIG_SENSOR_PAW3204=y`, `CONFIG_CLICK_SNAP_DOME=y`. Verify cursor tracks
desk movement, click doesn't jump cursor, click-and-drag works.

**Done when:** Can browse a website with dev board taped to finger.

**Watch for:** GPIO pins SCLK/SDIO default to 4/5 in Kconfig. Bit-bang timing
is conservative but untested on real silicon. SDIO direction caching — first
real hardware test. Drift at rest should be <1px/10s (dead zone + calibration).

**Hardware:** PAW3204 breakout (~$5–8), snap dome (~$0.10), jumper wires.

#### B3. Phase 3: Power measurement

Measure actual mA in each state with USB power monitor. Compare against
[POWER-BUDGET.md](docs/POWER-BUDGET.md).

**Measure:** Active@7.5ms, active@15ms, idle@15ms (adaptive interval), deep
sleep. Log average mA over 30min simulated use session.

**Watch for:** Light sleep between 15ms BLE events may not engage on SuperMini
modules (module-level overhead). 50-failure sensor counter triggers deep sleep
reboot — verify no false positives during surface lift.

**Hardware:** USB power monitor or INA219 breakout (~$3).

#### B4. Phase 4: Hub two-ring validation

Flash hub to ESP32-S3. Pair 2 C3 rings. **Depends on A1 (TinyUSB) completing
first** — without it, the hub can't output USB HID.

**Done when:** Ring 1 moves cursor, Ring 2 scrolls. Disconnect mid-click: no
stuck button on host. Reconnect: role resumes.

**Watch for:** Hub `sdkconfig` needs `CONFIG_TINYUSB_HID_ENABLED=y`. CDC+HID
composite USB if you also want serial debug/config.

**Hardware:** ESP32-S3 dev board (~$5), 2x ESP32-C3 from B1/B2.

#### B5. Phase 6: Ball+Hall sensor driver

Implement `sensor_interface.h` for 4-Hall-sensor DRV5053 array. Differential
rotation computation, boot calibration, temperature compensation.

**Watch for:** DRV5053 has no sleep mode — 4 sensors draw ~12mA continuously.
Power gating via MOSFET GPIO already wired in `power_manager.c`. ADC reads 4
channels — ensure sampling doesn't alias with BLE radio events.

**Hardware:** 4x DRV5053, 5mm steel ball, roller spindle assembly, magnets.

---

## 4. Dependency Graph

```
Track A (agentic)              Track B (hardware)
─────────────────              ──────────────────
A1 TinyUSB ──────────────────► B4 Hub validation
A2 Dual-footprint ───────────► EE schematic capture
A3 Ball diameter ─────────────► EE Pro ring shell CAD
A4 Battery safety ────────────► EE charge circuit design
A5 Regulatory ────────────────► EE PCB layout (keep-out zones)
A8 Vendor verification ───────► Parts ordering

B1 Phase 0 BLE ──────────────► B2 Phase 1-2 sensor+click
B2 Phase 1-2 ────────────────► B3 Phase 3 power measurement
A1 + B2 ─────────────────────► B4 Phase 4 hub
B2 ──────────────────────────► B5 Phase 6 ball+Hall
```

**Critical path:** A1 (TinyUSB) gates B4 (hub validation). Everything else in
Track A runs in parallel with Track B.

---

## 5. Audit Cycle

| After | Run | Why |
|-------|-----|-----|
| B1–B2 (first hardware) | `bellman-auditor` on full firmware | Catches timing, ISR, BLE edge cases only visible on real hardware |
| A1 (TinyUSB) | `concurrency-audit-architect` + `jackson-counterexample-auditor` on hub | TinyUSB adds new task/callback context |
| B5 (ball+Hall) | `dijkstra-correctness-auditor` on new driver | Differential rotation math is error-prone |
| Companion app MVP | `schneier-security-auditor` | USB serial command interface is a new attack surface |

---

## 6. Files Quick Reference

| Area | Key files |
|------|-----------|
| Ring main loop | `firmware/ring/main/main.c` |
| Ring state machine | `firmware/ring/components/state_machine/ring_state.c` |
| Ring config constants | `firmware/ring/components/state_machine/include/ring_config.h` |
| PAW3204 driver | `firmware/ring/components/sensors/paw3204.c` |
| Dead zone + calibration | `firmware/ring/components/click/dead_zone.c`, `calibration.c` |
| Power manager | `firmware/ring/components/power/power_manager.c` |
| BLE HID (ring) | `firmware/ring/components/hal/esp_idf/hal_ble_esp.c` |
| HAL headers (all 9) | `firmware/ring/components/hal/include/hal_*.h` |
| Hub main loop | `firmware/hub/main/main.c` |
| Hub BLE central | `firmware/hub/components/ble_central/ble_central.c` |
| Event composer | `firmware/hub/components/event_composer/event_composer.c` |
| Role engine | `firmware/hub/components/role_engine/role_engine.c` |
| USB HID stub | `firmware/hub/components/usb_hid/usb_hid_mouse.c` |
| Tests | `firmware/test/test_*.c` |
| Mock HAL | `firmware/test/mocks/mock_hal.c`, `mock_hal.h` |
| BOMs | `hardware/bom/*.csv` |
| Agentic checklist | `docs/AGENTIC_CHECKLIST.md` |
| Firmware roadmap | `docs/FIRMWARE-ROADMAP.md` |
| Design matrix | `docs/COMBINATORICS.md` |
