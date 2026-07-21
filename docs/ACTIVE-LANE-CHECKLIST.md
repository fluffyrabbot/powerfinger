<!-- SPDX-License-Identifier: MIT -->
# Active Lane Checklist

This is the canonical checklist for the current PowerFinger program.

## Scope

- **Primary claim:** a user with limited wrist mobility can browse, click,
  drag, and scroll on everyday surfaces without host-side remapping software
- **Active validation lane:** `R30-OLED-NONE-NONE` ring pair plus `USB-HUB`
- **Hedge lane:** `WSTD-BALL-NONE-NONE` wand
- **Deferred until Gate 6:** ball+Hall ring, puck, Pro optical-on-ball ring,
  OCR / camera work, IMU hybrids, direct BLE companion mode, OTA UX, packaged
  companion apps, and broader gesture expansion

## Source Of Truth

- Ring packet: [hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md](../hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md)
- Ring physical coupon ledger: [hardware/ring/R30-OLED-NONE-NONE/COUPON-RESULTS.md](../hardware/ring/R30-OLED-NONE-NONE/COUPON-RESULTS.md)
- Hub packet: [hardware/shared/USB-HUB/MANIFEST.md](../hardware/shared/USB-HUB/MANIFEST.md)
- Hub physical coupon ledger: [hardware/shared/USB-HUB/COUPON-RESULTS.md](../hardware/shared/USB-HUB/COUPON-RESULTS.md)
- Driver/hardware contract: [DRIVER-HARDWARE-CONTRACT.md](DRIVER-HARDWARE-CONTRACT.md)
- R30 board firmware config: [R30-OLED-FIRMWARE-CONFIG.md](R30-OLED-FIRMWARE-CONFIG.md)
- Local verification flow: [FIRMWARE-VERIFY-LOCAL.md](FIRMWARE-VERIFY-LOCAL.md)
- Gate order and pass/fail rules: [GO-NO-GO-RUBRIC.md](GO-NO-GO-RUBRIC.md)

If a repo change conflicts with this checklist, the change should be rewritten
or deferred unless the checklist itself is being updated deliberately.

## Verification Contract

Active-lane software verification is green only when all three are green:

1. `scripts/verify-firmware-local.sh --host-tests-only` (host-side unit tests,
   active-lane contract checks, Shenzhen reply scaffold self-test, and USB-HUB
   coupon ingest self-test)
2. `IDF_TARGET=esp32c3 idf.py -C firmware/ring -B build-idf/ring build`
3. `IDF_TARGET=esp32s3 idf.py -C firmware/hub -B build-idf/hub build`

The shared verifier `scripts/verify-firmware-local.sh` remains the preferred
entrypoint because it runs the host tests first and then builds the active lane
by default.

## Gate Execution Order

### Gate 0 — Claim Freeze

- Keep README and top-level build docs focused on the optical ring pair + hub
- Treat the wand as hedge-only, not as a parallel primary program
- Keep deferred features documented as backlog or defensive publication, not as
  active implementation promises

### Gate 1 — Single-Ring Human Control Loop

- Prove browse, click, double-click, drag, and text-selection tasks with one ring
- Check the PAW3204, lens/focal stack, and dome rows in
  [DRIVER-HARDWARE-CONTRACT.md](DRIVER-HARDWARE-CONTRACT.md) before treating a
  tracking failure as firmware-only
- Run the surfaces and metrics defined in [SURFACE-TEST-PROTOCOL.md](SURFACE-TEST-PROTOCOL.md)
- Publish measured outcomes back into the relevant test/rubric docs rather than
  separate ad hoc notes

### Gate 2 — Ring Package Closure

- Close the active ring packet using
  [hardware/ring/R30-OLED-NONE-NONE/FIRST-BOARD-CHECKLIST.md](../hardware/ring/R30-OLED-NONE-NONE/FIRST-BOARD-CHECKLIST.md)
  and
  [hardware/ring/R30-OLED-NONE-NONE/STACKUP-VERIFY.md](../hardware/ring/R30-OLED-NONE-NONE/STACKUP-VERIFY.md)
- Ingest printed-coupon observations into
  [hardware/ring/R30-OLED-NONE-NONE/COUPON-RESULTS.md](../hardware/ring/R30-OLED-NONE-NONE/COUPON-RESULTS.md)
  before turning generated worksheet rows into ring closure state
- Keep the packet-level KiCad interface contract aligned with
  [DRIVER-HARDWARE-CONTRACT.md](DRIVER-HARDWARE-CONTRACT.md) whenever GPIO,
  supply, sensor, or service-connector assumptions change
- Do not start secondary ring or puck hardware while ring package closure is red

### Gate 3 — Safety, Power, And RF Reality

- Replace estimate-only claims with measured active / idle / deep-sleep numbers
- Do not claim charge-safety behavior unless the `VBAT_SENSE`, NTC,
  charge-enable, and VBUS-detect rows in
  [DRIVER-HARDWARE-CONTRACT.md](DRIVER-HARDWARE-CONTRACT.md) are wired,
  Kconfig-enabled, and observed on the actual board
- Record enclosure charging, low-voltage cutoff, and BLE-link observations in
  the existing power / safety docs that already govern those claims

### Gate 4 — Two-Ring Composition

- Prove cursor, left-click, right-click, scroll, and click-drag through the hub
- Verify reconnect, bond-loss recovery, and no stuck-button failures
- Check the hub contract rows in
  [DRIVER-HARDWARE-CONTRACT.md](DRIVER-HARDWARE-CONTRACT.md) before treating USB,
  BLE, or role-state failures as purely software defects
- Close the hub packet using
  [hardware/shared/USB-HUB/FIRST-BOARD-CHECKLIST.md](../hardware/shared/USB-HUB/FIRST-BOARD-CHECKLIST.md)
  and
  [hardware/shared/USB-HUB/CONNECTOR-RETENTION-VERIFY.md](../hardware/shared/USB-HUB/CONNECTOR-RETENTION-VERIFY.md);
  ingest printed-coupon observations into
  [hardware/shared/USB-HUB/COUPON-RESULTS.md](../hardware/shared/USB-HUB/COUPON-RESULTS.md)
  before turning generated worksheet rows into closure state

## Evidence Publishing Rules

- Update existing governing docs with measured values and pass/fail outcomes
- Keep the ring and hub manifests synchronized with the current first-board
  status and linked evidence templates
- Do not turn deferred work into active scope until the current gate order is
  explicitly cleared
