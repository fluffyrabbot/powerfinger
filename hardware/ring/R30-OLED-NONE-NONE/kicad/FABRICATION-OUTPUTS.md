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
`b8ae6ac7eff9eb49f7b0646b847393fa6bf92b0e248472ba0bca23a7efeedb10`

Generated output classes:

| Class | Files | Notes |
|-------|-------|-------|
| Gerbers | 12 | `F.Cu`, `In1.Cu`, `In2.Cu`, `B.Cu`, paste, silkscreen, mask, `Edge.Cuts`, and Gerber job file |
| Drill | 5 | Split PTH/NPTH Excellon plus Gerber drill maps and drill report |
| Assembly review | 3 | Position CSV plus active-BOM/POS review CSV and markdown report; no warning-bearing KiCad schematic BOM is used |

The drill report records a four-layer copper stack (`F.Cu`, `In1.Cu`,
`In2.Cu`, `B.Cu`), 53 plated holes and zero unplated holes.

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
- The assembly review checks the key electrical endpoints from the schematic netlist before matching positions. The active BOM is `hardware/bom/R30-OLED-NONE-NONE.csv`. LED1 coordinates identify two wire terminations for an external emitter. The matched lens, holder and emitter positions require supplier drawings. Check kit quantities and component prices separately from placement matching.
- A Gerber command with `--subtract-soldermask --check-zones` crashed locally
  with KiCad CLI `10.0.2` / macOS Swift `Array index out of range`. The retained
  command omits those options and generated the listed outputs successfully.
- The generated outputs are suitable for local review of the DRC-clean source
  packet, not as proof that the ring should be quoted or built before physical
  fit and stackup evidence is recorded.

## File hashes

| SHA-256 | File |
|---|---|
| `cd0b2c30b0a0c2ecccf395e11536b1299cda43119af9c292007317b281fd4f73` | `assembly/r30_oled_none_none-active-bom-pos-review.csv` |
| `cc4a0c045cb100f4dd41128c864085aaa5de3f25913349097b7e188a3725ea6e` | `assembly/r30_oled_none_none-active-bom-pos-review.md` |
| `3a561dc5f5f7d7a2342724f51e2444e0c63dadbe1aac2d2e4bc1b32900d6df7f` | `assembly/r30_oled_none_none-pos.csv` |
| `78de7ad1dd100c5e2155c1c6cc4aa02d6eebd696f7fbeca304cf0cc4248342ba` | `drill/r30_oled_none_none-NPTH-drl_map.gbr` |
| `dbf79c70e318e4a73b2cb6fc450ab251b49fb988156730e2af862beb732c81e0` | `drill/r30_oled_none_none-NPTH.drl` |
| `b1a378ec8c6ac0987bda486263492a1aab39c26f16dd3127da4a9c02f886156d` | `drill/r30_oled_none_none-PTH-drl_map.gbr` |
| `52692d41091d3645fd0824b39dfc265973e644879c81d478be422b301990dab3` | `drill/r30_oled_none_none-PTH.drl` |
| `cf8abf3a95753f7612c036e58fd2ad4e802c2209adc5d0b2466ea626f869fcce` | `drill/r30_oled_none_none-drill-report.txt` |
| `b641807d171c34202e8dd303a3ee5337ce82dea6a522dc95834d8f13f434caa3` | `gerbers/r30_oled_none_none-B_Cu.gbl` |
| `61d62346a48b308f6aca352c14c7b71e89756e859646f4e3ef3b2f48f984d35a` | `gerbers/r30_oled_none_none-B_Mask.gbs` |
| `9386e7a8c21641e530bf06dc0021a583b48bb807fbf8ee556ab996adb8b5b5f8` | `gerbers/r30_oled_none_none-B_Paste.gbp` |
| `7f780ed7ddf1dd161d778b8cb9d77d9c5235a1c2dc6000a5c415ea6197b2c2b3` | `gerbers/r30_oled_none_none-B_Silkscreen.gbo` |
| `155e04e6a806bd5f907a8a9576231e4edf2a1a7b11483123f919d8f843a39252` | `gerbers/r30_oled_none_none-Edge_Cuts.gm1` |
| `06cc2ce0766016ee5a8c290f4e931c5edc09d6da87e84d8602a614987b059446` | `gerbers/r30_oled_none_none-F_Cu.gtl` |
| `5a660bd70b7ce7c787e4902fa899a62b0f5e1d6471a793c2388711085f8a0593` | `gerbers/r30_oled_none_none-F_Mask.gts` |
| `8efd7160652fef3c6673f62d60bceb0658ba20f16bd2fea7804994b1a66b1cbc` | `gerbers/r30_oled_none_none-F_Paste.gtp` |
| `2c6d08ac508be365ce133cead922b718779ee79470d2213c77b45d975de1fd5d` | `gerbers/r30_oled_none_none-F_Silkscreen.gto` |
| `bce67c3abc2a670ce316014d28509e50037dde1d8da701b6c02aea738fef0e8e` | `gerbers/r30_oled_none_none-In1_Cu.g1` |
| `02abf959cf1057b8b31ee8ef1b205bf8878923bc22db545340a403390bdeb407` | `gerbers/r30_oled_none_none-In2_Cu.g2` |
| `a5e8523c7fb84759062d460832bbe1952fe59374bf29bd4268aa776c5176d45f` | `gerbers/r30_oled_none_none-job.gbrjob` |
