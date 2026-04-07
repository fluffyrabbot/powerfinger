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

require_idf() {
    if command -v idf.py >/dev/null 2>&1; then
        return
    fi

    cat >&2 <<'EOF'
error: idf.py is not available in PATH.

Export the ESP-IDF environment before running firmware builds, for example:
  . $IDF_PATH/export.sh

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
