<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Stackup Verification

Fill this document with measured evidence from the first active-lane hardware
drop. Do not replace measured values with estimates.

## Geometry

| Check | Target | Measured | Status | Notes |
|-------|--------|----------|--------|-------|
| Finger-to-surface height | `~10 mm` max | | | |
| Sensor-to-surface gap | `2.4-3.2 mm` | | | |
| Battery bay closes without compression risk | Required | | | |
| Shell can reopen after first full assembly | Required | | | |
| Battery leads clear the service seam without pinch | Required | | | |
| Battery removal path fits through the top service opening | Required | | | |
| Closure hardware clears the finger opening and top shell skin | Required | | | |
| `42 x 18 mm` PCB pod does not create an accessibility-hostile ring profile | Required | | | |
| USB-C plug can insert without loading the shell or board rails | Required | | | |

## Quick-Print Coupon Evidence

Record coupon results here before treating the full shell as mechanically
closed. These rows are allowed to fail; failures should update `MANIFEST.md`
instead of pushing the packet into secondary variants.

| Coupon | Required Outcome | Measured | Status | Notes |
|--------|------------------|----------|--------|-------|
| `usb_c_coupon` | USB-C plug enters without binding or levering the board edge | | | |
| `board_retention_coupon` | `42 x 18 mm` board slides onto rails, stops repeatably, and lifts out by hand | | | |
| `lid_pad_coupon` | Lid pads touch only intended board-edge zones and do not trap components | | | |
| `battery_lead_coupon` | JST-SH lead and service loop clear the seam during cell lift-out | | | |
| `service_lid_coupon` | Lid/skirt/pry path can be reopened without destructive flex or tiny-tool dependence | | | |

## Optical And Mechanical Proof

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Sensor aperture stays aligned under realistic finger pressure | Required | | |
| Glide pads maintain focal distance without user technique | Required | | |
| Click actuation does not force destructive shell flex | Required | | |
| Lower shell keeps rim and glide-pad geometry unchanged after seam cycles | Required | | |
| Top service lid can be removed without peeling wear pads first | Preferred | | |
| Battery service loop is long enough for rework but short enough to avoid sensor snagging | Required | | |
| Board lifts out after lid removal without prying on USB-C, dome, or antenna end | Required | | |
| `SW1` dome pocket preserves click travel without destructive lid flex | Required | | |

## RF And Safety Proof

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Antenna keep-out preserved in final board + shell stack | Required | | |
| Battery choice still satisfies `docs/BATTERY-SAFETY.md` | Required | | |
| Charging and service access remain non-hermetic and repairable | Required | | |
| Service fastener or retention choice stays inside the packet BOM assumptions | Required | | |
| First-pass USB-C opening and board retention path are measured or still marked unproven in `MANIFEST.md` | Required | | |
| Board retention uses non-metal/plastic features near the antenna keep-out | Required | | |

## Board-Pass Observations, Not Measurements

These notes come from the first routed PCB source only. They do not replace the
measured rows above.

| Check | Board-pass status | Notes |
|-------|-------------------|-------|
| ESP32-C3 antenna keep-out | Preserved in PCB | Copper/component keep-out zones are drawn before routing; shell plastics and closure hardware still need physical verification around the outward antenna edge |
| Charge path | Electrically safer with add | Packet recommendation is `Q2` = 2N7002 SOT-23 plus `R6` = `100k` to keep the 5 V `Q1` gate pull-up off ESP32-C3 `GPIO10`; BSS138-class or load-switch substitutions need explicit BDFL acceptance before fabrication |
| PAW3204 placement | Aperture-aligned in PCB | Sensor is bottom-side at the `6.2 mm` aperture datum; focal distance and lens clip fit remain unmeasured |
| Shell electronics model | First-pass CAD updated | The shell now maps the `42 x 18 mm` board into a top pod instead of the older generic module pocket; this is still a CAD sanity pass, not fit evidence |
| Battery service | Connector and lead path modeled, coupon available, fit unproven | `J_BAT` is JST-SH right-angle 1.0 mm and the CAD now adds a local lead channel, service-loop relief, top lift window, and `battery_lead_coupon`; print/fit evidence is still required |
| Service USB-C | Opening modeled, coupon available, fit unproven | PCB uses a 16-pin USB 2.0 Type-C class with through-hole shell stakes; the CAD now cuts a first-pass left-edge opening and `usb_c_coupon`, but plug insertion and strain relief remain red until measured |
| Board retention | Molded retention path modeled, coupon available, fit unproven | CAD uses side rails, side stop lugs, lid compression pads, `board_retention_coupon`, and `lid_pad_coupon` so USB-C is not structural; removal force, tolerance, and RF effects are still unmeasured |
| Dome click | Pocket modeled, actuation unproven | CAD adds a `SW1` dome pocket and top relief; it does not prove accessible click force, tactile cap geometry, or dome replacement access |
| Service lid | Top-removal path modeled, coupon available, human proof open | Lid screws, pry relief, nested skirt, top battery lift path, and `service_lid_coupon` are represented; limited-dexterity handling and captured-hardware needs remain open |
| Rigid P0 outline | Red until shell fit | The `42 x 18 mm` board now drives the shell pod, but the model does not prove comfort, height, RF, or printable tolerances |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If the seam, battery-service path, or closure hardware is still provisional,
  say so explicitly instead of marking the packet mechanically closed
- If any row is red, do not start puck or secondary ring hardware
