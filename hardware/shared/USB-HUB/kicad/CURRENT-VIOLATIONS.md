<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Current ERC / DRC Snapshot

This packet has a routed first-board pass with **no schematic ERC violations**,
**no PCB DRC violations**, **no unconnected items**, and **no schematic-parity
issues** on the all-severity KiCad checks. The remaining quote blockers are
mechanical evidence items, not KiCad ERC/DRC hygiene.

Regenerate the raw reports locally before relying on these counts.

## Snapshot

Toolchain: `kicad-cli 10.0.2` (Homebrew, macOS).

| Check | Count | Notes |
|-------|-------|-------|
| `sch erc --severity-error` violations | 0 | Error gate is clean |
| `sch erc` all-severity messages | 0 | All-severity ERC is clean |
| `pcb drc --schematic-parity` all-severity violations | 0 | Local footprint provenance is clean |
| `pcb drc` unconnected items | 0 | Routing reaches every connected net |
| `pcb drc` schematic-parity issues | 0 | PCB and schematic reference the same parts |

## What Changed

- PCB footprints now point at source-controlled `PowerFinger_USB` first-board
  footprints instead of relying on upstream library cache copies.
- Schematic symbols now point at a project-local `sym-lib-table` plus
  `PowerFinger.kicad_sym` for the hub-specific local symbols, with `Device` and
  `power` explicitly configured for KiCad CLI/open-on-another-machine use.
- The no-BOM shell-clamp holes now use
  `PowerFinger_USB:MountingHole_1.4mm_Clamp`.
- `usb_hub.kicad_pro` pins the local footprint library and ignores
  `lib_footprint_mismatch`; the first-board footprints are intentionally local
  provenance sources, so upstream-copy comparison is no longer useful signal.
- The root sheet's controls/service sheet edge is on the KiCad connection grid,
  and the USB/power sheet's connector-side power flags are landed directly on
  the VBUS/GND wires instead of hanging from short stubs.
- `TP6`-`TP9` service pads are now explicit no-BOM schematic symbols matching
  the already-routed board footprints, so `EN`, `BOOT_N`, `UART_TX_DBG`, and
  `UART_RX_DBG` are modeled as real first-board service access instead of
  sheet-local dead-end labels.
- The previous `PWR_FLAG` assertions were removed because the captured
  connector, regulator, and module symbols now provide the power-driver
  semantics KiCad needs without pin-typing warnings.
- A small `EN` back-layer route jog now clears the ESP32-S3 antenna keepout
  under `kicad-cli 10.0.2`.
- The all-severity ERC no longer reports `footprint_link_issues`,
  `lib_symbol_issues`, `endpoint_off_grid`, `unconnected_wire_endpoint`,
  `isolated_pin_label`, or `pin_to_pin`.

## Remaining ERC Warning Categories

None. All-severity schematic ERC is clean in the current local KiCad run.

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

KiCad ERC/DRC hygiene is closed for this packet. Keep the next gap focused on
first-hardware evidence: printed host fit, clamp alignment, service-hatch reach,
adjacent-port clearance, and connector-retention observations.
