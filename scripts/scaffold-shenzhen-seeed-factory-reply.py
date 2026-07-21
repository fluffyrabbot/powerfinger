#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Create a dated repo-local intake folder for one Shenzhen / Seeed reply."""

from __future__ import annotations

import argparse
import contextlib
import datetime as dt
import io
import re
import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_ROOT = REPO_ROOT / "docs/sensors-converge-2026/factory-replies"
SLUG_RE = re.compile(r"^[a-z0-9][a-z0-9-]*$")


def die(message: str) -> None:
    raise SystemExit(f"error: {message}")


def rel(path: Path) -> str:
    resolved = path.resolve()
    try:
        return str(resolved.relative_to(REPO_ROOT))
    except ValueError:
        return str(resolved)


def parse_date(value: str) -> str:
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", value):
        die("reply date must use YYYY-MM-DD")
    try:
        dt.date.fromisoformat(value)
    except ValueError:
        die(f"invalid reply date: {value}")
    return value


def validate_slug(value: str) -> str:
    if not SLUG_RE.fullmatch(value):
        die("factory slug must be lowercase letters, numbers, and hyphens")
    return value


def write_file(path: Path, text: str, dry_run: bool) -> None:
    if dry_run:
        print(f"would write {rel(path)}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def touch(path: Path, dry_run: bool) -> None:
    if dry_run:
        print(f"would create {rel(path)}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.touch(exist_ok=False)


def mkdir(path: Path, dry_run: bool) -> None:
    if dry_run:
        print(f"would create {rel(path)}/")
        return
    path.mkdir(parents=True, exist_ok=False)


def render_readme(reply_date: str, factory_slug: str, include_r30_annex: bool) -> str:
    r30_note = (
        "Included. Use the annex subdirectory only for DFM/pre-fab review evidence."
        if include_r30_annex
        else "Not included. Add it only by creating a new scaffold with `--include-r30-annex` after the factory explicitly accepts annex review."
    )
    return f"""<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen / Seeed Factory Reply Intake

Factory slug: `{factory_slug}`
Reply date: `{reply_date}`
Primary scope: `USB-HUB` PCB fabrication and assembly quote plus connector/enclosure DFM review.
Optional R30 annex: {r30_note}

This directory is the single repo-local evidence intake for one factory reply.
Keep raw incoming files here, then reference this path from
`docs/sensors-converge-2026/SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`.

## Status Boundary

- Factory quote files and messages are quote evidence, not build verification.
- Distributor/source claims from the factory stay quote-only until independently
  checked before editing `docs/VENDOR-VERIFICATION.md`.
- `docs/REFERENCE-MANUFACTURERS.md` stays placeholder until the response-capture
  update gate points to this directory and the BDFL approves the row change.
- Factory-side hardware changes need editable source suitable for the
  CERN-OHL-S 2.0 hardware posture before they can be accepted upstream.

## Evidence Index

| Path | Purpose | Quote-only or verified? | Notes |
|---|---|---|---|
| `incoming/` | Raw factory email exports, message captures, screenshots, or attachments exactly as received. | Quote-only until reviewed | Preserve original names when practical. |
| `quote-files/` | Quote sheets, invoices, BOM/CPL/POS import notes, DFM reports, and payment terms returned by the factory. | Quote-only until reviewed | Do not copy costs into realized COGS fields. |
| `USB-HUB-SUBSTITUTIONS.md` | Proposed hub substitutions transcribed into repo fields. | Quote-only until accepted or independently verified | Keep exact MPN, package, footprint, source, MOQ, and serviceability impact. |
| `USB-HUB-DFM-ASKS.md` | Hub DFM requests that are not simple part substitutions. | Quote-only until source returned or tested | Keep BDFL decisions blank until made. |
| `source-return/` | Editable source returned by the factory plus any CAM-only files that need rejection or exception review. | Unverified until inspected | Record posture in `SOURCE-RETURN-INDEX.md`. |
"""


def render_substitutions() -> str:
    return """<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# USB-HUB Proposed Substitutions

Copy only real factory proposals here. Mirror accepted rows into
`SHENZHEN-FACTORY-RESPONSE-CAPTURE.md` after preserving raw evidence in this
directory.

| Ref / subsystem | Active part | Proposed substitute | Manufacturer / MPN / package / source link | Footprint unchanged? | Enclosure or fixture changed? | Accessibility / serviceability impact | MOQ / unit quote / availability basis | Source files changed or requested | Quote-vs-verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|---|---|---|---|---|
|  |  |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
"""


def render_dfm_asks() -> str:
    return """<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# USB-HUB DFM Asks

Use this for factory requests that are not simple part substitutions. Keep
raw request wording visible and leave `BDFL decision` blank until decided.

| Request ID | Factory request | Affected files / features | Why factory wants it | Accessibility / serviceability impact | Source-return needed? | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|---|---|---|
| HUB-DFM-001 |  |  |  |  | Yes / no / unknown | requested / quoted / declined / needs more info | Unverified / source returned / tested |  |  |
"""


def render_source_return_index() -> str:
    return """<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Source-Return Index

Record every factory-returned source artifact here before accepting design
changes. Editable source is required for upstream hardware changes; CAM-only
returns are not enough unless the BDFL explicitly grants an exception.

| Returned path | Source area | Editable source acceptable? | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|
|  | KiCad schematic / KiCad PCB / OpenSCAD / fixture / BOM-POS-CPL / CAM-only / other | Yes / no / unknown | promised / returned / refused / not applicable | Unverified / inspected / merged / rejected |  |  |
"""


def render_r30_readme(reply_date: str, factory_slug: str) -> str:
    return f"""<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# R30-OLED-NONE-NONE DFM / Pre-Fab Annex Reply

Factory slug: `{factory_slug}`
Reply date: `{reply_date}`
Scope: optional `R30-OLED-NONE-NONE` DFM/pre-fab review only.

Do not treat anything in this annex as a ring PCB fabrication or assembly quote.
Reference this subdirectory from the R30 annex section of
`SHENZHEN-FACTORY-RESPONSE-CAPTURE.md` only if the factory explicitly accepted
the annex.
"""


def create_common_files(root: Path, dry_run: bool) -> None:
    for directory in ("incoming", "quote-files", "source-return"):
        mkdir(root / directory, dry_run)
        touch(root / directory / ".gitkeep", dry_run)
    write_file(root / "USB-HUB-SUBSTITUTIONS.md", render_substitutions(), dry_run)
    write_file(root / "USB-HUB-DFM-ASKS.md", render_dfm_asks(), dry_run)
    write_file(root / "SOURCE-RETURN-INDEX.md", render_source_return_index(), dry_run)


def create_r30_annex(root: Path, reply_date: str, factory_slug: str, dry_run: bool) -> None:
    annex = root / "R30-OLED-NONE-NONE-DFM-ANNEX"
    mkdir(annex, dry_run)
    write_file(annex / "README.md", render_r30_readme(reply_date, factory_slug), dry_run)
    for directory in ("incoming", "quote-files", "source-return"):
        mkdir(annex / directory, dry_run)
        touch(annex / directory / ".gitkeep", dry_run)


def assert_exists(path: Path) -> None:
    if not path.exists():
        die(f"self-test expected path to exist: {path}")


def assert_not_exists(path: Path) -> None:
    if path.exists():
        die(f"self-test expected path to stay absent: {path}")


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
    with tempfile.TemporaryDirectory(prefix="powerfinger-shenzhen-reply-") as tmp:
        output_root = Path(tmp) / "factory-replies"

        run_quietly([
            "2026-06-04",
            "--factory-slug",
            "seeed-fusion-propagate",
            "--output-root",
            str(output_root),
        ])
        reply_dir = output_root / "2026-06-04-seeed-fusion-propagate-usb-hub-reply"
        assert_exists(reply_dir / "README.md")
        assert_exists(reply_dir / "incoming/.gitkeep")
        assert_exists(reply_dir / "quote-files/.gitkeep")
        assert_exists(reply_dir / "source-return/.gitkeep")
        assert_exists(reply_dir / "SOURCE-RETURN-INDEX.md")
        assert_exists(reply_dir / "USB-HUB-SUBSTITUTIONS.md")
        assert_exists(reply_dir / "USB-HUB-DFM-ASKS.md")
        assert_not_exists(reply_dir / "R30-OLED-NONE-NONE-DFM-ANNEX")

        run_quietly([
            "2026-06-05",
            "--factory-slug",
            "seeed-fusion-propagate",
            "--output-root",
            str(output_root),
            "--include-r30-annex",
        ])
        annex_dir = output_root / "2026-06-05-seeed-fusion-propagate-usb-hub-reply/R30-OLED-NONE-NONE-DFM-ANNEX"
        assert_exists(annex_dir / "README.md")
        assert_exists(annex_dir / "incoming/.gitkeep")
        assert_exists(annex_dir / "quote-files/.gitkeep")
        assert_exists(annex_dir / "source-return/.gitkeep")

        dry_run_root = Path(tmp) / "dry-run-replies"
        run_quietly([
            "2026-06-06",
            "--factory-slug",
            "seeed-fusion-propagate",
            "--output-root",
            str(dry_run_root),
            "--dry-run",
        ])
        assert_not_exists(dry_run_root)

        expect_rejected("invalid date", lambda: parse_date("2026-02-30"))
        expect_rejected("invalid factory slug", lambda: validate_slug("Seeed Fusion"))

    print("ok: Shenzhen factory reply scaffold self-test passed")
    return 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Scaffold one dated Shenzhen / Seeed factory reply evidence folder."
    )
    parser.add_argument("reply_date", nargs="?", help="Factory reply date in YYYY-MM-DD format.")
    parser.add_argument(
        "--factory-slug",
        default="seeed-fusion-propagate",
        help="Lowercase slug used in the evidence directory name.",
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        default=DEFAULT_ROOT,
        help="Root directory for source-controlled factory replies.",
    )
    parser.add_argument(
        "--include-r30-annex",
        action="store_true",
        help="Also scaffold an optional R30 DFM/pre-fab annex subdirectory.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the paths that would be created without writing files.",
    )
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="Run deterministic scaffold tests without touching the repository.",
    )
    return parser.parse_args(argv)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    if args.self_test:
        return self_test()
    if not args.reply_date:
        die("reply date is required unless --self-test is used")
    reply_date = parse_date(args.reply_date)
    factory_slug = validate_slug(args.factory_slug)
    output_root = args.output_root if args.output_root.is_absolute() else REPO_ROOT / args.output_root
    reply_dir = output_root / f"{reply_date}-{factory_slug}-usb-hub-reply"

    if reply_dir.exists():
        die(f"reply intake directory already exists: {rel(reply_dir)}")

    if args.dry_run:
        print(f"would scaffold {rel(reply_dir)}")
    else:
        reply_dir.mkdir(parents=True)

    write_file(reply_dir / "README.md", render_readme(reply_date, factory_slug, args.include_r30_annex), args.dry_run)
    create_common_files(reply_dir, args.dry_run)
    if args.include_r30_annex:
        create_r30_annex(reply_dir, reply_date, factory_slug, args.dry_run)

    print(f"Factory reply intake: {rel(reply_dir)}")
    print("Reference that path from SHENZHEN-FACTORY-RESPONSE-CAPTURE.md before updating downstream docs.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
