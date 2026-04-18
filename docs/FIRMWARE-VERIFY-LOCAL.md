# Firmware Local Verification

PowerFinger does not use GitHub Actions by default. The supported verification
path is local:

- Host-side unit tests in `firmware/test`
- Real ESP-IDF builds for the active firmware lane (`ring` + `hub`)

## Active-Lane Verification Contract

The shared verification contract for active-lane work is:

1. Host-side tests green
2. `firmware/ring` build green
3. `firmware/hub` build green

If any one of those is red, the active lane is red. The `pen` and `puck`
targets remain available for exploratory work, but they are outside the shared
bring-up contract until the ring + hub lane clears Gate 4.

## Pinned Local Baseline

- ESP-IDF baseline: `v5.2.2`
- Target toolchains: `esp32c3`, `esp32s3`
- Hub dependency bootstrap: `espressif/esp_tinyusb` resolves on first build via
  Espressif's component manager if it is not already cached locally

Override `POWERFINGER_IDF_VERSION` only when intentionally testing another
baseline and record that deviation in your local bring-up notes.

## Bootstrap A Local Toolchain

From the repo root:

```bash
scripts/setup-esp-idf-local.sh
scripts/verify-firmware-local.sh
```

The installer keeps the toolchain under `~/.powerfinger-sdk/` by default so the
repo stays clean. After the initial install, `scripts/verify-firmware-local.sh`
will try to activate that same local toolchain automatically from a fresh shell.
If `idf.py` is already in `PATH` but points at a different ESP-IDF version, the
verifier will prefer the pinned local baseline instead of silently building
against the wrong SDK.

If you want `idf.py` available in your current shell for direct one-off
iteration, export it explicitly:

```bash
eval "$(scripts/setup-esp-idf-local.sh --export)"
```

## Prerequisites

- CMake available for the host-side test suite
- `git` available if you use the repo-pinned bootstrap script

If you prefer to manage ESP-IDF yourself, exporting your own `IDF_PATH` is
still supported.

## Recommended Command

From the repo root:

```bash
scripts/verify-firmware-local.sh
```

Default behavior:

- Runs host-side unit tests
- Builds the active validation lane: `firmware/ring` and `firmware/hub`
- Writes ESP-IDF artifacts under `build-idf/ring/` and `build-idf/hub/`

## Other Modes

Build all ESP-IDF firmware projects:

```bash
scripts/verify-firmware-local.sh --all
```

Build a specific firmware project:

```bash
scripts/verify-firmware-local.sh hub
scripts/verify-firmware-local.sh pen
```

Run only the host-side unit tests:

```bash
scripts/verify-firmware-local.sh --host-tests-only
```

Skip host tests and build only firmware:

```bash
scripts/verify-firmware-local.sh --firmware-only ring hub
```

## Direct IDF Iteration

When iterating on a single target and you already know the project you want:

```bash
IDF_TARGET=esp32c3 idf.py -C firmware/ring -B build-idf/ring build
IDF_TARGET=esp32s3 idf.py -C firmware/hub -B build-idf/hub build
```

The local verifier script is still the preferred shared path because it keeps
the active lane and host tests in one place.

## Related Docs

- [ACTIVE-LANE-CHECKLIST.md](ACTIVE-LANE-CHECKLIST.md)
- [GO-NO-GO-RUBRIC.md](GO-NO-GO-RUBRIC.md)
- [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
