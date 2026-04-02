# Companion App Architecture

The companion app is the configuration UI for PowerFinger. It is not required
for basic operation. Rings work standalone over BLE HID, and multi-ring
composition works through the hub with no software on the host. The companion
app is how you customize, not how you use.

**Status:** This document is still mostly post-validation architecture, but the
ring now ships a minimal pre-hardware config surface: BLE characteristics for
DPI multiplier, dead-zone time, dead-zone distance, and firmware version, all
backed by deferred NVS persistence. It also now exposes the standard Device
Information identity fields (model, firmware revision, hardware revision, and
serial number) for bring-up and companion readback, plus a read-only
diagnostic snapshot characteristic. Deferred until after the first hardware
validation gates: gesture tables, OTA relay UX, charge-fault telemetry, and
rich companion workflows unless the BDFL reprioritizes them.

**What the app configures:**
- Role assignment (which ring is cursor, which is scroll, which is modifier)
- DPI/sensitivity per ring
- Dead zone parameters per ring
- Gesture mapping (e.g., simultaneous two-ring click)
- Hub settings (USB poll rate, ring scan policy)
- Firmware updates (hub via USB serial, rings via BLE OTA through hub)

**What the app does NOT do:**
- It is never required for basic cursor + click + scroll operation
- It has no cloud dependency (hard rule)
- It does not intercept or remap input on the host OS (that is the hub's job)

---

## 1. Configuration Data Model

Hub-owned multi-ring configuration is stored on the hub in NVS. Per-ring
runtime settings are stored on each ring in NVS and exposed over BLE. The
companion app reads and writes configuration over USB serial (or BLE for
hubless single-ring use). The app is a stateless UI.

### 1.1 Role Assignment

MAC address to role mapping, identical to the existing `role_engine.h`
interface:

```
role_entry_t {
    uint8_t mac[6];       // Ring BLE MAC address
    ring_role_t role;     // CURSOR (0), SCROLL (1), MODIFIER (2)
}
```

Maximum 4 entries (`HUB_MAX_RINGS`). Stored as an NVS blob with a version
byte, matching the existing `role_blob_t` format in `role_engine.c`.

### 1.2 Per-Ring Settings

Each ring stores its own settings in NVS (on the ring itself, not the hub),
exposed via BLE GATT characteristics (see section 2):

| Setting | Type | Range | Default | Units |
|---------|------|-------|---------|-------|
| DPI multiplier | uint8_t | 1-255 | 10 | Multiplier applied to raw sensor deltas. 10 = 1.0x, 20 = 2.0x, 1 = 0.1x. |
| Dead zone time | uint16_t | 0-2000 | 50 | Milliseconds after click actuation before deltas resume |
| Dead zone distance | uint8_t | 0-255 | 10 | Sensor counts accumulated during dead zone before deltas resume |

**DPI multiplier encoding:** A uint8_t with 0.1x resolution. Value 10 means
1.0x (native sensor DPI). Value 1 means 0.1x (minimum). Value 255 means
25.5x (maximum). This avoids floating point on the ring's ESP32-C3 while
giving 0.1x granularity. The ring firmware multiplies raw sensor deltas by
`(dpi_multiplier / 10)` using integer arithmetic:
`adjusted_delta = (raw_delta * dpi_multiplier) / 10`.

Zero is reserved (rejected with ATT error on write).

### 1.3 Gesture Mapping

Gestures are stored on the hub (they require multi-ring awareness). Defined
as a fixed-size table:

```
gesture_entry_t {
    uint8_t trigger;      // Gesture trigger ID (see table)
    uint8_t action;       // Action ID (see table)
}
```

| Trigger ID | Meaning |
|-----------|---------|
| 0x01 | Simultaneous click: cursor + scroll rings |
| 0x02 | Simultaneous click: cursor + modifier rings |
| 0x03 | Simultaneous click: scroll + modifier rings |
| 0x04 | Simultaneous click: all three rings |
| 0x05 | Double-click on cursor ring |
| 0x06 | Double-click on scroll ring |

| Action ID | Meaning |
|----------|---------|
| 0x00 | No action (gesture disabled) |
| 0x01 | Middle click |
| 0x02 | Back (mouse button 4) |
| 0x03 | Forward (mouse button 5) |
| 0x04 | Toggle scroll lock (scroll ring locks to vertical only) |
| 0x05 | DPI cycle (switch between two preset DPI values) |

Maximum 8 gesture entries. Stored in hub NVS as a versioned blob.

### 1.4 Hub Settings

| Setting | Type | Range | Default | Meaning |
|---------|------|-------|---------|---------|
| USB poll rate | uint8_t | 1, 2, 4, 8 | 1 | USB HID polling interval in ms |
| Scan policy | uint8_t | 0, 1, 2 | 1 | 0 = scan only on boot, 1 = scan continuously for new rings, 2 = scan only when fewer than expected rings connected |
| Expected ring count | uint8_t | 1-4 | 2 | Used by scan policy 2 to know when to stop scanning |

---

## 2. BLE GATT Configuration Service (Ring)

This service runs on each ring alongside the existing HID (0x1812), Device
Information (0x180A), and Battery (0x180F) services defined in
`hal_ble_esp.c`. It uses a custom 128-bit UUID base to avoid collision with
Bluetooth SIG assigned numbers. The current firmware implements the settings
and firmware-version characteristics below; OTA remains deferred.

### 2.1 UUID Scheme

**Base UUID:** `5046xxxx-7269-6E67-B054-706F77657266`

The base encodes "PF" (`0x50 0x46`) plus "ring" (`0x7269 0x6E67`) and
"powerf" (`0x706F 0x7765 0x7266`) in ASCII for easy identification in BLE
scanners. The `xxxx` field differentiates services and characteristics.

| Entity | UUID (short) | Full 128-bit UUID |
|--------|-------------|-------------------|
| PowerFinger Config Service | 0x0001 | `50460001-7269-6E67-B054-706F77657266` |
| DPI Multiplier Characteristic | 0x0101 | `50460101-7269-6E67-B054-706F77657266` |
| Dead Zone Time Characteristic | 0x0102 | `50460102-7269-6E67-B054-706F77657266` |
| Dead Zone Distance Characteristic | 0x0103 | `50460103-7269-6E67-B054-706F77657266` |
| Firmware Version Characteristic | 0x0201 | `50460201-7269-6E67-B054-706F77657266` |
| Diagnostics Snapshot Characteristic | 0x0401 | `50460401-7269-6E67-B054-706F77657266` |
| OTA Control Characteristic | 0x0301 | `50460301-7269-6E67-B054-706F77657266` |
| OTA Data Characteristic | 0x0302 | `50460302-7269-6E67-B054-706F77657266` |

### 2.2 Standard Services (Already Implemented)

These now exist in `hal_ble_esp.c`:

| Service | UUID | Characteristics |
|---------|------|----------------|
| Battery Service | 0x180F | Battery Level (0x2A19): Read, Notify. uint8_t 0-100 percent. |
| Device Information | 0x180A | Manufacturer Name (0x2A29), Model Number (0x2A24), Serial Number (0x2A25), Firmware Revision (0x2A26), Hardware Revision (0x2A27), PnP ID (0x2A50): Read |

The ring now exposes these standard Device Information characteristics:

| Characteristic | UUID | Properties | Format | Notes |
|---------------|------|------------|--------|-------|
| Model Number | 0x2A24 | Read | UTF-8 string | `Ring-Dev` for bare Phase 0 dev-board builds, `Ring-S` for standard builds, `Ring-P` for pro builds |
| Firmware Revision | 0x2A26 | Read | UTF-8 string | Current pre-hardware value: `0.1.0` |
| Hardware Revision | 0x2A27 | Read | UTF-8 string | Current pre-PCB value: `DEVBOARD-C3`; replace with board spin strings such as `P0-R1` once hardware exists |
| Serial Number | 0x2A25 | Read | UTF-8 string | Unique 12-digit uppercase hex string derived from the ring's Bluetooth MAC |

### 2.3 PowerFinger Config Service (0x0001) -- Characteristic Details

#### DPI Multiplier (0x0101)

| Property | Value |
|----------|-------|
| Properties | Read, Write |
| Format | uint8_t |
| Valid range | 1-255 |
| Default | 10 (1.0x native DPI) |
| Write error | 0 rejected with ATT error 0x13 (Value Not Allowed) |
| Persistence | Written to ring NVS on change, deferred to idle period |

#### Dead Zone Time (0x0102)

| Property | Value |
|----------|-------|
| Properties | Read, Write |
| Format | uint16_t, little-endian |
| Valid range | 0-2000 |
| Default | 50 (ms) |
| Write error | Values > 2000 rejected with ATT error 0x13 |
| Persistence | Written to ring NVS on change, deferred to idle period |

#### Dead Zone Distance (0x0103)

| Property | Value |
|----------|-------|
| Properties | Read, Write |
| Format | uint8_t |
| Valid range | 0-255 |
| Default | 10 (sensor counts) |
| Persistence | Written to ring NVS on change, deferred to idle period |

#### Firmware Version (0x0201)

| Property | Value |
|----------|-------|
| Properties | Read |
| Format | 3x uint8_t (major, minor, patch) |
| Example | `{ 0x01, 0x02, 0x03 }` = version 1.2.3 |

#### Diagnostics Snapshot (0x0401)

| Property | Value |
|----------|-------|
| Properties | Read |
| Format | Fixed 10-byte binary payload |
| Purpose | Exposes the ring's current bring-up snapshot without reading logs |

Payload layout:

| Byte(s) | Meaning |
|---------|---------|
| 0 | Payload version. Current value: `0x01` |
| 1 | `ring_state_t` value |
| 2 | `ring_diag_sensor_state_t` value (`0=unavailable`, `1=calibration_pending`, `2=ready`) |
| 3 | `ring_diag_bond_state_t` value (`0=unknown`, `1=restored`, `2=failed`) |
| 4 | Flags bitfield: bit 0 = connected, bit 1 = calibration valid, bit 2 = low-latency conn params rejected |
| 5 | Battery percent |
| 6-7 | Battery millivolts, little-endian, clamped to `0xFFFF` |
| 8-9 | Current BLE connection interval in 1.25ms units, little-endian |

#### OTA Control (0x0301)

| Property | Value |
|----------|-------|
| Properties | Write, Notify |
| Format | Variable length, command-dependent |

Commands written to OTA Control:

| Byte 0 (Command) | Payload | Meaning |
|------------------|---------|---------|
| 0x01 | uint32_t LE: total firmware size in bytes | Begin OTA. Ring erases inactive partition. |
| 0x02 | (none) | Abort OTA. Ring discards partial image. |
| 0x03 | (none) | Finalize OTA. Ring validates checksum and marks partition. |
| 0x04 | (none) | Reboot into new firmware. |

Notifications from OTA Control:

| Byte 0 (Status) | Payload | Meaning |
|-----------------|---------|---------|
| 0x10 | (none) | Ready for data (sent after Begin succeeds) |
| 0x11 | uint32_t LE: bytes received so far | Progress update (sent every 4096 bytes) |
| 0x12 | (none) | Finalize succeeded, ready for reboot |
| 0xE0 | uint8_t: error code | Error (see table) |

OTA error codes:

| Code | Meaning |
|------|---------|
| 0x01 | Partition erase failed |
| 0x02 | Write failed |
| 0x03 | Checksum validation failed |
| 0x04 | Image too large for partition |
| 0x05 | OTA not in progress (unexpected command) |

#### OTA Data (0x0302)

| Property | Value |
|----------|-------|
| Properties | Write Without Response |
| Format | Opaque byte array, max ATT MTU - 3 bytes (typically 244 bytes with 247 MTU) |
| Flow control | Writer must wait for 0x10 (Ready) before first chunk. After that, write as fast as BLE allows. Ring buffers internally. |

**OTA sequence:**

1. Writer sends Begin (0x01) with firmware size to OTA Control.
2. Ring erases inactive partition, sends Ready (0x10) notification.
3. Writer sends firmware data in chunks to OTA Data (write-without-response).
4. Ring sends Progress (0x11) notifications periodically.
5. Writer sends Finalize (0x03) to OTA Control after all data is sent.
6. Ring validates, sends Finalize Succeeded (0x12) or Error (0xE0).
7. Writer sends Reboot (0x04) to OTA Control.
8. Ring reboots. After reboot, ring firmware must call `hal_ota_confirm()`
   within 30 seconds or the bootloader rolls back.

This maps directly to the existing `hal_ota.h` interface: Begin maps to
`hal_ota_begin()`, each data chunk to `hal_ota_write()`, Finalize to
`hal_ota_finish()`, and Reboot to `hal_ota_reboot()`.

### 2.4 GATT Registration (Firmware Implementation Guidance)

Add the config service to the existing `s_gatt_svcs[]` array in
`hal_ble_esp.c`. The new service follows the same pattern as the existing
HID, Device Information, and Battery services. Each characteristic gets an
access callback that validates ranges on write and reads from a static
variable (DPI, dead zone) or NVS.

```c
// Add to s_gatt_svcs[] in hal_ble_esp.c:
{
    // PowerFinger Config Service
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = BLE_UUID128_DECLARE(
        0x66, 0x72, 0x65, 0x77, 0x6F, 0x70, 0x54, 0xB0,
        0x67, 0x6E, 0x69, 0x72, 0x01, 0x00, 0x00, 0xPF
    ),
    .characteristics = (struct ble_gatt_chr_def[]) {
        // DPI Multiplier
        // Dead Zone Time
        // Dead Zone Distance
        // Firmware Version
        // OTA Control (write + notify)
        // OTA Data (write-no-response)
        { 0 },
    },
},
```

The full UUID byte order follows NimBLE's little-endian convention. The
skeleton above shows placement; the actual characteristic definitions follow
the same `access_cb` + `flags` pattern as the existing HID characteristics.

---

## 3. Web Serial Protocol (Hub Configuration)

The hub (ESP32-S3) exposes a USB CDC serial port. The companion app
communicates over this port using a text-based command/response protocol.

### 3.1 Design Rationale

Text-based (not binary) because:
- Debuggable with any serial terminal (minicom, PuTTY, screen)
- Trivial to parse in JavaScript (Web Serial API returns strings)
- Hub CPU is an ESP32-S3 with 512KB SRAM; parsing overhead is irrelevant
- OTA data transfer is the one exception (binary, see section 3.3)

### 3.2 Command/Response Format

**Commands** are single-line ASCII, terminated by `\n`:

```
COMMAND [arg1] [arg2] ...\n
```

**Responses** are single-line or multi-line ASCII, terminated by a status
line:

```
OK [optional data]\n
```
or
```
ERR <code> <message>\n
```

Multi-line responses use `+` prefix for data lines, followed by the status
line:

```
+ line 1\n
+ line 2\n
OK\n
```

### 3.3 Command Reference

#### GET_HUB_INFO

Returns hub firmware version, hardware revision, and connected ring count.

```
> GET_HUB_INFO
+ fw=1.2.3
+ hw=P0-R1
+ rings=2
+ max_rings=4
+ usb_poll_ms=1
+ scan_policy=1
OK
```

#### GET_ROLES

Returns all role assignments.

```
> GET_ROLES
+ AA:BB:CC:DD:EE:01 CURSOR
+ AA:BB:CC:DD:EE:02 SCROLL
OK
```

If no rings are assigned:

```
> GET_ROLES
OK
```

#### SET_ROLE

Assign a role to a ring by MAC address.

```
> SET_ROLE AA:BB:CC:DD:EE:01 SCROLL
OK
```

Error if MAC not known:

```
> SET_ROLE AA:BB:CC:DD:EE:FF CURSOR
ERR 404 unknown_mac
```

Error if invalid role:

```
> SET_ROLE AA:BB:CC:DD:EE:01 FOOBAR
ERR 400 invalid_role
```

Valid role names: `CURSOR`, `SCROLL`, `MODIFIER` (case-insensitive on input,
uppercase on output).

#### GET_RING_INFO

Returns detailed info for a connected ring.

```
> GET_RING_INFO AA:BB:CC:DD:EE:01
+ mac=AA:BB:CC:DD:EE:01
+ role=CURSOR
+ fw=1.1.0
+ battery=87
+ dpi=10
+ dead_zone_time=50
+ dead_zone_dist=10
+ rssi=-42
+ connected=1
OK
```

If the ring is known but not currently connected:

```
> GET_RING_INFO AA:BB:CC:DD:EE:01
+ mac=AA:BB:CC:DD:EE:01
+ role=CURSOR
+ connected=0
OK
```

#### GET_RINGS

Returns summary info for all known rings.

```
> GET_RINGS
+ AA:BB:CC:DD:EE:01 CURSOR connected battery=87
+ AA:BB:CC:DD:EE:02 SCROLL connected battery=92
+ AA:BB:CC:DD:EE:03 MODIFIER disconnected
OK
```

#### SET_HUB

Set a hub configuration parameter.

```
> SET_HUB usb_poll_ms 1
OK

> SET_HUB scan_policy 2
OK

> SET_HUB expected_rings 2
OK
```

Error if invalid parameter or value:

```
> SET_HUB usb_poll_ms 3
ERR 400 invalid_value (must be 1, 2, 4, or 8)
```

#### SET_GESTURE

Configure a gesture mapping.

```
> SET_GESTURE 0x01 0x01
OK
```

First argument is trigger ID, second is action ID (see section 1.3).

#### GET_GESTURES

Returns all gesture mappings.

```
> GET_GESTURES
+ 0x01 0x01 cursor+scroll=middle_click
+ 0x05 0x05 double_click_cursor=dpi_cycle
OK
```

The human-readable description after the IDs is informational (not parsed).

#### FORGET_RING

Remove a ring from the role engine and delete its bond.

```
> FORGET_RING AA:BB:CC:DD:EE:03
OK
```

#### OTA_BEGIN

Start a firmware update. Target is `hub` or a ring MAC address.

```
> OTA_BEGIN hub 142368
OK ready
```

```
> OTA_BEGIN AA:BB:CC:DD:EE:01 98304
OK ready
```

The second argument is the firmware image size in bytes. The hub validates
that the image fits in the target's OTA partition before responding.

#### OTA_DATA

Binary data transfer. After `OTA_BEGIN` succeeds, the protocol switches to
binary mode for the data phase:

1. App sends raw binary data in chunks (recommended: 4096 bytes).
2. Hub acknowledges each chunk: `OK <bytes_received_total>\n`
3. If a chunk write fails: `ERR 500 write_failed\n` and OTA is aborted.

The text/binary mode switch is scoped: after all bytes are received (total
matches the size from `OTA_BEGIN`), the protocol returns to text mode
automatically.

For ring OTA, the hub receives the full image first (stored in PSRAM), then
relays to the ring over BLE OTA (section 2.3). The companion app sees hub
acknowledgments only; the BLE relay progress is reported via `OTA_STATUS`.

#### OTA_STATUS

Poll OTA progress (useful during BLE relay to ring).

```
> OTA_STATUS
+ target=AA:BB:CC:DD:EE:01
+ phase=transferring
+ bytes_sent=49152
+ bytes_total=98304
+ percent=50
OK
```

Phases: `erasing`, `transferring`, `validating`, `rebooting`, `confirming`,
`complete`, `failed`.

#### OTA_FINALIZE

Finalize the OTA update (validate checksum, mark partition).

```
> OTA_FINALIZE
OK validated
```

Error if checksum fails:

```
> OTA_FINALIZE
ERR 500 checksum_failed
```

#### OTA_REBOOT

Reboot the target into new firmware.

```
> OTA_REBOOT
OK rebooting
```

After this command, the USB serial connection to the hub will drop (if
target is hub) or the ring will disconnect and reconnect over BLE (if target
is a ring).

### 3.4 Error Codes

| Code | Meaning |
|------|---------|
| 400 | Bad request (invalid command, argument, or value) |
| 404 | Not found (unknown MAC, unknown parameter) |
| 409 | Conflict (OTA already in progress, ring busy) |
| 500 | Internal error (flash write failed, BLE communication error) |

### 3.5 Serial Port Parameters

| Parameter | Value |
|-----------|-------|
| Baud rate | 115200 (ESP32-S3 USB CDC ignores baud rate, but set it for compatibility) |
| Data bits | 8 |
| Stop bits | 1 |
| Parity | None |
| Flow control | None |
| Line ending | `\n` (LF only; `\r\n` accepted on input, `\n` on output) |

---

## 4. UI Wireframes (Text Description)

The UI is a single-page application. Layout is responsive (works on both
desktop browser windows and mobile screens).

### 4.1 Main Screen: Ring Overview

```
┌─────────────────────────────────────────────────┐
│  PowerFinger                        [Hub: v1.2] │
│                                                 │
│  ┌──────────────┐  ┌──────────────┐             │
│  │ Ring 1       │  │ Ring 2       │             │
│  │ AA:BB:...:01 │  │ AA:BB:...:02 │             │
│  │              │  │              │             │
│  │ Role:        │  │ Role:        │             │
│  │ [CURSOR ▼]   │  │ [SCROLL ▼]   │             │
│  │              │  │              │             │
│  │ Battery: 87% │  │ Battery: 92% │             │
│  │ FW: 1.1.0   │  │ FW: 1.1.0   │             │
│  │ Signal: Good │  │ Signal: Good │             │
│  │              │  │              │             │
│  │ [Settings]   │  │ [Settings]   │             │
│  └──────────────┘  └──────────────┘             │
│                                                 │
│  [+ Scan for new rings]                         │
│                                                 │
│  ──────────────────────────────────────         │
│  [Hub Settings]  [Gestures]  [Firmware Update]  │
└─────────────────────────────────────────────────┘
```

Each ring is a card. The role dropdown allows reassignment (sends `SET_ROLE`
command). Cards are ordered by connection order. Disconnected rings show as
grayed-out cards with a "disconnected" label.

The battery level uses a color scale: green (>50%), yellow (20-50%), red
(<20%).

### 4.2 Per-Ring Settings

Opened by clicking [Settings] on a ring card. Slides in as a panel or
modal.

```
┌─────────────────────────────────────────────────┐
│  Ring 1 Settings                         [Back] │
│                                                 │
│  DPI / Sensitivity                              │
│  ├─────────────●─────────────────────┤          │
│  0.1x          1.0x                 25.5x       │
│  Current: 1.0x (value: 10)                      │
│                                                 │
│  Dead Zone (click stabilization)                │
│  ┌─ Time ──────────────────────────────┐        │
│  │ ├────●──────────────────────────────┤        │
│  │ 0ms      50ms                2000ms │        │
│  └─────────────────────────────────────┘        │
│  ┌─ Distance ──────────────────────────┐        │
│  │ ├────●──────────────────────────────┤        │
│  │ 0        10                    255  │        │
│  └─────────────────────────────────────┘        │
│                                                 │
│  [Reset to Defaults]                            │
└─────────────────────────────────────────────────┘
```

Slider changes are sent to the ring in real time via BLE GATT write (for
direct BLE connection) or via hub relay (hub reads/writes ring GATT
characteristics on behalf of the app). Changes take effect immediately so
the user can feel the difference while adjusting.

### 4.3 Firmware Update

```
┌─────────────────────────────────────────────────┐
│  Firmware Update                         [Back] │
│                                                 │
│  Hub (ESP32-S3)                                 │
│  Current: v1.2.3                                │
│  [Select hub firmware file...]                  │
│                                                 │
│  ──────────────────────────────────────         │
│                                                 │
│  Ring Firmware                                  │
│  [Select ring firmware file...]                 │
│                                                 │
│  Target rings:                                  │
│  [x] Ring 1 (AA:BB:...:01) - v1.1.0            │
│  [x] Ring 2 (AA:BB:...:02) - v1.1.0            │
│                                                 │
│  [Start Update]                                 │
│                                                 │
│  ┌─ Progress ──────────────────────────┐        │
│  │ Ring 1: ████████████░░░░░░░  65%    │        │
│  │ Ring 2: Waiting...                  │        │
│  │                                     │        │
│  │ Status: Transferring to Ring 1      │        │
│  └─────────────────────────────────────┘        │
│                                                 │
│  ⚠ Do not disconnect hub during update.         │
│  Rings update sequentially (one at a time).     │
│  If an update fails, the ring automatically     │
│  rolls back to the previous firmware.           │
└─────────────────────────────────────────────────┘
```

The file selector accepts `.bin` files only. The app validates the file size
against the target's OTA partition size before starting. Ring updates go
through the hub (app sends image to hub, hub relays to ring over BLE OTA).
Rings are updated one at a time to avoid saturating the hub's BLE bandwidth.

### 4.4 Hub Settings

```
┌─────────────────────────────────────────────────┐
│  Hub Settings                            [Back] │
│                                                 │
│  USB Poll Rate                                  │
│  ( ) 1ms (lowest latency, default)              │
│  ( ) 2ms                                        │
│  ( ) 4ms                                        │
│  ( ) 8ms (lowest CPU usage)                     │
│                                                 │
│  Ring Scan Policy                               │
│  ( ) Scan on boot only                          │
│  (x) Scan continuously                          │
│  ( ) Scan when rings missing                    │
│      Expected rings: [2 ▼]                      │
│                                                 │
│  ──────────────────────────────────────         │
│  Known Rings                                    │
│  AA:BB:CC:DD:EE:01  CURSOR   connected          │
│  AA:BB:CC:DD:EE:02  SCROLL   connected          │
│  AA:BB:CC:DD:EE:03  MODIFIER disconnected       │
│                              [Forget]           │
└─────────────────────────────────────────────────┘
```

### 4.5 Gestures

```
┌─────────────────────────────────────────────────┐
│  Gesture Mapping                         [Back] │
│                                                 │
│  Cursor + Scroll simultaneous click:            │
│  [Middle Click ▼]                               │
│                                                 │
│  Cursor + Modifier simultaneous click:          │
│  [Back ▼]                                       │
│                                                 │
│  Scroll + Modifier simultaneous click:          │
│  [Forward ▼]                                    │
│                                                 │
│  All three rings simultaneous click:            │
│  [No Action ▼]                                  │
│                                                 │
│  Double-click cursor ring:                      │
│  [DPI Cycle ▼]                                  │
│                                                 │
│  Double-click scroll ring:                      │
│  [Toggle Scroll Lock ▼]                         │
│                                                 │
│  [Reset Gestures to Defaults]                   │
└─────────────────────────────────────────────────┘
```

---

## 5. Platform Support Matrix

### 5.1 Primary: Web Serial (Zero Install)

| Browser | Version | Web Serial | Status |
|---------|---------|-----------|--------|
| Chrome | 89+ | Yes | Primary target |
| Edge | 89+ | Yes | Same engine as Chrome |
| Opera | 76+ | Yes | Same engine as Chrome |
| Firefox | All | No | Not supported, no plans from Mozilla |
| Safari | All | No | Not supported |

Web Serial requires HTTPS or localhost. The app can be hosted on GitHub
Pages (HTTPS) or run locally via `npx serve`. No backend server needed.

**Technology stack:** Vanilla HTML/CSS/JavaScript or Svelte (small bundle,
no framework runtime). No React, no Angular (unnecessary weight for a
configuration UI). The entire app should be a single HTML file with inlined
JS/CSS for maximum portability (can be opened from a local file if needed,
though Web Serial requires a secure context).

### 5.2 Desktop: Tauri (Native App)

For users who want a native app or whose browser does not support Web
Serial. Tauri wraps the same web UI in a native window with direct serial
port access via Tauri's `serialport` plugin (no Web Serial API needed).

| OS | Support |
|----|---------|
| Windows 10+ | Yes |
| macOS 10.15+ | Yes |
| Linux (X11/Wayland) | Yes |

Tauri is preferred over Electron because:
- ~10MB binary vs ~150MB (Electron bundles Chromium)
- Uses the OS's native webview (WebView2 on Windows, WebKit on macOS)
- Rust backend for serial port access (safer than Node.js)

The Tauri app wraps the same HTML/JS/CSS as the web version. Serial port
access uses `tauri-plugin-serialport` instead of the Web Serial API. The
UI code is identical; only the serial transport layer is swapped.

### 5.3 Mobile: Flutter (BLE Direct)

Mobile devices do not connect to the hub via USB. The mobile app talks
directly to rings over BLE (using the GATT configuration service from
section 2). This covers single-ring configuration only (role assignment
requires the hub).

| Platform | BLE Support | Hub Config |
|----------|------------|------------|
| Android 5.0+ | Yes (flutter_blue_plus) | No (no USB serial on mobile) |
| iOS 13+ | Yes (flutter_blue_plus) | No |

The mobile app can:
- Read/write DPI and dead zone settings on a single ring
- Read battery level and firmware version
- Perform OTA update on a single ring (direct BLE, no hub relay)

The mobile app cannot:
- Configure role assignments (requires hub)
- Configure gestures (requires hub)
- Update hub firmware (requires USB)

### 5.4 Fallback: CLI Tool (Python)

For headless systems, automation, and users who prefer a terminal:

```bash
# Install
pip install powerfinger-cli

# List connected rings (via hub USB serial)
pf-cli --port /dev/ttyACM0 get-rings

# Set role
pf-cli --port /dev/ttyACM0 set-role AA:BB:CC:DD:EE:01 CURSOR

# Adjust DPI
pf-cli --port /dev/ttyACM0 set-ring-dpi AA:BB:CC:DD:EE:01 20

# Update hub firmware
pf-cli --port /dev/ttyACM0 ota-hub firmware-hub-v1.3.0.bin

# Update ring firmware (via hub)
pf-cli --port /dev/ttyACM0 ota-ring AA:BB:CC:DD:EE:01 firmware-ring-v1.2.0.bin
```

The CLI tool uses `pyserial` to send the same text commands from section 3.
No additional protocol; the CLI is a thin wrapper around the serial
command/response protocol.

### 5.5 Safari/iOS Fallback: Web Bluetooth

Safari and iOS do not support Web Serial but do support Web Bluetooth
(with limitations). A stripped-down version of the web UI can connect
directly to a single ring over BLE and configure DPI, dead zone, and
perform OTA. This provides the same capability as the Flutter mobile app
but without installing an app.

| Browser | Web Bluetooth | Limitations |
|---------|--------------|-------------|
| Safari 15.4+ (macOS) | Partial | Requires user gesture for each connection |
| Safari (iOS 16+) | Partial | Same as macOS Safari |
| Chrome (Android) | Full | Works well |
| Chrome (desktop) | Full | Works, but Web Serial is preferred for hub access |

Web Bluetooth can only access the ring's GATT services (section 2). It
cannot configure the hub (no USB serial access).

---

## 6. Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Companion App                           │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                    UI Layer                           │  │
│  │  Ring Cards │ Settings Panels │ OTA │ Hub Config      │  │
│  └──────────────────────┬────────────────────────────────┘  │
│                         │                                   │
│  ┌──────────────────────┴────────────────────────────────┐  │
│  │               Transport Abstraction                   │  │
│  │                                                       │  │
│  │  ┌─────────────┐  ┌──────────┐  ┌─────────────────┐  │  │
│  │  │ Web Serial  │  │  Tauri   │  │ Web Bluetooth   │  │  │
│  │  │ (Chrome)    │  │ (native) │  │ (Safari/mobile) │  │  │
│  │  └──────┬──────┘  └────┬─────┘  └───────┬─────────┘  │  │
│  └─────────┼──────────────┼────────────────┼─────────────┘  │
│            │              │                │                 │
└────────────┼──────────────┼────────────────┼─────────────────┘
             │              │                │
        USB Serial     USB Serial     BLE GATT
             │              │                │
             ▼              ▼                │
        ┌────────────────────┐               │
        │       Hub          │               │
        │  (ESP32-S3)        │               │
        │                    │               │
        │  Serial command    │               │
        │  parser            │               │
        │       │            │               │
        │       ▼            │               │
        │  Role engine       │               │
        │  Gesture engine    │               │
        │  OTA relay         │               │
        │       │            │               │
        │       ▼            │               │
        │  BLE central       │               │
        └───────┬────────────┘               │
                │                            │
           BLE GATT                          │
                │                            │
                ▼                            ▼
        ┌────────────────────┐    ┌────────────────────┐
        │     Ring 1         │    │     Ring 2         │
        │  (ESP32-C3)        │    │  (ESP32-C3)        │
        │                    │    │                    │
        │  Config service    │    │  Config service    │
        │  (DPI, dead zone,  │    │  (DPI, dead zone,  │
        │   OTA)             │    │   OTA)             │
        └────────────────────┘    └────────────────────┘
```

---

## 7. Implementation Notes

### 7.1 Firmware Changes Required

Adding the GATT configuration service (section 2) to the ring firmware
requires changes to:

1. **`hal_ble_esp.c`** -- Add the PowerFinger Config Service to the
   `s_gatt_svcs[]` array. This is now done for the minimal pre-hardware
   surface: DPI multiplier, dead zone time, dead zone distance, firmware
   version, and a read-only diagnostics snapshot.

2. **`ring_settings.c` / `ring_settings.h`** -- Persist the runtime config in a
   versioned NVS blob and expose lock-free getters so the sensor processing
   loop and BLE callbacks share one source of truth without duplicating state.

3. **`ring_diagnostics.c` / `ring_diagnostics.h`** -- Keep the in-memory
   bring-up snapshot and encode it into a stable versioned BLE payload so logs
   and the diagnostic characteristic expose the same facts.

4. **Device Information Service** -- Add Model Number (0x2A24), Firmware
   Revision (0x2A26), Hardware Revision (0x2A27), and Serial Number
   (0x2A25) characteristics to the existing 0x180A service. This is now done
   for the ring, with `DEVBOARD-C3` used until real PCB revisions exist.

5. **Hub firmware** -- Add a USB CDC serial command parser that dispatches
   the commands from section 3. The parser runs in a dedicated FreeRTOS
   task, communicates with the role engine and BLE central via their
   existing APIs.

### 7.2 Hub as Configuration Relay

When the companion app adjusts a ring's DPI via the hub (Web Serial), the
flow is:

1. App sends `SET_RING_DPI AA:BB:CC:DD:EE:01 20` (or equivalent command,
   which can be added to the serial protocol as needed).
2. Hub's serial parser identifies the target ring by MAC.
3. Hub's BLE central writes the value to the ring's DPI characteristic
   (0x0101) over the existing BLE connection.
4. Ring's GATT callback updates the DPI multiplier and persists to NVS.
5. Hub responds `OK` to the serial port.

This relay pattern means the companion app does not need a direct BLE
connection to configure rings -- the hub handles all BLE communication.

### 7.3 No Authentication on Serial

The USB serial protocol has no authentication. Physical access to the USB
port implies authorization. This is consistent with the threat model: the
hub is plugged into the user's computer, and anyone with physical USB
access already has full control of the host.

The BLE GATT configuration service is protected by BLE bonding (already
implemented in `hal_ble_esp.c`). Only bonded devices can read or write
configuration characteristics.

### 7.4 Configuration Versioning

Both the NVS role blob and gesture blob include a version byte. When
firmware adds new configuration fields, the version byte increments, and
the loader handles migration (populate new fields with defaults if loading
an older version). This avoids factory-reset on firmware update.
