# SPDX-License-Identifier: MIT
"""Trial driver: populate R30 power_and_charge sub-sheet via kicad-sch-api.

This script is the trial harness for validating that kicad-sch-api can
populate one R30 sub-sheet end-to-end with stock KiCad symbols and footprints
while preserving existing hierarchical labels and locking reference designators
to the BOM CSV verbatim.

Run via: uv tool run --from kicad-sch-api python3 trial-scripts/populate_power_and_charge.py
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import kicad_sch_api as ksa

SHEET = (
    "hardware/ring/R30-OLED-NONE-NONE/kicad/sheets/power_and_charge.kicad_sch"
)

# Symbol/footprint plan locked from probing /Applications/KiCad/.../symbols.
# Pin-compatible stock symbols are reused with overridden Value+MPN fields
# rather than authoring custom .kicad_sym files. See trial report for rationale.

# layout grid: A4 portrait. Existing hierarchical labels live on the right
# side at x=187.96 (VREG_3V3, VBAT_SENSE, NTC_SENSE) and on the left at
# x=25.4 (CHARGE_EN, VBUS_5V). Place charge-path components in the middle
# band, x=60..170, y=80..200.

PLAN = [
    # (ref, lib_id, footprint, value, position, rotation, props)
    ("Q1", "Device:Q_PMOS", "Package_TO_SOT_SMD:SOT-23",
     "SI2301", (80, 90), 0,
     {"mfg_part_num": "SI2301CDS", "manufacturer": "—", "lcsc": ""}),
    ("R4", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (70, 80), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("Q2", "Device:Q_NMOS", "Package_TO_SOT_SMD:SOT-23",
     "2N7002", (60, 100), 0,
     {"mfg_part_num": "2N7002", "manufacturer": "—", "lcsc": ""}),
    ("R6", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (50, 110), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("U3", "Battery_Management:MCP73831-2-OT", "Package_TO_SOT_SMD:SOT-23-5",
     "TP4054", (110, 90), 0,
     {"mfg_part_num": "TP4054", "manufacturer": "—", "lcsc": "C32574"}),
    ("R1", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "20k", (125, 105), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R11", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (130, 80), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("U4", "Regulator_Linear:AP2112K-3.3", "Package_TO_SOT_SMD:SOT-23-5",
     "RT9080-33GJ5", (160, 90), 0,
     {"mfg_part_num": "RT9080-33GJ5", "manufacturer": "Richtek",
      "lcsc": "C882092"}),
    ("C1", "Device:C", "Capacitor_SMD:C_0402_1005Metric",
     "100nF", (105, 110), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    # NOTE: BOM CSV lists C1 with Qty=2 (two 100nF instances sharing a
    # single ref-prefix). KiCad's ref-format validator rejects "C1_alt", so
    # the second instance is named C3 (next free C-prefix; C2 is the 10uF
    # bulk). Flagged in trial report.
    ("C3", "Device:C", "Capacitor_SMD:C_0402_1005Metric",
     "100nF", (170, 110), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("C2", "Device:C", "Capacitor_SMD:C_0402_1005Metric",
     "10uF", (150, 110), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R7", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (140, 130), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R8", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (140, 145), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R9", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "220k", (60, 130), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R10", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "100k", (60, 145), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("NTC1", "Device:Thermistor_NTC", "Resistor_SMD:R_0402_1005Metric",
     "10k_B3950", (170, 145), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R3", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "10k", (170, 160), 0,
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default=str(Path(__file__).resolve().parent.parent),
                    help="repository root")
    ap.add_argument("--dry-run", action="store_true",
                    help="print plan and exit without writing")
    args = ap.parse_args()

    sheet_path = Path(args.repo) / SHEET
    if not sheet_path.exists():
        print(f"FATAL: sheet not found at {sheet_path}", file=sys.stderr)
        return 2

    sch = ksa.load_schematic(str(sheet_path))
    print(f"loaded {sheet_path}")
    print(f"  existing components: {len(list(sch.components.all()))}")
    print(f"  existing hierarchical labels: {len(sch.hierarchical_labels)}")

    if args.dry_run:
        for ref, lib_id, fp, val, pos, rot, props in PLAN:
            print(f"  would add {ref:8s} {lib_id:40s} {fp:50s} value={val}")
        return 0

    added: list[str] = []
    for ref, lib_id, fp, val, pos, rot, props in PLAN:
        try:
            comp = sch.components.add(
                lib_id=lib_id,
                reference=ref,
                value=val,
                position=pos,
                footprint=fp,
                rotation=rot,
                **props,
            )
            added.append(ref)
            print(f"  added {ref:8s} ({lib_id})")
        except Exception as exc:
            print(f"  FAIL  {ref}: {exc!r}", file=sys.stderr)
            return 3

    sch.save()
    print(f"saved. added {len(added)}/{len(PLAN)} components: {added}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
