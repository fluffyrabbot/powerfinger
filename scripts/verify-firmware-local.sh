#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# PowerFinger — Local firmware verification helper
#
# Preferred local verification path for the active firmware lane.
# - Runs host-side unit tests unless --firmware-only is set
# - Builds selected ESP-IDF firmware projects when idf.py is available

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_root="${POWERFINGER_IDF_BUILD_ROOT:-$repo_root/build-idf}"
idf_version="${POWERFINGER_IDF_VERSION:-v5.2.2}"
idf_root="${POWERFINGER_IDF_ROOT:-$HOME/.powerfinger-sdk}"
idf_setup_script="$repo_root/scripts/setup-esp-idf-local.sh"

run_host_tests=true
run_firmware=true
run_kicad_checks=false
kicad_strict=false
use_all_projects=false
ring_profile=""

declare -a requested_projects=()
declare -a all_projects=(ring pen puck hub)
declare -a default_projects=(ring hub)
declare -a kicad_packets=(
    "R30-OLED-NONE-NONE|hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch|hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb"
    "USB-HUB|hardware/shared/USB-HUB/kicad/usb_hub.kicad_sch|hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb"
)

usage() {
    cat <<'EOF'
Usage:
  scripts/verify-firmware-local.sh [options] [ring|pen|puck|hub ...]

Options:
  --all               Build every ESP-IDF firmware project
  --firmware-only     Skip host-side unit tests
  --host-tests-only   Run only host-side unit tests
  --with-kicad        Also run kicad-cli ERC/DRC on the active hardware packets
  --kicad-only        Run kicad-cli ERC/DRC only (skips host tests + firmware)
  --kicad-strict      Exit non-zero when kicad-cli reports any violation
                      (default: report counts and continue, since the active
                      packets currently have a known-red baseline tracked in
                      kicad/CURRENT-VIOLATIONS.md)
  --ring-profile NAME Build the ring with a board profile fragment.
                      Supported: r30-oled-none-none
  -h, --help          Show this help text

Defaults:
  - Host-side tests run first
  - Firmware verification builds the active lane: ring + hub
  - ESP-IDF build outputs go under build-idf/<project>/
  - Ring profile builds write under build-idf/<profile>/
  - kicad-cli checks are off by default (opt in with --with-kicad)
  - kicad-cli reports go under build-kicad/<packet>/
  - If idf.py is not already in PATH, the script will try a repo-pinned
    local ESP-IDF install under $HOME/.powerfinger-sdk/

Examples:
  scripts/verify-firmware-local.sh
  scripts/verify-firmware-local.sh hub
  scripts/verify-firmware-local.sh --all
  scripts/verify-firmware-local.sh --host-tests-only
  scripts/verify-firmware-local.sh --ring-profile r30-oled-none-none
  scripts/verify-firmware-local.sh --with-kicad
  scripts/verify-firmware-local.sh --kicad-only
  scripts/verify-firmware-local.sh --kicad-only --kicad-strict
EOF
}

append_project() {
    local project="$1"
    local existing
    for existing in "${requested_projects[@]}"; do
        if [[ "$existing" == "$project" ]]; then
            return
        fi
    done
    requested_projects+=("$project")
}

parse_target() {
    local defaults_file="$1"
    sed -n 's/^CONFIG_IDF_TARGET="\([^"]*\)"$/\1/p' "$defaults_file" | head -n 1
}

project_was_requested() {
    local needle="$1" project
    for project in "${requested_projects[@]}"; do
        if [[ "$project" == "$needle" ]]; then
            return 0
        fi
    done
    return 1
}

normalize_idf_version() {
    local raw="$1"

    case "$raw" in
        ESP-IDF\ v*)
            printf 'v%s\n' "${raw#ESP-IDF v}"
            ;;
        v[0-9]*)
            printf '%s\n' "$raw"
            ;;
        [0-9]*)
            printf 'v%s\n' "$raw"
            ;;
        *)
            return 1
            ;;
    esac
}

detect_active_idf_version() {
    local raw_version

    if ! command -v idf.py >/dev/null 2>&1; then
        return 1
    fi

    raw_version="$(idf.py --version 2>/dev/null | head -n 1 || true)"
    if [[ -z "$raw_version" ]]; then
        return 1
    fi

    normalize_idf_version "$raw_version"
}

activate_local_idf() {
    local export_snippet active_version

    active_version="$(detect_active_idf_version || true)"
    if [[ "$active_version" == "$idf_version" ]]; then
        return 0
    fi
    if [[ ! -x "$idf_setup_script" ]]; then
        return 1
    fi

    export_snippet="$("$idf_setup_script" --export 2>/dev/null || true)"
    if [[ -z "$export_snippet" ]]; then
        return 1
    fi

    # setup-esp-idf-local.sh emits trusted shell exports for the repo-pinned
    # local toolchain path. This keeps the shared verifier usable on a fresh
    # machine without forcing contributors to manage their shell profile first.
    eval "$export_snippet"

    active_version="$(detect_active_idf_version || true)"
    [[ "$active_version" == "$idf_version" ]]
}

require_idf() {
    local active_version=""

    if activate_local_idf; then
        return
    fi

    active_version="$(detect_active_idf_version || true)"

    cat >&2 <<EOF
error: pinned ESP-IDF ${idf_version} is not active in this shell.

Pinned active-lane baseline: ESP-IDF ${idf_version}
Expected local install root: ${idf_root}
EOF

    if [[ -n "$active_version" ]]; then
        cat >&2 <<EOF
Detected idf.py version on PATH: ${active_version}

The shared verifier only trusts the pinned baseline. Install or reactivate the
repo-pinned toolchain before running firmware builds.
EOF
    fi

    cat >&2 <<EOF

To install the repo-pinned local toolchain:
  ${idf_setup_script}

To activate that toolchain in your current shell:
  eval "\$(${idf_setup_script} --export)"

If you already manage ESP-IDF yourself, export it before running firmware builds:
  . \$IDF_PATH/export.sh

If you only want the host-side unit tests, rerun with:
  scripts/verify-firmware-local.sh --host-tests-only
EOF
    exit 1
}

run_host_tests_step() {
    echo "==> Running host-side unit tests"
    cmake -S "$repo_root/firmware/test" -B "$repo_root/build-test"
    cmake --build "$repo_root/build-test"
    ctest --test-dir "$repo_root/build-test" --output-on-failure
}

run_kicad_checks_step() {
    if ! command -v kicad-cli >/dev/null 2>&1; then
        echo "warn: kicad-cli not found on PATH; skipping ERC/DRC checks" >&2
        echo "      install with: brew install --cask kicad" >&2
        if [[ "$kicad_strict" == true ]]; then
            echo "error: --kicad-strict was requested but kicad-cli is not available" >&2
            exit 1
        fi
        return 0
    fi

    local kicad_root="$repo_root/build-kicad"
    mkdir -p "$kicad_root"

    local total_violations=0
    local total_unconnected=0
    local total_parity=0

    echo "==> Running kicad-cli ERC + DRC ($(kicad-cli version 2>/dev/null | head -1))"

    local entry packet sch_path pcb_path packet_dir
    local sch_output drc_output sch_violations drc_violations drc_unconnected drc_parity

    for entry in "${kicad_packets[@]}"; do
        IFS='|' read -r packet sch_path pcb_path <<<"$entry"
        packet_dir="$kicad_root/$packet"
        mkdir -p "$packet_dir"

        if [[ ! -f "$repo_root/$sch_path" ]]; then
            echo "  $packet: missing schematic at $sch_path; skipping" >&2
            continue
        fi
        if [[ ! -f "$repo_root/$pcb_path" ]]; then
            echo "  $packet: missing PCB at $pcb_path; skipping" >&2
            continue
        fi

        sch_output="$packet_dir/erc.txt"
        drc_output="$packet_dir/drc.txt"

        local sch_summary sch_status
        if sch_summary="$(
            kicad-cli sch erc \
                --severity-all \
                -o "$sch_output" \
                "$repo_root/$sch_path" 2>&1
        )"; then
            sch_status=0
        else
            sch_status=$?
            echo "error: kicad-cli ERC failed for $packet (exit $sch_status)" >&2
            printf '%s\n' "$sch_summary" >&2
            printf "       report path: %s\n" "$sch_output" >&2
            exit "$sch_status"
        fi

        sch_violations="$(printf '%s\n' "$sch_summary" | sed -n 's/^Found \([0-9][0-9]*\) violations$/\1/p' | head -1)"
        if [[ -z "$sch_violations" ]]; then
            echo "error: could not parse kicad-cli ERC summary for $packet" >&2
            printf '%s\n' "$sch_summary" >&2
            printf "       report path: %s\n" "$sch_output" >&2
            exit 1
        fi

        local drc_summary drc_status
        if drc_summary="$(
            kicad-cli pcb drc \
                --severity-all \
                --schematic-parity \
                -o "$drc_output" \
                "$repo_root/$pcb_path" 2>&1
        )"; then
            drc_status=0
        else
            drc_status=$?
            echo "error: kicad-cli DRC failed for $packet (exit $drc_status)" >&2
            printf '%s\n' "$drc_summary" >&2
            printf "       report path: %s\n" "$drc_output" >&2
            exit "$drc_status"
        fi

        drc_violations="$(printf '%s\n' "$drc_summary" | sed -n 's/^Found \([0-9][0-9]*\) violations$/\1/p' | head -1)"
        drc_unconnected="$(printf '%s\n' "$drc_summary" | sed -n 's/^Found \([0-9][0-9]*\) unconnected items$/\1/p' | head -1)"
        drc_parity="$(printf '%s\n' "$drc_summary" | sed -n 's/^Found \([0-9][0-9]*\) schematic parity issues$/\1/p' | head -1)"
        if [[ -z "$drc_violations" || -z "$drc_unconnected" || -z "$drc_parity" ]]; then
            echo "error: could not parse kicad-cli DRC summary for $packet" >&2
            printf '%s\n' "$drc_summary" >&2
            printf "       report path: %s\n" "$drc_output" >&2
            exit 1
        fi

        printf "  %-22s ERC=%s  DRC=%s  unconnected=%s  parity=%s\n" \
            "$packet" "$sch_violations" "$drc_violations" "$drc_unconnected" "$drc_parity"
        printf "                          %s\n" "$sch_output"
        printf "                          %s\n" "$drc_output"

        total_violations=$((total_violations + sch_violations + drc_violations))
        total_unconnected=$((total_unconnected + drc_unconnected))
        total_parity=$((total_parity + drc_parity))
    done

    local total=$((total_violations + total_unconnected + total_parity))
    printf "  %-22s total=%s (violations=%s, unconnected=%s, parity=%s)\n" \
        "SUMMARY" "$total" "$total_violations" "$total_unconnected" "$total_parity"
    echo "  Track changes against each packet's kicad/CURRENT-VIOLATIONS.md."

    if [[ "$kicad_strict" == true && "$total" -gt 0 ]]; then
        echo "error: --kicad-strict requested but $total kicad-cli violations remain" >&2
        exit 1
    fi
}

ring_profile_defaults_file() {
    local profile="$1"
    case "$profile" in
        r30-oled-none-none)
            printf '%s\n' "$repo_root/firmware/ring/sdkconfig.defaults.r30_oled_none_none"
            ;;
        *)
            echo "error: unsupported ring profile '$profile'" >&2
            echo "supported ring profiles: r30-oled-none-none" >&2
            exit 1
            ;;
    esac
}

ring_profile_build_dir() {
    local profile="$1"
    case "$profile" in
        r30-oled-none-none)
            printf '%s\n' "$build_root/r30-oled-none-none"
            ;;
        *)
            echo "error: unsupported ring profile '$profile'" >&2
            exit 1
            ;;
    esac
}

require_sdkconfig_value() {
    local sdkconfig_path="$1"
    local key="$2"
    local expected="$3"
    local actual

    actual="$(sed -n "s/^${key}=\(.*\)$/\1/p" "$sdkconfig_path" | head -n 1)"
    if [[ "$actual" != "$expected" ]]; then
        echo "error: $sdkconfig_path has $key=${actual:-<unset>}, expected $expected" >&2
        exit 1
    fi
}

verify_r30_oled_profile_sdkconfig() {
    local sdkconfig_path="$1"

    if [[ ! -f "$sdkconfig_path" ]]; then
        echo "error: expected generated sdkconfig at $sdkconfig_path" >&2
        exit 1
    fi

    echo "==> Checking R30-OLED resolved sdkconfig contract"
    require_sdkconfig_value "$sdkconfig_path" CONFIG_SENSOR_PAW3204 y
    require_sdkconfig_value "$sdkconfig_path" CONFIG_CLICK_SNAP_DOME y
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_SENSOR_SCLK_PIN 4
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_SENSOR_SDIO_PIN 5
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_DOME_PIN 8
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_WAKE_GPIO_MASK 256
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_VBAT_ADC_CHANNEL 0
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_NTC_ADC_CHANNEL 1
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_VBUS_DETECT_PIN 3
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_CHARGE_ENABLE_PIN 10
    require_sdkconfig_value "$sdkconfig_path" CONFIG_POWERFINGER_HALL_POWER_PIN -1
}

verify_ring_profile_sdkconfig() {
    local profile="$1"
    local sdkconfig_path="$2"

    case "$profile" in
        r30-oled-none-none)
            verify_r30_oled_profile_sdkconfig "$sdkconfig_path"
            ;;
        *)
            echo "error: unsupported ring profile '$profile'" >&2
            exit 1
            ;;
    esac
}

build_ring_profile_step() {
    local profile="$1"
    local project_dir="$repo_root/firmware/ring"
    local defaults_file="$project_dir/sdkconfig.defaults"
    local profile_defaults_file build_dir sdkconfig_path target

    profile_defaults_file="$(ring_profile_defaults_file "$profile")"
    build_dir="$(ring_profile_build_dir "$profile")"
    sdkconfig_path="$build_dir/sdkconfig"

    if [[ ! -f "$profile_defaults_file" ]]; then
        echo "error: missing $profile_defaults_file" >&2
        exit 1
    fi

    target="$(parse_target "$defaults_file")"
    if [[ -z "$target" ]]; then
        echo "error: could not determine CONFIG_IDF_TARGET from $defaults_file" >&2
        exit 1
    fi

    mkdir -p "$build_dir"
    echo "==> Configuring ring profile $profile (target: $target)"
    IDF_TARGET="$target" idf.py \
        -C "$project_dir" \
        -B "$build_dir" \
        -DSDKCONFIG="$sdkconfig_path" \
        -DSDKCONFIG_DEFAULTS="$defaults_file;$profile_defaults_file" \
        reconfigure

    verify_ring_profile_sdkconfig "$profile" "$sdkconfig_path"

    echo "==> Building ring profile $profile (target: $target)"
    IDF_TARGET="$target" idf.py \
        -C "$project_dir" \
        -B "$build_dir" \
        -DSDKCONFIG="$sdkconfig_path" \
        -DSDKCONFIG_DEFAULTS="$defaults_file;$profile_defaults_file" \
        build
}

build_project_step() {
    local project="$1"
    local project_dir="$repo_root/firmware/$project"
    local defaults_file="$project_dir/sdkconfig.defaults"
    local build_dir="$build_root/$project"
    local target

    if [[ ! -d "$project_dir" ]]; then
        echo "error: unknown firmware project '$project'" >&2
        exit 1
    fi
    if [[ ! -f "$defaults_file" ]]; then
        echo "error: missing $defaults_file" >&2
        exit 1
    fi

    target="$(parse_target "$defaults_file")"
    if [[ -z "$target" ]]; then
        echo "error: could not determine CONFIG_IDF_TARGET from $defaults_file" >&2
        exit 1
    fi

    if [[ "$project" == "ring" && -n "$ring_profile" ]]; then
        build_ring_profile_step "$ring_profile"
        return
    fi

    mkdir -p "$build_root"
    echo "==> Building $project (target: $target)"
    IDF_TARGET="$target" idf.py -C "$project_dir" -B "$build_dir" build
}

while (($# > 0)); do
    case "$1" in
        --all)
            use_all_projects=true
            ;;
        --firmware-only)
            run_host_tests=false
            ;;
        --host-tests-only)
            run_firmware=false
            ;;
        --with-kicad)
            run_kicad_checks=true
            ;;
        --kicad-only)
            run_host_tests=false
            run_firmware=false
            run_kicad_checks=true
            ;;
        --kicad-strict)
            kicad_strict=true
            run_kicad_checks=true
            ;;
        --ring-profile)
            shift
            if [[ $# -eq 0 || "$1" == -* ]]; then
                echo "error: --ring-profile requires a profile name" >&2
                usage >&2
                exit 1
            fi
            ring_profile="$1"
            ;;
        --ring-profile=*)
            ring_profile="${1#--ring-profile=}"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        ring|pen|puck|hub)
            append_project "$1"
            ;;
        *)
            echo "error: unknown argument '$1'" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

if [[ "$use_all_projects" == true ]]; then
    requested_projects=("${all_projects[@]}")
elif ((${#requested_projects[@]} == 0)); then
    requested_projects=("${default_projects[@]}")
fi

if [[ -n "$ring_profile" && "$run_firmware" == true ]] && ! project_was_requested ring; then
    echo "error: --ring-profile requires the ring firmware target" >&2
    echo "       omit explicit targets or include 'ring'" >&2
    exit 1
fi

if [[ "$run_host_tests" == true ]]; then
    run_host_tests_step
fi

if [[ "$run_firmware" == true ]]; then
    require_idf
    for project in "${requested_projects[@]}"; do
        build_project_step "$project"
    done
fi

if [[ "$run_kicad_checks" == true ]]; then
    run_kicad_checks_step
fi

echo "==> Verification complete"
