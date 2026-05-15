#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# PowerFinger - Assemble the active first-board mechanical print/preview packet.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
default_output="$repo_root/build/first-board-mechanical-packet"

usage() {
    cat <<'EOF'
Usage:
  scripts/generate-first-board-mechanical-packet.sh [--first-sweep] [output-dir]

Regenerates a single local print/preview packet containing the current USB-HUB
and R30-OLED-NONE-NONE first-board mechanical coupon bundles.

With --first-sweep, also assembles FIRST-SWEEP/ with the four cheapest first
physical-check coupons and matching previews:
  USB-HUB host-fit coupon
  USB-HUB clamp-alignment gauge
  R30 service-access coupon
  R30 board-retention coupon

With no output directory, regenerates:
  build/first-board-mechanical-packet

The generated files are local print/preview artifacts only. They do not prove
USB-HUB host fit, adjacent-port clearance, connector strain, R30 off-board
service-pad access, board retention, off-board battery-service harness routing,
ring stackup, focal distance, comfort, click behavior, or RF behavior until
printed and checked against real hardware or representative fixtures.
EOF
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

first_sweep=0
args=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --first-sweep)
            first_sweep=1
            shift
            ;;
        --*)
            usage >&2
            exit 2
            ;;
        *)
            args+=("$1")
            shift
            ;;
    esac
done

if [[ "${#args[@]}" -gt 1 ]]; then
    usage >&2
    exit 2
fi

output_dir="${args[0]:-$default_output}"
case "$output_dir" in
    /*) ;;
    *) output_dir="$repo_root/$output_dir" ;;
esac

[[ "$output_dir" != "$repo_root" && "$output_dir" != "/" ]] || die "refusing to export into $output_dir"

usb_generator="$repo_root/scripts/generate-usb-hub-validation-coupons.sh"
r30_generator="$repo_root/scripts/generate-r30-ring-fit-coupons.sh"
[[ -x "$usb_generator" ]] || die "missing executable generator: scripts/generate-usb-hub-validation-coupons.sh"
[[ -x "$r30_generator" ]] || die "missing executable generator: scripts/generate-r30-ring-fit-coupons.sh"

if [[ "$output_dir" == "$default_output" ]]; then
    rm -rf "$output_dir"
elif [[ -d "$output_dir" ]] && find "$output_dir" -mindepth 1 -maxdepth 1 | grep -q .; then
    die "custom output directory is not empty: $output_dir"
fi

mkdir -p "$output_dir"

usb_output="$output_dir/USB-HUB"
r30_output="$output_dir/R30-OLED-NONE-NONE"

"$usb_generator" "$usb_output"
"$r30_generator" "$r30_output"

generated_at="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
manifest="$output_dir/PACKET-MANIFEST.md"
readme="$output_dir/README.md"
first_sweep_output="$output_dir/FIRST-SWEEP"

file_count() {
    find "$1" -type f | wc -l | tr -d '[:space:]'
}

file_sha256() {
    shasum -a 256 "$1" | awk '{ print $1 }'
}

write_bundle_summary_row() {
    local variant="$1"
    local bundle_dir="$2"
    local generator="$3"
    local target="$4"
    local coupon_manifest="$bundle_dir/COUPON-MANIFEST.md"
    local worksheet="$bundle_dir/PHYSICAL-CHECK-WORKSHEET.md"

    [[ -f "$coupon_manifest" ]] || die "missing coupon manifest: $coupon_manifest"
    [[ -f "$worksheet" ]] || die "missing physical-check worksheet: $worksheet"

    printf '| `%s` | `%s` | `%s` | `%s` | %s | `%s` | `%s` |\n' \
        "$variant" \
        "$generator" \
        "${bundle_dir#$output_dir/}" \
        "$target" \
        "$(file_count "$bundle_dir")" \
        "$(file_sha256 "$coupon_manifest")" \
        "$(file_sha256 "$worksheet")"
}

copy_first_sweep_file() {
    local rel_path="$1"
    local src="$output_dir/$rel_path"
    local dest="$first_sweep_output/$rel_path"

    [[ -f "$src" ]] || die "missing first-sweep source: $rel_path"
    mkdir -p "$(dirname "$dest")"
    cp -p "$src" "$dest"
}

write_file_index_row() {
    local root_dir="$1"
    local generated_file="$2"
    local rel_path="${generated_file#$root_dir/}"
    local bytes
    local sha256

    bytes="$(wc -c < "$generated_file" | tr -d '[:space:]')"
    sha256="$(file_sha256 "$generated_file")"
    printf '| `%s` | %s | `%s` |\n' "$rel_path" "$bytes" "$sha256"
}

assemble_first_sweep() {
    rm -rf "$first_sweep_output"
    mkdir -p "$first_sweep_output"

    while IFS= read -r rel_path; do
        copy_first_sweep_file "$rel_path"
    done <<'EOF'
USB-HUB/usb-hub-host-fit-coupon.stl
USB-HUB/usb-hub-host-fit-coupon.openscad.log
USB-HUB/previews/usb-hub-host-fit-coupon.png
USB-HUB/previews/usb-hub-host-fit-coupon.preview.openscad.log
USB-HUB/usb-hub-clamp-alignment-gauge.stl
USB-HUB/usb-hub-clamp-alignment-gauge.openscad.log
USB-HUB/previews/usb-hub-clamp-alignment-gauge.png
USB-HUB/previews/usb-hub-clamp-alignment-gauge.preview.openscad.log
R30-OLED-NONE-NONE/r30-oled-none-none-service-access-coupon.stl
R30-OLED-NONE-NONE/r30-oled-none-none-service-access-coupon.openscad.log
R30-OLED-NONE-NONE/previews/r30-oled-none-none-service-access-coupon.png
R30-OLED-NONE-NONE/previews/r30-oled-none-none-service-access-coupon.preview.openscad.log
R30-OLED-NONE-NONE/r30-oled-none-none-board-retention-coupon.stl
R30-OLED-NONE-NONE/r30-oled-none-none-board-retention-coupon.openscad.log
R30-OLED-NONE-NONE/previews/r30-oled-none-none-board-retention-coupon.png
R30-OLED-NONE-NONE/previews/r30-oled-none-none-board-retention-coupon.preview.openscad.log
EOF

    {
        printf '<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->\n'
        printf '# First-Sweep Physical Coupon Worksheet\n\n'
        printf 'Generated by `scripts/generate-first-board-mechanical-packet.sh --first-sweep` at `%s`.\n\n' "$generated_at"
        printf 'Use this worksheet only after printing the four first-sweep STLs and checking them against real hosts, plugs, boards, board blanks, tools, or fixtures. Leave result fields blank until physical checks are complete.\n\n'
        printf '| Coupon artifact | Physical check | Result | Fixture / host / board used | Observation or photo reference |\n'
        printf '|---|---|---|---|---|\n'
        printf '| `USB-HUB/usb-hub-host-fit-coupon.stl` | USB-A shoulder seats against real host-port faces without printed body interference |  |  |  |\n'
        printf '| `USB-HUB/usb-hub-host-fit-coupon.stl` | Wider body clears adjacent USB-A ports on real hosts |  |  |  |\n'
        printf '| `USB-HUB/usb-hub-clamp-alignment-gauge.stl` | `MH1` / `MH2` clamp holes align against a board or board blank |  |  |  |\n'
        printf '| `R30-OLED-NONE-NONE/r30-oled-none-none-service-access-coupon.stl` | Service-access coupon lets the J1 service fixture/pogo path reach pads without binding, levering the board edge, or scraping the shell opening |  |  |  |\n'
        printf '| `R30-OLED-NONE-NONE/r30-oled-none-none-board-retention-coupon.stl` | `43 x 18 mm` board or blank slides onto rails, stops repeatably, and lifts out by hand |  |  |  |\n'
        printf '\nTransfer only real observations back into the lane worksheets and packet docs; this generated first-sweep folder is still print/preview scaffolding.\n'
    } > "$first_sweep_output/PHYSICAL-CHECK-WORKSHEET.md"

    {
        printf '<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->\n'
        printf '# First-Sweep Print Packet\n\n'
        printf 'Generated at `%s` from the full first-board mechanical packet.\n\n' "$generated_at"
        printf 'This folder isolates the first physical sweep recommended for cheap, high-signal checks before printing larger or less-informative coupon sets. It includes four STLs plus matching preview PNGs and OpenSCAD logs.\n\n'
        printf '%s\n' '- Start with USB-HUB host-fit and clamp-alignment checks.'
        printf '%s\n' '- Start with R30 service-access and board-retention checks.'
        printf '\nDo not fill result fields from these files alone. Print, check, then copy observed evidence back into the lane-level worksheets and source packet docs.\n'
    } > "$first_sweep_output/README.md"

    first_sweep_manifest="$first_sweep_output/PACKET-MANIFEST.md"
    {
        printf '<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->\n'
        printf '# First-Sweep Print Packet Manifest\n\n'
        printf 'Generated by `scripts/generate-first-board-mechanical-packet.sh --first-sweep` at `%s`.\n\n' "$generated_at"
        printf 'Scope: four selected first-sweep STL coupons, matching previews, OpenSCAD logs, README, and a blank worksheet. No physical evidence is claimed here.\n\n'
        printf '| Path | Bytes | SHA-256 |\n'
        printf '|---|---:|---|\n'
        while IFS= read -r generated_file; do
            write_file_index_row "$first_sweep_output" "$generated_file"
        done < <(find "$first_sweep_output" -type f ! -path "$first_sweep_manifest" | LC_ALL=C sort)
    } > "$first_sweep_manifest"
}

if [[ "$first_sweep" -eq 1 ]]; then
    assemble_first_sweep
fi

{
    printf '<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->\n'
    printf '# First-Board Mechanical Print / Preview Packet\n\n'
    printf 'Generated by `scripts/generate-first-board-mechanical-packet.sh` at `%s`.\n\n' "$generated_at"
    printf 'Scope: assembled local STL, PNG preview, hash-manifest, OpenSCAD-log, README, and blank worksheet scaffolding for the active USB-HUB and R30-OLED-NONE-NONE first-board mechanical coupon lanes. No physical fit, strain, clearance, stackup, focal-distance, comfort, click, or RF measurements are claimed by this generated packet.\n\n'
    printf '| Variant | Generator | Bundle directory | Evidence target | Files | COUPON-MANIFEST SHA-256 | PHYSICAL-CHECK-WORKSHEET SHA-256 |\n'
    printf '|---|---|---|---|---:|---|---|\n'
    write_bundle_summary_row \
        "USB-HUB" \
        "$usb_output" \
        "scripts/generate-usb-hub-validation-coupons.sh" \
        "hardware/shared/USB-HUB/CONNECTOR-RETENTION-VERIFY.md and hardware/shared/USB-HUB/MANIFEST.md"
    write_bundle_summary_row \
        "R30-OLED-NONE-NONE" \
        "$r30_output" \
        "scripts/generate-r30-ring-fit-coupons.sh" \
        "hardware/ring/R30-OLED-NONE-NONE/STACKUP-VERIFY.md and hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md"
    if [[ "$first_sweep" -eq 1 ]]; then
        printf '\nFIRST-SWEEP/ is included as a selected print queue and blank worksheet for the first physical coupon checks. It is not evidence until physical observations are recorded.\n'
    fi
    printf '\n## Generated File Index\n\n'
    printf '| Path | Bytes | SHA-256 |\n'
    printf '|---|---:|---|\n'
    while IFS= read -r generated_file; do
        write_file_index_row "$output_dir" "$generated_file"
    done < <(find "$output_dir" -type f \
        ! -path "$manifest" \
        ! -path "$readme" \
        | LC_ALL=C sort)
} > "$manifest"

{
    printf '<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->\n'
    printf '# First-Board Local Mechanical Packet\n\n'
    printf 'Generated at `%s`.\n\n' "$generated_at"
    printf 'This directory assembles the current generated print/preview bundles for the active hub and ring first-board mechanical evidence lanes:\n\n'
    printf '%s\n' '- `USB-HUB/` comes from `scripts/generate-usb-hub-validation-coupons.sh` and targets direct-plug host fit, adjacent-port clearance, clamp alignment, service-hatch reach, and connector-retention observations.'
    printf '%s\n' '- `R30-OLED-NONE-NONE/` comes from `scripts/generate-r30-ring-fit-coupons.sh` and targets J1 service-pad fixture access, board retention, lid pads, off-board battery-service harness routing, service-lid handling, and combined fit-coupon observations.'
    if [[ "$first_sweep" -eq 1 ]]; then
        printf '%s\n' '- `FIRST-SWEEP/` isolates the four recommended first physical-check STLs plus matching previews, logs, manifest, README, and blank worksheet.'
    fi
    printf '\nUse the STLs for quick prints and `previews/*.png` for visual sanity checks before printing. Leave all worksheet result fields blank until physical coupons, boards, board blanks, plugs, tools, hosts, or representative fixtures have actually been checked.\n\n'
    printf 'Transfer only real observations back into the packet evidence files named in `PACKET-MANIFEST.md`.\n'
} > "$readme"

printf 'Generated first-board mechanical packet in %s\n' "$output_dir"
printf 'USB-HUB bundle: %s\n' "$usb_output"
printf 'R30-OLED-NONE-NONE bundle: %s\n' "$r30_output"
if [[ "$first_sweep" -eq 1 ]]; then
    printf 'First-sweep packet: %s\n' "$first_sweep_output"
fi
printf 'Packet manifest: %s\n' "$manifest"
printf 'Packet README: %s\n' "$readme"
