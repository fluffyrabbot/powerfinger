#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cad_source="$repo_root/hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad"
pcb_source="$repo_root/hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb"
output_dir=$(mktemp -d "${TMPDIR:-/tmp}/powerfinger-r30-geometry.XXXXXX")
trap 'rm -rf "$output_dir"' EXIT

openscad_bin=${OPENSCAD_BIN:-$(command -v openscad || true)}
kicad_cli_bin=${KICAD_CLI_BIN:-$(command -v kicad-cli || true)}

if [[ -z "$openscad_bin" ]]; then
  echo "OpenSCAD is required; set OPENSCAD_BIN to the OpenSCAD executable." >&2
  exit 2
fi
if [[ -z "$kicad_cli_bin" ]]; then
  echo "KiCad CLI is required; set KICAD_CLI_BIN to kicad-cli." >&2
  exit 2
fi

run_openscad() {
  local export_mode=$1
  local allow_empty=${2:-false}
  local output_path="$output_dir/r30-$export_mode.stl"
  local log_path="$output_dir/r30-$export_mode.log"
  local status=0
  "$openscad_bin" \
    -D "export_mode=\"$export_mode\"" \
    -D 'show_exploded_view=false' \
    -D 'show_reference_solids=false' \
    -D 'show_reference_fasteners=false' \
    -o "$output_path" \
    "$cad_source" >"$log_path" 2>&1 || status=$?
  if grep -q 'ERROR:' "$log_path"; then
    echo "OpenSCAD reported an ERROR for export_mode=$export_mode" >&2
    cat "$log_path" >&2
    exit 1
  fi
  if [[ "$status" -ne 0 ]] && ! { [[ "$allow_empty" == true ]] && grep -qi "top level object is empty" "$log_path"; }; then
    echo "OpenSCAD failed for export_mode=$export_mode (status=$status)" >&2
    cat "$log_path" >&2
    exit 1
  fi
}

for export_mode in shell service_access_coupon board_retention_coupon lid_pad_coupon \
  battery_harness_coupon service_lid_coupon fit_coupons contact_probe; do
  run_openscad "$export_mode"
  if [[ ! -s "$output_dir/r30-$export_mode.stl" ]]; then
    echo "OpenSCAD produced an empty STL for export_mode=$export_mode" >&2
    exit 1
  fi
done

# collision_probe is expected to be empty: OpenSCAD emits no STL when all
# posed-solid intersections are empty. A non-empty file is a real collision.
run_openscad collision_probe true
if [[ -s "$output_dir/r30-collision_probe.stl" ]]; then
  echo "CSG collision_probe produced a non-empty mesh" >&2
  exit 1
fi

"$kicad_cli_bin" pcb drc --exit-code-violations \
  --output "$output_dir/r30-drc.txt" "$pcb_source" \
  >"$output_dir/r30-kicad.log" 2>&1

echo "R30 enclosure geometry assertions passed for all export modes."
echo "KiCad PCB DRC passed with zero violations and zero unconnected pads."
