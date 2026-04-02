# Multi-Ring Composition Protocol Specification

Formal specification of the protocol by which the PowerFinger USB hub dongle
manages role assignment, conflict resolution, event composition, and companion
app interaction for up to 4 simultaneously connected rings.

This document formalizes behavior that is partially implemented in hub firmware
(`role_engine.c`, `event_composer.c`, `ble_central.c`, `main.c`) and partially
implicit. Sections marked **[IMPLEMENTED]** describe behavior that exists in
code. Sections marked **[SPECIFIED]** describe behavior that must be implemented
to complete the protocol.

---

## 1. Definitions

| Term | Meaning |
|------|---------|
| Hub | ESP32-S3 USB dongle running BLE central + USB HID device firmware |
| Ring | PowerFinger BLE peripheral (fingertip ring or wand) |
| MAC | 6-byte BLE peer identity address, used as the ring's persistent identifier |
| Role | Mapping that determines how a ring's HID events translate to USB HID report fields |
| Slot | Index 0..3 in the hub's connection table (`s_rings[]` in `ble_central.c`) |
| NVS | Non-volatile storage (ESP-IDF flash key-value store) |
| Composition | Process of merging per-ring state into a single USB HID report |

### 1.1 Roles

Defined in `role_engine.h` as `ring_role_t`:

| Role | Enum | Cursor effect | Button mapping | Delta mapping |
|------|------|--------------|----------------|---------------|
| CURSOR | `ROLE_CURSOR` (0) | X/Y move pointer | bit 0 -> left click (0x01) | dx/dy -> `cursor_dx`/`cursor_dy` |
| SCROLL | `ROLE_SCROLL` (1) | X/Y scroll | bit 0 -> right click (0x02) | dx -> `scroll_h`, dy -> `scroll_v` |
| MODIFIER | `ROLE_MODIFIER` (2) | None | bit 0 -> middle click (0x04) | Ignored |

Multiple rings may share the same role. This is explicitly permitted (see
section 5 for composition semantics when roles overlap).

---

## 2. Ring Connection State Machine

Each ring slot (0..3) in the hub maintains the following state machine. The
states describe the hub's view of the ring, not the ring's own state.

### 2.1 States

| State | Description |
|-------|-------------|
| `EMPTY` | Slot is unused. No MAC associated. No NVS entry references this slot. |
| `KNOWN_DISCONNECTED` | MAC has a persisted role in NVS but the ring is not connected. |
| `CONNECTING` | BLE GAP connect initiated. Slot reserved, MAC populated, `connected = false`. |
| `ACTIVE` | BLE connected, paired, subscribed to HID notifications. `connected = true`. |
| `ROLE_SWAPPING` | **[SPECIFIED]** Composition paused for this ring while a role change is applied. |

### 2.2 Transitions

```
                  ┌────────────────────────────────────────────┐
                  │                                            │
    ┌─────────┐   │  BLE_GAP_EVENT_DISC     ┌────────────┐    │
    │  EMPTY  │───┼─(new MAC, free slot)───>│ CONNECTING  │    │
    └─────────┘   │                          └──────┬─────┘    │
         ^        │                                 │          │
         │        │              BLE_GAP_EVENT_CONNECT (ok)     │
    role_engine   │                                 v          │
    _forget()     │                          ┌──────────┐      │
         │        │                          │  ACTIVE   │<─┐  │
         │        │                          └─────┬────┘   │  │
    ┌────┴────────┴──┐  BLE_GAP_EVENT_DISCONNECT   │        │  │
    │    KNOWN_      │<────────────────────────────┘   reconnect
    │  DISCONNECTED  │                                      │  │
    └────────────────┘──────────(scan match)────────────────>┘  │
                  │                                            │
                  │  companion: role_swap / role_set            │
                  │  (only while ACTIVE)                       │
                  │        ┌───────────────┐                   │
                  └───────>│ ROLE_SWAPPING │───(complete)──────┘
                           └───────────────┘
```

### 2.3 Transition Table

| From | Trigger | Guard | To | Action |
|------|---------|-------|----|--------|
| EMPTY | BLE_GAP_EVENT_DISC | `find_free_slot() >= 0` AND advertisement carries HID UUID 0x1812 + PowerFinger HID service-data marker | CONNECTING | Cancel scan, initiate `ble_gap_connect()` |
| CONNECTING | BLE_GAP_EVENT_CONNECT (status=0) | Slot available | ACTIVE | Copy MAC, call `role_engine_get_role()` (assigns default if new), call `event_composer_mark_connected(ring_index, role)`, initiate security + GATT discovery, resume scan |
| CONNECTING | BLE_GAP_EVENT_CONNECT (status!=0) | -- | EMPTY | Resume scan |
| ACTIVE | BLE_GAP_EVENT_DISCONNECT | -- | KNOWN_DISCONNECTED | Call `event_composer_ring_disconnected()`, clear slot `connected` flag, decrement count, resume scan |
| ACTIVE | companion: `role_set` or `role_swap` | Target ring is ACTIVE | ROLE_SWAPPING | See section 4 |
| ROLE_SWAPPING | Swap complete | -- | ACTIVE | Resume composition for this ring |
| KNOWN_DISCONNECTED | BLE_GAP_EVENT_DISC (same MAC) | Free slot available | CONNECTING | Same as EMPTY->CONNECTING, but MAC is already known to role engine |
| KNOWN_DISCONNECTED | `role_engine_forget()` | -- | EMPTY | Remove NVS entry, free slot for new MAC |
| Any | BLE host reset (`on_reset`) | -- | KNOWN_DISCONNECTED (if had MAC) or EMPTY | Disconnect all, notify event composer for each, zero `s_connected_count` |

### 2.4 Invariants

The following invariants must hold in every reachable state:

**INV-1 (No stuck buttons):** If a ring transitions out of ACTIVE for any
reason, `event_composer_ring_disconnected(ring_index)` must be called before any
subsequent `event_composer_compose()` invocation. This is the accessibility
invariant. A stuck button on a device used by someone with limited mobility may
require third-party intervention to resolve.

- **Current implementation:** `on_ring_connection()` in `main.c` calls
  `event_composer_ring_disconnected()` synchronously in the `connected=false`
  path.

**INV-2 (No stale resurrection):** A ring that has transitioned out of ACTIVE
must not contribute to composition until it transitions back to ACTIVE via a
new `event_composer_mark_connected()` call. Stale BLE notifications arriving
after disconnect must be silently dropped.

- **Current implementation:** `event_composer_feed()` checks `r->connected`
  and returns immediately if false.

**INV-3 (Slot count consistency):** `ble_central_connected_count()` must equal
the number of slots where `connected == true`. Scan is suppressed when count
equals `HUB_MAX_RINGS`.

- **Current implementation:** `s_connected_count` is incremented in
  `BLE_GAP_EVENT_CONNECT` and decremented in `BLE_GAP_EVENT_DISCONNECT` and
  `on_reset`.

**INV-4 (Role uniqueness is NOT required):** Multiple rings may hold the same
role. The composition rules (section 5) handle this by aggregating contributions
from all rings sharing a role. This is a deliberate design decision to support
two-cursor or two-scroll configurations.

**INV-5 (NVS persistence consistency):** The NVS role blob must reflect the
in-memory `s_entries[]` state. Writes are deferred (outside the mutex) to avoid
blocking the NimBLE task during flash sector erase, but must complete before the
next power cycle for persistence to be reliable.

---

## 3. Role Assignment Protocol

### 3.1 First Connection (Auto-Assignment) **[IMPLEMENTED]**

When a ring's MAC is not found in the role engine's `s_entries[]` table:

**Precondition:** `s_entry_count < HUB_MAX_RINGS`.

**Assignment rule:** The ring receives `default_role_for_index(s_entry_count)`:

| Entry index | Default role |
|-------------|-------------|
| 0 | CURSOR |
| 1 | SCROLL |
| 2 | MODIFIER |
| 3 | MODIFIER |

**Postcondition:** New entry appended to `s_entries[]`, `s_entry_count`
incremented, NVS blob written (outside mutex).

**Fallback (all slots full):** `role_engine_get_role()` returns `ROLE_CURSOR`.
This is a degraded state -- the ring will function but its role is not persisted
and cannot be reassigned.

**Reference:** `role_engine_get_role()` in `role_engine.c`, lines 121-162.

### 3.2 Reconnection **[IMPLEMENTED]**

When a previously known ring reconnects, `role_engine_get_role()` finds its MAC
in `s_entries[]` and returns the persisted role. No NVS write occurs.

**Edge case -- reconnect after role was reassigned while disconnected:** The role
engine's in-memory state is authoritative. If a companion app changed this
ring's role while it was disconnected, the reconnecting ring receives the new
role. The ring itself has no role state -- roles are hub-side only.

### 3.3 NVS Persistence Format **[IMPLEMENTED]**

```c
typedef struct {
    uint8_t version;                    // ROLE_NVS_VERSION = 1
    uint8_t count;                      // number of entries (0..HUB_MAX_RINGS)
    role_entry_t entries[HUB_MAX_RINGS]; // MAC + role pairs
} role_blob_t;
```

Each `role_entry_t` is 7 bytes (6 bytes MAC + 1 byte `ring_role_t`).

**Version migration:** On load, if `blob.version != ROLE_NVS_VERSION`, the
entire blob is discarded and `s_entry_count` is reset to 0. Future versions
must implement migration logic before incrementing `ROLE_NVS_VERSION`.

**NVS key:** `"roles"` (defined as `ROLE_NVS_KEY`).

**Write timing:** Outside the role engine mutex. This creates a window where
in-memory state has advanced past NVS state. On power loss during this window,
the most recent role change is lost. This is acceptable: the ring will
re-auto-assign on next connection.

### 3.4 Conflict Resolution **[PARTIALLY IMPLEMENTED]**

**Scenario:** Two rings are stored in NVS with the same role (possible if a
companion app assigned them that way, or if NVS data was manually manipulated).

**Current behavior:** Both rings receive the same role. Composition merges
their contributions (section 5). This is valid per INV-4.

**Scenario: ring connects and its NVS-stored role conflicts with a policy
requiring unique roles.** **[SPECIFIED]**

If a future firmware version enforces unique role assignment (one ring per role),
the conflict resolution rule is:

1. The already-connected ring keeps its role.
2. The newly connecting ring is reassigned to the next available default role
   (i.e., the lowest-index role not currently held by any connected ring).
3. The reassignment is persisted to NVS.
4. The companion app is notified of the reassignment (see section 4).

The current firmware does NOT enforce unique roles. This subsection defines
the algorithm for when/if that policy is added.

### 3.5 Ring Factory Reset **[SPECIFIED]**

When a previously bonded ring performs a factory reset, it will present as an
unbonded device requesting fresh pairing. The hub must:

1. Detect the repeat pairing request via `BLE_GAP_EVENT_REPEAT_PAIRING`.
2. Delete the old bond for this peer: `ble_store_util_delete_peer()`.
3. Return `BLE_GAP_REPEAT_PAIRING_RETRY` to accept re-pairing.

**Current implementation:** `ble_central.c` handles `BLE_GAP_EVENT_REPEAT_PAIRING`
exactly as described above (lines 245-249).

**Role engine interaction:** The old MAC->role mapping in NVS remains valid
because the MAC does not change on factory reset (BLE identity address is
device-permanent for public addresses). No `role_engine_forget()` call is
needed unless the user explicitly requests it via the companion app.

If the ring changes its MAC on factory reset (e.g., because it uses a random
resolvable address and the IRK is lost), the hub will see it as a new device.
The old entry remains in NVS as a stale entry. The companion app should provide
a "forget ring" action that calls `role_engine_forget()` to reclaim the slot.

---

## 4. Companion App Interaction Protocol **[SPECIFIED]**

The companion app communicates with the hub via either:
- **Web Serial** (USB CDC, when hub is connected to a PC), or
- **BLE GATT characteristic** (when hub exposes a configuration service).

Both transports carry the same command/response protocol defined below.

### 4.1 Command Format

Commands are length-prefixed binary frames:

```
[command_id: uint8_t] [payload: variable]
```

Response:

```
[command_id: uint8_t] [status: uint8_t] [payload: variable]
```

Status codes: `0x00` = OK, `0x01` = INVALID_ARG, `0x02` = NOT_FOUND,
`0x03` = CONFLICT, `0x04` = BUSY (role swap in progress).

### 4.2 Commands

#### `GET_ROLES` (0x01)

Request roles for all known rings.

**Request payload:** empty.

**Response payload:**
```
[count: uint8_t]
[entry 0: mac[6] + role: uint8_t + connected: uint8_t]
...
[entry N: mac[6] + role: uint8_t + connected: uint8_t]
```

The `connected` byte is `0x01` if the ring is currently in the ACTIVE state,
`0x00` otherwise. This allows the companion app to display both connected and
remembered-but-disconnected rings.

**Implementation:** Iterate `s_entries[0..s_entry_count-1]` from role engine.
For each, check whether any active BLE central slot holds that MAC.

#### `ROLE_SET` (0x02)

Assign a specific role to a specific ring.

**Request payload:**
```
[mac: 6 bytes] [new_role: uint8_t]
```

**Preconditions:**
- `new_role < ROLE_COUNT`
- MAC exists in role engine (`s_entries[]`)

**Procedure:**
1. Validate arguments. Return `INVALID_ARG` if `new_role >= ROLE_COUNT`.
2. Call `role_engine_set_role(mac, new_role)`. Return `NOT_FOUND` if MAC unknown.
3. If the ring is currently ACTIVE:
   a. Transition ring to ROLE_SWAPPING.
   b. Call `event_composer_set_role(ring_index, new_role)`.
   c. Allow `event_composer_set_role()` to clear any buffered deltas and
      button state for that ring.
   d. Transition ring back to ACTIVE.
4. Return OK.

**Postcondition:** NVS updated. If the ring is ACTIVE and the hub also calls
`event_composer_set_role(ring_index, new_role)`, subsequent
`event_composer_feed()` calls for this ring use the new cached role.

**Atomicity:** The role change in the role engine is mutex-protected. The event
composer has its own cached role per ring, so ACTIVE rings require an explicit
`event_composer_set_role()` update when the companion app changes roles.
`event_composer_set_role()` clears buttons and accumulated deltas before
switching roles so a held click cannot silently remap from left to right or
middle click.

**Implication for ROLE_SWAPPING state:** Cached-role architecture removes the
per-notification role-engine lookup from the BLE hot path. The tradeoff is that
ROLE_SWAPPING now has real firmware work to do: update the cached role and drop
any buffered deltas at the swap point. This intentionally favors safety over
perfect continuity. Losing a few milliseconds of motion is preferable to
misclassifying a held click or buffered deltas under the wrong role.

#### `ROLE_SWAP` (0x03)

Swap roles between two rings atomically.

**Request payload:**
```
[mac_a: 6 bytes] [mac_b: 6 bytes]
```

**Preconditions:**
- Both MACs exist in role engine
- Both MACs are different

**Procedure:**
1. Read current roles: `role_a = role_engine_get_role(mac_a)`,
   `role_b = role_engine_get_role(mac_b)`.
2. Apply both changes under a single mutex acquisition. This requires a new
   internal function `role_engine_swap(mac_a, mac_b)` that:
   a. Acquires the mutex.
   b. Finds both entries.
   c. Swaps their `role` fields.
   d. Releases the mutex.
   e. Writes NVS (outside mutex).
3. Return OK.

**Atomicity requirement:** There must be no window between the two role
assignments where one ring has been reassigned but the other has not. If
`role_engine_set_role()` is called twice sequentially, there is a window (between
the two calls) where both rings hold the same role. While INV-4 permits this,
the user's intent is a swap, and a momentary state where (e.g.) both rings are
CURSOR would produce a jarring double-cursor effect. The single-mutex-acquisition
`role_engine_swap()` eliminates this window.

**New function signature:**
```c
hal_status_t role_engine_swap(const uint8_t mac_a[6], const uint8_t mac_b[6]);
```

#### `FORGET_RING` (0x04)

Remove a ring's role assignment and free its slot.

**Request payload:**
```
[mac: 6 bytes]
```

**Preconditions:**
- MAC exists in role engine

**Procedure:**
1. If the ring is currently ACTIVE, disconnect it first (drop the BLE
   connection via `ble_gap_terminate()`).
2. Call `role_engine_forget(mac)`.
3. Return OK.

**Postcondition:** Slot freed. NVS updated. If ring was connected, INV-1
(no stuck buttons) is satisfied by the disconnect handler calling
`event_composer_ring_disconnected()`.

---

## 5. Multi-Ring Composition Rules

Formalized from `event_composer.c`. These rules define how per-ring state is
merged into the single `composed_report_t` sent via USB HID at 1ms intervals.

### 5.1 Inputs

For each connected ring `i` (where `s_rings[i].connected == true`):
- `buttons_i`: last button state received via `event_composer_feed()`
- `acc_dx_i`, `acc_dy_i`: accumulated deltas since last `event_composer_compose()`
- `role_i`: cached role stored in the event composer when the ring connected or
  when `event_composer_set_role()` last updated it

### 5.2 Composition Algorithm

Executed by `event_composer_compose()` under spinlock:

```
cursor_dx = 0, cursor_dy = 0
scroll_h_acc = 0, scroll_v_acc = 0  (int16_t, wider than output)
buttons = 0

for each connected ring i:
    switch role_i:
        CURSOR:
            buttons |= (buttons_i & 0x01)        // left click
            cursor_dx += acc_dx_i
            cursor_dy += acc_dy_i

        SCROLL:
            if (buttons_i & 0x01):
                buttons |= 0x02                   // right click
            scroll_h_acc += acc_dx_i
            scroll_v_acc += acc_dy_i

        MODIFIER:
            if (buttons_i & 0x01):
                buttons |= 0x04                   // middle click

    // Clear this ring's accumulators
    acc_dx_i = 0
    acc_dy_i = 0

// Clamp scroll to int8_t after summing across all scroll-role rings
scroll_h = clamp(scroll_h_acc, -127, 127)
scroll_v = clamp(scroll_v_acc, -127, 127)
```

### 5.3 Composition Properties

**P-1 (Button OR):** Buttons are OR'd, not replaced. If ring A presses left
click and ring B presses left click, the output has one left click (not two).
Release requires ALL rings contributing to that button to release.

**P-2 (Delta SUM):** Cursor and scroll deltas are summed across all rings with
the matching role. Two CURSOR rings moving right at 10px each produce a 20px
rightward movement.

**P-3 (Scroll clamping):** Scroll output is clamped to `int8_t` range (-127 to
+127) after summation. This prevents overflow when multiple scroll-role rings
contribute large deltas simultaneously. The `clamp_i8()` function in
`event_composer.c` enforces this.

**P-4 (Accumulator saturation):** Per-ring accumulators (`acc_dx`, `acc_dy`) use
saturating addition via `sat_add_i16()`. Overflow wraps to `INT16_MAX` or
`INT16_MIN`, not around to the opposite sign.

**P-5 (Accumulator drain):** Accumulators are zeroed after each
`event_composer_compose()` call. Deltas are consumed exactly once.

**P-6 (Modifier ignores deltas):** A MODIFIER-role ring's dx/dy are accumulated
but never read by the composition loop. They are cleared on compose like any
other ring. This means motion data from a MODIFIER ring is silently discarded.

### 5.4 Disconnect Composition Semantics

When `event_composer_ring_disconnected(ring_index)` is called:

1. `buttons` is set to 0 for that ring.
2. `acc_dx` and `acc_dy` are set to 0 for that ring.
3. `connected` is set to `false` for that ring.

All three writes occur under the spinlock. The next `event_composer_compose()`
call will skip this ring entirely (the `if (!r->connected) continue;` guard).

The main loop in `main.c` tracks `prev_report_nonzero` and sends a final
zero-report when the composed output transitions from non-zero to all-zero.
This ensures the host OS receives an explicit button-release event, not just
silence.

### 5.5 Role Change During Active Use

When a ring's role changes (via companion app command) while it is actively
producing deltas:

1. The role engine's in-memory state changes atomically (mutex-protected).
2. The hub calls `event_composer_set_role(ring_index, new_role)` for the ACTIVE
   ring.
3. `event_composer_set_role()` clears `buttons`, `acc_dx`, and `acc_dy`, then
   stores the new cached role.
4. Subsequent `event_composer_feed()` calls are composed under the new role.

**Impact analysis:** At the swap point, one compose window's worth of buffered
deltas may be intentionally dropped. This is acceptable: it avoids a more
serious accessibility hazard where a held click silently changes semantic
meaning across roles.

**A brief pause is acceptable.** Cached-role architecture optimizes the BLE hot
path and makes the role swap behavior explicit instead of implicit.

---

## 6. Edge Cases

### 6.1 Ring Disconnect During Drag Operation

**Scenario:** User is dragging (button held + cursor moving) and the ring
loses BLE connection.

**Specified behavior:**
1. `BLE_GAP_EVENT_DISCONNECT` fires in `ble_central.c`.
2. `on_ring_connection(ring_index, false, ...)` is called in `main.c`.
3. `event_composer_ring_disconnected(ring_index)` zeroes buttons and deltas.
4. Next `event_composer_compose()` produces a report without this ring's
   contributions.
5. Main loop detects transition from non-zero to zero (or reduced non-zero)
   and sends the updated report.
6. Host OS sees button release, ending the drag.

**Latency:** At most 1ms (next USB poll). The button release is not deferred.

**Reference:** `event_composer_ring_disconnected()` in `event_composer.c`,
lines 99-119. `on_ring_connection()` in `main.c`, lines 43-59.

### 6.2 Ring Reconnect with Stale Role

**Scenario:** Ring A was CURSOR, disconnected, companion app changed ring A
to SCROLL while it was offline. Ring A reconnects.

**Specified behavior:**
1. `on_ring_connection(ring_index, true, ...)` calls `role_engine_get_role(mac)`.
2. Role engine finds the MAC in `s_entries[]` with role=SCROLL (the new role).
3. Ring A operates as SCROLL.

The ring itself stores no role information. Roles are hub-side only.

### 6.3 Fifth Ring Connection Attempt

**Scenario:** 4 rings are connected (`s_connected_count == HUB_MAX_RINGS`).
A 5th ring advertises.

**Specified behavior:**
1. `start_scan()` checks `s_connected_count >= HUB_MAX_RINGS` and returns
   without starting a scan. No discovery events fire.
2. If a scan was already running when the 4th ring connected, discovery events
   for the 5th ring reach `gap_event_handler`. `find_free_slot()` returns -1.
   The event is ignored with a warning log.
3. The 5th ring is neither connected nor rejected at the BLE level -- it simply
   never receives a connection request.

**When a slot opens:** On disconnect of any ring, `start_scan()` is called
from the disconnect handler, and the 5th ring may be discovered on the next
scan cycle.

### 6.4 BLE Host Reset

**Scenario:** NimBLE host resets (e.g., controller crash, HCI timeout).

**Specified behavior:**
1. `on_reset()` in `ble_central.c` iterates all slots.
2. For each connected slot, sets `connected = false`, decrements count,
   calls `s_conn_cb(ring_index, false, ...)`.
3. This triggers `event_composer_ring_disconnected()` for each ring via
   `on_ring_connection()` in `main.c`.
4. All buttons are released, all deltas zeroed (INV-1 satisfied).
5. After `on_sync()` fires (host re-synchronized), scanning resumes.

**Reference:** `on_reset()` in `ble_central.c`, lines 303-320.

### 6.5 NVS Corruption

**Scenario:** NVS blob is unreadable, version-mismatched, or contains roles
outside the valid range.

**Specified behavior:**
1. `role_engine_init()` checks `blob.version == ROLE_NVS_VERSION` and
   `blob.count <= HUB_MAX_RINGS`.
2. Each entry is validated: `blob.entries[i].role < ROLE_COUNT`. Invalid
   entries are skipped (not copied to `s_entries[]`).
3. If the entire blob fails validation, `s_entry_count` is reset to 0.
   All rings will receive fresh default role assignments on next connection.

**Reference:** `role_engine_init()` in `role_engine.c`, lines 80-119.

### 6.6 Duplicate MAC in NVS

**Scenario:** Due to a bug or manual NVS manipulation, two entries in the
blob have the same MAC.

**Current behavior:** `role_engine_get_role()` performs a linear scan and
returns the first match. The second entry is unreachable and wastes a slot.

**Specified fix:** `role_engine_init()` should deduplicate entries on load.
For duplicate MACs, keep the last entry (most recently written) and discard
earlier ones. Log a warning.

### 6.7 Role Swap Between Connected and Disconnected Ring

**Scenario:** Companion app sends `ROLE_SWAP(mac_a, mac_b)` where ring A is
ACTIVE and ring B is KNOWN_DISCONNECTED.

**Specified behavior:** The swap proceeds normally. Both entries exist in the
role engine regardless of connection state. Ring A immediately begins operating
under its new role after the hub updates the event composer's cached role.
Ring B will use its new role when it reconnects.

### 6.8 Rapid Disconnect-Reconnect

**Scenario:** Ring disconnects and reconnects within a few hundred milliseconds
(e.g., momentary BLE interference).

**Specified behavior:**
1. Disconnect handler fires: buttons released, deltas zeroed, `connected = false`.
2. Scan resumes, ring is rediscovered.
3. New connection established: `event_composer_mark_connected(ring_index, role)`
   is called, which resets buttons to 0, deltas to 0, and seeds the cached role.
4. Role is re-read from role engine (same as before disconnect).
5. User experiences a brief tracking dropout. No stuck buttons, no stale state.

The gap between disconnect and reconnect is at minimum ~100ms (BLE scan
interval + connection setup). During this gap, the ring contributes nothing
to composition.

---

## 7. Implementation Notes

### 7.1 Functions Not Yet Implemented

The following functions are specified by this protocol but do not exist in the
current codebase:

| Function | Module | Purpose |
|----------|--------|---------|
| `role_engine_swap(mac_a, mac_b)` | `role_engine.c` | Atomic role swap under single mutex acquisition |
| `role_engine_get_all(entries_out, count_out)` | `role_engine.c` | Bulk read for `GET_ROLES` command |
| Companion command parser | New module | Parse and dispatch `GET_ROLES`, `ROLE_SET`, `ROLE_SWAP`, `FORGET_RING` |
| NVS deduplication in `role_engine_init()` | `role_engine.c` | Remove duplicate MAC entries on load |

### 7.2 Thread Safety Summary

| Resource | Protection | Accessed by |
|----------|-----------|-------------|
| `s_entries[]` (role engine) | FreeRTOS mutex | NimBLE task (via `role_engine_get_role`), companion command task |
| `s_rings[]` (event composer) | portMUX spinlock | NimBLE task (via `feed`, `ring_disconnected`), main loop (via `compose`) |
| `s_rings[]` (BLE central) | Single-task access | NimBLE task only (all GAP callbacks run on same task) |
| NVS writes | Unprotected (single writer assumed) | Whichever task called a role_engine mutator |

**Concern:** If the companion command parser runs on a different task than the
NimBLE task, and both call `role_engine_set_role()` concurrently, the NVS write
(outside the mutex) could race. The mutex protects in-memory state but not the
`flush_to_nvs()` call. Fix: either serialize companion commands onto the NimBLE
task, or add a separate NVS write mutex.

### 7.3 Design Decision: Role Cached In Event Composer

The current architecture caches each ring's role in the event composer at
connect time and updates it explicitly on role changes. `event_composer_feed()`
therefore stays on the hot path without taking the role-engine mutex.

**Advantages:**
- Removes one `role_engine_get_role()` call from every BLE HID notification.
- Keeps the BLE notification path deterministic and cheap.
- Makes live role changes explicit via `event_composer_set_role()`, where
  button-release safety can be enforced in one place.

**Disadvantage:**
- Companion-driven role changes now require an explicit cache update in the
  event composer for ACTIVE rings.
- Buffered motion at the swap point is intentionally discarded.

This architecture is the right tradeoff for accessibility and should be
preserved unless profiling shows the hot-path simplification is unnecessary.

---

## 8. Test Plan

The following tests should be added to the test suite to validate the protocol
behaviors specified in this document. Each test is described by its scenario
and expected outcome. Tests reference existing test infrastructure in
`firmware/test/`.

### 8.1 Role Engine Tests

**T-RE-1: Default role assignment order.**
Connect four rings with distinct MACs in sequence. Verify roles are assigned
as CURSOR, SCROLL, MODIFIER, MODIFIER respectively.

**T-RE-2: Role persistence roundtrip.**
Assign roles, simulate NVS save (call `role_engine_init()` after populating
mock NVS), verify roles are restored correctly.

**T-RE-3: Role set changes persisted role.**
Assign default role, call `role_engine_set_role()` to change it, re-init
role engine from NVS, verify the changed role persists.

**T-RE-4: Forget removes entry and shifts remaining.**
Add 3 rings. Forget the middle one. Verify the third ring's entry shifted
down. Add a new ring and verify it gets the slot freed by forget.

**T-RE-5: Full table fallback.**
Fill all 4 slots. Call `role_engine_get_role()` with a 5th MAC. Verify
return is `ROLE_CURSOR` (fallback) and `s_entry_count` does not exceed
`HUB_MAX_RINGS`.

**T-RE-6: Swap atomicity.**
Add two rings (CURSOR, SCROLL). Call `role_engine_swap()`. Verify both roles
are exchanged. Verify NVS blob reflects the swap.

**T-RE-7: Set with invalid role.**
Call `role_engine_set_role()` with `role >= ROLE_COUNT`. Verify
`HAL_ERR_INVALID_ARG` is returned and no state changes.

**T-RE-8: NVS corruption recovery.**
Populate mock NVS with a blob where `version != ROLE_NVS_VERSION`. Call
`role_engine_init()`. Verify `s_entry_count == 0` (clean slate).

**T-RE-9: NVS entry with invalid role is skipped.**
Populate mock NVS with a valid blob but one entry has `role = 0xFF`. Call
`role_engine_init()`. Verify that entry is skipped and valid entries are loaded.

**T-RE-10: Duplicate MAC deduplication.**
Populate mock NVS with two entries sharing the same MAC but different roles.
Call `role_engine_init()`. Verify only one entry survives (the last one).

### 8.2 Event Composer Protocol Tests

(Supplements existing tests in `test_event_composer.c`.)

**T-EC-1: Role change mid-stream.**
Connect ring 0 as CURSOR. Feed 3 reports with dx=10. Compose (expect
cursor_dx=30). Call `event_composer_set_role(0, ROLE_SCROLL)`. Compose. Verify
the compose immediately after the swap is zeroed (old state cleared). Feed 3
more reports. Compose. Verify cursor_dx=0, scroll_h=30.

**T-EC-2: Two rings same role.**
Connect ring 0 and ring 1 both as CURSOR. Feed dx=10 to ring 0, dx=5 to
ring 1. Compose. Verify cursor_dx=15.

**T-EC-3: Disconnect during button hold, other ring unaffected.**
Connect ring 0 (CURSOR, button pressed) and ring 1 (SCROLL, button pressed).
Disconnect ring 0. Compose. Verify buttons = 0x02 (right click from ring 1
only), cursor_dx = 0, scroll contributions from ring 1 present.

**T-EC-4: Reconnect clears stale state.**
Connect ring 0, feed buttons=0x01 and dx=50. Disconnect ring 0.
Reconnect ring 0 (mark_connected). Compose before any new feed. Verify
buttons=0, cursor_dx=0 (mark_connected clears accumulators).

**T-EC-5: All four rings active.**
Connect 4 rings: CURSOR, SCROLL, MODIFIER, CURSOR. Feed data to all four.
Compose. Verify: cursor_dx is sum of ring 0 and ring 3 deltas, scroll from
ring 1, buttons OR'd correctly (left from rings 0/3, right from ring 1,
middle from ring 2).

**T-EC-6: Scroll clamping with multiple scroll rings.**
Connect ring 0 and ring 1 both as SCROLL. Feed dy=127 to both. Compose.
Verify scroll_v = 127 (clamped), not 254.

### 8.3 Integration / Sequence Tests

These tests validate multi-step protocol sequences and would require a
test harness that wraps the role engine, event composer, and mock BLE central.

**T-INT-1: Full lifecycle.**
Ring A connects (auto-assigned CURSOR). Ring B connects (auto-assigned SCROLL).
Both produce data. Verify composition. Ring A disconnects. Verify ring B
unaffected. Ring A reconnects. Verify role is still CURSOR. Companion app
swaps roles. Verify ring A is now SCROLL, ring B is now CURSOR.

**T-INT-2: Forget and re-pair.**
Ring A connects (CURSOR, index 0). Ring B connects (SCROLL, index 1).
Forget ring A. Ring C connects. Verify ring C gets index 0 and role CURSOR
(the freed slot's default).

**T-INT-3: Power cycle persistence.**
Assign roles to 3 rings. Simulate power cycle (re-init role engine from NVS).
Connect same 3 rings. Verify roles match pre-power-cycle assignments.

**T-INT-4: Concurrent role change and BLE notification.**
This test validates the thread safety claim. In a threaded test environment:
spawn one thread calling `role_engine_set_role()` in a loop, another calling
`role_engine_get_role()` in a loop. Run for 10,000 iterations. Verify no
crashes, no corrupt roles (role always < ROLE_COUNT).

---

## 9. Open Questions

These items require BDFL decision before implementation:

1. **Should unique role enforcement be added?** The current protocol allows
   multiple rings with the same role. The composition rules handle this
   correctly. Enforcing uniqueness would simplify the mental model but reduce
   flexibility (e.g., two-finger cursor control for accessibility).
   **Recommendation:** Keep current behavior (roles are not unique). Document
   the two-cursor use case as an accessibility feature.

2. **Should the companion app command protocol use binary or JSON?** Binary is
   more efficient on constrained BLE bandwidth. JSON is easier to debug over
   Web Serial. **Recommendation:** Binary over BLE GATT, JSON over Web Serial.
   Define a shared semantic model with two serialization formats.

3. **Should `FORGET_RING` disconnect an active ring?** The current spec says
   yes. An alternative is to return an error if the ring is connected, requiring
   the user to disconnect first. **Recommendation:** Disconnect automatically.
   Requiring manual disconnect is an unnecessary friction point, especially for
   accessibility users.

4. **NVS write race (section 7.2).** If companion commands run on a separate
   task, the NVS write path needs a second mutex or task serialization.
   **Recommendation:** Route all companion commands through a FreeRTOS queue
   consumed by the main loop task, avoiding concurrent NVS writers entirely.
