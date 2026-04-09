<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Placement Constraints

References:

- [hardware/bom/R30-OLED-NONE-NONE.csv](../../bom/R30-OLED-NONE-NONE.csv)
- [docs/PROTOTYPE-SPEC.md](../../../docs/PROTOTYPE-SPEC.md)
- [docs/GLIDE-SYSTEM.md](../../../docs/GLIDE-SYSTEM.md)
- [docs/BATTERY-SAFETY.md](../../../docs/BATTERY-SAFETY.md)
- [docs/REGULATORY-PRESCAN.md](../../../docs/REGULATORY-PRESCAN.md)

## Electrical Blocks To Capture

- `ESP32-C3-MINI-1-N4`
- PAW3204-class optical sensor and matched lens stack
- TP4054 charge controller with `20 kohm` RPROG
- RT9080-33GJ5 low-Iq regulator
- P-channel MOSFET for charge enable/disable
- NTC thermistor and divider for cell temperature monitoring
- Dome click switch
- USB-C charge/debug entry
- Protected `80-100 mAh` LiPo connection

## Mechanical / RF Placement Rules

- Place the ESP32-C3 module so the antenna end faces outward toward the shell
  exterior, not inward toward the finger.
- Reserve the module antenna keep-out first:
  approximately `10 mm` beyond the antenna end, spanning the module width, with
  no copper, traces, or components in that zone.
- Keep a continuous ground plane under the module body, excluding the antenna
  keep-out.
- Keep the optical sensor and lens stack mechanically tied to the underside
  aperture. Do not rely on a floating lens position that the shell later has to
  “find.”
- Keep the sensor cavity and LED path away from shiny charging hardware or tall
  metal around the aperture.
- The battery envelope from the BOM (`<= 20 x 15 x 4 mm`) must fit without
  violating the reopenable service seam.

## Safety / Service Rules

- The battery must remain removable without de-soldering the USB-C connector.
- The NTC placement must reflect actual cell temperature, not board-ambient
  wishful thinking.
- The charge MOSFET and TP4054 belong near the USB/VBUS entry, not deep inside
  the RF-sensitive region.
- The USB-C connector cannot be the only structural retention point for the
  board inside the shell.
- The dome switch path should remain replaceable without first excavating the
  battery.

## PCB Direction Of Travel

- Start with an honest schematic and placement envelope for the final ring
  electronics, even if early bench bring-up uses a rigid lash-up.
- Preserve a migration path to flex or rigid-flex by avoiding placement choices
  that only work on a large rectangular dev board.

