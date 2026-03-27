# Handoff Summary

## 1. Current State

**Branch:** `main`. Working tree has uncommitted changes from this session
(audit fixes + BOM corrections + config updates). `build-test/` contains host
test artifacts.

**What was done this session:**

1. **6 auditors run on all hub firmware and 13 new docs:**
   - Bellman (hub firmware) — 4 critical, 5 medium, 3 low findings
   - Concurrency (hub firmware) — 3 critical, 2 high, 2 medium findings
   - Jackson counterexample (role engine + composer) — 5 counterexamples found
   - Brooks conceptual integrity (13 docs) — 4 inconsistencies found
   - Reliability (full firmware) — 5 critical, 5 high, 4 medium findings
   - Data integrity (BOMs + config) — 10 discrepancies confirmed

2. **Critical firmware fixes applied:**
   - `ble_gap_conn_find()` return checked before `delete_peer` (memory safety)
   - `ble_gap_connect()` return checked, scan resumes on failure
   - Short BLE report logged instead of silently discarded
   - `usb_hid_mouse_init()` return checked — fatal restart on failure
   - `usb_hid_mouse_send()` return checked — warnings logged
   - `cursor_dx`/`cursor_dy` int16 overflow fixed — int32 accumulation with
     clamping (accessibility hazard for multi-CURSOR-ring users)
   - `flush_to_nvs` snapshot race fixed — entries captured under mutex
   - `on_reset` count underflow fixed — zero first, then process callbacks

3. **BOM corrections applied (all 3 ring/wand BOMs):**
   - RPROG 10kΩ→20kΩ (50mA charge rate, thermal safety)
   - YS8205 removed from optical ring alt list (not a standalone sensor)
   - AH3503 alt note corrected (not pin-compatible, different package/pinout)
   - LTC4054ES5-4.2 added as true pin-compatible TP4054 alt
   - ESP32-C3-MINI-1-N4 NRND note added (successor: -N4X)
   - NTC thermistor + voltage divider resistor added ($0.02 + $0.01)
   - SI2301 P-ch MOSFET + gate pull-up resistor added ($0.03 + $0.01)
   - TP4056 and MCP73831 alt caveats clarified

4. **Config constants updated (ring_config.h):**
   - `HAPTIC_SUPPRESS_MS` 20→35 (A6 analysis)
   - `DEAD_ZONE_DISTANCE_PRO` = 15 added (A6 analysis, Pro tier)

5. **Doc inconsistencies fixed:**
   - RPROG 10k→20k corrected in PROTOTYPE-SPEC.md, CONSUMER-TIERS.md,
     POWER-BUDGET.md (all now reference BATTERY-SAFETY.md §5)
   - AGENTIC_CHECKLIST.md marked fully complete (all items [x])

6. **Tests verified:** 175 tests, 0 failures after all changes.

---

## 2. Remaining Audit Findings (Not Yet Fixed)

### From Bellman Audit

| ID | Severity | Finding | File |
|----|----------|---------|------|
| M1 | Medium | Magic number: BLE connection timeout 30000 | ble_central.c:145 |
| M2 | Medium | CCCD assumed at val_handle+1 (not guaranteed) | ble_central.c:85 |
| M3 | Medium | Magic numbers: scan interval/window 0x50/0x30 | ble_central.c:279-280 |
| M4 | Medium | HID descriptor Logical Min -32767 vs INT16_MIN | hid_report_descriptor.c:47 |
| M5 | Medium | No app-level timeout on GATT discovery | ble_central.c:186 |
| L3 | Low | Ring wheel field silently dropped at hub | main.c:39-41 |

### From Jackson Counterexample Audit

| ID | Severity | Finding | File |
|----|----------|---------|------|
| CE-2 | Medium | Forget + re-pair assigns wrong default role (index-based, not role-occupancy-based) | role_engine.c:59-66 |
| CE-5 | Medium | Last-writer-wins NVS race (stale count can overwrite newer blob) | role_engine.c flush sites |

### From Concurrency Audit

| ID | Severity | Finding | File |
|----|----------|---------|------|
| C2 | Critical | Partial MAC read race on dual-core (ble_central.c has no spinlock) | ble_central.c:375-380 |
| H4 | High | NimBLE task blocked ~200ms during NVS flash erase in role_engine_get_role | role_engine.c:156-159 |
| H5 | High | Role engine used without mutex if xSemaphoreCreateMutex fails | role_engine.c:83, main.c:79 |
| M6 | Medium | Compose acquires spinlock on every 1ms tick even when no ring connected | event_composer.c:131 |

### From Reliability Audit

| ID | Severity | Finding | File |
|----|----------|---------|------|
| R1 | Critical | OTA never confirmed — rollback fires on every reboot | both sdkconfig.defaults |
| R2 | Critical | Light sleep leaves Hall sensors unpowered — motion deadlock | power_manager.c:163 |
| R3 | Critical | Deep sleep wake-source config return unchecked — can brick device | power_manager.c:152 |
| R4 | Critical | ADC init failure causes immediate false low-battery shutdown | power_manager.c:60 |
| R5 | Critical | Hub BLE host reset doesn't restart scanning (on_sync handles it, but not documented) | ble_central.c on_reset |
| R6 | High | assert on NULL queue — crash loop instead of restart | ring main.c:176 |
| R7 | High | s_conn_param_rejected persists across reconnections | power_manager.c:27 |
| R8 | High | Battery ADC read failure is silent, skips safety check | power_manager.c:113 |
| R9 | High | os_mbuf_append return ignored in GATT callbacks | hal_ble_esp.c:212-229 |
| R10 | High | CCCD write has no completion callback — may silently fail | ble_central.c:93-98 |
| R11 | Medium | SPI double-init returns error (no guard like GPIO has) | hal_spi_esp.c:35 |
| R12 | Medium | NVS commit failure silently ignored in role engine | role_engine.c:77 |
| R13 | Medium | RING_EVT_NONE (-1) as table index is UB if passed to dispatch | ring_state.c:192 |

### From Brooks Conceptual Integrity Audit

| ID | Severity | Finding |
|----|----------|---------|
| B1 | Medium | Companion app protocol format conflict (text in COMPANION-APP-ARCH.md vs binary in MULTI-RING-PROTOCOL.md §4) |
| B2 | Low | Ball material terminology drift (steel vs ceramic across docs) |
| B3 | Low | DPI multiplier encoding inconsistency (10=1.0x vs 128=mid-range) |
| B4 | Low | BLE latency target: <20ms (CLAUDE.md) vs <15ms (PROTOTYPE-SPEC.md) |

---

## 3. Recommended Next Steps

### 3.1 Priority Fixes (Next Session)

1. **Add spinlock to ble_central.c** for s_rings[] and s_connected_count
   (Concurrency C2 — partial MAC read on dual-core)
2. **Fix power_manager wake-source return checks** (Reliability R3 — brick risk)
3. **Fix power_manager Hall re-enable after light sleep** (Reliability R2)
4. **Fix ADC init failure degradation** (Reliability R4)
5. **Replace assert with esp_restart** in ring main.c (Reliability R6)
6. **Fix default_role_for_index** to consider occupied roles (Jackson CE-2)

### 3.2 Remaining from Prior Session

- HAL migration prep changes from A9 (NRF52840-MIGRATION.md) — not yet applied
- 20 additional tests from MULTI-RING-PROTOCOL.md §8
- Test coverage gaps: calibration, power manager, PAW3204

### 3.3 Doc Reconciliation

- Resolve companion app protocol format conflict (B1)
- Update CONSUMER-TIERS.md open questions with cross-references to answers
- Reconcile ball material terminology across docs
- Reconcile DPI multiplier encoding

---

## 4. Files Modified This Session

| File | Changes |
|------|---------|
| `firmware/hub/main/main.c` | Check usb_hid_mouse_init/send return values |
| `firmware/hub/components/event_composer/event_composer.c` | Fix cursor_dx/dy overflow with int32 accumulation |
| `firmware/hub/components/ble_central/ble_central.c` | Check ble_gap_conn_find, ble_gap_connect returns; log short reports; fix on_reset count |
| `firmware/hub/components/role_engine/role_engine.c` | Fix flush_to_nvs stale-snapshot race with mutex-held snapshot |
| `firmware/ring/components/state_machine/include/ring_config.h` | HAPTIC_SUPPRESS_MS 20→35, add DEAD_ZONE_DISTANCE_PRO=15 |
| `hardware/bom/R30-OLED-NONE-NONE.csv` | RPROG 20k, remove YS8205, add LTC4054, NRND note, NTC+MOSFET, TP4056/MCP73831 caveats |
| `hardware/bom/R30-BALL-NONE-NONE.csv` | RPROG 20k, fix AH3503, add LTC4054, NRND note, NTC+MOSFET |
| `hardware/bom/WSTD-BALL-NONE-NONE.csv` | RPROG 20k, fix AH3503, add LTC4054, NRND note, NTC+MOSFET |
| `docs/PROTOTYPE-SPEC.md` | RPROG 10k→20k correction |
| `docs/CONSUMER-TIERS.md` | RPROG 10k→20k correction |
| `docs/POWER-BUDGET.md` | RPROG 10k→20k supersession note |
| `docs/AGENTIC_CHECKLIST.md` | All items marked complete |
