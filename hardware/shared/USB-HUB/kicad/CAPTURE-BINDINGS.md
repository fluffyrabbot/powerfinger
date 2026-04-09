<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Capture Bindings

This file turns the active hub BOM and interface contract into capture-ready
binding guidance for the first KiCad pass.

It exists so the first hub board does not quietly drift into generic connector
placeholders, inaccessible recovery hardware, or a mechanically weak USB plug.

Read [P0-COMPONENT-LOCKS.md](P0-COMPONENT-LOCKS.md) alongside this file when the
work moves from abstract bindings into actual first-board symbol placement.

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
| `J1` | Vendor-specific | Exact `SOFNG USB-05` direct-plug connector plus local footprint `PowerFinger_USB:USB_A_Plug_SOFNG_USB-05` | The first board is a true USB-A dongle. Do not let distributor taxonomy drift this into a receptacle or cable header |
| `U2` | Stock package | Exact `RT9080-33GJ5` symbol + `SOT-23-5` footprint | The first hub board needs an explicit regulator, not an inherited dev-board rail |
| `C1`, `C2`, `C3` | Stock package | Standard MLCC symbols with committed footprint sizes and roles | Keep `C1` at `0402`, and keep `C2` / `C3` at `0603` so the power path stays honest about effective capacitance |
| `R1` | Stock package | Separate USB series resistor symbols with 0402 footprints | Do not collapse the USB pair into a single note blob |
| `D1` | Stock package | Exact `USBLC6-2SC6` symbol and `SOT-23-6` land pattern | Lock the official ST part and its land pattern together so USB protection is no longer generic |
| `LED1`, `R2` | Stock package | Standard status LED path | LED visibility must not compromise service access or use a strapping pin |
| `SW1` | Function placeholder | Compact recovery switch footprint or deliberately pad-only service path | The current BOM line should not force a 6x6 tact onto a ~20x12 mm dongle if that breaks the board envelope |
| `PCB1`, `ENCL1` | Custom / mechanical | Mechanical notes, connector reinforcement, and extraction clearance | The board must not behave like a floating USB tongue trapped inside the shell |

## Required Non-BOM Capture Items

- service pads for `EN`, `BOOT_N`, `UART_TX_DBG`, `UART_RX_DBG`, `3V3`, and `GND`
- explicit native USB routing notes for `USB_D+` and `USB_D-`
- an explicit note that `usb_and_power` owns `VBUS_5V` to `VREG_3V3`
- connector reinforcement notes shared between PCB and enclosure

## Blocking Locks Before First Routed Board

- place the locked `J1`, `U2`, `D1`, `C2`, and `C3` parts as real symbols on
  `usb_and_power.kicad_sch`
- verify the local `J1` footprint against the SOFNG PCB layout and the
  enclosure support geometry
- exact recovery-switch approach for `SW1` or an explicit decision to rely on
  service pads plus enclosure actuation only
- confirm the final `R1` value after the first ESP32-S3 native USB reference check
