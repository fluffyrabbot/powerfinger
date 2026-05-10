#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# PowerFinger - Local contract drift checks for the active R30 lane.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if (($# > 0)); then
  ruby "$repo_root/scripts/contract_check.rb" --repo-root "$repo_root" "$@"
else
  ruby "$repo_root/scripts/contract_check.rb" --repo-root "$repo_root"
  ruby "$repo_root/scripts/contract_check.rb" --repo-root "$repo_root" --self-test
fi
