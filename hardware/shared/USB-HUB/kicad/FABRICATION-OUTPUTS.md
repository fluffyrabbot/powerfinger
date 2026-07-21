<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Fabrication Output Review

This note records the local fabrication-output proof for the DRC-clean routed
USB-HUB first-board KiCad source. The generated files are intentionally under
`build-kicad/` and are not checked in.

This is not physical hardware evidence. It does not prove connector retention,
adjacent-port clearance, host insertion/removal behavior, enclosure fit,
service-hatch reach, RF behavior, yield, realized COGS, or factory acceptance.

Use `BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` to turn these generated outputs into
explicit prototype-house / Seeed-style intake questions for the active hub
quote path.

## Current Local Output

Output directory:
`build-kicad/USB-HUB/fabrication-review/`

Review archive:
`build-kicad/USB-HUB/usb-hub-fabrication-review.zip`

Archive SHA-256:
`c72cd55be079671a1adc68b89a7a250ed81e8175a9c4a179ebf1228098672432`

Generated output classes:

| Class | Files | Notes |
|-------|-------|-------|
| Gerbers | 10 | `F.Cu`, `B.Cu`, paste, silkscreen, mask, `Edge.Cuts`, and Gerber job file |
| Drill | 5 | Split PTH/NPTH Excellon plus Gerber drill maps and drill report |
| Assembly review | 3 | Position CSV plus active-BOM/POS review CSV and markdown report; no warning-bearing KiCad schematic BOM is used |

The drill report records a two-layer copper stack (`F.Cu`, `B.Cu`), 25 plated
through holes, and 4 unplated through holes.

## Commands

Run the KiCad verifier first:

```bash
scripts/verify-firmware-local.sh --kicad-only
```

Generate the review outputs:

```bash
scripts/generate-usb-hub-fabrication-review.py
```

The generator runs KiCad Gerber, drill, and POS exports, merges the POS export
with `hardware/bom/USB-HUB.csv`, writes the assembly review, and creates the
review archive.

## Review Caveats

- `scripts/generate-usb-hub-fabrication-review.py` intentionally avoids the
  KiCad schematic BOM export and merges `hardware/bom/USB-HUB.csv` with the
  KiCad POS export instead. This preserves board refs such as `C1A`, `C1B`,
  `R1A`, and `R1B` while keeping the source-controlled active BOM as the quote
  contract.
- The generated active-BOM/POS review has no open rows. `C1A`/`C1B` map to the
  active BOM's two-count `C1` line, `R1A`/`R1B` map to the active BOM's
  two-count `R1` line, `C5`/`C6` are deliberate no-BOM DNI shunt-cap
  placeholders, and `TP1`-`TP9` are deliberate no-BOM service/test pads.
- `PCB1` and `ENCL1` remain active BOM rows without KiCad assembly placements
  because they are the board and enclosure deliverables, not sourced SMT/THT
  placements.
- The generated outputs are suitable for local review of the DRC-clean source
  packet and for first board-house intake. They are not proof that the hub can
  be built without connector-retention and host-clearance evidence.
- Generated KiCad timestamp fields are normalized to fixed values, and the
  review archive uses deterministic ZIP metadata, so repeated runs of the same
  source packet can be compared by hash.

## File Hashes

| SHA-256 | File |
|---------|------|
| `7ec7ce5c575900a99c1fc7b870030caa6b71841f194a7b015f00eae5c0f76e20` | `gerbers/usb_hub-F_Cu.gtl` |
| `f64fc33dc8a6ace808db69bf5e639b5afbc22eb1f5d40dfa16a0fa483ea93def` | `gerbers/usb_hub-B_Cu.gbl` |
| `58c042ac4c386b823d7a63b8eb4668569728dc9a8b92bf13986a465060226229` | `gerbers/usb_hub-F_Paste.gtp` |
| `e2dbc8ab647495ce92e66bb20b3f08866e83f4d75bacaf7e21d4e8b212d57947` | `gerbers/usb_hub-B_Paste.gbp` |
| `673f4cd8453bb5344787e80b13030b90c74e879c3ef21bc3967b8557671147d9` | `gerbers/usb_hub-F_Silkscreen.gto` |
| `8887ab97fe9d61673ad474881a0510648187c471a8aab9f8cc77bdc03dfefdf5` | `gerbers/usb_hub-B_Silkscreen.gbo` |
| `37d3c7a1f5dea17a52ecf62d924b5cc7f966c34fb504973634373cb786bec036` | `gerbers/usb_hub-F_Mask.gts` |
| `30c9c3ccd725104e876ccf1b3b4bcf2ad9801b80f6ee19d68855be0b9874cd82` | `gerbers/usb_hub-B_Mask.gbs` |
| `eb4c0115734d8ee4db719e8d044bed0ff307e58b00fcf409c867f5917a4f2c64` | `gerbers/usb_hub-Edge_Cuts.gm1` |
| `8be44af677e374c4eb5ccf2acab509193501964872d8bc89aa8078e90e48f7ca` | `gerbers/usb_hub-job.gbrjob` |
| `24fe9660a0d54fa90893875895a18fdf6dbc046a31bfcb63be17e50fd303dffb` | `drill/usb_hub-PTH.drl` |
| `1ebc9f7c691f06ae8301a36207d9239e5fb763e5fa090bd7b9317c54f1995b32` | `drill/usb_hub-NPTH.drl` |
| `f414b7f44a83bf0fb8544272d62c6b846ded00a448a08a4dbf6c1e962365bcf5` | `drill/usb_hub-PTH-drl_map.gbr` |
| `b357b4cfa5db0ad50a9039c507e943787e5d677e64406e55fa5d4baaf3acaabf` | `drill/usb_hub-NPTH-drl_map.gbr` |
| `e9f366a98dd9d6c2510bbdfcaaae50ff480d710273e2135ba27ab353ebfa0741` | `drill/usb_hub-drill-report.txt` |
| `e0887a045ccde230de036592fb4a1bad88aea6139d625a0c85ba46683b4f9338` | `assembly/usb_hub-pos.csv` |
| `936d16e69f6d6cfd46a6c306d1764a38ffe3570c16ee737deb0740cb3a94db3f` | `assembly/usb_hub-active-bom-pos-review.csv` |
| `b806f5e5e370a9b678925b6676128f60fc24b42decf9d6c06d4a86f47a9d438c` | `assembly/usb_hub-active-bom-pos-review.md` |
