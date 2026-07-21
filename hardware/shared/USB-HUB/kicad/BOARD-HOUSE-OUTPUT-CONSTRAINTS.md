<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Board-House Output Constraints

This checklist turns the DRC-clean USB-HUB KiCad source and local
fabrication-output review into explicit prototype-house intake questions. It is
for the active hub PCB fab/assembly quote path. It is not release-to-build
sign-off and does not prove connector retention, adjacent-port clearance, host
insertion/removal behavior, enclosure fit, RF behavior, yield, or realized COGS.

Use this with `FABRICATION-OUTPUTS.md`, `CURRENT-VIOLATIONS.md`, the KiCad
source, and the active BOM. If a board house asks for different plot, drill,
POS, BOM, stackup, mask, paste, panelization, or source-return settings,
regenerate the local outputs and update this checklist before treating the
packet as answered.

## Current Local Output Basis

| Intake item | Current packet answer | Status |
|-------------|-----------------------|--------|
| Board variant | `USB-HUB` stepped direct-plug USB-A hub dongle | Known |
| Board outline | Stepped direct-plug outline with USB-A nose and wider `54 x 26 mm` module/service body | Known in source; host fit unproven |
| Layer count | 2 copper layers: `F.Cu`, `B.Cu` | Known in source; vendor stackup unchosen |
| Fabrication archive | Local review zip named in `FABRICATION-OUTPUTS.md`; exported by the Shenzhen/Seeed quote-packet script | Generated review only |
| Gerbers | Copper, paste, silkscreen, mask, `Edge.Cuts`, and Gerber job file | Generated review only |
| Drill files | Split PTH/NPTH Excellon, metric, decimal zeros, drill maps, drill report | Generated review only |
| Assembly placement | POS CSV plus active-BOM/POS review derived from source BOM | Generated review only |
| KiCad gate | ERC=0, DRC=0, unconnected=0, schematic parity=0 per `CURRENT-VIOLATIONS.md` | Clean snapshot |
| Active BOM | `hardware/bom/USB-HUB.csv` | Source controlled |
| Connector | `SOFNG USB-05` direct-plug USB-A male connector, LCSC `C112454` | Source controlled; retention unproven |

## Board-House Questions To Close

Ask the prototype house or Seeed-style intake contact to answer these as part
of the hub quote and DFM review.

| Constraint | Packet request | Closure needed |
|------------|----------------|----------------|
| Accepted file set | Confirm whether the generated Gerber, split Excellon drill, Gerber job, POS CSV, active-BOM/POS review, and source BOM are sufficient, or list the exact extra files required. | Vendor says accepted, or outputs are regenerated to match their requested file set. |
| Layer naming/order | Confirm `F.Cu` and `B.Cu` map correctly to their two-layer intake. | Vendor confirms layer order or provides required naming convention. |
| Stackup and thickness | Provide default two-layer prototype stackup options, finished board thickness choices, copper weights, and price impact for a direct-plug dongle. | BDFL chooses an option; do not infer host-fit or connector-retention closure from vendor defaults. |
| USB-A plug handling | Confirm the SOFNG USB-05 through-hole pins, shell tabs, and guide holes are manufacturable from the submitted drill/slot output. | Vendor CAM/DFM response has no unresolved connector hole, slot, plating, or shell-tab issue. |
| Minimum trace/space | Confirm the submitted board clears their prototype minimum trace/space after CAM import. | Vendor CAM/DFM response has no unresolved trace/space issue. |
| Minimum finished drill and annular ring | Confirm smallest finished drill, via class, oval-slot handling, and annular-ring expectations for this board. | Vendor CAM/DFM response has no unresolved drill/via/slot issue. |
| Solder mask web and expansion | Confirm current mask settings are acceptable around the USB connector, 0402 passives, ESP32-S3 module pads, and service row. | Vendor accepts current mask output or requests specific KiCad setting changes. |
| Paste stencil assumptions | Confirm whether both paste layers should be retained, whether any apertures need reduction, and whether hand assembly changes the paste requirement. | Assembly path has explicit paste/stencil handling. |
| Module assembly | Confirm the ESP32-S3-MINI-1-N8 land pattern, antenna keep-out, and module placement notes are acceptable for their assembly process. | Vendor accepts module handling or returns exact source changes needed. |
| No-BOM copper features | Confirm `C5`/`C6` DNI placeholders, `TP1`-`TP9` service pads, `MH1`/`MH2` clamp holes, and connector guide holes are understood as copper/mechanical features, not missing assembly lines. | No unresolved BOM/POS import rows remain in vendor review. |
| Panelization and handling | Confirm whether the stepped USB-A nose and wider rear body need tabs, rails, fiducials, tooling holes, or mouse-bites for fabrication/assembly. | Panelization or handling requirements are recorded and source changes are planned if needed. |
| Antenna keep-out handling | Confirm no vendor-added copper, rails, clamps, panel tabs, or fixture metal enter the ESP32-S3 antenna keep-out without a source update. | Vendor handling plan preserves antenna keep-out or flags needed source changes. |
| Connector retention DFM | Review whether the PCB shell tabs plus `MH1`/`MH2` enclosure clamp path look manufacturable before physical retention evidence exists. | Vendor gives DFM feedback; physical connector-retention proof remains open until measured. |
| Deliverable source return | Confirm any CAM/DFM modifications will be returned as editable notes/source suitable for CERN-OHL-S publication. | Return format is agreed before accepting manufacturer-side edits. |

## Still Not Proven By This Checklist

- USB-A shoulder seating, adjacent-port clearance, insertion/removal feel, or
  connector strain on real hosts.
- Printed enclosure fit, clamp alignment, service-hatch reach, service-pad
  access, or accessible disassembly.
- RF behavior with the real shell, hand position, host body, and connector
  retention hardware.
- Assembly yield, realized prototype cost, or manufacturer acceptance. The
  `~$5-6` hub BOM target remains a repo target until vendor quotes and
  physical evidence exist.

## Closure Rule

The hub board-house output-constraints blocker is closed only when this file
records a specific vendor or board-house response for the rows above, any
requested KiCad output changes are regenerated and re-hashed in
`FABRICATION-OUTPUTS.md`, and `FIRST-BOARD-CHECKLIST.md` still marks physical
host-fit, adjacent-port, serviceability, and connector-retention rows as
unproven until measured hardware evidence exists.
