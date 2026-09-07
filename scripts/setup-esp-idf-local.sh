#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Install/export the repository's declared, reproducible ESP-IDF toolchain.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
manifest="$repo_root/toolchains/esp-idf-local.json"
if [[ ! -f "$manifest" ]]; then echo "error: missing $manifest" >&2; exit 1; fi
read_manifest() {
    python3 - "$manifest" "$1" <<'PY'
import json, sys
with open(sys.argv[1], encoding="utf-8") as f: data = json.load(f)
value = data
for part in sys.argv[2].split('.'): value = value[part]
print(value if not isinstance(value, list) else ','.join(value))
PY
}

idf_version="$(read_manifest version)"
idf_commit="$(read_manifest commit)"
targets="$(read_manifest targets)"
if [[ -n "${POWERFINGER_IDF_VERSION:-}" && "$POWERFINGER_IDF_VERSION" != "$idf_version" ]]; then
    echo "error: POWERFINGER_IDF_VERSION conflicts with manifest-pinned $idf_version" >&2
    exit 2
fi
install_root="${POWERFINGER_IDF_ROOT:-$HOME/.powerfinger-sdk}"
tools_root="${POWERFINGER_IDF_TOOLS_PATH:-$install_root/espressif-tools}"
idf_dir="${install_root}/esp-idf-${idf_version}"
mode=install

usage() {
    cat <<EOF
Usage: $0 [--export|--print-path|--check|--idf-version $idf_version|--help]
The authoritative version and commit are in toolchains/esp-idf-local.json.
Install path: $idf_dir
EOF
}
while (($#)); do
    case "$1" in
        --export) mode=export;;
        --print-path) mode=print-path;;
        --check) mode=check;;
        --idf-version)
            [[ $# -ge 2 ]] || { echo "error: --idf-version requires a value" >&2; exit 2; }
            [[ "$2" == "$idf_version" ]] || { echo "error: SDK version is manifest-pinned to $idf_version" >&2; exit 2; }
            shift
            ;;
        -h|--help) usage; exit 0;;
        *) echo "error: unknown argument '$1'" >&2; usage >&2; exit 2;;
    esac
    shift
done

check_checkout() {
    if [[ ! -d "$idf_dir/.git" || ! -f "$idf_dir/export.sh" ]]; then
        echo "error: ESP-IDF $idf_version is not installed at $idf_dir" >&2; return 1
    fi
    local head dirty submodules
    head="$(git -C "$idf_dir" rev-parse HEAD)" || { echo "error: cannot read SDK HEAD" >&2; return 1; }
    [[ "$head" == "$idf_commit" ]] || { echo "error: SDK HEAD $head != pinned $idf_commit" >&2; return 1; }
    dirty="$(git -C "$idf_dir" status --porcelain --untracked-files=all)" || { echo "error: cannot inspect SDK modifications" >&2; return 1; }
    [[ -z "$dirty" ]] || { echo "error: SDK checkout has modifications" >&2; printf '%s\n' "$dirty" >&2; return 1; }
    submodules="$(git -C "$idf_dir" submodule status --recursive)" || { echo "error: cannot inspect SDK submodules" >&2; return 1; }
    while IFS= read -r line; do
        [[ -z "$line" || "${line:0:1}" != "-" && "${line:0:1}" != "+" && "${line:0:1}" != "U" ]] || { echo "error: SDK submodule state is not clean" >&2; return 1; }
    done <<< "$submodules"
}

emit_export() {
    check_checkout
    printf 'export POWERFINGER_IDF_VERSION=%q\n' "$idf_version"
    printf 'export POWERFINGER_IDF_ROOT=%q\n' "$install_root"
    printf 'export IDF_TOOLS_PATH=%q\n' "$tools_root"
    printf '. %q >/dev/null\n' "$idf_dir/export.sh"
}

case "$mode" in
    print-path) printf '%s\n' "$idf_dir";;
    check) check_checkout; echo "ESP-IDF $idf_version ($idf_commit) is clean at $idf_dir";;
    export) emit_export;;
    install)
        if [[ ! -d "$idf_dir/.git" ]]; then
            mkdir -p "$install_root"
            git clone --branch "$idf_version" --depth 1 --recurse-submodules https://github.com/espressif/esp-idf.git "$idf_dir"
        fi
        check_checkout
        export IDF_TOOLS_PATH="$tools_root"
        "$idf_dir/install.sh" "$targets"
        echo "ESP-IDF $idf_version ($idf_commit) ready at $idf_dir"
        ;;
esac
