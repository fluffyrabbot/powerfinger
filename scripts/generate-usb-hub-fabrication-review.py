#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate the USB-HUB local fabrication-review packet."""

from __future__ import annotations

import csv
import hashlib
import re
import shutil
import subprocess
import zipfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
VARIANT = "USB-HUB"
SLUG = "usb-hub"
ACTIVE_BOM = REPO_ROOT / "hardware/bom/USB-HUB.csv"
PCB = REPO_ROOT / "hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb"
OUT_ROOT = REPO_ROOT / "build-kicad/USB-HUB"
REVIEW_DIR = OUT_ROOT / "fabrication-review"
GERBER_DIR = REVIEW_DIR / "gerbers"
DRILL_DIR = REVIEW_DIR / "drill"
ASSEMBLY_DIR = REVIEW_DIR / "assembly"
POS_CSV = ASSEMBLY_DIR / "usb_hub-pos.csv"
REVIEW_CSV = ASSEMBLY_DIR / "usb_hub-active-bom-pos-review.csv"
REVIEW_MD = ASSEMBLY_DIR / "usb_hub-active-bom-pos-review.md"
ARCHIVE = OUT_ROOT / "usb-hub-fabrication-review.zip"
NORMALIZED_ISO = "1970-01-01T00:00:00+00:00"
NORMALIZED_LOCAL = "1970-01-01 00:00:00"
NORMALIZED_SHORT = "1970-01-01T00:00:00"
ZIP_TIMESTAMP = (1980, 1, 1, 0, 0, 0)

BOARD_TO_ACTIVE_REF = {
    "C1A": "C1",
    "C1B": "C1",
    "R1A": "R1",
    "R1B": "R1",
}

PLACEMENT_ONLY_CONTRACT = {
    "C5": "No-BOM mechanical DNI USB_D- shunt-cap placeholder; intentionally unconnected in this pass.",
    "C6": "No-BOM mechanical DNI USB_D+ shunt-cap placeholder; intentionally unconnected in this pass.",
    "TP1": "No-BOM VBUS service/test pad.",
    "TP2": "No-BOM 3V3 service/test pad.",
    "TP3": "No-BOM GND service/test pad.",
    "TP4": "No-BOM USB D- trace-access pad.",
    "TP5": "No-BOM USB D+ trace-access pad.",
    "TP6": "No-BOM EN service/test pad.",
    "TP7": "No-BOM BOOT service/test pad.",
    "TP8": "No-BOM UART TX service/test pad.",
    "TP9": "No-BOM UART RX service/test pad.",
}

NO_PLACEMENT_EXPECTED = {
    "ENCL1",
    "PCB1",
}


def rel(path: Path) -> str:
    return str(path.relative_to(REPO_ROOT))


def run(command: list[str]) -> None:
    subprocess.run(command, cwd=REPO_ROOT, check=True)


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


def generate_fab_outputs() -> None:
    if REVIEW_DIR.exists():
        shutil.rmtree(REVIEW_DIR)
    GERBER_DIR.mkdir(parents=True)
    DRILL_DIR.mkdir(parents=True)
    ASSEMBLY_DIR.mkdir(parents=True)

    run(
        [
            "kicad-cli",
            "pcb",
            "export",
            "gerbers",
            "--output",
            rel(GERBER_DIR),
            "--layers",
            "F.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts",
            rel(PCB),
        ]
    )
    run(
        [
            "kicad-cli",
            "pcb",
            "export",
            "drill",
            "--output",
            rel(DRILL_DIR),
            "--format",
            "excellon",
            "--excellon-units",
            "mm",
            "--excellon-zeros-format",
            "decimal",
            "--excellon-separate-th",
            "--generate-map",
            "--map-format",
            "gerberx2",
            "--generate-report",
            "--report-path",
            rel(DRILL_DIR / "usb_hub-drill-report.txt"),
            rel(PCB),
        ]
    )
    run(
        [
            "kicad-cli",
            "pcb",
            "export",
            "pos",
            "--output",
            rel(POS_CSV),
            "--format",
            "csv",
            "--units",
            "mm",
            "--side",
            "both",
            "--exclude-dnp",
            rel(PCB),
        ]
    )


def normalize_generated_timestamps() -> None:
    patterns = [
        (
            re.compile(r"%TF\.CreationDate,[^*]+\*%"),
            f"%TF.CreationDate,{NORMALIZED_ISO}*%",
        ),
        (
            re.compile(r"; #@! TF\.CreationDate,.*"),
            f"; #@! TF.CreationDate,{NORMALIZED_ISO}",
        ),
        (
            re.compile(r"G04 Created by KiCad \(PCBNEW ([^)]+)\) date [^*]+\*"),
            rf"G04 Created by KiCad (PCBNEW \1) date {NORMALIZED_LOCAL}*",
        ),
        (
            re.compile(r"; DRILL file KiCad ([^ ]+) date .*"),
            rf"; DRILL file KiCad \1 date {NORMALIZED_SHORT}",
        ),
        (
            re.compile(r"Created on .*"),
            f"Created on {NORMALIZED_SHORT}",
        ),
        (
            re.compile(r'"CreationDate": "[^"]+"'),
            f'"CreationDate": "{NORMALIZED_ISO}"',
        ),
    ]

    for path in REVIEW_DIR.rglob("*"):
        if not path.is_file():
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        normalized = text
        for pattern, replacement in patterns:
            normalized = pattern.sub(replacement, normalized)
        if normalized != text:
            path.write_text(normalized, encoding="utf-8")


def review_note(status: str, bom_ref: str, board_ref: str) -> str:
    if status == "matched":
        return "Exact active BOM ref has a KiCad placement."
    if status == "multi_ref_match":
        return f"Board ref intentionally maps to active BOM multi-quantity line {bom_ref}."
    if status == "placement_only_contract":
        return PLACEMENT_ONLY_CONTRACT.get(board_ref, "No-BOM board feature intentionally present in the PCB source.")
    if status == "not_populated_by_contract":
        return "Active BOM quantity is 0; no sourced assembly placement expected."
    if status == "no_pcb_placement_expected":
        return "Active BOM item is the PCB, enclosure, or another off-board/mechanical item."
    if status == "placement_only_no_active_bom":
        return "KiCad placement exists but active BOM has no matching populated line or explicit placement-only contract."
    if status == "bom_only_no_placement":
        return "Active BOM populated line has no KiCad placement or explicit no-placement classification."
    return ""


def generate_assembly_review() -> None:
    bom_rows = [row for row in read_csv(ACTIVE_BOM) if (row.get("Ref") or "").strip()]
    pos_rows = read_csv(POS_CSV)
    bom_by_ref = {row["Ref"].strip(): row for row in bom_rows}
    matched_bom_refs: set[str] = set()
    review_rows: list[dict[str, str]] = []
    status_counts: dict[str, int] = {}

    def add_count(status: str) -> None:
        status_counts[status] = status_counts.get(status, 0) + 1

    def row_from(
        board_ref: str,
        status: str,
        bom_ref: str,
        bom: dict[str, str] | None,
        pos: dict[str, str] | None,
    ) -> dict[str, str]:
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
            status = "not_populated_by_contract" if qty == 0 else ("multi_ref_match" if active_ref != board_ref else "matched")
        elif board_ref in PLACEMENT_ONLY_CONTRACT:
            status = "placement_only_contract"
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
        handle.write("# USB-HUB Active BOM / Placement Review\n\n")
        handle.write("Generated from `hardware/bom/USB-HUB.csv` and the KiCad POS export.\n")
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
        handle.write(f"- `{rel(REVIEW_CSV)}`\n")
        handle.write(f"- `{rel(REVIEW_MD)}`\n")

    print(f"Wrote {rel(REVIEW_CSV)}")
    print(f"Wrote {rel(REVIEW_MD)}")
    print("Status counts:")
    for status in sorted(status_counts):
        print(f"  {status}: {status_counts[status]}")


def create_archive() -> None:
    if ARCHIVE.exists():
        ARCHIVE.unlink()
    with zipfile.ZipFile(ARCHIVE, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(REVIEW_DIR.rglob("*")):
            if not path.is_file():
                continue
            info = zipfile.ZipInfo(str(path.relative_to(REVIEW_DIR)))
            info.date_time = ZIP_TIMESTAMP
            info.compress_type = zipfile.ZIP_DEFLATED
            with path.open("rb") as handle:
                archive.writestr(info, handle.read())


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    generate_fab_outputs()
    normalize_generated_timestamps()
    generate_assembly_review()
    create_archive()
    print(f"Wrote {rel(ARCHIVE)}")
    print(f"Archive SHA-256: {sha256(ARCHIVE)}")


if __name__ == "__main__":
    main()
