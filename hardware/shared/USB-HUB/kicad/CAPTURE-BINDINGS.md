<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Capture Bindings

This file turns the active hub BOM and interface contract into capture-ready
binding guidance for the first KiCad pass.

It exists so the first hub board does not quietly drift into generic connector
placeholders, inaccessible recovery hardware, or a mechanically weak USB plug.

## Binding Status Meanings

- `Stock package`: a normal package footprint and generic symbol are acceptable.
- `Vendor-specific`: use the real module or connector geometry, not a generic
  placeholder body.
- `Custom / mechanical`: requires connector reinforcement, custom copper, or
  explicit service geometry.
- `Function placeholder`: the function is mandatory, but the exact package
  should be chosen during capture to fit the board envelope honestly.

## Active Hub Binding Plan

| Refs | Binding status | Capture representation | Notes |
|------|----------------|------------------------|-------|
| `U1` | Vendor-specific | Exact `ESP32-S3-MINI-1-N8` module symbol/footprint pair | Preserve native USB pins and antenna keep-out from the vendor module geometry |
| `J1` | Vendor-specific | Reinforced USB-A plug footprint for the first board | Prefer through-hole or hybrid reinforcement for the first dongle pass; only pivot to USB-C plus cable under the documented fallback trigger |
| `U2` | Stock package | `RT9080-33GJ5` SOT-23-5 LDO symbol + footprint | The first hub board needs an explicit regulator, not an inherited dev-board rail |
| `C1`, `C2`, `C3` | Stock package | Standard MLCC symbols with committed footprint sizes and roles | Keep `C2` at VBUS entry and `C3` tied to the LDO stability requirement instead of treating all caps as interchangeable filler |
| `R1` | Stock package | Separate USB series resistor symbols with 0402 footprints | Do not collapse the USB pair into a single note blob |
| `D1` | Stock package | 2-channel USB ESD array in SOT-23-6 or equivalent footprint | Lock the exact part and land pattern together |
| `LED1`, `R2` | Stock package | Standard status LED path | LED visibility must not compromise service access or use a strapping pin |
| `SW1` | Function placeholder | Compact recovery switch footprint or deliberately pad-only service path | The current BOM line should not force a 6x6 tact onto a ~20x12 mm dongle if that breaks the board envelope |
| `PCB1`, `ENCL1` | Custom / mechanical | Mechanical notes, connector reinforcement, and extraction clearance | The board must not behave like a floating USB tongue trapped inside the shell |

## Required Non-BOM Capture Items

- service pads for `EN`, `BOOT_N`, `UART_TX_DBG`, `UART_RX_DBG`, `3V3`, and `GND`
- explicit native USB routing notes for `USB_D+` and `USB_D-`
- an explicit note that `usb_and_power` owns `VBUS_5V` to `VREG_3V3`
- connector reinforcement notes shared between PCB and enclosure

## Blocking Locks Before First Routed Board

- exact `J1` USB-A plug MPN and the corresponding mechanical support strategy
- exact `U2` footprint and the committed `C3` output-cap package/value pairing
- exact recovery-switch approach for `SW1` or an explicit decision to rely on
  service pads plus enclosure actuation only
- exact ESD array MPN and footprint choice
