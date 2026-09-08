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

## Connection policy

The power manager requests 15 ms immediately on connection. Motion or a held
click selects 7.5 ms; the idle transition selects 15 ms. Desired, in-flight,
and confirmed intervals are tracked separately. While an update is pending,
activity changes are coalesced into the latest desired target, which is
considered when completion arrives. Submission success is never treated as
confirmation that the central accepted the interval.

A rejection or confirmation of a different interval suppresses that requested
target for the current connection; the other target remains eligible. Transient
submission failures use three retries per connection with exponential backoff
of 250, 500, and 1000 ms. Repeated motion/click events cannot bypass that delay.
An update without a completion after NimBLE's 40-second procedure window
suspends further negotiation until reconnect, preventing a late callback from
being paired with a newer request. This suspension does not disconnect the
usable HID link. Thermal, battery, sleep, and watchdog behavior remain active.

The HAL rejects update callbacks for a stale connection handle, checks descriptor
lookup before reading the interval, and forwards update/rejection events through
the application queue to the power manager. Physical traces are still required
to establish which intervals each central accepts and the resulting latency and
power consumption.

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

For connection-policy traces, capture these sequences separately: untouched
connection, motion while the initial idle request is pending, idle while an
active request is pending, central rejection of each target, disconnect during
an update, and reconnect. Record the negotiated interval rather than counting
request submissions as success. A central that declines the requested interval
must remain usable without a request storm; record that host as unqualified for
the requested interval until measurements establish its supported behavior.

If the programmed connection policy or measured power falls short, fix that
behavior and repeat the affected tests before promoting the baseline. The
existing power budgets remain estimates until measurements support them.

## Local verification result

The connection-policy follow-up passed all 36 stages of `scripts/verify-firmware-local.sh --all`:
14 verifier regressions, host CTest executables with and without ASan/UBSan
(including 14 additional power-manager/controller transition tests),
companion protocol tests, contract/operator checks, all five firmware
configurations, and strict ERC/DRC for both active boards with zero findings.
The local report is `build-verification/ble-policy.json`; it records GCC 15.2.0,
resolved firmware configurations, stage logs, and binary/ELF hashes. Reports
are generated artifacts and must be retained with a physical qualification run.
Upstream SDK CMake emits component include-directory warnings. The generic
`SENSOR_NONE` build also reports existing unused sensor recovery/calibration
helpers and a calibration scheduling variable in `app_runtime.c`; these
warnings are retained in its build log.

## Evidence availability

The workstation still exposes no ESP development-board serial port. A follow-up
IORegistry USB inventory found ordinary hubs, a keyboard, and storage devices,
but no identifiable PowerFinger or ESP board. `system_profiler` returned an
empty USB list, so that output alone was not used as proof of an empty USB bus.
No device was flashed and no physical measurements were collected. BLE, USB,
latency, power, and wake evidence are all **pending**.

Promotion requires a reviewed evidence record satisfying the table, with any
exceptions explicitly decided by the BDFL. Keep this status pending until then.
