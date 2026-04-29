<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# Agent Tooling

Conventions and recommended tools for agent-driven work in this repository.
Light-touch — only items proven by an in-tree trial land here. Promotion to
`CLAUDE.md` requires a recurring use-case, not a one-shot success.

## KiCad schematic editing

**Recommended driver:** `kicad-sch-api` (PyPI, MIT). Validated against R30
`power_and_charge` sub-sheet — see commits `f47aa99`, `a6a2c13`, `e1cb484`,
`0641629` on `main` for the trial commits, and `trial-scripts/` for the
driver scripts that produced them.

The library handles `lib_symbols` aggregation correctly (a defect that
disqualified `kicad-mcp-pro` v3.1.3) and exposes a clean Python surface for
populating components, wiring nets via labels, attaching power flags, and
exporting netlists/BOMs. KiCad 10 + stock symbol/footprint libraries are
expected to be installed locally; no cloud calls.

### How to invoke

Install once (Python 3.13 — there is a known `io.json_loader` import bug on
Python 3.14 in a sibling tool, `circuit-synth`):

```bash
uv tool install --python 3.13 kicad-sch-api
```

Drive the library from a script (preferred over MCP transport for
agents — fewer moving parts, the same surface):

```python
import kicad_sch_api as ksa
sch = ksa.load_schematic("path/to/sheet.kicad_sch")
sch.components.add(
    lib_id="Device:R",
    reference="R42",
    value="100k",
    position=(80, 90),
    footprint="Resistor_SMD:R_0402_1005Metric",
    mfg_part_num="…",
    manufacturer="…",
    lcsc="…",
)
sch.add_label("VBAT_SENSE", position=sch.get_component_pin_position("R42", "2"))
sch.save()
```

### Gotchas surfaced by the trial

- `components.add` does not auto-fill the symbol's default `Value` from
  the lib_symbol — pass `value=` explicitly. Power symbols
  (`power:GND`, `power:PWR_FLAG`) need `value="GND"` / `value="PWR_FLAG"`
  set or KiCad ERC reports `multiple_net_names`.
- The reference-format validator rejects `C1_alt`-style refs. Use
  standard `<letter><digits>` form. Resolve BOM CSV
  multi-instance-per-ref oddities by splitting the BOM row.
- `add_label` places a label at exactly the coordinate passed. Compute
  the actual pin position via `get_component_pin_position(ref, pin)`
  before placing the label — symbol bodies offset the pin from the
  placement origin.
- Project-local `sym-lib-table` and `fp-lib-table` are required to
  silence kicad-cli ERC's `lib_symbol_issues` and `footprint_link_issues`
  warnings. Reference `${KICAD10_SYMBOL_DIR}` and `${KICAD10_FOOTPRINT_DIR}`.

### MCP server (optional)

`kicad-sch-api` ships a bundled MCP server, `kicad-sch-mcp`, callable from
`/Users/<you>/.local/bin/kicad-sch-mcp`. Registering it in
`.claude/settings.json` would let Claude Code drive the library through the
MCP transport rather than via a Python script. The trial did not register
the server (registration requires a session restart) and confirmed that
direct Python use is sufficient. Register only if a recurring use-case
prefers MCP-mediated calls.

### When NOT to use this stack

- Authoring custom KiCad symbols (`.kicad_sym` files) is out of scope —
  hand-author those, then load via `lib_id`.
- PCB-side editing (footprint placement, routing, DRC) is out of scope —
  use KiCad GUI or `kicad-cli pcb` directly.
- Compiling a circuit from a higher-level DSL — the `circuit-synth`
  Python DSL exists on top of `kicad-sch-api` but is deferred until a
  recurring need surfaces. The MCP-layer trial showed that the lower
  library is at the right abstraction level for surgical edits.

## Other agent tooling

None yet. Keep this list short — every entry is a thing future
contributors must learn. Trial-then-land, not speculate-then-land.
