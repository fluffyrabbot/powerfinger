# SPDX-License-Identifier: MIT
"""Trial driver: wire R30 power_and_charge sub-sheet via kicad-sch-api.

Strategy: place a wire label at every component pin position, naming the
net. KiCad treats same-named labels on a sheet as one net. Hierarchical
labels (VBUS_5V, VREG_3V3, VBAT_SENSE, NTC_SENSE, CHARGE_EN) already exist
and are matched by name.

Internal nets:
  VBUS_CHG       — Q1.D → U3.VCC (TP4054 input via P-MOSFET high-side switch)
  Q1_GATE        — Q1.G ← R4 pull-up to VBUS_5V; Q2.D pulls low when CHARGE_EN
  VBAT_PROTECTED — TP4054.BAT → battery rail → U4.VIN/EN, divider tops
  U3_PROG        — TP4054.PROG → R1 → GND (charge current set)
  CHRG_STAT_INT  — TP4054.STAT (open-drain) → R11 pull-up to VREG_3V3
  VBUS_DETECT_INT — R9/R10 divider mid-point (no parent sheet pin yet)
"""
from __future__ import annotations

import sys
from pathlib import Path

import kicad_sch_api as ksa

SHEET = (
    "hardware/ring/R30-OLED-NONE-NONE/kicad/sheets/power_and_charge.kicad_sch"
)

# (ref, pin_number, net_name) — None means leave unconnected (NC pin)
TOPOLOGY = [
    # Q1 — P-MOSFET high-side switch (S=VBUS_5V, D=VBUS_CHG, G=Q1_GATE)
    ("Q1", "S", "VBUS_5V"),
    ("Q1", "D", "VBUS_CHG"),
    ("Q1", "G", "Q1_GATE"),
    # Q2 — N-MOSFET low-side gate driver
    ("Q2", "D", "Q1_GATE"),
    ("Q2", "S", "GND"),
    ("Q2", "G", "CHARGE_EN"),
    # R4 — Q1 gate pull-up to VBUS_5V (default-off)
    ("R4", "1", "VBUS_5V"),
    ("R4", "2", "Q1_GATE"),
    # R6 — Q2 gate pulldown to GND (default-off)
    ("R6", "1", "CHARGE_EN"),
    ("R6", "2", "GND"),
    # U3 — TP4054 (MCP73831-2-OT pinout: STAT=1, VSS=2, VBAT=3, VDD=4, PROG=5)
    ("U3", "1", "CHRG_STAT_INT"),
    ("U3", "2", "GND"),
    ("U3", "3", "VBAT_PROTECTED"),
    ("U3", "4", "VBUS_CHG"),
    ("U3", "5", "U3_PROG"),
    # R1 — TP4054 charge current set (20k → ~50mA)
    ("R1", "1", "U3_PROG"),
    ("R1", "2", "GND"),
    # R11 — CHRG_STAT pull-up
    ("R11", "1", "VREG_3V3"),
    ("R11", "2", "CHRG_STAT_INT"),
    # U4 — RT9080 LDO (AP2112K pinout: VIN=1, GND=2, EN=3, NC=4, VOUT=5)
    ("U4", "1", "VBAT_PROTECTED"),
    ("U4", "2", "GND"),
    ("U4", "3", "VBAT_PROTECTED"),  # EN tied to VIN (always enabled)
    # U4.4 = NC, intentionally unlabeled
    ("U4", "5", "VREG_3V3"),
    # C1 — TP4054 input decoupling
    ("C1", "1", "VBUS_CHG"),
    ("C1", "2", "GND"),
    # C2 — LDO input bulk
    ("C2", "1", "VBAT_PROTECTED"),
    ("C2", "2", "GND"),
    # C3 — LDO output decoupling
    ("C3", "1", "VREG_3V3"),
    ("C3", "2", "GND"),
    # R7/R8 — VBAT_SENSE divider (100k/100k, top to VBAT_PROTECTED)
    ("R7", "1", "VBAT_PROTECTED"),
    ("R7", "2", "VBAT_SENSE"),
    ("R8", "1", "VBAT_SENSE"),
    ("R8", "2", "GND"),
    # R9/R10 — VBUS_DETECT divider (220k/100k, top to VBUS_5V)
    ("R9", "1", "VBUS_5V"),
    ("R9", "2", "VBUS_DETECT_INT"),
    ("R10", "1", "VBUS_DETECT_INT"),
    ("R10", "2", "GND"),
    # R3 — NTC divider top from VREG_3V3
    ("R3", "1", "VREG_3V3"),
    ("R3", "2", "NTC_SENSE"),
    # NTC1 — divider bottom to GND
    ("NTC1", "1", "NTC_SENSE"),
    ("NTC1", "2", "GND"),
]


def main() -> int:
    repo = Path(__file__).resolve().parent.parent
    sheet_path = repo / SHEET
    sch = ksa.load_schematic(str(sheet_path))
    print(f"loaded {sheet_path}")

    # collect U4 NC pin position so a no-connect marker can be placed
    nc_pos = None
    for p_num, p_pos in sch.list_component_pins("U4"):
        if p_num == "4":
            nc_pos = p_pos
            break

    placed = 0
    for ref, pin, net in TOPOLOGY:
        try:
            pos = sch.get_component_pin_position(ref, pin)
        except Exception as exc:
            print(f"  FAIL  pin lookup {ref}.{pin}: {exc!r}", file=sys.stderr)
            return 3
        if pos is None:
            print(f"  FAIL  no pin position for {ref}.{pin}", file=sys.stderr)
            return 3
        sch.add_label(net, position=(pos.x, pos.y))
        placed += 1
        print(f"  label {net:18s} at {ref}.{pin} ({pos.x:.2f}, {pos.y:.2f})")

    # Add a no-connect marker on U4.NC if API supports it; otherwise skip
    # (kicad-sch-api may not expose no_connect — the NC pin will register as
    # pin_not_connected, which is correct semantics for a true NC pin)
    if nc_pos is not None and hasattr(sch, "add_no_connect"):
        try:
            sch.add_no_connect(position=(nc_pos.x, nc_pos.y))
            print(f"  no-connect at U4.4 ({nc_pos.x:.2f}, {nc_pos.y:.2f})")
        except Exception as exc:
            print(f"  WARN  no_connect not supported: {exc!r}", file=sys.stderr)

    sch.save()
    print(f"saved. placed {placed} labels.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
