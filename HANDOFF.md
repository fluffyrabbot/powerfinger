# Handoff Summary

## 1. Current State

**Branch:** `main`, pushed to origin. Working tree clean except `build-test/`
(host test artifacts).

**What was done this session (2026-03-28/29):**

1. **6-agent audit cycle** launched in parallel:
   - Bellman (code smells, security) — 1C / 3H / 5M / 4L
   - Lamport (invariants, protocols) — 3 critical invariant gaps
   - Armstrong (fault isolation) — supervision score 2/5
   - Reliability (edge cases, degradation) — 2C / 4H / 6M
   - Concurrency (races, locks) — 2 concrete races
   - Hickey (simplicity) — 3.5/5 separation score

2. **Pass 1 — Critical safety (commit b9d582c):**
   - C1: CCCD write return value checked — disconnect ring on failure
   - C2: conn_handle validated in GATT discovery callbacks — prevents slot reuse corruption
   - C3: ble_gap_conn_find return checked in ring REPEAT_PAIRING — prevents bond store corruption
   - C4: Role change zeros buttons/accumulators — prevents stuck-button hazard
   - M4 bonus: Scroll accumulators widened to int32_t — prevents UB with multiple SCROLL rings

3. **Pass 2 — High reliability (commit 9034006):**
   - H1: USB HID send failure escalation — 5000-tick threshold → esp_restart()
   - H6: Defensive s_connected_count recompute in on_reset
   - H8: GATT discovery timeout — 10s timeout disconnects zombie rings

4. **Pass 3 — Robustness (commit 0d2f89c):**
   - H3: ble_gap_security_initiate return checked (hub + ring)
   - H7: ble_gap_adv_set_fields return checked (ring)
   - M1: ble_gattc_disc_all_dscs return checked
   - M2: ble_gattc_disc_all_chrs return checked
   - M3: s_connected_count read under spinlock in start_scan()
   - M8: NVS write failure re-sets dirty flag for retry

5. **Tests:** 175 tests, 0 failures after all changes.

---

## 2. Remaining Audit Findings (Triaged)

### Architectural (require design work)

| ID | Severity | Finding | Notes |
|----|----------|---------|-------|
| H2 | High | NVS flush blocks USB HID for ~200ms | Needs background FreeRTOS task. Cursor freezes on first ring connect. |
| M9 | Medium | Role lookup per-report (mutex 500+/sec on hot path) | Cache role at connect time in event_composer. Hickey + Concurrency flagged. |
| M10 | Medium | Timeout logic in main loop, not state machine | Hickey + Lamport: idle/adv timeouts should be state machine's responsibility. |
| M11 | Medium | Connection param management duplicated | power_manager + state machine both control conn params. Single owner needed. |

### Medium Priority

| ID | Severity | Finding | Notes |
|----|----------|---------|-------|
| H4 | High | portMAX_DELAY mutex in NimBLE callback | Low risk with deferred flush pattern, but bounded timeout would be safer. |
| H5 | High | Sensor failure → infinite reboot loop | Current 60s deep sleep cycles are tolerable. NVS reboot counter better. |
| M5 | Medium | Dual advertising timeout paths | BLE stack + main loop timer can fire independently. Unify to one. |
| M6 | Medium | Battery level always reports 100% | No user warning before low-battery shutdown. |
| M7 | Medium | BLE event queue drop → stale state machine | Add periodic reconciliation check (state vs hal_ble_is_connected). |
| M12 | Medium | No PowerFinger-specific device authentication | Hub connects to any BLE HID mouse. Add manufacturer-specific adv data. |
| M13 | Medium | SLEEP_TIMEOUT in CONNECTED_ACTIVE silently dropped | Specify: is sleep timeout only from IDLE, or from any connected state? |

### Low Priority (hardening, documentation)

| ID | Finding |
|----|---------|
| L1 | Callback pointers not documented as write-once |
| L2 | BLE UUIDs as inline hex (0x2902, 0x2A4D, 0x1812) |
| L3 | Float math in Phase 0 test path pulls in libm |
| L4 | NVS blob struct packing not guaranteed across compilers |
| L5 | Timer wraparound reliance on unsigned semantics undocumented |
| L6 | Counter type mismatch: uint8_t vs threshold could overflow |
| L7 | Duplicate MAC not validated on NVS role blob load |
| L8 | No boot reason logging (esp_reset_reason()) |
| L9 | LOCK/UNLOCK macro names hide spinlock vs mutex distinction |
| L10 | event_composer_mark_connected naming asymmetry |
| L11 | RING_EVT_NONE = -1 sentinel forces signed comparison |
| L12 | Hub shutdown: no zero USB report before esp_restart() |

### Hickey Simplification Opportunities

- Phase 0 fake motion loop duplicates main loop event pump — extract shared function
- Redundant memset(&actions, 0) in ring main.c — state machine already zeroes internally
- #ifdef ESP_PLATFORM scattered through 6 files — should be behind hal_log.h / hal_sync.h

### Lamport Specification Gaps

- No CALIBRATING state in state machine (CLAUDE.md says it exists)
- `subscribed` flag set but never read — use it in H8 timeout or remove
- `wheel` field parsed by hub but not forwarded through event composer
- Role change during active use not formally specified
- Multiple CURSOR rings: OR'd buttons + summed deltas not documented as spec

---

## 3. Recommended Next Steps

### 3.1 Next Implementation Session

1. **H2: NVS flush to background task** — biggest remaining UX issue (200ms cursor freeze)
2. **M9: Cache role at connect time** — removes mutex from hot path, simplifies event_composer_feed
3. **M12: Add manufacturer-specific adv data** — prevent hub connecting to non-PowerFinger HID devices

### 3.2 Design Decisions Needed (BDFL)

- **M10/M11**: Should timeout logic + conn param management move into the state machine? (Hickey + Lamport both recommend this; trades main loop simplicity for state machine complexity)
- **M13**: Should SLEEP_TIMEOUT fire from CONNECTED_ACTIVE? (Currently silently dropped)
- **H5**: How aggressive should sensor-reboot-loop prevention be? (Current: 60s deep sleep cycles. Option: NVS counter, permanent sleep after N reboots.)

### 3.3 Prior Session Carry-Forward

- HAL migration prep for nRF52840 (from A9 doc)
- 20 additional tests from MULTI-RING-PROTOCOL.md §8
- Test coverage gaps: calibration, power manager, PAW3204
- Doc reconciliation: companion app protocol format, ball material terminology, DPI encoding, BLE latency target

---

## 4. Commits This Session

| Hash | Description |
|------|-------------|
| `b9d582c` | fix(firmware): 4 critical safety fixes (C1-C4 + M4) |
| `9034006` | fix(hub): 3 high-priority reliability fixes (H1, H6, H8) |
| `0d2f89c` | fix(firmware): 7 robustness fixes (H3, H7, M1-M3, M8) |

## 5. Files Modified This Session

| File | Changes |
|------|---------|
| `firmware/hub/components/ble_central/ble_central.c` | C1 (CCCD write check), C2 (conn_handle validation in discovery), H3 (security initiate check), H6 (count recompute), H8 (discovery timeout + connect_time_ms), M1-M3 (return checks, lock hygiene) |
| `firmware/hub/components/ble_central/include/ble_central.h` | H8: ble_central_check_discovery_timeout() declaration |
| `firmware/hub/components/event_composer/event_composer.c` | C4 (role change zeros buttons), M4 (int32_t scroll accumulators) |
| `firmware/hub/components/role_engine/role_engine.c` | M8 (NVS retry on flush failure) |
| `firmware/hub/main/main.c` | H1 (USB fail escalation), H8 (discovery timeout call) |
| `firmware/ring/components/hal/esp_idf/hal_ble_esp.c` | C3 (REPEAT_PAIRING conn_find check), H3 (security initiate check), H7 (adv fields check) |
