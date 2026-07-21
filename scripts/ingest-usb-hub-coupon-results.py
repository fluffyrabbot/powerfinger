#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Ingest real USB-HUB coupon worksheet results into the checked-in ledger."""

from __future__ import annotations

import argparse
import contextlib
import io
import re
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RESULTS_FILE = REPO_ROOT / "hardware/shared/USB-HUB/COUPON-RESULTS.md"
ACCEPTED_RESULTS = {"pass", "fail", "partial", "blocked"}


@dataclass(frozen=True)
class Check:
    check_id: str
    artifact: str
    physical_check: str


CHECKS = [
    Check(
        "HUB-USB-A-SHOULDER",
        "usb-hub-host-fit-coupon.stl",
        "USB-A shoulder seats against real host-port faces without printed body interference",
    ),
    Check(
        "HUB-ADJACENT-PORT",
        "usb-hub-host-fit-coupon.stl",
        "Wider body clears adjacent USB-A ports on real hosts",
    ),
    Check(
        "HUB-MH1-MH2-ALIGNMENT",
        "usb-hub-clamp-alignment-gauge.stl",
        "`MH1` / `MH2` clamp holes align against a board or board blank",
    ),
    Check(
        "HUB-SERVICE-HATCH-REACH",
        "usb-hub-service-hatch-reach-gauge.stl",
        "Probe and service opener reach the service row and hatch notch without pulling on the connector",
    ),
    Check(
        "HUB-CONNECTOR-LOAD-PATH",
        "usb-hub-validation-set.stl",
        "Clamp/enclosure path captures connector insertion/removal load instead of relying on solder joints alone",
    ),
]

CHECKS_BY_ID = {check.check_id: check for check in CHECKS}


def normalize(value: str) -> str:
    value = value.replace("`", "")
    return " ".join(value.strip().lower().split())


CHECK_ALIASES = {
    normalize("USB-A shoulder seats against real host-port faces without printed body interference"): "HUB-USB-A-SHOULDER",
    normalize("Wider body clears adjacent USB-A ports on real hosts"): "HUB-ADJACENT-PORT",
    normalize("`MH1` / `MH2` clamp holes align against a board or board blank"): "HUB-MH1-MH2-ALIGNMENT",
    normalize("Probe and spudger reach the service row and hatch notch without pulling on the connector"): "HUB-SERVICE-HATCH-REACH",
    normalize("Probe and non-marring opener reach the service row and hatch notch without pulling on the connector"): "HUB-SERVICE-HATCH-REACH",
    normalize("Probe and service opener reach the service row and hatch notch without pulling on the connector"): "HUB-SERVICE-HATCH-REACH",
    normalize("Assembled clamp/enclosure path shares connector insertion/removal load instead of relying on solder joints alone"): "HUB-CONNECTOR-LOAD-PATH",
    normalize("Clamp/enclosure path captures connector insertion/removal load instead of relying on solder joints alone"): "HUB-CONNECTOR-LOAD-PATH",
}


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
            if "usb-hub" in normalize(artifact):
                die(f"unknown USB-HUB worksheet row: {physical_check}")
            continue

        if not result:
            continue

        normalized_result = normalize(result).replace(" ", "-")
        if normalized_result not in ACCEPTED_RESULTS:
            die(
                f"{check_id} has unsupported result '{result}'; use one of: "
                + ", ".join(sorted(ACCEPTED_RESULTS))
            )

        fixture = get_column(row, "Fixture / host / board used")
        observation = get_column(row, "Observation or photo reference")
        if not fixture:
            die(f"{check_id} has result '{result}' but no fixture / host / board used")
        if not observation:
            die(f"{check_id} has result '{result}' but no observation or photo reference")

        for field, value in {
            "Fixture / host / board used": fixture,
            "Observation or photo reference": observation,
            "Source worksheet": worksheet_ref,
        }.items():
            validate_table_cell(field, value)

        if check_id in updates:
            die(f"duplicate worksheet row for {check_id}; split repeated checks into separate evidence refs")

        updates[check_id] = {
            "Result": normalized_result,
            "Fixture / host / board used": fixture,
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
        "Fixture / host / board used": "",
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
                "Fixture / host / board used": get_column(row, "Fixture / host / board used"),
                "Observation or photo reference": get_column(row, "Observation or photo reference"),
                "Source worksheet": get_column(row, "Source worksheet"),
            }
        )

    return records


def render_results_doc(records: dict[str, dict[str, str]]) -> str:
    lines = [
        "<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->",
        "# USB-HUB Physical Coupon Results",
        "",
        "This is the source-controlled ingress point for real USB-HUB first physical",
        "coupon observations. It closes the handoff from generated local worksheets back",
        "to the checked-in hub evidence lane without treating generated STLs, previews,",
        "or OpenSCAD logs as measured results.",
        "",
        "## Ingest Command",
        "",
        "Fill only the USB-HUB rows in a generated worksheet after physical checks run,",
        "then ingest them from the repository root:",
        "",
        "```sh",
        "scripts/ingest-usb-hub-coupon-results.py build/first-board-mechanical-packet/FIRST-SWEEP/PHYSICAL-CHECK-WORKSHEET.md",
        "```",
        "",
        "For the USB-HUB-only first-print packet, use:",
        "",
        "```sh",
        "scripts/ingest-usb-hub-coupon-results.py build/usb-hub-mechanical/FIRST-PRINT/PHYSICAL-CHECK-WORKSHEET.md",
        "```",
        "",
        "Accepted generated-worksheet `Result` values are `pass`, `fail`, `partial`, and",
        "`blocked`. Leave the worksheet result blank when a row has not been physically",
        "checked; this checked-in ledger keeps unmeasured rows as `not-run`. Any",
        "non-blank worksheet result must also include the fixture, host, board, or tool",
        "used plus an observation or photo reference.",
        "",
        "## Current Source-Controlled Results",
        "",
        "| Check ID | Coupon artifact | Physical check | Result | Fixture / host / board used | Observation or photo reference | Source worksheet |",
        "|---|---|---|---|---|---|---|",
    ]

    for check in CHECKS:
        record = records[check.check_id]
        values = [
            f"`{check.check_id}`",
            f"`{check.artifact}`",
            check.physical_check,
            record["Result"],
            record["Fixture / host / board used"],
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
            "| `HUB-USB-A-SHOULDER` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real host-port fixture and observation reference are ingested. |",
            "| `HUB-ADJACENT-PORT` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a real adjacent-port host or representative port fixture is checked. |",
            "| `HUB-MH1-MH2-ALIGNMENT` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until a board or board blank is checked against the printed gauge. |",
            "| `HUB-SERVICE-HATCH-REACH` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until the intended probe and opener are checked against a printed coupon. |",
            "| `HUB-CONNECTOR-LOAD-PATH` | `CONNECTOR-RETENTION-VERIFY.md`, `MANIFEST.md`, `FIRST-BOARD-CHECKLIST.md` | Keep non-green until connector insertion/removal load sharing is observed on the printed clamp/enclosure path. |",
            "",
            "Do not remove a missing-artifact row from `MANIFEST.md` or check off a physical",
            "proof row in `FIRST-BOARD-CHECKLIST.md` from generated artifacts alone.",
            "",
        ]
    )

    return "\n".join(lines)


def write_self_test_worksheet(path: Path, row: str) -> None:
    path.write_text(
        "\n".join(
            [
                "| Coupon artifact | Physical check | Result | Fixture / host / board used | Observation or photo reference |",
                "|---|---|---|---|---|",
                row,
                "",
            ]
        ),
        encoding="utf-8",
    )


def assert_contains(path: Path, expected: str) -> None:
    if expected not in path.read_text(encoding="utf-8"):
        die(f"self-test expected {rel_path(path)} to contain: {expected}")


def assert_not_exists(path: Path) -> None:
    if path.exists():
        die(f"self-test expected {rel_path(path)} to stay absent")


def expect_rejected(description: str, func) -> None:
    try:
        func()
    except SystemExit as exc:
        if str(exc).startswith("error: "):
            return
        raise
    die(f"self-test expected rejection for {description}")


def run_quietly(argv: list[str]) -> None:
    with contextlib.redirect_stdout(io.StringIO()):
        main(argv)


def self_test() -> int:
    with tempfile.TemporaryDirectory(prefix="powerfinger-usb-hub-coupon-") as tmp:
        tmp_root = Path(tmp)
        worksheet = tmp_root / "PHYSICAL-CHECK-WORKSHEET.md"
        results_file = tmp_root / "COUPON-RESULTS.md"

        write_self_test_worksheet(
            worksheet,
            "| `usb-hub-host-fit-coupon.stl` | USB-A shoulder seats against real host-port faces without printed body interference | pass | MacBook USB-A fixture A | photos/usb-a-shoulder-pass.jpg |",
        )
        run_quietly([str(worksheet), "--results-file", str(results_file), "--dry-run"])
        assert_not_exists(results_file)

        run_quietly([str(worksheet), "--results-file", str(results_file)])
        assert_contains(results_file, "`HUB-USB-A-SHOULDER`")
        assert_contains(results_file, "| pass | MacBook USB-A fixture A | photos/usb-a-shoulder-pass.jpg |")
        assert_contains(results_file, "`HUB-ADJACENT-PORT`")
        assert_contains(results_file, "| not-run |")

        missing_fixture = tmp_root / "MISSING-FIXTURE.md"
        write_self_test_worksheet(
            missing_fixture,
            "| `usb-hub-host-fit-coupon.stl` | USB-A shoulder seats against real host-port faces without printed body interference | fail |  | photos/missing-fixture.jpg |",
        )
        expect_rejected(
            "completed row without fixture",
            lambda: extract_worksheet_updates(missing_fixture),
        )

        bad_result = tmp_root / "BAD-RESULT.md"
        write_self_test_worksheet(
            bad_result,
            "| `usb-hub-host-fit-coupon.stl` | USB-A shoulder seats against real host-port faces without printed body interference | maybe | MacBook USB-A fixture A | photos/bad-result.jpg |",
        )
        expect_rejected(
            "unsupported result",
            lambda: extract_worksheet_updates(bad_result),
        )

    print("ok: USB-HUB coupon ingest self-test passed")
    return 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Ingest completed USB-HUB physical coupon worksheet rows into COUPON-RESULTS.md."
    )
    parser.add_argument(
        "worksheet",
        nargs="?",
        type=Path,
        help="Generated PHYSICAL-CHECK-WORKSHEET.md with real USB-HUB result rows filled in.",
    )
    parser.add_argument(
        "--results-file",
        type=Path,
        default=DEFAULT_RESULTS_FILE,
        help="Checked-in USB-HUB result ledger to update.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Validate and report worksheet rows without writing the result ledger.",
    )
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="Run deterministic ingest tests without touching the repository.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        return self_test()
    if args.worksheet is None:
        die("worksheet is required unless --self-test is used")
    worksheet = args.worksheet if args.worksheet.is_absolute() else REPO_ROOT / args.worksheet
    results_file = args.results_file if args.results_file.is_absolute() else REPO_ROOT / args.results_file

    updates = extract_worksheet_updates(worksheet)
    if not updates:
        print("No completed USB-HUB coupon result rows found; results file unchanged.")
        return 0

    records = read_existing_results(results_file)
    for check_id, update in updates.items():
        records[check_id].update(update)

    print(f"USB-HUB coupon result rows ready: {len(updates)}")
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
