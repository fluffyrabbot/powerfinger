#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate the R30 active-BOM-to-KiCad-placement assembly review."""

from __future__ import annotations

import csv
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
ACTIVE_BOM = REPO_ROOT / "hardware/bom/R30-OLED-NONE-NONE.csv"
PCB = REPO_ROOT / "hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb"
OUT_DIR = REPO_ROOT / "build-kicad/R30-OLED-NONE-NONE/fabrication-review/assembly"
POS_CSV = OUT_DIR / "r30_oled_none_none-pos.csv"
REVIEW_CSV = OUT_DIR / "r30_oled_none_none-active-bom-pos-review.csv"
REVIEW_MD = OUT_DIR / "r30_oled_none_none-active-bom-pos-review.md"

# Board-to-BOM aliases belong here only as temporary migration evidence. The
# current R30 active BOM is expected to use board-real references.
BOARD_TO_ACTIVE_REF: dict[str, str] = {}

NO_PLACEMENT_EXPECTED = {
    "ANT1",
    "BT1",
    "LENS1",
    "PAD1",
    "PCB1",
    "RIM1",
    "SHELL1",
}


def read_csv(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8-sig") as handle:
        return [dict(row) for row in csv.DictReader(handle)]


def write_csv(path: Path, rows: list[dict[str, str]], fieldnames: list[str]) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def parse_qty(value: str) -> int | None:
    value = (value or "").strip()
    if not value:
        return None
    try:
        return int(value)
    except ValueError:
        return None


def export_pos() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            "kicad-cli",
            "pcb",
            "export",
            "pos",
            "--output",
            str(POS_CSV.relative_to(REPO_ROOT)),
            "--format",
            "csv",
            "--units",
            "mm",
            "--side",
            "both",
            "--exclude-dnp",
            str(PCB.relative_to(REPO_ROOT)),
        ],
        cwd=REPO_ROOT,
        check=True,
    )


def review_note(status: str, bom_ref: str, board_ref: str) -> str:
    if status == "matched":
        return "Exact active BOM ref has a KiCad placement."
    if status == "alias":
        return f"Board ref intentionally maps to active BOM logical ref {bom_ref}."
    if status == "placement_for_qty_zero_contract":
        return "Placed footprint is a non-BOM service/copper contract item."
    if status == "placement_only_no_active_bom":
        return "KiCad placement exists but active BOM has no matching populated line."
    if status == "not_populated_by_contract":
        return "Active BOM quantity is 0; no placement expected."
    if status == "no_pcb_placement_expected":
        return "Active BOM item is off-board, mechanical, or integrated into another assembly."
    if status == "bom_only_no_placement":
        return "Active BOM populated line has no KiCad placement or explicit no-placement classification."
    return ""


def main() -> None:
    export_pos()

    bom_rows = [row for row in read_csv(ACTIVE_BOM) if (row.get("Ref") or "").strip()]
    pos_rows = read_csv(POS_CSV)
    bom_by_ref = {row["Ref"].strip(): row for row in bom_rows}
    matched_bom_refs: set[str] = set()
    review_rows: list[dict[str, str]] = []
    status_counts: dict[str, int] = {}

    def add_count(status: str) -> None:
        status_counts[status] = status_counts.get(status, 0) + 1

    def row_from(board_ref: str, status: str, bom_ref: str, bom: dict[str, str] | None, pos: dict[str, str] | None) -> dict[str, str]:
        bom = bom or {}
        pos = pos or {}
        add_count(status)
        return {
            "BoardRef": board_ref,
            "MatchStatus": status,
            "ActiveBomRef": bom_ref,
            "ActiveBomQty": bom.get("Qty", ""),
            "Description": bom.get("Description", ""),
            "ActiveBomValue": bom.get("Value", ""),
            "ActiveBomPackage": bom.get("Package", ""),
            "Manufacturer": bom.get("Manufacturer", ""),
            "MPN": bom.get("MPN", ""),
            "Supplier": bom.get("Supplier", ""),
            "SupplierPN": bom.get("Supplier PN", ""),
            "UnitCost": bom.get("Unit Cost (1-10)", ""),
            "PosX_mm": pos.get("PosX", ""),
            "PosY_mm": pos.get("PosY", ""),
            "Rotation_deg": pos.get("Rot", ""),
            "Side": pos.get("Side", ""),
            "PlacementValue": pos.get("Val", ""),
            "PlacementPackage": pos.get("Package", ""),
            "ReviewNote": review_note(status, bom_ref, board_ref),
        }

    for pos in pos_rows:
        board_ref = pos["Ref"].strip().strip('"')
        active_ref = BOARD_TO_ACTIVE_REF.get(board_ref, board_ref)
        bom = bom_by_ref.get(active_ref)
        if bom:
            matched_bom_refs.add(active_ref)
            qty = parse_qty(bom.get("Qty", ""))
            status = "placement_for_qty_zero_contract" if qty == 0 else ("alias" if active_ref != board_ref else "matched")
        else:
            status = "placement_only_no_active_bom"
        review_rows.append(row_from(board_ref, status, active_ref if bom else "", bom, pos))

    for bom in bom_rows:
        bom_ref = bom["Ref"].strip()
        if bom_ref in matched_bom_refs:
            continue
        qty = parse_qty(bom.get("Qty", ""))
        if qty == 0:
            status = "not_populated_by_contract"
        elif bom_ref in NO_PLACEMENT_EXPECTED:
            status = "no_pcb_placement_expected"
        else:
            status = "bom_only_no_placement"
        review_rows.append(row_from("", status, bom_ref, bom, None))

    fieldnames = [
        "BoardRef",
        "MatchStatus",
        "ActiveBomRef",
        "ActiveBomQty",
        "Description",
        "ActiveBomValue",
        "ActiveBomPackage",
        "Manufacturer",
        "MPN",
        "Supplier",
        "SupplierPN",
        "UnitCost",
        "PosX_mm",
        "PosY_mm",
        "Rotation_deg",
        "Side",
        "PlacementValue",
        "PlacementPackage",
        "ReviewNote",
    ]
    write_csv(REVIEW_CSV, review_rows, fieldnames)

    flagged_statuses = {
        "placement_only_no_active_bom",
        "bom_only_no_placement",
    }
    flagged_rows = [row for row in review_rows if row["MatchStatus"] in flagged_statuses]

    with REVIEW_MD.open("w", encoding="utf-8") as handle:
        handle.write("# R30-OLED-NONE-NONE Active BOM / Placement Review\n\n")
        handle.write("Generated from `hardware/bom/R30-OLED-NONE-NONE.csv` and the KiCad POS export.\n")
        handle.write("The KiCad schematic BOM export is intentionally not used.\n\n")
        handle.write("## Summary\n\n")
        handle.write(f"- KiCad placement refs reviewed: {len(pos_rows)}\n")
        handle.write(f"- Active BOM refs reviewed: {len(bom_rows)}\n")
        for status in sorted(status_counts):
            handle.write(f"- `{status}`: {status_counts[status]}\n")
        handle.write("\n## Open Review Rows\n\n")
        if flagged_rows:
            handle.write("| Board Ref | Active BOM Ref | Status | Note |\n")
            handle.write("|---|---|---|---|\n")
            for row in flagged_rows:
                handle.write(
                    f"| `{row['BoardRef']}` | `{row['ActiveBomRef']}` | `{row['MatchStatus']}` | {row['ReviewNote']} |\n"
                )
        else:
            handle.write("No open review rows.\n")
        handle.write("\n## Outputs\n\n")
        handle.write(f"- `{REVIEW_CSV.relative_to(REPO_ROOT)}`\n")
        handle.write(f"- `{REVIEW_MD.relative_to(REPO_ROOT)}`\n")

    print(f"Wrote {REVIEW_CSV.relative_to(REPO_ROOT)}")
    print(f"Wrote {REVIEW_MD.relative_to(REPO_ROOT)}")
    print("Status counts:")
    for status in sorted(status_counts):
        print(f"  {status}: {status_counts[status]}")


if __name__ == "__main__":
    main()
