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
| Battery harness clears the service seam without pinch | Required | | | |
| Battery removal path fits through the top service opening | Required | | | |
| Closure hardware clears the finger opening and top shell skin | Required | | | |
| `43 x 18 mm` PCB pod does not create an accessibility-hostile ring profile | Required | | | |
| Off-board service fixture can reach J1 pads without loading the shell or board rails | Required | | | |

## Quick-Print Coupon Evidence

Record coupon results here before treating the full shell as mechanically
closed. These rows are allowed to fail; failures should update `MANIFEST.md`
instead of pushing the packet into secondary variants.

Regenerate the local blank worksheet, coupon STLs, preview PNGs, and hashes with
`scripts/generate-r30-ring-fit-coupons.sh`. Copy only real printed/fixture
observations back into this table.

For the combined first-board first sweep, regenerate
`build/first-board-mechanical-packet/FIRST-SWEEP/` with
`scripts/generate-first-board-mechanical-packet.sh --first-sweep`; it selects
the R30 off-board service-pad access coupon and board-retention coupon plus the
matching previews/logs and a blank worksheet. This generated folder is still
print/preview scaffolding until real observations are recorded.

| Coupon | Required Outcome | Measured | Status | Notes |
|--------|------------------|----------|--------|-------|
| `service_access_coupon` | Off-board service access coupon lets the fixture/pogo path reach J1 without levering the board edge | | | |
| `board_retention_coupon` | `43 x 18 mm` board slides onto rails, stops repeatably, and lifts out by hand | | | |
| `lid_pad_coupon` | Lid pads touch only intended board-edge zones and do not trap components | | | |
| `battery_harness_coupon` | Off-board battery harness/service loop clears the seam during cell lift-out | | | |
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
| Board lifts out after lid removal without prying on service pads, dome, or antenna end | Required | | |
| `SW1` dome pocket preserves click travel without destructive lid flex | Required | | |

## RF And Safety Proof

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Antenna keep-out preserved in final board + shell stack | Required | | |
| Battery choice still satisfies `docs/BATTERY-SAFETY.md` | Required | | |
| Charging and service access remain non-hermetic and repairable | Required | | |
| Service fastener or retention choice stays inside the packet BOM assumptions | Required | | |
| First-pass service-pad access and board retention path are measured or still marked unproven in `MANIFEST.md` | Required | | |
| Board retention uses non-metal/plastic features near the antenna keep-out | Required | | |

## Board-Pass Observations, Not Measurements

These notes come from the first routed PCB source only. They do not replace the
measured rows above.

| Check | Board-pass status | Notes |
|-------|-------------------|-------|
| ESP32-C3 antenna keep-out | Preserved in PCB | Copper/component keep-out zones are drawn before routing; shell plastics and closure hardware still need physical verification around the outward antenna edge |
| Charge path | Fixture-fed P0 service jumper | `Q1` is now a non-BOM copper VBUS service jumper into TP4054 `VCC`; `GPIO10`, `R4`, `Q2`, and `R6` are not part of the first-board charge-service contract. Any onboard charge-enable switch needs explicit BDFL acceptance before fabrication |
| Regulator land pattern | Compact U4 footprint retained | `U4` keeps the RT9080-33GJ5 part and pinout but now uses a source-controlled compact service-clearance SOT-23-5 land pattern; this is KiCad proof only, not assembly evidence |
| PAW3204 placement | Aperture-aligned in PCB | Sensor is bottom-side at the `6.2 mm` aperture datum; focal distance and lens clip fit remain unmeasured |
| Shell electronics model | First-pass CAD updated | The shell now maps the `43 x 18 mm` board into a top pod instead of the older generic module pocket; this is still a CAD sanity pass, not fit evidence |
| Battery service | Harness path modeled, coupon available, fit unproven | `J_BAT` is now a same-net off-board battery service-pad interface rather than an onboard JST-SH body; the CAD keeps a local harness channel, service-loop relief, top lift window, and `battery_harness_coupon`, but print/fit and replaceable-harness evidence are still required |
| Off-board service pads | Access modeled, coupon available, fit unproven | PCB now uses a same-net off-board service-pad footprint at `J1` instead of an onboard USB-C receptacle; `service_access_coupon` now checks fixture/pogo access and board-edge loading, not plug insertion |
| Board retention | Molded retention path modeled, coupon available, fit unproven | CAD uses side rails, side stop lugs, lid compression pads, `board_retention_coupon`, and `lid_pad_coupon` so service pads are not structural; removal force, tolerance, and RF effects are still unmeasured |
| Dome click | Pocket modeled, actuation unproven | CAD adds a `SW1` dome pocket and top relief; it does not prove accessible click force, tactile cap geometry, or dome replacement access |
| Service lid | Top-removal path modeled, coupon available, human proof open | Lid screws, pry relief, nested skirt, top battery lift path, and `service_lid_coupon` are represented; limited-dexterity handling and captured-hardware needs remain open |
| Rigid P0 outline | Red until shell fit | The `43 x 18 mm` board now drives the shell pod, but the model does not prove comfort, height, RF, or printable tolerances |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If the seam, battery-service path, or closure hardware is still provisional,
  say so explicitly instead of marking the packet mechanically closed
- If any row is red, do not start puck or secondary ring hardware
