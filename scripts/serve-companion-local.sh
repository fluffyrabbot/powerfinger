#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# PowerFinger - Local static server for the companion Web Serial app.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
port="${POWERFINGER_COMPANION_PORT:-4173}"

echo "Serving PowerFinger companion on http://127.0.0.1:${port}"
echo "Use Chrome or Edge so Web Serial is available."

exec python3 -m http.server "${port}" --directory "${repo_root}/companion/web"
