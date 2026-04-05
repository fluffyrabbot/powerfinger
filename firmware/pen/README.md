# PowerPen Firmware — WSTD-BALL-NONE-NONE

ESP-IDF build target for the core PowerPen prototype. Shares most components
with the ring firmware via `EXTRA_COMPONENT_DIRS`; the pen target adds
target-specific configuration plus a local `pen_wake/` component for DRV5032
spurious-wake debounce and runtime wake-mask filtering.

## Build

```bash
cd firmware/pen
idf.py build
idf.py -p /dev/ttyUSBx flash
```

## Current Status

- Host-side verification covers the pen-specific dual-click path and DRV5032
  wake debounce (`test_click_pen`, `test_pen_wake_debounce`).
- The pen target now resolves both `firmware/pen/components/` and
  `firmware/ring/components/`.
- Real ESP-IDF build, pairing, surface tracking, wake current, and latency
  still require local hardware and an installed ESP-IDF toolchain.

## What's Different from Ring

| Concern | Ring | PowerPen |
|---------|------|----------|
| Sensor | PAW3204 optical (SPI) | Ball+Hall 4x DRV5053 (ADC) |
| Click | Single dome (one GPIO) | Barrel switch + tip dome (two GPIOs) |
| Dead zone | Always on (dome press moves sensor) | Per-click-source: barrel off, tip on |
| Pin map | See `ring/sdkconfig.defaults` | See `pen/sdkconfig.defaults` |
| BLE device name | "PowerFinger Ring" | "PowerPen" |
| Hall power gating | N/A for optical ring | MOSFET gate on Hall VCC rail |
| Wake sensor | N/A (optical has motion detect pin) | DRV5032 digital Hall switch (0.54µA) + spurious wake debounce |
| Wake GPIOs | Single dome pin | Three-GPIO bitmask (barrel, tip dome, DRV5032) |
| VBAT divider | TBD | 10MΩ/10MΩ (0.165µA sleep draw) |
| PCB | Flex or rigid-flex | Rigid strip (~8mm wide) |
| State machine | `ring_state_t` / `ring_event_t` | Same — reused unchanged |

## Shared Components (from ring/)

Shared unchanged:
- `hal/` — GPIO, SPI, ADC, timer, sleep, storage, OTA, BLE
- `ble_hid/` — HID report descriptor, GAP handler
- `sensors/` — `sensor_interface.h` (ball+Hall driver implements this)
- `click/calibration.c` — Sensor zero-offset calibration

Shared with implemented pen-specific extensions:
- `click/` — source-aware click polling plus `click_pen.c` for barrel + tip
- `click/dead_zone.c` — instanceable dead-zone contexts; pen uses tip-only suppression
- `power/` — wake GPIO bitmask, Hall gate-off on failed calibration, runtime wake-mask override
- `diagnostics/` — snapshot carries `drv5032_wake_enabled` and `spurious_wake_count`

Pen-local:
- `pen/components/pen_wake/` — DRV5032 spurious-wake debounce and GPIO8 wake disable/restore logic

## Firmware Tasks

See `docs/POWERPEN-SPEC.md` sections PP1–PP6 for the incremental task list.

## Spec

Full specification: [docs/POWERPEN-SPEC.md](../../docs/POWERPEN-SPEC.md)
