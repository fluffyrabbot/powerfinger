<!-- SPDX-License-Identifier: MIT -->
# Component Modularity Architecture

PowerFinger is still greenfield. The repo should treat hardware substitutions
the way software treats interface-compatible implementations: a part is
swappable only when its electrical, mechanical, firmware, sourcing, safety, and
accessibility contracts are all still satisfied.

This document defines the first contract spine for the active
`R30-OLED-NONE-NONE` lane. It is intentionally small. It records current truth;
it does not claim that generators, schema validation, or manufacturing evidence
already exist.

## Current Contract Spine

| Layer | First contract file | Observed source of truth mirrored |
| --- | --- | --- |
| Variant composition | `variants/r30-oled-none-none.yaml` | Active packet manifest, firmware profile, BOM, CAD packet |
| Component anchors | `hardware/contracts/components.r30-oled-active.yaml` | Active BOM rows, substitution gates, first-board source caveats |
| Board interface | `hardware/contracts/board.r30-rigid-p0.yaml` | R30 KiCad interface contract and board-profile docs |
| Motion sensor | `hardware/contracts/sensor.paw3204.yaml` | PAW3204 driver, dual-footprint analysis, BOM notes |
| Battery envelope | `hardware/contracts/battery.lipo-protected-ring-100mah.yaml` | BOM, battery safety rules, CAD keepout, stackup template |
| Enclosure envelope | `hardware/contracts/enclosure.r30-serviceable-shell-v1.yaml` | R30 OpenSCAD model and stackup/coupon verification template |

These files are now locally checked contracts for the active lane. Packet docs
and schematics still remain authoritative for physical layout and measured
evidence, but the YAML records are no longer allowed to drift silently from the
variant ID, selected firmware drivers, board-profile Kconfig, BOM target, packet
manifest, or interface-contract non-claims.

## Boundary Rules

- A `variant` selects compatible contracts. It should not hide open risks.
- A `board` contract owns pins, buses, voltage assumptions, wake lines, service
  paths, antenna keep-outs, and firmware symbols.
- A `sensor` contract owns protocol, output shape, lens/aperture/focal
  geometry, and surface claims.
- A `battery` contract owns protected-cell requirements, connector/service
  requirements, charging assumptions, and safety evidence.
- An `enclosure` contract owns physical envelopes, service access, fit coupons,
  tolerance assumptions, and human-accessibility constraints.
- Firmware should eventually consume a generated board/profile fragment from
  the variant contract. For now, the contract cross-checks the existing
  `sdkconfig.defaults.r30_oled_none_none` path.

## What Is Still Prose-Only

- Some BOM alternates are still CSV notes and `SOURCE-ALTERNATIVES.md`, but the
  active BOM file, target, critical active-line anchors, U2 sensor substitution
  decision, charge controller alternates, charge-gate MOSFET alternates, and
  battery harness receptacle constraints are now checked in a dedicated
  component contract.
- CAD parameters are still OpenSCAD variables, not generated from envelope
  records. The checker compares headline envelope values only.
- Gesture intent is not yet a common API between physical input and HID output.
- EVT/DVT/PVT evidence is represented by checklists and gate docs, not a
  structured manufacturing evidence tree.
- Supplier qualification is documented as a baseline, not as tested alternates
  with pass/fail records.

## Local Drift Check

Run the local contract drift gate with:

```bash
scripts/check-contracts-local.sh
```

With no arguments, the gate runs the active-lane consistency check and the
fixture-backed negative self-test. The checker intentionally stays narrow. It
verifies that the active variant points at existing contracts, that every
contract has the minimum required schema shape, that referenced files exist,
that the active board/sensor/battery and enclosure contracts agree with each
other, that the R30 board contract agrees with the current board-profile
Kconfig pins, and that the packet manifest, BOM target, and interface contract
still name the active pin and non-claim seams. It also generates the expected
R30 board-profile `sdkconfig.defaults` fragment from the active variant plus
board contract and diffs that generated output against the checked-in firmware
fragment.
Production non-claims for `CHRG_STAT`, PAW3204 reset, PAW3204 motion wake, glass
support, and absent premium silicon are structured fields in the active
contracts rather than prose-only assertions.

The reusable checker lives in `scripts/contract_check.rb`. Its fixture-backed
self-test covers the current hard drift classes, and its print mode emits the
generated R30 board-profile fragment:

```bash
ruby scripts/contract_check.rb --self-test
ruby scripts/contract_check.rb --print-sdkconfig-profile
```

## Immediate Next Architecture Step

The checker implementation now has dedicated classes for cross-contract
validation, packet-text validation, generated sdkconfig validation, and
component/BOM validation, plus a shared validator base for common checker
delegation, a board-interface validator for hard pin/interface expectations, a
dedicated self-test fixture runner, and a focused sdkconfig profile generator.
The generator owns sdkconfig value derivation for selected firmware symbols,
GPIO pin values, ADC channel values, wake masks, and the disabled Hall rail.
The board-interface validator owns the active R30 pin expectation table and
non-production behavior claims, so `scripts/contract_check.rb` stays a CLI
wrapper and shared contract helper instead of owning board-profile semantics.

The next narrow implementation step is to split the remaining contract-shape
checks out of `scripts/contract_check.rb` into a focused schema/identity
validator. Keep `--print-sdkconfig-profile` byte-for-byte stable, preserve
hard-drift failure semantics, and leave KiCad geometry parsing out of scope.
Keep fixture coverage around:

- `variants/r30-oled-none-none.yaml`
- `hardware/contracts/board.r30-rigid-p0.yaml`
- `firmware/ring/sdkconfig.defaults.r30_oled_none_none`
- `hardware/ring/R30-OLED-NONE-NONE/kicad/INTERFACE-CONTRACT.md`

Validation should keep failing only on hard contract drift: pin mismatch,
missing contract reference, BOM/manifest mismatch, incompatible active
contracts, or a claimed firmware behavior with no firmware symbol. It should
not try to parse KiCad geometry yet.
