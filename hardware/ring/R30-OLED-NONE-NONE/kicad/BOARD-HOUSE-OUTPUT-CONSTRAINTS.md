<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Board-House Output Constraints

This checklist turns the DRC-clean R30 KiCad source and local
fabrication-output review into explicit prototype-house intake questions. It is
for DFM/pre-fab review only. It is not a release-to-build sign-off and does not
prove physical ring fit, stackup height, focal distance, RF behavior, battery
service, click feel, yield, or realized COGS.

Use this with `FABRICATION-OUTPUTS.md`, `CURRENT-VIOLATIONS.md`, the KiCad
source, and the active BOM. If a board house asks for different plot/drill/POS
settings, regenerate the local outputs and update this checklist before asking
for a ring PCB fab or assembly quote.

## Current Local Output Basis

| Intake item | Current packet answer | Status |
|-------------|-----------------------|--------|
| Board variant | `R30-OLED-NONE-NONE` optical ring rigid P0 | Known |
| Board outline | `43 x 18 mm` KiCad `Edge.Cuts` outline | Known in source; fit unproven |
| Layer count | 4 copper layers: `F.Cu`, `In1.Cu`, `In2.Cu`, `B.Cu` | Known in source; physical stackup unproven |
| Fabrication archive | Local review zip named in `FABRICATION-OUTPUTS.md` | Generated review only |
| Gerbers | Copper, paste, silkscreen, mask, `Edge.Cuts`, and Gerber job file | Generated review only |
| Drill files | Split PTH/NPTH Excellon, metric, decimal zeros, drill maps, drill report | Generated review only |
| Assembly placement | POS CSV plus active-BOM/POS review derived from source BOM | Generated review only |
| KiCad gate | ERC=0, DRC=0, unconnected=0, schematic parity=0 per `CURRENT-VIOLATIONS.md` | Clean snapshot |
| Active BOM | `hardware/bom/R30-OLED-NONE-NONE.csv` | Source controlled |

## Board-House Questions To Close

Ask the prototype house or Seeed-style intake contact to answer these before
the ring annex becomes a PCB fab/assembly quote path.

| Constraint | Packet request | Closure needed |
|------------|----------------|----------------|
| Accepted file set | Confirm whether the Gerber, split Excellon drill, Gerber job, POS CSV, and BOM review files are sufficient, or list the exact extra files required. | Vendor says accepted, or outputs are regenerated to match their requested file set. |
| Layer naming/order | Confirm `F.Cu` / `In1.Cu` / `In2.Cu` / `B.Cu` map correctly to their four-layer intake. | Vendor confirms layer order or provides required naming convention. |
| Stackup selection | Provide default four-layer prototype stackup options, total thickness choices, copper weights, dielectric notes, and price impact. | BDFL chooses an option and records it in `STACKUP-VERIFY.md`; do not infer physical ring closure from vendor defaults. |
| Minimum trace/space | Confirm the submitted board clears their prototype minimum trace/space after CAM import. | Vendor CAM/DFM response has no unresolved trace/space issue. |
| Minimum finished drill and annular ring | Confirm smallest finished drill, via class, and annular-ring expectations for this board. | Vendor CAM/DFM response has no unresolved drill/via issue. |
| Solder mask web and expansion | Confirm whether the current mask settings are acceptable for the dense service, sensor, and regulator pockets. | Vendor accepts current mask output or requests specific KiCad setting changes. |
| Paste stencil assumptions | Confirm whether both paste layers should be retained, whether any apertures need reduction, and whether hand assembly changes the paste requirement. | Assembly path has explicit paste/stencil handling. |
| Surface finish | Recommend prototype finish for small service pads and optical-ring bring-up without exceeding the BOM intent. | Finish is chosen with cost noted; no durability claim is added without evidence. |
| Board thickness tolerance | State the thickness tolerance for each candidate stackup. | Tolerance is carried into shell/stackup verification before build release. |
| Small outline handling | Confirm whether the `43 x 18 mm` board needs tabs, rails, panelization, fiducials, tooling holes, or mouse-bites for fabrication/assembly. | Panelization or handling requirements are recorded and source changes are planned if needed. |
| Off-board service pads | Confirm the same-net USB service-pad and battery-service-pad approach is manufacturable and testable with a pogo/fixture path. | Vendor accepts the fixture assumption or returns specific pad/spacing/fixture changes. |
| Test points and qty-0 copper | Confirm that `Q1` non-BOM service jumper and qty-0 test/service pads are understood as copper/service features, not missing assembly lines. | Assembly notes/BOM markings are accepted or revised. |
| DNP/unplaced rows | Confirm `LED1`, off-board/mechanical rows, and service/test refs in the active-BOM/POS review are interpreted correctly. | No unresolved BOM/POS import rows remain in vendor review. |
| Antenna keep-out handling | Confirm no vendor-added copper, rails, clamps, or panel tabs enter the ESP32-C3 antenna keep-out without a source update. | Vendor handling plan preserves antenna keep-out or flags needed source changes. |
| Deliverable source return | Confirm any CAM/DFM modifications will be returned as editable notes/source suitable for CERN-OHL-S publication. | Return format is agreed before accepting manufacturer-side edits. |

## Still Not Proven By This Checklist

- Finger-to-surface height, optical focal distance, or the `43 x 18 mm` PCB pod
  comfort profile.
- Printed shell fit, board retention, service-pad access, battery harness
  removal, screw choice, or service-lid accessibility.
- RF behavior with the real shell, hand, battery, and service hardware.
- PAW3204 lens/emitter sourcing, assembly yield, click force, or battery
  service reliability.
- Real prototype cost. The `~$9` ring BOM target remains a repo target until
  vendor quotes and physical evidence exist.

## Closure Rule

The R30 annex blocker is closed only when this file records a specific vendor
or board-house response for the rows above, any requested KiCad output changes
are regenerated and re-hashed in `FABRICATION-OUTPUTS.md`, and
`STACKUP-VERIFY.md` still marks physical fit and stackup rows as unproven until
measured hardware evidence exists.
