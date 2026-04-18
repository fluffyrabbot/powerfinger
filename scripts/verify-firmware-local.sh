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
use_all_projects=false

declare -a requested_projects=()
declare -a all_projects=(ring pen puck hub)
declare -a default_projects=(ring hub)

usage() {
    cat <<'EOF'
Usage:
  scripts/verify-firmware-local.sh [options] [ring|pen|puck|hub ...]

Options:
  --all               Build every ESP-IDF firmware project
  --firmware-only     Skip host-side unit tests
  --host-tests-only   Run only host-side unit tests
  -h, --help          Show this help text

Defaults:
  - Host-side tests run first
  - Firmware verification builds the active lane: ring + hub
  - ESP-IDF build outputs go under build-idf/<project>/
  - If idf.py is not already in PATH, the script will try a repo-pinned
    local ESP-IDF install under $HOME/.powerfinger-sdk/

Examples:
  scripts/verify-firmware-local.sh
  scripts/verify-firmware-local.sh hub
  scripts/verify-firmware-local.sh --all
  scripts/verify-firmware-local.sh --host-tests-only
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

if [[ "$run_host_tests" == true ]]; then
    run_host_tests_step
fi

if [[ "$run_firmware" == true ]]; then
    require_idf
    for project in "${requested_projects[@]}"; do
        build_project_step "$project"
    done
fi

echo "==> Verification complete"
