# CLAUDE.md — PowerFinger

## Project Overview

PowerFinger is an open-source assistive input device family: fingertip rings and
pen-shaped wands that function as surface-agnostic BLE HID mice/controllers.
Hardware designs under CERN-OHL-S 2.0, firmware/software under MIT.

Repository: https://github.com/fluffyrabbot/powerfinger

## Hard Rules

- **BDFL-led.** When uncertain about design choices, ask. Always provide a
  recommendation with reasoning.
- **Accessibility-first.** Every design decision must work for users with limited
  mobility, not just able-bodied desk users. If a feature helps gamers but hurts
  accessibility, cut it.
- **BOM ceiling.** No single variant may exceed $25 BOM at prototype scale. The
  cheapest ring variant must stay under $10.
- **License compliance.** All hardware files: CERN-OHL-S 2.0. All firmware/software:
  MIT. Never introduce proprietary dependencies. Never use a permissive hardware
  license — the reciprocal requirement and patent retaliation clause are load-
  bearing for the project's IP defense strategy.
- **No cloud dependency.** Every device must function fully offline. Cloud/API
  features (LLM for OCR, etc.) are optional enhancements, never requirements.
- **No planned obsolescence.** Every component must be replaceable. Design files
  must include assembly/disassembly instructions. Use commodity parts with
  multiple source vendors.
- **Surface agnosticism is a spectrum.** Direct optical doesn't work on glass —
  that's fine, document it honestly. Don't overclaim. The combinatorics matrix
  tracks surface compatibility per variant.
- **No time estimates.** Never add timelines or duration projections to docs or
  design documents.
- **Explore agents use Opus.** Always set `model: "opus"` when spawning Explore
  subagents. Smaller models are prone to inference and imprecision.

## Repository Structure

```
powerfinger/
  README.md              — Mission, philosophy, quick start
  CLAUDE.md              — This file (agent instructions)
  LICENSE-HARDWARE       — CERN-OHL-S 2.0 (hardware)
  LICENSE-SOFTWARE       — MIT (firmware, companion app)
  docs/
    COMBINATORICS.md     — Full form factor x sensing x options matrix
    IP-STRATEGY.md       — Patent landscape, defense strategy, prior art
  hardware/
    ring/                — Fingertip ring designs (by angle, by sensor)
    wand/                — Pen/wand designs
    shared/              — Common components (BLE module, battery, etc.)
  firmware/
    ble-hid/             — ESP-IDF BLE HID mouse profile
    sensors/             — Sensor drivers (Hall, optical, laser)
    power/               — Power management, sleep/wake
  companion/             — Phone/laptop companion app (Flutter or PWA)
```

## Build & Dev Commands

*Not yet established.* When firmware development begins:
- Target MCU: ESP32-C3 (ring, wand basic) or ESP32-S3 (camera variants)
- Framework: ESP-IDF (Apache 2.0)
- Build: `idf.py build`
- Flash: `idf.py -p /dev/ttyUSBx flash`

## Key Technical Constraints

- **BLE HID latency < 15ms.** ESP32-C3 supports 7.5ms connection intervals.
- **Ring height budget: ~10mm** finger-to-surface. Sensor + PCB + battery must
  fit in ~7–8mm above surface with 2.5–3mm air gap below.
- **Optical sensor focal distance: 2.4–3.2mm.** Ring needs standoff feet or
  raised rim to maintain this.
- **Power target:** 1–2 weeks battery life on 80–100mAh (ring), 2–4 weeks on
  100–150mAh (wand).

## Code Smell Checklist (LLM Failure Modes)

**Read source files before writing call sites.** Wrong method names are the #1
first-pass failure.

### General (All Code)

| Smell | Wrong | Right |
|-------|-------|-------|
| Silent error discard | Ignoring return value | Log or propagate every error |
| Magic numbers | `delay_ms(30)` | Named `const` with rationale |
| No timeout on I/O | Raw blocking call | Explicit timeout with named constant |
| Unbounded collect | Collecting user/sensor input without limit | Bounded buffer or `.take(LIMIT)` |
| Flat retry delay | Fixed sleep between retries | Exponential backoff |

### Embedded / Firmware Specific

| Smell | Wrong | Right |
|-------|-------|-------|
| Blocking in ISR | Long computation in interrupt handler | Set flag, process in main loop |
| Heap allocation in hot path | `malloc` in sensor polling loop | Pre-allocated static buffers |
| Polling when interrupts available | Busy-wait on sensor ready | GPIO interrupt + wake from sleep |
| Uncalibrated sensor reads | Raw ADC/Hall values to HID | Calibration offset + dead zone |
| Hard-coded pin assignments | `gpio_set_level(5, 1)` | Named constants or Kconfig |
| No watchdog | Firmware hangs silently | WDT with reset or error state |

### PowerFinger Specific

- Never hard-code DPI/sensitivity — always configurable via BLE characteristic
- Never assume a specific hand size — parameterize ring geometry
- Never assume a specific surface — test on at least: wood desk, glass, fabric,
  paper, glossy magazine, matte plastic
- Never assume single-ring operation — firmware must be stateless enough for
  multi-ring composition via companion app

## Git Workflow

**Atomic commits (critical for parallel agents):** Always stage + commit + push
in ONE bash call:
```bash
git add file1 file2 && git commit -m "type(scope): description" && git push
```
- Never `git add -A` or `git commit -a`. Stage only your files.
- Never `git stash`, `git checkout --`, `git restore` without asking first.
- If push fails: stop and report. Don't pull/merge/rebase/force-push.
- Conventional commits: `type(scope): message`
- Local: work on `main`. Cloud sandbox: feature branches + PRs.

## Codex Orchestration (Claude → GPT 5.4)

Claude's role is **ideation, research, architecture, and orchestration**.
Implementation delegates to Codex with GPT 5.4 for precision. Use the Bash tool
to invoke Codex non-interactively:

```bash
# Standard delegation (sandboxed, no approval prompts):
codex exec --full-auto --model gpt-5.4 "<scoped implementation prompt>"

# Unsandboxed (when network/system access needed):
codex exec --dangerously-bypass-approvals-and-sandbox --model gpt-5.4 "<prompt>"

# Capture structured output for review:
codex exec --full-auto --model gpt-5.4 --json "<prompt>" | tee /tmp/codex-out.jsonl

# Write final message to file:
codex exec --full-auto --model gpt-5.4 -o /tmp/result.md "<prompt>"
```

**Prompt construction for delegation:**
- Scope precisely: name exact files to read/modify, tests to run, boundaries
  not to cross
- Include hard rules inline: repeat BOM ceiling, license constraints, no cloud
  dependency in the prompt
- End with: "Run `idf.py build` to verify. Stop when complete."
- Do NOT assume Codex reads CLAUDE.md — repeat critical constraints in the prompt

**After Codex returns:** Always review output (`git diff`), verify no code smell
checklist violations, then commit atomically per Git Workflow above.

**Execution model:** Codex handles long-horizon prompts well. Always invoke via
Bash with `run_in_background: true` (no timeout cap, automatic completion
notification). Wait for the notification before reviewing output — don't poll or
sleep. Do parallel work only when explicitly parallelizing.

## Defensive Publication

Every hardware design committed to this repo constitutes a defensive publication
establishing prior art. Commit messages for new designs MUST include:
- The specific sensing mechanism
- The form factor and angle
- The intended use case (pointing, OCR, accessibility)
- The BOM estimate

This creates timestamped, enabling disclosure searchable by patent examiners.

## Design Philosophy References

- RFC-1318 (MeSH repo): OSS OCR thimble/wand scanner — sibling concept, OCR
  pipeline architecture
- RFC-1319 (MeSH repo): Fingertip ring mouse — full sensing analysis, IP
  landscape, multi-ring paradigm

## Pointers

- Patent landscape: `docs/IP-STRATEGY.md`
- Design matrix: `docs/COMBINATORICS.md`
- CERN-OHL-S 2.0: https://opensource.org/license/cern-ohl-s
- ESP-IDF BLE HID: https://github.com/espressif/esp-idf/tree/master/examples/bluetooth
