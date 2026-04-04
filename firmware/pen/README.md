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
| Wake sensor | N/A (optical has motion detect pin) | DRV5032 digital Hall switch (0.54µA) |
| PCB | Flex or rigid-flex | Rigid strip (~8mm wide) |
| State machine types | `ring_state_t` / `ring_event_t` | `pen_state_t` / `pen_event_t` |

## Shared Components (from ring/)

- `hal/` — GPIO, SPI, ADC, timer, sleep, storage, OTA, BLE
- `ble_hid/` — HID report descriptor, GAP handler
- `power/` — Power manager (already form-factor-agnostic)
- `sensors/` — `sensor_interface.h` (ball+Hall driver implements this)
- `click/` — Click interface, calibration, dead zone
- `diagnostics/` — Diagnostics snapshot
- `runtime_health/` — Fault policy

## PowerPen-Specific Components

- `state_machine/` — `pen_state.h` / `pen_state.c` (own types, same structure)

## Firmware Tasks

See `docs/POWERPEN-SPEC.md` sections P1–P7 for the incremental task list.

## Spec

Full specification: [docs/POWERPEN-SPEC.md](../../docs/POWERPEN-SPEC.md)
