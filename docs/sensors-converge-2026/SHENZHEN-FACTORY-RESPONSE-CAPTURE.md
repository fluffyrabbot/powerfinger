<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Shenzhen Factory Response Capture

Use this sheet to paste factory answers back into the repo without converting
ambiguous email prose into hidden design decisions.

Scope is intentionally narrow:

- Primary quote path: `USB-HUB` PCB fabrication and assembly quote,
  connector/enclosure DFM review, serviceability feedback, and proposed
  substitutions.
- Optional annex path: `R30-OLED-NONE-NONE` DFM/pre-fab review only, if the
  factory explicitly accepts the annex.

Do not record secondary rings, wands, puck variants, OCR, cloud features, or
companion-app work here.

## Capture Rules

- Factory text belongs in the `Factory answer / evidence` column.
- Repo interpretation belongs in `Maintainer note`.
- BDFL decisions stay explicit in `BDFL decision`. Leave blank until decided.
- `Quoted` means the factory offered price, MOQ, lead-time, availability, DFM,
  or source-return information. It does not mean verified.
- `Verified` means the repo has direct evidence: a received quote packet,
  delivered sample, build/run record, returned editable source, or other
  checked-in evidence that supports the claim.
- Do not update `docs/REFERENCE-MANUFACTURERS.md` from this sheet until a real
  quote, run, or direct verification exists.
- Do not update part-level sourcing claims in `docs/VENDOR-VERIFICATION.md`
  from this sheet unless the source is independently verified.
- Capture raw reply files in one dated evidence directory under
  `docs/sensors-converge-2026/factory-replies/` before changing downstream
  truth docs.

## Repo-Local Evidence Intake

Create one dated intake directory per factory reply before filling the tables
below:

```sh
scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD
```

Use the annex flag only when the factory explicitly accepts
`R30-OLED-NONE-NONE` as DFM/pre-fab review input:

```sh
scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD --include-r30-annex
```

Default evidence shape:

```text
docs/sensors-converge-2026/factory-replies/YYYY-MM-DD-seeed-fusion-propagate-usb-hub-reply/
  incoming/
  quote-files/
  source-return/
  USB-HUB-SUBSTITUTIONS.md
  USB-HUB-DFM-ASKS.md
  SOURCE-RETURN-INDEX.md
```

Use `incoming/` for raw messages and attachments, `quote-files/` for quote
sheets or DFM reports, `source-return/` for factory-returned editable source or
CAM-only files, and the Markdown ledgers for substitutions, DFM requests, and
source-return posture. Reference the dated directory from the response header
so quote-only and verified state can be traced without copying the same reply
into multiple docs.

## Response Header

| Field | Factory answer / evidence | Maintainer note |
|---|---|---|
| Factory / contact name |  |  |
| Response date |  |  |
| Repo evidence directory |  |  |
| Scope accepted | `USB-HUB` / `USB-HUB + R30 annex` / other:  |  |
| Quote path | PCB fab / assembly / enclosure DFM / connector DFM / other:  |  |
| Files reviewed by factory |  |  |
| Files returned by factory |  |  |
| Quote identifier / invoice / thread link |  |  |
| Payment terms or ordering preconditions |  |  |
| License/source-return acknowledgement |  |  |

## USB-HUB Quote And Verification Status

Keep `USB-HUB` as the send-now quote path. Use this table for the factory's
quoted status and the repo's verified status without merging the two.

| Area | Factory answer / evidence | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|
| PCB fabrication from `hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb` |  | Not received / quoted / declined / needs more info | Unverified / verified by quote / verified by run |  |  |
| PCB assembly from `hardware/bom/USB-HUB.csv` |  | Not received / quoted / declined / needs more info | Unverified / verified by quote / verified by run |  |  |
| SOFNG `USB-05` male USB-A sourcing and assembly |  | Not received / quoted / substitute proposed / declined | Unverified / verified by quote / verified by sample / verified by run |  |  |
| `MH1` / `MH2` connector clamp and direct-plug retention DFM |  | Not received / DFM accepted / DFM changes requested / blocked | Unverified / reviewed / physically tested |  |  |
| Reopenable enclosure and service hatch |  | Not received / DFM accepted / DFM changes requested / blocked | Unverified / reviewed / printed-tested |  |  |
| ESP32-S3 antenna keep-out around enclosure and clamp hardware |  | Not received / accepted / changes requested / blocked | Unverified / reviewed / RF-tested |  |  |
| Test or fixture access for `EN`, `BOOT_N`, UART, power, ground, USB, trace pads |  | Not received / accepted / changes requested / blocked | Unverified / reviewed / fixture-tested |  |  |
| Adjacent-port clearance for stepped USB-A body |  | Not received / accepted / changes requested / blocked | Unverified / reviewed / physically tested |  |  |
| Overall `USB-HUB` quote posture |  | Not received / quote-only / ready for BDFL review / declined | Unverified / quote captured / run captured |  |  |

## USB-HUB Proposed Substitutions

Every proposed substitute must keep exact manufacturer, MPN, package, supplier
link, footprint impact, enclosure/fixture impact, serviceability impact, and
status visible. Add rows only for actual factory proposals.

| Ref / subsystem | Active part | Proposed substitute | Manufacturer / MPN / package / supplier link | Footprint unchanged? | Enclosure or fixture changed? | Accessibility / serviceability impact | MOQ / unit quote / availability basis | Source files changed or requested | Quote-vs-verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `U1` MCU/radio/USB | `ESP32-S3-MINI-1-N8` |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `J1` USB connector | SOFNG `USB-05`, LCSC `C112454`, USB-A male plug |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `U2` regulator | RT9080-33GJ5, SOT-23-5 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `D1` USB ESD | USBLC6-2SC6, SOT-23-6 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `R1A` / `R1B` USB series resistors | 22R, 0402 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `C2` / `C3` power capacitors | `10uF` 0603 input, `1uF` effective 0603 output |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `C1`, `LED1`, `R2`, passives | Commodity support parts |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `SW1` recovery control | Service-size tactile or pad-actuated recovery path |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| PCB / enclosure retention | Stepped USB-A dongle with `MH1` / `MH2` clamp load path |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |

## USB-HUB DFM Requests

Use this table for factory requests that are not simple part substitutions.
Leave `BDFL decision` blank until the design choice is actually made.

| Request ID | Factory request | Affected files / features | Why factory wants it | Accessibility / serviceability impact | Source-return needed? | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|---|---|---|
| HUB-DFM-001 |  |  |  |  | Yes / no / unknown | Not received / requested / quoted / declined | Unverified / source returned / tested |  |  |
| HUB-DFM-002 |  |  |  |  | Yes / no / unknown | Not received / requested / quoted / declined | Unverified / source returned / tested |  |  |
| HUB-DFM-003 |  |  |  |  | Yes / no / unknown | Not received / requested / quoted / declined | Unverified / source returned / tested |  |  |

## Source-Return Posture

Hardware changes conveyed by the factory must come back as editable source
suitable for publication under the existing CERN-OHL-S 2.0 hardware posture.
Use this table before accepting DFM changes into the repo.

| Source area | Factory posture / returned file | Acceptable editable source | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|
| KiCad schematic changes |  | `.kicad_sch` plus any project-local symbols | Not received / promised / returned / refused | Unverified / inspected / merged |  |  |
| KiCad PCB/layout changes |  | `.kicad_pcb` plus any project-local footprints | Not received / promised / returned / refused | Unverified / inspected / merged |  |  |
| OpenSCAD enclosure changes |  | `.scad` source, not STL-only edits | Not received / promised / returned / refused | Unverified / inspected / merged |  |  |
| Fixture or test-jig changes |  | Editable drawings/source plus net/access notes | Not received / promised / returned / refused | Unverified / inspected / tested |  |  |
| BOM/POS/CPL format changes |  | Editable CSV or KiCad-source-derived data | Not received / promised / returned / refused | Unverified / inspected / exported |  |  |
| Gerber/CAM-only changes |  | CAM-only is not enough for upstream source | Not received / promised / returned / refused | Unverified / rejected / exception requested |  |  |

## R30 Annex Response Capture

Use this section only if the factory accepted the `R30-OLED-NONE-NONE`
DFM/pre-fab review annex. Do not treat these rows as a ring PCB fab/assembly
quote path.

| Area | Factory answer / evidence | Quote status | Verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|
| R30 KiCad DFM/pre-fab review |  | Not received / reviewed / changes requested / declined | Unverified / reviewed / tested |  |  |
| Board-house output constraints |  | Not received / reviewed / changes requested / declined | Unverified / reviewed / exported |  |  |
| PAW3204 sensor/lens/emitter kit sourcing |  | Not received / quoted / substitute proposed / declined | Unverified / verified by quote / verified by sample |  |  |
| Protected `80-100 mAh` LiPo and replaceable harness path |  | Not received / quoted / substitute proposed / declined | Unverified / verified by quote / verified by sample |  |  |
| Off-board USB service pads and fixture path |  | Not received / reviewed / changes requested / blocked | Unverified / reviewed / fixture-tested |  |  |
| Raised rim, glide pads, shell tolerance, and focal-distance checks |  | Not received / reviewed / changes requested / blocked | Unverified / reviewed / physically tested |  |  |

### R30 Proposed Substitutions

Add rows only for actual factory proposals. Keep `ADNS-2080` as a future board
profile unless the BDFL explicitly approves evaluating a new R30 path.

| Ref / subsystem | Active part | Proposed substitute | Manufacturer / MPN / package / supplier link | Footprint unchanged? | Shell or fixture changed? | Accessibility / serviceability impact | MOQ / unit quote / availability basis | Source files changed or requested | Quote-vs-verified status | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `U1` MCU/radio | `ESP32-C3-MINI-1-N4` |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `U2` optical sensor | PAW3204DB-TJ3L sensor/lens/emitter kit |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `LENS1` optical stack | Sensor-specific lens and clip |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `BT1` / `J_BAT` battery service | Protected `80-100 mAh` LiPo with replaceable service harness to off-board pads |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `U3` charger | TP4054 SOT-23-5 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `U4` regulator | RT9080-33GJ5 SOT-23-5 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `D1` USB service ESD | TPD2E2U06DCK-class rail-less dual TVS, SC-70/SOT-323 |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| `SW1` click element | 5mm 150-250gf snap dome class |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| Service interfaces | Off-board USB pads and battery service pads |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |
| Shell / focal-distance system | Raised rim plus UHMWPE glide pads |  |  | Yes / no / unknown | Yes / no / unknown |  |  |  | Quote-only / verified / rejected / needs review |  |  |

## Reference-Manufacturer Update Gate

Use this table before editing `docs/REFERENCE-MANUFACTURERS.md`. A placeholder
entry can remain placeholder until the row has evidence beyond conversation.

| Candidate update | Evidence path in repo | Quoted-only or realized? | Fields eligible to update | Fields that must stay `TBD` | Maintainer note | BDFL decision |
|---|---|---|---|---|---|---|
| Seeed Studio Fusion + Propagate `USB-HUB` quote |  | Quoted-only / realized / none | MOQ / quoted cost / lead time / notes / none | Realized COGS / yield / defects / other:  |  |  |
| Seeed Studio Fusion + Propagate `R30` annex feedback |  | DFM-only / quoted-only / none | Notes / none | Realized COGS / yield / defects / fab quote / other:  |  |  |
| JLCPCB / EasyEDA fallback |  | Quoted-only / realized / none | MOQ / quoted cost / lead time / notes / none | Realized COGS / yield / defects / other:  |  |  |
| PCBWay fallback |  | Quoted-only / realized / none | MOQ / quoted cost / lead time / notes / none | Realized COGS / yield / defects / other:  |  |  |
| Local Bao'an / Longgang prototype house |  | Contact-only / quoted-only / realized / none | Contact name / quote scope / payment terms / notes / none | Realized COGS / yield / defects / other:  |  |  |

## Vendor-Verification Update Gate

Use this table before editing `docs/VENDOR-VERIFICATION.md`. Factory comments
are useful, but they are not independent distributor verification by themselves.

| Part or subsystem | Factory claim | Independent source checked? | Eligible doc update | Maintainer note | BDFL decision |
|---|---|---|---|---|---|
| ESP32-S3-MINI-1-N8 / N4R2 |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |
| SOFNG `USB-05` |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |
| RT9080-33GJ5 |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |
| USBLC6-2SC6 |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |
| PAW3204 kit |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |
| Protected LiPo and harness |  | Yes / no | None / quoted-source note / verified sourcing update |  |  |

## Closeout Checklist

Before treating a factory response as actionable:

- The response is pasted or summarized in this sheet.
- All proposed substitutions have exact manufacturer, MPN, package, and source
  link or are marked incomplete.
- Every footprint, enclosure, fixture, and serviceability impact is marked
  yes, no, or unknown.
- Source-return posture is marked for every factory-side hardware change.
- Quote status and verified status are separated.
- The dated evidence directory is recorded in the response header.
- Any `docs/REFERENCE-MANUFACTURERS.md` or `docs/VENDOR-VERIFICATION.md` update
  is backed by the update-gate tables above.
- BDFL decisions are explicit and not inferred from factory wording.
