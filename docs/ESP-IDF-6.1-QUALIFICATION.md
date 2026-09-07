# ESP-IDF 6.1 qualification

Status: **hardware qualification pending**. The repository pins the 6.1 build
candidate; a successful local verification report establishes software build
compatibility only. It does not establish BLE latency, power consumption, wake
reliability, or USB behavior on physical devices. Do not promote this candidate
as a hardware-qualified baseline until the evidence below passes review.

## Reproducible candidate

- ESP-IDF release: `v6.1`, commit `fff9895c82d744c7237be8847347bdd1b07c6643`.
- Previous build baseline: `v5.2.2`, commit
  `3b8741b172dc951e18509698dee938304bcf1523` (repository commit `aeb4725`).
- Run `scripts/setup-esp-idf-local.sh`, then
  `scripts/verify-firmware-local.sh --all`.
- Preserve the verification JSON, stage logs, resolved SDKCONFIG, and firmware
  binary hashes. Build outputs are isolated by SDK commit to prevent stale
  compiler caches from crossing SDK versions.
- Physical qualification must identify its exact repository commit and binary
  hashes. Rebuild and repeat affected measurements after firmware changes.

Upstream references: [release notes](https://github.com/espressif/esp-idf/releases/tag/v6.1)
and [migration guides](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32/migration-guides/index.html).

## Migration changes

The HAL declares GPIO and SPI driver components explicitly and uses IDF 6.1's
`gpio_wakeup_enable_on_hp_periph_powerdown_sleep` API and corresponding
capability macro. Wake classification reads the new wake-cause bitmap and
prioritizes GPIO when a timer also fires. The verifier recognizes two-part release tags, records the
shared Xtensa compiler, and preserves activation diagnostics on failure without
mixing activation banners into version output. The regenerated hub lock uses
lock format 3, esp_tinyusb 2.1.1, and TinyUSB 0.21.0~1.

## Known connection-policy gap

The current power manager requests 7.5 ms on motion/click and 15 ms after the
idle transition. It does not request 15 ms immediately after connection, so the
initial interval is selected by the central. It also treats successful request
submission as requested state; asynchronous remote rejection is reported by the
BLE HAL but does not reconcile the power manager's rejection state. These are
pre-existing behavior gaps, not evidence of an SDK regression. Resolve them
with connection/update state tests and physical traces before claiming the
connection policy is qualified.

## Physical acceptance gate

Record board revision, variant, sensor and click configuration, battery capacity
and measured voltage, host OS, BLE adapter, USB cable, instrumentation, firmware
hash, test procedure, raw trace paths, sample count, and observed failures. Use
actual measured values; leave unavailable evidence marked pending.

| Check | Procedure and acceptance evidence |
| --- | --- |
| BLE HID | Pair, reconnect after host/device restart, move, click, drag, and release. Exercise two-ring operation and disconnect during a held button. Confirm no stuck inputs or cross-device state leakage. Preserve host input traces and connection logs. |
| Latency and connection policy | Measure sensor-to-host HID latency under active movement. Report distribution and maximum with instrument resolution; require latency below 20 ms. Capture negotiated connection intervals and verify the intended 15 ms default / 7.5 ms active policy, including return to idle. A compiled configuration alone does not prove this policy. |
| Power and wake | Measure active, connected-idle, light-sleep, and deep-sleep current plus wake latency for each physical variant. Exercise every supported wake source repeatedly, record counts and failures, and require no missed wake or stuck state. Compare matched measurements with 5.2.2 where available; explain regressions and update POWER-BUDGET.md using measured duty cycles, never extrapolate from compiler success. |
| USB hub | Verify composite HID and companion CDC enumeration, input forwarding, disconnect/reconnect, host suspend/resume, CDC configuration round trips, and simultaneous BLE input with CDC traffic. Record host versions and logs; require no lost releases, resets, or hung interfaces. |
| Accessibility and surfaces | Test low-force clicks, sustained drag, slow movement and accessible recovery from disconnect. Follow SURFACE-TEST-PROTOCOL.md for wood, glass, fabric, paper, glossy magazine, and matte plastic. Record known optical limitations honestly; require no regression on supported surfaces. |

If the programmed connection policy or measured power falls short, fix that
behavior and repeat the affected tests before promoting the baseline. The
existing power budgets remain estimates until measurements support them.

## Local verification result

The migration passed all 36 stages of `scripts/verify-firmware-local.sh --all`:
14 verifier regressions, host CTest executables with and without ASan/UBSan,
companion protocol tests, contract/operator checks, all five firmware
configurations, and strict ERC/DRC for both active boards with zero findings.
The local report is `build-verification/idf61.json`; it records GCC 15.2.0,
resolved firmware configurations, stage logs, and binary/ELF hashes. Reports
are generated artifacts and must be retained with a physical qualification run.
Upstream SDK CMake emits component include-directory warnings; no repository
firmware compiler warnings were observed in the final build logs.

## Evidence availability

The migration workstation exposed no ESP development-board serial port and its
USB inventory returned no devices during the initial check. No device was
flashed and no physical measurements were collected. BLE, USB, latency, power,
and wake evidence are all **pending**.

Promotion requires a reviewed evidence record satisfying the table, with any
exceptions explicitly decided by the BDFL. Keep this status pending until then.
