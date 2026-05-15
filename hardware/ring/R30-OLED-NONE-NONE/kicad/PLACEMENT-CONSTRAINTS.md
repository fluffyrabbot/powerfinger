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
- Non-BOM VBUS service jumper into TP4054 `VCC`
- NTC thermistor and divider for cell temperature monitoring
- Dome click switch
- USB-C charge/debug entry
- Protected `80-100 mAh` LiPo connection

## First Board Footprint Locks

The first routed PCB pass in `r30_oled_none_none.kicad_pcb` locks these
placement classes for P0:

- `U1`: `ESP32-C3-MINI-1-N4` module envelope, antenna end toward board `+X`
  and shell exterior
- `U2`: bottom-side `PAW3204DB-TJ3L` 8-pin optical package centered on the
  aperture datum
- `J_BAT`: source-controlled off-board same-net battery service pads for
  `VBAT+`, `VBAT-`, and grounded shield/service pads
- `J1`: source-controlled off-board same-net USB service pads for USB2 data,
  VBUS, CC pulldowns, GND, and shield continuity
- `U3`: SOT-23-5 TP4054 footprint, with the TP4054 pinout kept distinct from
  MCP73831-style chargers
- `U4`: source-controlled RT9080-33GJ5 compact SOT-23-5 service-clearance
  footprint; this keeps the same regulator and pinout while relieving the
  first-board regulator/click corridor
- `Q1`: source-controlled non-BOM VBUS service jumper before TP4054 `VCC`

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
- The first routed board pass is `43 x 18 mm`; that width matches the current
  band-width placeholder, but it does not prove the shell closes. Treat this as
  a red stackup item until the board outline, USB opening, battery bay, and
  ESP32 antenna keep-out are checked against physical CAD or a print.
- The current shell CAD now uses this board outline directly as a top pod and
  maps `J1`, `J_BAT`, `SW1`, `U2`, and the antenna keep-out from KiCad
  coordinates. Any PCB placement change to those references must be mirrored in
  `cad/r30_oled_none_none_shell_blank.scad` before the packet is mechanically
  honest.

## Safety / Service Rules

- The battery must remain removable without de-soldering the service pads or
  the cell.
- The NTC placement must reflect actual cell temperature, not board-ambient
  wishful thinking.
- The VBUS service jumper and TP4054 belong near the USB/VBUS entry, not deep
  inside the RF-sensitive region.
- This P0 does not allocate ESP32-C3 `GPIO10` to charge control. Reintroducing
  an onboard charge-enable switch needs a BOM, schematic, PCB, firmware, and
  repairability decision, not a silent routing equivalent.
- The off-board service pads cannot be a structural retention point for the
  board inside the shell.
- The first CAD retention path is molded side rails, side stop lugs, and lid
  compression pads; do not add hidden adhesive, one-shot snaps, or metal clips
  near the antenna keep-out without a BDFL decision and BOM update.
- The dome switch path should remain replaceable without first excavating the
  battery.

## PCB Direction Of Travel

- Start with an honest schematic and placement envelope for the final ring
  electronics, even if early bench bring-up uses a rigid lash-up.
- Preserve a migration path to flex or rigid-flex by avoiding placement choices
  that only work on a large rectangular dev board.

## Flex Migration Readiness

The first board is a rigid P0. A later flex or rigid-flex respin must be
possible without re-choosing footprints. That invariant lives here rather than
at respin time.

- Treat the MCU / radio region, the optical sensor + lens + dome region, and
  the battery + charge-path region as three independent footprint zones.
- No component, copper pour, or keep-out on a zone edge may overhang into an
  adjacent zone's footprint. A valid rigid-to-flex split line must already
  exist between each zone pair on the rigid P0.
- `J1` service entry belongs in the battery + charge-path zone, near the VBUS
  entry rule in `Safety / Service Rules`, not across the zone boundary into
  the MCU region.
- `J_BAT` battery service-pad interface belongs in the battery zone and must
  not force the battery envelope across the MCU or sensor zone boundary.
- Antenna keep-out, battery envelope, sensor aperture, and dome actuation area
  are zone-bounded by earlier rules — do not compromise those boundaries to
  save a small amount of copper on the rigid P0.
