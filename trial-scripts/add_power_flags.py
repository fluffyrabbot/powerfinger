# SPDX-License-Identifier: MIT
"""Add power flag/ground symbols to silence power_pin_not_driven warnings.

KiCad ERC requires every "power input" pin to have a "power output" pin on
the same net. The MCP73831-2-OT symbol declares VDD (pin 4) and VSS (pin 2)
as power inputs, so the VBUS_CHG and GND nets need an explicit power-output
port. PWR_FLAG provides one. GND also needs a power:GND symbol which acts
as an implicit ground reference.

Placed off to the side of the active component grid to avoid visual clutter.
"""
from __future__ import annotations

import sys
from pathlib import Path

import kicad_sch_api as ksa

SHEET = (
    "hardware/ring/R30-OLED-NONE-NONE/kicad/sheets/power_and_charge.kicad_sch"
)

# Power flags need to be physically connected to the wire/label net via
# their single pin. We place each flag at the pin position of a labeled
# net so the flag's pin coincides with an existing wire label.
# Position chosen to be clear of the active components.

PLACEMENTS = [
    # (lib_id, value, net_label, position)
    # value: the symbol Value field. ksa.components.add does not auto-fill
    #   from the lib_symbol's default Value property, so power symbols need
    #   their net name passed explicitly (otherwise the instance has an
    #   empty Value and KiCad ERC reports a multiple_net_names conflict).
    # net_label: redundant wire label for non-intrinsic-named flags.
    ("power:PWR_FLAG", "PWR_FLAG", "VBUS_5V", (40, 80)),
    ("power:PWR_FLAG", "PWR_FLAG", "VBUS_CHG", (95, 75)),
    ("power:GND", "GND", None, (40, 120)),
    ("power:PWR_FLAG", "PWR_FLAG", "GND", (45, 120)),
]


def main() -> int:
    repo = Path(__file__).resolve().parent.parent
    sheet_path = repo / SHEET
    sch = ksa.load_schematic(str(sheet_path))
    print(f"loaded {sheet_path}")

    # Use a deterministic local reference scheme to avoid collision; KiCad
    # auto-generates "#PWR0001"-style refs for power symbols on save, but
    # ksa requires a valid format string up front.
    for idx, (lib_id, value, net, pos) in enumerate(PLACEMENTS, start=1):
        ref = f"#PWR{idx:04d}"
        try:
            sch.components.add(
                lib_id=lib_id,
                reference=ref,
                value=value,
                position=pos,
                rotation=0,
            )
            pin_pos = sch.get_component_pin_position(ref, "1")
            if net is not None:
                sch.add_label(net, position=(pin_pos.x, pin_pos.y))
            print(f"  added {ref:8s} {lib_id:18s} pin1 at "
                  f"({pin_pos.x:.2f}, {pin_pos.y:.2f}) net={net or '(intrinsic GND)'}")
        except Exception as exc:
            print(f"  FAIL  {ref}: {exc!r}", file=sys.stderr)
            return 3

    # Tie the power:GND symbol (#PWR0003) and the adjacent PWR_FLAG (#PWR0004)
    # together so the GND net has a power-output source.
    sch.add_wire_between_pins("#PWR0003", "1", "#PWR0004", "1")
    print("  wire #PWR0003.1 ↔ #PWR0004.1 (joins GND symbol to GND PWR_FLAG)")

    sch.save()
    return 0


if __name__ == "__main__":
    sys.exit(main())
