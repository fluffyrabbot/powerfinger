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
| `U1` | Vendor-specific | `ESP32-S3-MINI-1-N8` module symbol plus local footprint `PowerFinger_USB:ESP32-S3-MINI-1-N8_FirstBoard` | Preserve native USB pins and put the antenna keep-out at the rear of the stepped dongle |
| `J1` | Vendor-specific | Exact `SOFNG USB-05` direct-plug connector plus local footprint `PowerFinger_USB:USB_A_Plug_SOFNG_USB-05` | The first board is a true USB-A dongle. The PCB nose is narrow for host clearance; the wider body starts behind the connector shoulder |
| `U2` | Stock package | Exact `RT9080-33GJ5` symbol + `SOT-23-5` footprint | The first hub board needs an explicit regulator, not an inherited dev-board rail |
| `C1`, `C2`, `C3` | Stock package | Standard MLCC symbols with committed footprint sizes and roles | Keep `C1` at `0402`, and keep `C2` / `C3` at `0603` so the power path stays honest about effective capacitance |
| `R1A`, `R1B` | Stock package | Separate `22R` USB series resistor symbols with 0402 footprints | These are the resolved implementation of the BOM's two-count `R1` line; place them close to `U1` |
| `D1` | Stock package | Exact `USBLC6-2SC6` symbol and `SOT-23-6` land pattern | Lock the official ST part and its land pattern together so USB protection is no longer generic |
| `LED1`, `R2` | Stock package | Standard status LED path | LED visibility must not compromise service access or use a strapping pin |
| `SW1` | Function placeholder | Pad-actuated `BOOT_N` service footprint for this board pass | The first board uses pads plus a possible recessed actuator; `EN` remains reachable on a separate service pad |
| `PCB1`, `ENCL1` | Custom / mechanical | Stepped `54 x 26 mm` PCB, connector shell tabs, shell-clamp holes, antenna keep-out, and extraction clearance | The board must not behave like a floating USB tongue trapped inside the shell |

## Required Non-BOM Capture Items

- service pads for `EN`, `BOOT_N`, `UART_TX_DBG`, `UART_RX_DBG`, `3V3`, and `GND`
- explicit native USB routing notes for `USB_D+` and `USB_D-`; the clean pass
  leaves USB data inspection to trace/connector access rather than connected
  service pads
- an explicit note that `usb_and_power` owns `VBUS_5V` to `VREG_3V3`
- connector reinforcement notes shared between PCB and enclosure
- `C5` / `C6` no-BOM mechanical DNI footprints near the ESP32-S3 side of the
  USB pair; they are intentionally unconnected in the first DRC-clean routing
- `R3` / `C4` EN RC support on the first PCB pass so `EN` is not left floating

## Remaining Checks Before Fabrication

- keep KiCad ERC/DRC clean with the local footprints loaded
- compare the SOFNG USB-05 footprint against a printed 1:1 plot before ordering
- carry the `MH1` / `MH2` shell-clamp holes into the enclosure CAD
- confirm the stepped USB-A body clears adjacent host ports
