<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Current ERC / DRC Snapshot

This packet has a routed first-board pass with **no PCB DRC violations**, **no
unconnected items**, and **no schematic-parity issues** on the all-severity PCB
DRC pass. The remaining all-severity KiCad messages are schematic ERC warnings,
not PCB routing, native-USB, or footprint provenance drift.

Regenerate the raw reports locally before relying on these counts.

## Snapshot

Toolchain: `kicad-cli 10.0.1` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc --severity-error` violations | 0 | Error gate is clean |
| `sch erc` all-severity messages | 36 | All warnings |
| `pcb drc --schematic-parity` all-severity violations | 0 | Local footprint provenance is clean |
| `pcb drc` unconnected items | 0 | Routing reaches every connected net |
| `pcb drc` schematic-parity issues | 0 | PCB and schematic reference the same parts |

## What Changed

- PCB footprints now point at source-controlled `PowerFinger_USB` first-board
  footprints instead of relying on upstream library cache copies.
- The no-BOM shell-clamp holes now use
  `PowerFinger_USB:MountingHole_1.4mm_Clamp`.
- `usb_hub.kicad_pro` pins the local footprint library and ignores
  `lib_footprint_mismatch`; the first-board footprints are intentionally local
  provenance sources, so upstream-copy comparison is no longer useful signal.
- The all-severity ERC no longer reports `footprint_link_issues`.

## Remaining ERC Warning Categories

| Rule | Count | Source of the problem |
|------|-------|----------------------|
| `isolated_pin_label` | 15 | Bring-up/service labels intentionally dead-end on sheet-local access points |
| `lib_symbol_issues` (`PowerFinger`) | 11 | Local symbol library `PowerFinger` is still not configured in a project symbol table |
| `lib_symbol_issues` (`Device`) | 3 | Default `Device` symbol library is still not configured in a project symbol table |
| `pin_to_pin` | 3 | Unspecified-type pins tied directly to passive pins |
| `endpoint_off_grid` | 2 | Two root-sheet wire endpoints are off the connection grid |
| `unconnected_wire_endpoint` | 2 | Two USB/power sheet wire ends should be landed or removed |

## How To Regenerate

```bash
mkdir -p build-kicad/USB-HUB
kicad-cli sch erc \
  -o build-kicad/USB-HUB/erc.txt \
  hardware/shared/USB-HUB/kicad/usb_hub.kicad_sch

kicad-cli pcb drc \
  --schematic-parity \
  --refill-zones \
  -o build-kicad/USB-HUB/drc.txt \
  hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb
```

Or use the wrapper:

```bash
scripts/verify-firmware-local.sh --kicad-only
```

## Closing The Remaining Gap

1. Add or repair the project symbol library table so `PowerFinger` and `Device`
   symbol-library warnings stop depending on global KiCad configuration.
2. Resolve the service-label ERC warnings by either wiring the access labels to
   real cross-sheet nets or documenting them as intentional no-connect service
   stubs in the schematic source.
3. Fix the two off-grid endpoints and two unconnected wire endpoints.
4. Re-run all-severity ERC + DRC and update this file's counts in the same
   commit.
