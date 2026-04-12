<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE First Board Checklist

This checklist is the execution contract for the active ring hardware lane.

## PCB Capture And Layout

- Lock every active BOM line to a concrete symbol / footprint pair
- Complete `kicad/r30_oled_none_none.kicad_pcb`
- Preserve the module antenna keep-out and document any exceptions
- Validate the charge path, `RT9080-33GJ5`, `20 kohm` charge resistor, NTC
  divider, and charge-enable MOSFET against `docs/BATTERY-SAFETY.md`
- Validate the PAW3204-class sensor placement against focal-distance and aperture
  requirements

## Mechanical Closure

- Turn `cad/r30_oled_none_none_shell_blank.scad` into a printable shell with a
  reopenable seam
- Preserve glide-pad pockets and structural rim geometry
- Add battery wiring clearance and connector strain-relief allowances
- Confirm the shell still supports battery replacement without destructive rework

## Bring-Up Evidence Required Before Secondary Variants

- Complete [STACKUP-VERIFY.md](STACKUP-VERIFY.md)
- Link assembled board / shell observations from `MANIFEST.md`
- Keep the packet honest if any item remains provisional or fails
