<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB P0 Component Locks

This file records the first exact part locks for the active hub's USB and power
path.

It exists to stop the next KiCad pass from regressing into "generic USB-A plug"
or "some ESD array later" thinking after the repo has already chosen a direct
USB dongle direction.

References:

- [CAPTURE-BINDINGS.md](CAPTURE-BINDINGS.md)
- [SCHEMATIC-CAPTURE.md](SCHEMATIC-CAPTURE.md)
- [hardware/bom/USB-HUB.csv](../../bom/USB-HUB.csv)

## Locked First-Board Parts

| Ref | Locked part | Why it is locked now | Footprint policy | Notes |
|-----|-------------|----------------------|------------------|-------|
| `J1` | `SOFNG USB-05` (`LCSC C112454`) | Keeps the first hub aligned with the USB-A direct-plug dongle recommendation instead of drifting toward a host receptacle or cable-only board | Use a local footprint named `PowerFinger_USB:USB_A_Plug_SOFNG_USB-05` derived from the manufacturer PCB layout | LCSC catalog text may describe this family as a receptacle, but the manufacturer drawing is `USB-A/M`; preserve the male-plug intent in the schematic, footprint, and enclosure |
| `U2` | `RT9080-33GJ5` (`LCSC C882092`) | Already the family-standard low-Iq regulator and now the explicit hub power-tree owner | `Package_TO_SOT_SMD:SOT-23-5` is acceptable if the pinout and paste pattern match the Richtek datasheet | Keep the hub aligned with the same low-power logic rail assumptions as the ring lane |
| `D1` | `USBLC6-2SC6` (`LCSC C7519`) | Official USB 2.0 ESD device with D+ / D- plus VBUS protection in the same package | Start from an ST `SOT23-6L`-compatible land pattern and check it against the package drawing before routing | Place adjacent to `J1`; do not route a long unprotected stub from the connector to the TVS |
| `C2` | `10uF` input capacitor in `0603` | Makes the VBUS-side bulk part honest about effective capacitance on the first board | Commodity 0603 MLCC | Exact vendor can remain commodity, but the package should stay locked |
| `C3` | `1uF` output capacitor in `0603` | The RT9080 requires at least `1uF` effective output capacitance for stability | Commodity 0603 MLCC | Exact dielectric and voltage rating still need the same-commit choice when the KiCad symbols land |

## Remaining Non-Locked Items

- `R1` value is still intentionally open pending the first actual ESP32-S3
  native USB schematic placement and reference-design check.
- `SW1` remains a service-path choice, not a finalized part lock.
- `LED1` and `R2` are deliberately left commodity until board edge visibility is
  checked against enclosure service seams.

## What This Unblocks Next

- place `J1`, `U2`, `D1`, `C2`, and `C3` as real symbols on
  `sheets/usb_and_power.kicad_sch`
- decide whether `J1` needs a local footprint only or a local courtyard plus
  enclosure support geometry
- run the first honest board-envelope and host-clearance check against the
  direct-plug dongle assumption

## Source Links For The Locked Parts

- `J1`: [SOFNG USB-05 datasheet via LCSC](https://datasheet.lcsc.com/lcsc/2206151015_SOFNG-USB-05_C112454.pdf)
- `U2`: [Richtek RT9080 datasheet](https://www.richtek.com/assets/product_file/RT9080/DS9080-08.pdf)
- `D1`: [ST USBLC6-2 product page](https://www.st.com/en/protections-and-emi-filters/usblc6-2.html)
