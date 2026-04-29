<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Current ERC / DRC Violations

This packet has a routed first-board pass with **no schematic-parity issues**
and **no unconnected items**, but the schematic sub-sheets and footprint
library bindings still carry known violations that block a fabrication claim.

This file is a snapshot of those violations so the manifest cannot quietly
drift. Regenerate the raw reports locally before relying on these counts.

## Snapshot

Toolchain: `kicad-cli 10.0.1` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc` violations | 50 | All severity-error |
| `pcb drc` violations | 27 | All severity-error |
| `pcb drc` unconnected items | 0 | Routing reaches every connected net |
| `pcb drc` schematic-parity issues | 0 | PCB and schematic reference the same parts |

The hub schematic is further along than the ring schematic, but several
hierarchical labels still connect to only one pin (15 `isolated_pin_label`
errors), and `lib_symbol_issues` plus `footprint_link_issues` show that the
project is not pulling in the local symbol/footprint libraries the way it
should.

## ERC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `isolated_pin_label` | 15 | Hierarchical labels reach exactly one pin and dead-end on the sheet |
| `lib_symbol_issues` (`PowerFinger`) | 11 | Local symbol library `PowerFinger` not configured in the project |
| `footprint_link_issues` (`Capacitor_SMD`) | 5 | Default capacitor footprint library not on the project library path |
| `footprint_link_issues` (`Resistor_SMD`) | 4 | Default resistor footprint library not on the project library path |
| `pin_to_pin` | 3 | Unspecified-type pin tied directly to a passive pin |
| `lib_symbol_issues` (`Device`) | 3 | Default `Device` symbol library not on the project library path |
| `unconnected_wire_endpoint` | 2 | Wire ends that should land on a pin or label |

## DRC top categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `lib_footprint_mismatch` (`TestPoint_Pad_D1.0mm`) | 9 | PCB footprint differs from the upstream library copy |
| `lib_footprint_mismatch` (`C_0402_1005Metric`) | 5 | PCB footprint differs from the upstream library copy |
| `lib_footprint_mismatch` (`R_0402_1005Metric`) | 4 | PCB footprint differs from the upstream library copy |
| `lib_footprint_mismatch` (`C_0603_1608Metric`) | 2 | PCB footprint differs from the upstream library copy |
| `lib_footprint_issues` (`MountingHole_1.4mm`) | 2 | Required footprint not in the configured library |
| `lib_footprint_mismatch` (one each) | 5 | `USB_A_Plug_SOFNG_USB-05`, `SOT-23-6`, `SOT-23-5`, `LED_0402_1005Metric`, `ESP32-S3-MINI-1-N8_FirstBoard` |

These are almost entirely **library hygiene** issues, not routing or
electrical issues. Closing them is mostly a matter of regenerating footprint
caches against the configured libraries (or re-pointing the project at the
correct local libraries).

## How to regenerate

```bash
mkdir -p build-kicad/USB-HUB
kicad-cli sch erc \
  --severity-all \
  -o build-kicad/USB-HUB/erc.txt \
  hardware/shared/USB-HUB/kicad/usb_hub.kicad_sch

kicad-cli pcb drc \
  --severity-all \
  --schematic-parity \
  -o build-kicad/USB-HUB/drc.txt \
  hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb
```

Or use the wrapper:

```bash
scripts/verify-firmware-local.sh --kicad-only
```

## Closing the gap

1. Add the local `PowerFinger` symbol library to the project library table so
   `lib_symbol_issues` clears.
2. Re-pair the schematic to the project-local footprint libraries
   (`fp-lib-table` already lists `PowerFinger_USB.pretty`); update the schematic
   to use those paths so `footprint_link_issues` clears.
3. Resolve the 15 `isolated_pin_label` errors by either wiring those labels to
   a real net or removing them.
4. Re-pull each PCB footprint from its library so `lib_footprint_mismatch`
   clears, or pin the PCB to project-local footprints.
5. Re-run ERC + DRC and update this file's counts in the same commit.
6. Only then update `MANIFEST.md` to drop the "not fabrication-released"
   language.
