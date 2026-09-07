# Local Verification

PowerFinger verification runs locally. SDK installation and initial dependency
retrieval require network access; device operation remains fully offline. The
public entrypoint is `scripts/verify-firmware-local.sh`; it delegates to the
standard-library orchestrator in `scripts/verify_firmware_local.py`. Every
run writes a JSON report and per-stage log files under an ignored build directory.
Commands and tests have timeouts; output is streamed to disk rather than
accumulated in the agent's context.

## Default

```bash
scripts/verify-firmware-local.sh
```

The default selection is verifier regression tests, host CTests, Ruby contract
checks, Python operator self-tests, the companion Node protocol test, a separate
ASan+UBSan host CTest build, the active `r30-oled-none-none` ring profile, `hub`,
and strict KiCad 10 ERC/DRC for both active packets. Failures are collected across
independent stages; missing prerequisites fail explicitly and dependent checks
are recorded as skipped. Such a run cannot pass.

The report records source revision/dirty state, declared and observed tool
versions, selections, statuses, reasons, and artifact paths. A dirty source
tree is recorded for review and does not by itself turn green checks red.
Each invocation replaces any previous report with a `running` result before
checking anything, then atomically writes the final result. Interrupted or
unfinished runs must not be treated as passes.

## Modes

```bash
scripts/verify-firmware-local.sh --fast
scripts/verify-firmware-local.sh --firmware-only ring pen puck hub
scripts/verify-firmware-local.sh --all
scripts/verify-firmware-local.sh --doctor
scripts/verify-firmware-local.sh --hardware-only
scripts/verify-firmware-local.sh --hardware-report-only
```

`--fast` (also the compatibility alias `--host-tests-only`) runs verifier tests,
host, contracts, operator, and companion checks only. `--firmware-only` excludes all
host and hardware checks. `--all` adds the generic development ring, pen, and
puck to the firmware selection. The `ring` selector means the active R30
profile; use `ring-generic` explicitly for the fake-sensor development build.
`--doctor` performs prerequisite and pinned-SDK checks without builds.
`--hardware-only` requires KiCad and passes only when both packets have valid
zero-finding JSON reports. `--hardware-report-only` records findings for
inspection but always returns nonzero and never reports `pass`; it is not
verification. `--with-kicad` and `--kicad-strict` are retained aliases for
adding strict hardware checks, and `--kicad-only` aliases `--hardware-only`.
Contradictory modes are rejected.

Use `--report PATH` to choose the JSON report location. Use
`POWERFINGER_IDF_BUILD_ROOT` to relocate ESP-IDF build outputs; SDKCONFIG files
remain inside the selected private `build-idf/verify/<profile>/` directory and are resolved
from committed defaults, so an ignored `firmware/<project>/sdkconfig` cannot
silently select a previous session's configuration.

## SDK baseline

`toolchains/esp-idf-local.json` is authoritative: ESP-IDF `v5.2.2`, commit
`3b8741b172dc951e18509698dee938304bcf1523`, with `esp32c3` and `esp32s3`
targets. The verifier and setup script consume that same manifest. The committed
hub `dependencies.lock` records the resolved component versions; its hash is
checked against the manifest before and after firmware builds. Update the lock
and manifest together only when deliberately changing dependencies.

Setup never updates an existing installation automatically; it validates HEAD, tracked or
untracked modifications, and submodule state before export or build.

```bash
scripts/setup-esp-idf-local.sh
scripts/setup-esp-idf-local.sh --check
eval "$(scripts/setup-esp-idf-local.sh --export)"
```

A fresh checkout may use the setup script, then the verifier. If the SDK is
absent, host-only and fast checks remain usable while required firmware stages
fail explicitly.

## KiCad and host prerequisites

The strict hardware path requires `kicad-cli` 10.x (the tested host is 10.0.6)
and uses `--severity-all --format json --exit-code-violations`; it does not parse English
console summaries or modify source boards. Reports must exist and contain the
required ERC `sheets[*].violations` or DRC `violations`, `unconnected_items`,
and `schematic_parity` arrays. Host prerequisites are CMake 3.16 or newer,
a C11 compiler with ASan/UBSan support (Clang or GCC), Ruby with YAML and CSV,
Python 3.10 or newer, Node.js 18 or newer, Git, and Bash. Exact host patch
versions are not pinned. OpenSCAD is recorded when available for CAD provenance;
these checks do not render mechanical models.

## Profile and all-target builds

The active optical profile is selected automatically. To request the profile
explicitly with a ring build:

```bash
scripts/verify-firmware-local.sh --firmware-only --ring-profile r30-oled-none-none ring
```

No flashing, hardware-in-the-loop, SDK migration, or CAD mutation is part of
this verifier. Those activities require separate review and evidence.
