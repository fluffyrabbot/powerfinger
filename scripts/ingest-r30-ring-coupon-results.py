#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Ingest real R30 ring coupon worksheet results into the checked-in ledger."""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RESULTS_FILE = REPO_ROOT / "hardware/ring/R30-OLED-NONE-NONE/COUPON-RESULTS.md"
ACCEPTED_RESULTS = {"pass", "fail", "partial", "blocked"}


@dataclass(frozen=True)
class Check:
    check_id: str
    artifact: str
    physical_check: str


CHECKS = [
    Check(
        "R30-SERVICE-ACCESS",
        "r30-oled-none-none-service-access-coupon.stl",
        "Service-access coupon lets the J1 service fixture/pogo path reach pads without binding, levering the board edge, or scraping the shell opening",
    ),
    Check(
        "R30-BOARD-RETENTION",
        "r30-oled-none-none-board-retention-coupon.stl",
        "`43 x 18 mm` board or blank slides onto rails, stops repeatably, and lifts out by hand",
    ),
    Check(
        "R30-LID-PAD-CLEARANCE",
        "r30-oled-none-none-lid-pad-coupon.stl",
        "Lid pads contact only intended board-edge keep-out zones and do not trap components",
    ),
    Check(
        "R30-BATTERY-HARNESS",
        "r30-oled-none-none-battery-harness-coupon.stl",
        "Off-board `J_BAT` harness and service loop clear the seam during protected-cell lift-out",
    ),
    Check(
        "R30-SERVICE-LID-REMOVAL",
        "r30-oled-none-none-service-lid-coupon.stl",
        "Lid/skirt/pry path reopens without destructive flex, tiny-tool dependence, or loose-hardware handling that blocks accessibility",
    ),
    Check(
        "R30-COMBINED-FIT-SHEET",
        "r30-oled-none-none-fit-coupons.stl",
        "Combined coupon print exposes no obvious generation-time geometry mismatch before individual physical checks",
    ),
]

CHECKS_BY_ID = {check.check_id: check for check in CHECKS}


def normalize(value: str) -> str:
    value = value.replace("`", "")
    return " ".join(value.strip().lower().split())


CHECK_ALIASES = {normalize(check.physical_check): check.check_id for check in CHECKS}


def die(message: str) -> None:
    raise SystemExit(f"error: {message}")


def split_markdown_row(line: str) -> list[str]:
    stripped = line.strip()
    if not stripped.startswith("|"):
        return []
    return [cell.strip() for cell in stripped.strip("|").split("|")]


def is_separator(cells: list[str]) -> bool:
    return bool(cells) and all(re.fullmatch(r":?-{3,}:?", cell.strip()) for cell in cells)


def parse_markdown_rows(text: str) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    header: list[str] | None = None

    for line in text.splitlines():
        cells = split_markdown_row(line)
        if len(cells) < 2:
            header = None
            continue
        if header is None:
            header = cells
            continue
        if is_separator(cells):
            continue
        padded = cells + [""] * max(0, len(header) - len(cells))
        rows.append(dict(zip(header, padded[: len(header)])))

    return rows


def get_column(row: dict[str, str], *names: str) -> str:
    by_normalized_name = {normalize(key): value for key, value in row.items()}
    for name in names:
        value = by_normalized_name.get(normalize(name))
        if value is not None:
            return value.strip()
    return ""


def rel_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(REPO_ROOT))
    except ValueError:
        return str(resolved)


def validate_table_cell(field: str, value: str) -> None:
    if "|" in value:
        die(f"{field} contains '|', which would break the markdown result table")


def extract_worksheet_updates(worksheet: Path) -> dict[str, dict[str, str]]:
    if not worksheet.is_file():
        die(f"worksheet not found: {worksheet}")

    updates: dict[str, dict[str, str]] = {}
    worksheet_ref = rel_path(worksheet)
    text = worksheet.read_text(encoding="utf-8")

    for row in parse_markdown_rows(text):
        physical_check = get_column(row, "Physical check")
        artifact = get_column(row, "Coupon artifact")
        result = get_column(row, "Result")

        if not physical_check:
            continue

        check_id = CHECK_ALIASES.get(normalize(physical_check))
        if check_id is None:
            if "r30-oled-none-none" in normalize(artifact):
                die(f"unknown R30 worksheet row: {physical_check}")
            continue

        if not result:
            continue

        normalized_result = normalize(result).replace(" ", "-")
        if normalized_result not in ACCEPTED_RESULTS:
            die(
                f"{check_id} has unsupported result '{result}'; use one of: "
                + ", ".join(sorted(ACCEPTED_RESULTS))
            )

        fixture = get_column(
            row,
            "Fixture / board / tool used",
            "Fixture / host / board used",
            "Fixture / board / tool used",
        )
        observation = get_column(row, "Observation or photo reference")
        if not fixture:
            die(f"{check_id} has result '{result}' but no fixture / board / tool used")
        if not observation:
            die(f"{check_id} has result '{result}' but no observation or photo reference")

        for field, value in {
            "Fixture / board / tool used": fixture,
            "Observation or photo reference": observation,
            "Source worksheet": worksheet_ref,
        }.items():
            validate_table_cell(field, value)

        if check_id in updates:
            die(f"duplicate worksheet row for {check_id}; split repeated checks into separate evidence refs")

        updates[check_id] = {
            "Result": normalized_result,
            "Fixture / board / tool used": fixture,
            "Observation or photo reference": observation,
            "Source worksheet": worksheet_ref,
        }

    return updates


def default_record(check: Check) -> dict[str, str]:
    return {
        "Check ID": check.check_id,
        "Coupon artifact": check.artifact,
        "Physical check": check.physical_check,
        "Result": "not-run",
        "Fixture / board / tool used": "",
        "Observation or photo reference": "",
        "Source worksheet": "",
    }


def read_existing_results(results_file: Path) -> dict[str, dict[str, str]]:
    records = {check.check_id: default_record(check) for check in CHECKS}
    if not results_file.exists():
        return records

    text = results_file.read_text(encoding="utf-8")
    for row in parse_markdown_rows(text):
        if not get_column(row, "Coupon artifact"):
            continue
        check_id = get_column(row, "Check ID").strip("`")
        if not check_id:
            continue
        if check_id not in CHECKS_BY_ID:
            die(f"unknown check ID in {rel_path(results_file)}: {check_id}")
        records[check_id].update(
            {
                "Result": get_column(row, "Result") or "not-run",
                "Fixture / board / tool used": get_column(row, "Fixture / board / tool used"),
                "Observation or photo reference": get_column(row, "Observation or photo reference"),
                "Source worksheet": get_column(row, "Source worksheet"),
            }
        )

    return records


def render_results_doc(records: dict[str, dict[str, str]]) -> str:
    lines = [
        "<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->",
        "# R30-OLED-NONE-NONE Physical Coupon Results",
        "",
        "This is the source-controlled ingress point for real R30 first physical coupon",
        "observations. It closes the handoff from generated local worksheets back to the",
        "checked-in ring evidence lane without treating generated STLs, previews, or",
        "OpenSCAD logs as measured results.",
        "",
        "## Ingest Command",
        "",
        "Fill only the R30 rows in a generated worksheet after physical checks run, then",
        "ingest them from the repository root:",
        "",
        "```sh",
        "scripts/ingest-r30-ring-coupon-results.py build/first-board-mechanical-packet/FIRST-SWEEP/PHYSICAL-CHECK-WORKSHEET.md",
        "```",
        "",
        "For the R30-only mechanical bundle, use:",
        "",
        "```sh",
        "scripts/ingest-r30-ring-coupon-results.py build/r30-oled-none-none-mechanical/PHYSICAL-CHECK-WORKSHEET.md",
        "```",
        "",
        "Accepted generated-worksheet `Result` values are `pass`, `fail`, `partial`,",
        "and `blocked`. Leave the worksheet result blank when a row has not been",
        "physically checked; this checked-in ledger keeps unmeasured rows as `not-run`.",
        "Any non-blank worksheet result must also include the fixture, board, tool, or",
        "coupon print used plus an observation or photo reference.",
        "",
        "## Current Source-Controlled Results",
        "",
        "| Check ID | Coupon artifact | Physical check | Result | Fixture / board / tool used | Observation or photo reference | Source worksheet |",
        "|---|---|---|---|---|---|---|",
    ]

    for check in CHECKS:
        record = records[check.check_id]
        values = [
            f"`{check.check_id}`",
            f"`{check.artifact}`",
            check.physical_check,
            record["Result"],
            record["Fixture / board / tool used"],
            record["Observation or photo reference"],
            record["Source worksheet"],
        ]
        for value in values:
            validate_table_cell("result table value", value)
        lines.append("| " + " | ".join(values) + " |")

    lines.extend(
        [
            "",
            "## Downstream Truth Surfaces",
            "",
            "| Check ID | Evidence surface | Closure rule |",
            "|---|---|---|",
            "| `R30-SERVICE-ACCESS` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real service fixture, board or board blank, and observation reference are ingested. |",
            "| `R30-BOARD-RETENTION` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real board or representative board blank is checked against the printed rails and stop lugs. |",
            "| `R30-LID-PAD-CLEARANCE` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until lid-pad contact is checked against the intended board-edge keep-out zones. |",
            "| `R30-BATTERY-HARNESS` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the off-board battery harness and service loop are checked during cell lift-out. |",
            "| `R30-SERVICE-LID-REMOVAL` | `STACKUP-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the intended lid, tool, and removal path are physically checked for reopenability and accessibility. |",
            "| `R30-COMBINED-FIT-SHEET` | `STACKUP-VERIFY.md`, `MANIFEST.md` | Use only as print-sanity context; do not close individual stackup rows from the combined coupon sheet unless the individual checks above are also observed. |",
            "",
            "Do not remove a missing-artifact row from `MANIFEST.md` or check off a physical",
            "proof row in `FIRST-BOARD-CHECKLIST.md` from generated artifacts alone.",
            "",
        ]
    )

    return "\n".join(lines)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Ingest completed R30 physical coupon worksheet rows into COUPON-RESULTS.md."
    )
    parser.add_argument(
        "worksheet",
        type=Path,
        help="Generated PHYSICAL-CHECK-WORKSHEET.md with real R30 result rows filled in.",
    )
    parser.add_argument(
        "--results-file",
        type=Path,
        default=DEFAULT_RESULTS_FILE,
        help="Checked-in R30 result ledger to update.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Validate and report worksheet rows without writing the result ledger.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    worksheet = args.worksheet if args.worksheet.is_absolute() else REPO_ROOT / args.worksheet
    results_file = args.results_file if args.results_file.is_absolute() else REPO_ROOT / args.results_file

    updates = extract_worksheet_updates(worksheet)
    if not updates:
        print("No completed R30 coupon result rows found; results file unchanged.")
        return 0

    records = read_existing_results(results_file)
    for check_id, update in updates.items():
        records[check_id].update(update)

    print(f"R30 coupon result rows ready: {len(updates)}")
    for check_id in sorted(updates):
        print(f"- {check_id}: {updates[check_id]['Result']}")

    if args.dry_run:
        print("Dry run only; result ledger not written.")
        return 0

    results_file.parent.mkdir(parents=True, exist_ok=True)
    results_file.write_text(render_results_doc(records), encoding="utf-8")
    print(f"Updated {rel_path(results_file)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
