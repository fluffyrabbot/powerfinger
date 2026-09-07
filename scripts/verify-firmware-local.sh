#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Public PowerFinger verification entrypoint; orchestration lives in Python.
set -euo pipefail
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec python3 "$repo_root/scripts/verify_firmware_local.py" "$@"
