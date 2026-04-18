#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# PowerFinger — install and export a repo-pinned local ESP-IDF toolchain.

set -euo pipefail

idf_version="${POWERFINGER_IDF_VERSION:-v5.2.2}"
install_root="${POWERFINGER_IDF_ROOT:-$HOME/.powerfinger-sdk}"
tools_root="${POWERFINGER_IDF_TOOLS_PATH:-$install_root/espressif-tools}"
idf_dir="${install_root}/esp-idf-${idf_version}"
targets="esp32c3,esp32s3"
mode="install"

usage() {
    cat <<'EOF'
Usage:
  scripts/setup-esp-idf-local.sh
  scripts/setup-esp-idf-local.sh --export
  scripts/setup-esp-idf-local.sh --print-path
  scripts/setup-esp-idf-local.sh --idf-version v5.2.2

Defaults:
  - Installs the repo-pinned ESP-IDF baseline under $HOME/.powerfinger-sdk/
  - Installs tools for esp32c3 and esp32s3 only
  - Leaves repo-tracked files untouched

Common workflow:
  scripts/setup-esp-idf-local.sh
  scripts/verify-firmware-local.sh
  # Optional: activate idf.py in the current shell for direct iteration
  eval "$(scripts/setup-esp-idf-local.sh --export)"
EOF
}

while (($# > 0)); do
    case "$1" in
        --export)
            mode="export"
            ;;
        --print-path)
            mode="print-path"
            ;;
        --idf-version)
            if (($# < 2)); then
                echo "error: --idf-version requires a value" >&2
                exit 1
            fi
            idf_version="$2"
            idf_dir="${install_root}/esp-idf-${idf_version}"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown argument '$1'" >&2
            usage >&2
            exit 1
            ;;
    esac
    shift
done

emit_export_snippet() {
    printf 'export POWERFINGER_IDF_VERSION=%q\n' "$idf_version"
    printf 'export POWERFINGER_IDF_ROOT=%q\n' "$install_root"
    printf 'export IDF_TOOLS_PATH=%q\n' "$tools_root"
    printf '. %q >/dev/null\n' "${idf_dir}/export.sh"
}

require_checkout() {
    mkdir -p "$install_root"
    if [[ -d "${idf_dir}/.git" ]]; then
        echo "==> Reusing ${idf_dir}"
        return
    fi

    echo "==> Cloning ESP-IDF ${idf_version} into ${idf_dir}"
    git clone --branch "$idf_version" --depth 1 \
        https://github.com/espressif/esp-idf.git "$idf_dir"
}

case "$mode" in
    export)
        if [[ ! -f "${idf_dir}/export.sh" ]]; then
            echo "error: ${idf_dir} is not installed yet." >&2
            echo "Run scripts/setup-esp-idf-local.sh first." >&2
            exit 1
        fi
        emit_export_snippet
        ;;
    print-path)
        printf '%s\n' "$idf_dir"
        ;;
    install)
        require_checkout
        export IDF_TOOLS_PATH="$tools_root"
        echo "==> Installing ESP-IDF tools for ${targets}"
        "${idf_dir}/install.sh" "$targets"
        echo "==> Local ESP-IDF baseline ready"
        echo "==> The shared verifier can auto-activate this pinned toolchain"
        echo "==> from a fresh shell."
        echo "==> If you want idf.py in your current shell for direct iteration:"
        echo "eval \"\$(scripts/setup-esp-idf-local.sh --export)\""
        echo
        echo "The first hub build will still resolve esp_tinyusb via Espressif's"
        echo "component manager if it is not already cached locally."
        ;;
esac
