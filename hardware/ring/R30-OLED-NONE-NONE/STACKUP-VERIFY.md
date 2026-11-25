<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Stackup Verification

Fill this document with measured evidence from the first active-lane hardware
drop. Do not replace measured values with estimates.

## Geometry

| Check | Target | Model or measurement | Status | Notes |
|-------|--------|----------|--------|-------|
| Finger-to-surface height | `~10 mm` max | `10.0 mm nominal CAD stack` | CAD pass; physical print/pad tolerance unmeasured | `7.8 mm` shell top to `-2.2 mm` installed-pad surface; pad is exposed below rim |
| Sensor-to-surface gap | `2.4-3.2 mm` | `2.9 mm fixture gauge target` | Gauge setting only; matched lens projection unmeasured | KiCad U2 board-local `(-4.0, 0.0)`; gauge datum only, obtain package and lens projection before defining the board opening |
| Battery bay closes without compression risk | Required | | | |
| Shell can reopen after first full assembly | Required | | | |
| Battery harness clears the service seam without pinch | Required | | | |
| Battery removal path fits through the top service opening | Required | | | |
| Closure hardware clears the finger opening and top shell skin | Required | | | |
| `43 x 18 mm` PCB pod does not create an accessibility-hostile ring profile | Required | Non-wearable lateral validation fixture: electronics pod `46.6 x 26.6 mm` at `y=23.4`; separate cell pod `24.0 x 26.6 mm` at `x=-36.0` | CAD intervals clear the finger passage and each other; accessibility/comfort unmeasured | Do not treat this fixture as wearable |
| Final ESP32-C3 body and antenna void fit the populated pod | Required | U1 board-local `(+13.20,0)`, body `16.6 x 13.2 x 2.4 mm`; copper void `x=+16.1..+21.5`, full board Y | CAD assertion only; RF and physical stackup unmeasured | Service screws clear void by `0.4 mm` nominal in Y |
| Off-board service fixture can reach J1 pads without loading the shell or board rails | Required | | | |

### Nominal CAD interval regression

These are source-derived axis-aligned intervals, not physical measurements. The
OpenSCAD assertions fail if any listed positive clearance is lost.

| Volume | X interval (mm) | Y interval (mm) | Z interval (mm) |
|--------|-----------------|-----------------|-----------------|
| Finger passage keepout | `[-9.55, 9.55]` radial | `[-9.55, 9.55]` radial | through shell |
| PCB envelope | `[-21.50, 21.50]` | `[14.40, 32.40]` | `[3.10, 4.10]` |
| Populated U1 radio body | `[4.90, 21.50]` | `[16.80, 30.00]` | `[4.10, 6.50]` |
| Antenna copper void | `[16.10, 21.50]` | `[14.40, 32.40]` | `[4.10, 6.50]` |
| Protected-cell envelope | `[-46.00, -26.00]` | `[15.90, 30.90]` | `[2.60, 6.80]` |
| Electronics pod | `[-23.30, 23.30]` | `[10.10, 36.70]` | `[0.00, 7.80]` |
| Cell pod | `[-48.00, -24.00]` | `[10.10, 36.70]` | `[0.00, 7.80]` |
| Installed glide-pad contact plane | shell-local | shell-local | `-2.20` |

The source-level `collision_probe` export is empty (all five posed-solid
intersections are empty); `contact_probe` is non-empty and contains the
exposed glide-pad and joined service-bridge solids. These are CAD mesh checks,
not physical fit or wearability evidence.

Derived clearances: PCB-to-finger radial `4.85 mm` at the board y edge,
cell-to-PCB x `4.50 mm` (`4.15 mm` after the PCB clearance allowance), pod-gap
`0.70 mm`, board-end-stop gap `0.25 mm`, and battery-to-lid lift-window top
`0.10 mm` nominal. The lateral profile, harness bend, and printed tolerances
remain physical gates.

## Quick-Print Coupon Evidence

Record coupon results here before treating the full shell as mechanically
closed. These rows are allowed to fail; failures should update `MANIFEST.md`
instead of pushing the packet into secondary variants.

Regenerate the local blank worksheet, coupon STLs, preview PNGs, and hashes with
`scripts/generate-r30-ring-fit-coupons.sh`. Copy only real printed/fixture
observations into the generated worksheet, then ingest those rows into
[COUPON-RESULTS.md](COUPON-RESULTS.md) with
`scripts/ingest-r30-ring-coupon-results.py` before mirroring closure-relevant
outcomes back into this table.

For the combined first-board first sweep, regenerate
`build/first-board-mechanical-packet/FIRST-SWEEP/` with
`scripts/generate-first-board-mechanical-packet.sh --first-sweep`; it selects
the R30 off-board service-pad access coupon and board-retention coupon plus the
matching previews/logs and a blank worksheet. This generated folder is still
print/preview scaffolding until real observations are recorded.

## Source-Controlled Coupon Result Ingest

Generated worksheets are field-capture artifacts; they are not the durable ring
truth surface. After real R30 coupon checks are filled in, ingest the worksheet
from the repository root:

```sh
scripts/ingest-r30-ring-coupon-results.py build/first-board-mechanical-packet/FIRST-SWEEP/PHYSICAL-CHECK-WORKSHEET.md
```

For the R30-only mechanical bundle, use:

```sh
scripts/ingest-r30-ring-coupon-results.py build/r30-oled-none-none-mechanical/PHYSICAL-CHECK-WORKSHEET.md
```

The source-controlled result ledger is
[COUPON-RESULTS.md](COUPON-RESULTS.md). Keep the coupon rows below non-green
until that ledger contains real result rows with fixture, board, tool, coupon
print, or photo references.

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
| ESP32-C3 antenna keep-out | Preserved in PCB | CAD now uses final KiCad U1 anchor `(133.70,100)` / board-local `(+13.20,0)`, body envelope `16.6 x 13.2 x 2.4 mm`, and copper void `x=136.60..142.00` / local `+16.1..+21.5` across full board Y; shell plastics and RF behavior remain unmeasured |
| Charge path | Fixture-fed P0 service jumper | `Q1` is now a non-BOM copper VBUS service jumper into TP4054 `VCC`; `GPIO10`, `R4`, `Q2`, and `R6` are not part of the first-board charge-service contract. Any onboard charge-enable switch needs explicit BDFL acceptance before fabrication |
| Regulator land pattern | Compact U4 footprint retained | `U4` keeps the RT9080-33GJ5 part and pinout but now uses a source-controlled compact service-clearance SOT-23-5 land pattern; this is KiCad proof only, not assembly evidence |
| PCB copper stack | Four-layer KiCad stack retained | The live board now declares `F.Cu`, `In1.Cu`, `In2.Cu`, and `B.Cu`; the via-to-via `VREG_3V3` support span, left-service `USB_D-` span, and local `USB_D+` service span use `In1.Cu`, the `NTC_SENSE` U1 ADC span uses `In2.Cu`, and the physical stackup/price/height remain unmeasured |
| Fabrication outputs | Local Gerber/drill/POS review generated | `kicad/FABRICATION-OUTPUTS.md` records the `build-kicad/R30-OLED-NONE-NONE/fabrication-review/` outputs, zip hash, four-layer drill report, and active-BOM/POS review result with no open rows; this is generated file evidence only, not physical stackup or assembly proof |
| Board-house output constraints | Checklist added, vendor response open | `kicad/BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` lists the prototype-house intake questions for file set, layer order, stackup options, drill/mask/paste rules, panelization, service pads, BOM/POS handling, and editable source return; none of those answers prove physical ring fit or stackup height |
| PAW3204 placement | Aperture-aligned in PCB | Sensor is bottom-side at the `6.2 mm` aperture datum and CAD tunnel is tied to KiCad board-local `(-4.0, 0.0)`; focal distance and lens clip fit remain unmeasured |
| Shell electronics model | First-pass CAD updated | Non-wearable lateral validation fixture maps board and cell into separate pods with a service bridge; scalar assertions plus CSG mesh probes check nominal intervals and lid clearance, but this is not physical fit evidence |
| Battery service | Harness path modeled, coupon available, fit unproven | `J_BAT` is now a same-net off-board battery service-pad interface rather than an onboard JST-SH body; the CAD keeps a local harness channel, service-loop relief, top lift window, and `battery_harness_coupon`, but print/fit and replaceable-harness evidence are still required |
| Off-board service pads | Access modeled, coupon available, fit unproven | PCB now uses a same-net off-board service-pad footprint at `J1` instead of an onboard USB-C receptacle; `service_access_coupon` now checks fixture/pogo access and board-edge loading, not plug insertion |
| Board retention | Molded retention path modeled, coupon available, fit unproven | CAD uses side rails, side stop lugs, lid compression pads, `board_retention_coupon`, and `lid_pad_coupon` so service pads are not structural; removal force, tolerance, and RF effects are still unmeasured |
| Dome click | Pocket modeled, actuation unproven | CAD adds a `SW1` dome pocket and top relief; it does not prove accessible click force, tactile cap geometry, or dome replacement access |
| Service lid | Top-removal path modeled, coupon available, human proof open | Lid screws, pry relief, nested skirt, top battery lift path, and `service_lid_coupon` are represented; limited-dexterity handling and captured-hardware needs remain open |
| Rigid P0 outline | Red until shell fit | The `43 x 18 mm` board now drives the shell pod and nominal height/gap assertions pass; the widened pod does not yet prove comfort, RF, or printable tolerances |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If the seam, battery-service path, or closure hardware is still provisional,
  say so explicitly instead of marking the packet mechanically closed
- If any row is red, do not start puck or secondary ring hardware
