# PowerPen Firmware — WSTD-BALL-NONE-NONE

ESP-IDF build target for the core PowerPen prototype. Shares all components with
the ring firmware via `EXTRA_COMPONENT_DIRS`; the pen target adds only
configuration differences (pin map, device name, click topology, wake sensor).

## Build

```bash
cd firmware/pen
idf.py build
idf.py -p /dev/ttyUSBx flash
```

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

Shared with interface extensions (PP1 prerequisite):
- `click/click_interface.h` — extend for multi-source (`click_source_t`)
- `click/dead_zone.c` — refactor to instanceable `dead_zone_ctx_t`
- `power/` — extend wake GPIO config to bitmask, add Hall gate-off on cal fail
- `diagnostics/` — decouple from `ring_state_t`
- `runtime_health/` — rename `ring_` prefix to `pf_`

## Firmware Tasks

See `docs/POWERPEN-SPEC.md` sections PP1–PP6 for the incremental task list.

## Spec

Full specification: [docs/POWERPEN-SPEC.md](../../docs/POWERPEN-SPEC.md)
