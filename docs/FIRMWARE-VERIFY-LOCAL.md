# Firmware Local Verification

PowerFinger does not use GitHub Actions by default. The supported verification
path is local:

- Host-side unit tests in `firmware/test`
- Real ESP-IDF builds for the firmware projects you want to touch

## Prerequisites

- ESP-IDF exported in your shell so `idf.py` is on `PATH`
- CMake available for the host-side test suite

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
