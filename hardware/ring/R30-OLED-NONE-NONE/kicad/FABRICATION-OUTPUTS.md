<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Fabrication Output Review

This note records the local fabrication-output proof for the DRC-clean routed
R30 first-board KiCad source. The generated files are intentionally under
`build-kicad/` and are not checked in.

This is not physical hardware evidence. It does not prove stackup height,
focal distance, RF behavior, service access, click feel, battery replacement,
yield, realized COGS, or factory acceptance.

Use `BOARD-HOUSE-OUTPUT-CONSTRAINTS.md` to turn these local generated outputs
into explicit prototype-house / Seeed-style intake questions before asking for
a ring PCB fab or assembly quote.

## Current Local Output

Output directory:
`build-kicad/R30-OLED-NONE-NONE/fabrication-review/`

Review archive:
`build-kicad/R30-OLED-NONE-NONE/r30-oled-none-none-fabrication-review.zip`

Archive SHA-256:
`0dd63a2361fecc7fd9870917a8fb1c3dbeec75227a1099f392a53a230516c52c`

Generated output classes:

| Class | Files | Notes |
|-------|-------|-------|
| Gerbers | 12 | `F.Cu`, `In1.Cu`, `In2.Cu`, `B.Cu`, paste, silkscreen, mask, `Edge.Cuts`, and Gerber job file |
| Drill | 5 | Split PTH/NPTH Excellon plus Gerber drill maps and drill report |
| Assembly review | 3 | Position CSV plus active-BOM/POS review CSV and markdown report; no warning-bearing KiCad schematic BOM is used |

The drill report records a four-layer copper stack (`F.Cu`, `In1.Cu`,
`In2.Cu`, `B.Cu`), 40 plated through holes, and 0 unplated through holes.

## Commands

Run the KiCad verifier first:

```bash
scripts/verify-firmware-local.sh --kicad-only
```

Generate the review outputs:

```bash
mkdir -p \
  build-kicad/R30-OLED-NONE-NONE/fabrication-review/gerbers \
  build-kicad/R30-OLED-NONE-NONE/fabrication-review/drill \
  build-kicad/R30-OLED-NONE-NONE/fabrication-review/assembly

kicad-cli pcb export gerbers \
  --output build-kicad/R30-OLED-NONE-NONE/fabrication-review/gerbers \
  --layers F.Cu,In1.Cu,In2.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts \
  hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb

kicad-cli pcb export drill \
  --output build-kicad/R30-OLED-NONE-NONE/fabrication-review/drill \
  --format excellon \
  --excellon-units mm \
  --excellon-zeros-format decimal \
  --excellon-separate-th \
  --generate-map \
  --map-format gerberx2 \
  --generate-report \
  --report-path build-kicad/R30-OLED-NONE-NONE/fabrication-review/drill/r30_oled_none_none-drill-report.txt \
  hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb

scripts/generate-r30-assembly-review.py

cd build-kicad/R30-OLED-NONE-NONE/fabrication-review
zip -r ../r30-oled-none-none-fabrication-review.zip gerbers drill assembly
```

## Review Caveats

- `scripts/generate-r30-assembly-review.py` intentionally avoids the KiCad
  schematic BOM export and merges `hardware/bom/R30-OLED-NONE-NONE.csv` with
  the KiCad POS export instead. This preserves board refs such as `C1A`, `R2A`,
  and `TP_VBAT` without `?` suffixes.
- The source-controlled active BOM contract remains
  `hardware/bom/R30-OLED-NONE-NONE.csv`. The generated active-BOM/POS review has
  no open rows: board-real populated refs match placements, `Q1` plus
  `TP_CHRG`/`TP_MOT`/`TP_RST`/`TP_VBAT`/`TP_VBUS` are deliberate qty-0
  service/copper placements, `LED1` is an unplaced qty-0 fallback, and the
  remaining no-placement rows are off-board/mechanical items (`ANT1`, `BT1`,
  `LENS1`, `PAD1`, `PCB1`, `RIM1`, `SHELL1`).
- A Gerber command with `--subtract-soldermask --check-zones` crashed locally
  with KiCad CLI `10.0.2` / macOS Swift `Array index out of range`. The retained
  command omits those options and generated the listed outputs successfully.
- The generated outputs are suitable for local review of the DRC-clean source
  packet, not as proof that the ring should be quoted or built before physical
  fit and stackup evidence is recorded.

## File Hashes

| SHA-256 | File |
|---------|------|
| `7a69d08d588c3d44183a9fe5ad95af7bee1a6bc00e66459178d0e6ac655014ec` | `gerbers/r30_oled_none_none-F_Cu.gtl` |
| `5d08c319d83ca1d450be078c8690490d21bbcc64fd5f66d180df0d6fbf608181` | `gerbers/r30_oled_none_none-In1_Cu.g1` |
| `a36a34270c4f71c391132e0446365709ca56d4121f1507c121573e84ceb1ba36` | `gerbers/r30_oled_none_none-In2_Cu.g2` |
| `ed4bc973d04563406b48429f22f39a1d862c53e020205b991463edf2145c54f9` | `gerbers/r30_oled_none_none-B_Cu.gbl` |
| `d4bb34d897344d2a1c9cf54883423748486b31fa3e82a897e61cf559d2b15da8` | `gerbers/r30_oled_none_none-F_Mask.gts` |
| `ee1b4df2383b1e6474bba8e991bf75c026a41a96597ee5adfcd59c1ccce1bbbc` | `gerbers/r30_oled_none_none-B_Mask.gbs` |
| `2c47b0ad803bf19b074de4e61c7d7b84a7b2d43f6e4b559ebc05785c8a877418` | `gerbers/r30_oled_none_none-F_Paste.gtp` |
| `b91aa013a68ecf566078f226441bc6e51543f68688e6865e997bae917bc47935` | `gerbers/r30_oled_none_none-B_Paste.gbp` |
| `04f520ae80d33f3d1434645152f5711b201bc04e0a1166fc3f3f9bf047c00d33` | `gerbers/r30_oled_none_none-F_Silkscreen.gto` |
| `509620e1aea6c0706672587a6c2684f9fdfee3206186b9af87c187a4fa0e11e5` | `gerbers/r30_oled_none_none-B_Silkscreen.gbo` |
| `378ef1379fddcf141dc49511185ca8cff32fa754f0bef1cc5c976eef02a03d39` | `gerbers/r30_oled_none_none-Edge_Cuts.gm1` |
| `47d14b7cb35b6e3ce0f55f3280d9711a942f4952fcab1c0d44bfe24c7866784d` | `gerbers/r30_oled_none_none-job.gbrjob` |
| `7bc2ab58fdb86153e3e2fcde93aacc57ad913a992498dcfc7c0818777f70a209` | `drill/r30_oled_none_none-PTH.drl` |
| `af0aa0b41bf6f4ef622c595b328159efc710b0152bc506aafcba8f4187a22290` | `drill/r30_oled_none_none-NPTH.drl` |
| `b365d113e77d8e8a164867600d27a0f4465b71cb21e1d33577e6c03e8d60c09c` | `drill/r30_oled_none_none-PTH-drl_map.gbr` |
| `ad3b9db331a954e5ae99429874c1a536cbabf4ab1967ae98840c3d95a6ea57e8` | `drill/r30_oled_none_none-NPTH-drl_map.gbr` |
| `7e05bbb486ec6e016c19d9719037613edc13649e723b9faaa66b6f22a49c66f2` | `drill/r30_oled_none_none-drill-report.txt` |
| `574c2af18e1c92f41e4ea164cd6bfd554a9a61d4e9c19c1cfbc0738ee30a6b72` | `assembly/r30_oled_none_none-pos.csv` |
| `ce69888d2e9718128bba37fa1b6c14a6e7ebd9ec885c0cfe78c57c3ab3a3dc88` | `assembly/r30_oled_none_none-active-bom-pos-review.csv` |
| `c8c3fe7b1dd44e7d0db0159b3101019e319af383abd6ce2a6b74f69c1081c778` | `assembly/r30_oled_none_none-active-bom-pos-review.md` |
