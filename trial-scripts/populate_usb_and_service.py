# SPDX-License-Identifier: MIT
"""Populate and wire R30 usb_and_service sub-sheet via kicad-sch-api.

Components per BOM-BLOCK-MAP.md "USB / service entry" block:
  J1   — USB-C 16-pin receptacle, top-mount horizontal with TH stakes
         (GCT USB4105 stock symbol; USB4215 footprint class per BOM)
  R2   — CC1 pull-down (5.1kΩ, BOM Qty=2 split into R2+R12)
  R12  — CC2 pull-down (5.1kΩ, sibling instance of R2)
  D1   — USBLC6-2SC6 dual-channel ESD array, SOT-23-6

Hierarchical labels already in stub: VBUS_5V, USB_D+, USB_D-.
"""
from __future__ import annotations

import sys
from pathlib import Path

import kicad_sch_api as ksa

SHEET = "hardware/ring/R30-OLED-NONE-NONE/kicad/sheets/usb_and_service.kicad_sch"

PLAN = [
    # (ref, lib_id, footprint, value, position, props)
    ("J1", "Connector:USB_C_Receptacle_USB2.0_16P",
     "Connector_USB:USB_C_Receptacle_GCT_USB4105-xx-A_16P_TopMnt_Horizontal",
     "USB-C", (90, 80),
     {"mfg_part_num": "USB4105-GF-A", "manufacturer": "GCT",
      "lcsc": ""}),
    ("R2", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "5.1k", (140, 90),
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("R12", "Device:R", "Resistor_SMD:R_0402_1005Metric",
     "5.1k", (155, 90),
     {"mfg_part_num": "", "manufacturer": "—", "lcsc": ""}),
    ("D1", "Power_Protection:USBLC6-2SC6",
     "Package_TO_SOT_SMD:SOT-23-6",
     "USBLC6-2SC6", (130, 60),
     {"mfg_part_num": "USBLC6-2SC6", "manufacturer": "STMicroelectronics",
      "lcsc": ""}),
]

# (ref, pin, net) — None means leave unconnected (SBU pins are NC)
TOPOLOGY = [
    # J1 USB-C — pins consolidated by symbol (GND, VBUS each share position)
    ("J1", "A1", "GND"),
    ("J1", "A4", "VBUS_5V"),
    ("J1", "A5", "CC1"),
    ("J1", "A6", "USB_D+"),
    ("J1", "A7", "USB_D-"),
    ("J1", "A8", None),  # SBU1 — NC
    ("J1", "B5", "CC2"),
    ("J1", "B6", "USB_D+"),
    ("J1", "B7", "USB_D-"),
    ("J1", "B8", None),  # SBU2 — NC
    ("J1", "SH", "GND"),
    # R2 — CC1 pull-down (5.1k Rd presents board as USB sink)
    ("R2", "1", "CC1"),
    ("R2", "2", "GND"),
    # R12 — CC2 pull-down sibling
    ("R12", "1", "CC2"),
    ("R12", "2", "GND"),
    # D1 USBLC6-2SC6 — pinout: 1/6=I/O1 (D-), 3/4=I/O2 (D+), 5=VBUS, 2=GND
    ("D1", "1", "USB_D-"),
    ("D1", "2", "GND"),
    ("D1", "3", "USB_D+"),
    ("D1", "4", "USB_D+"),
    ("D1", "5", "VBUS_5V"),
    ("D1", "6", "USB_D-"),
]

POWER_FLAGS: list = [
    # No power symbols on this sheet — J1 and D1 have only passive /
    # bidirectional pins, so there are no power_input pins to drive.
    # The cross-sheet GND net inherits its driver from the power:GND
    # symbol already placed in power_and_charge.
]


def main() -> int:
    repo = Path(__file__).resolve().parent.parent
    sheet_path = repo / SHEET
    sch = ksa.load_schematic(str(sheet_path))
    print(f"loaded {sheet_path}")

    for ref, lib_id, fp, val, pos, props in PLAN:
        sch.components.add(
            lib_id=lib_id, reference=ref, value=val, position=pos,
            footprint=fp, **props,
        )
        print(f"  added {ref:5s} {lib_id}")

    for idx, (lib_id, value, net, pos) in enumerate(POWER_FLAGS, start=1):
        ref = f"#PWR{idx:04d}"
        sch.components.add(lib_id=lib_id, reference=ref, value=value,
                           position=pos)
        pin_pos = sch.get_component_pin_position(ref, "1")
        if net is not None:
            sch.add_label(net, position=(pin_pos.x, pin_pos.y))
        print(f"  added {ref} {lib_id}")

    placed = 0
    nc_added = 0
    for ref, pin, net in TOPOLOGY:
        try:
            pos = sch.get_component_pin_position(ref, pin)
        except Exception as exc:
            print(f"  FAIL {ref}.{pin}: {exc!r}", file=sys.stderr)
            return 3
        if net is None:
            sch.no_connects.add(position=(pos.x, pos.y))
            nc_added += 1
        else:
            sch.add_label(net, position=(pos.x, pos.y))
            placed += 1

    sch.save()
    print(f"saved. {placed} labels placed, {nc_added} no-connect markers.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
