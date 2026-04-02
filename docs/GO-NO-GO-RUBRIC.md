# Go / No-Go Validation Rubric

Concept-soundness and productization gates for PowerFinger. This document turns
the existing design, firmware, safety, and regulatory docs into a blocking
validation sequence.

References:
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)
- Surface test protocol: [SURFACE-TEST-PROTOCOL.md](SURFACE-TEST-PROTOCOL.md)
- Battery safety: [BATTERY-SAFETY.md](BATTERY-SAFETY.md)
- Regulatory pre-scan: [REGULATORY-PRESCAN.md](REGULATORY-PRESCAN.md)
- Vendor verification: [VENDOR-VERIFICATION.md](VENDOR-VERIFICATION.md)
- Multi-ring protocol: [MULTI-RING-PROTOCOL.md](MULTI-RING-PROTOCOL.md)
- Wand competitive analysis: [WAND-COMPETITIVE.md](WAND-COMPETITIVE.md)

---

## How to Use This Rubric

- Work gates strictly in order. Do not spend meaningful time on downstream
  scope while the current gate is red.
- A gate is **GO** only when every "Must prove" item is satisfied.
- A single **red flag** is enough to pause and re-scope, even if other items
  look good.
- "Nice to prove" items increase confidence but do not unblock the next gate.
- "Deferred" items do not count as progress until Gate 6.

---

## Scope Freeze

This rubric assumes two active lanes only:

- **Validation lane:** `R30-OLED-NONE-NONE` ring pair plus `USB-HUB`
- **Hedge lane:** `WSTD-BALL-NONE-NONE` wand

Deferred until Gate 6:

- Pro optical-on-ball ring
- Piezo + haptic click
- IMU hybrids
- OCR / camera features
- Nail mount, wrist bracelet, tethered sensor
- Rich companion-app work beyond basic configuration

---

## Gate 0 — Product Claim Freeze

**Question:** What exact claim is being validated first?

**Must prove**

- One sentence can describe the primary claim without mentioning future
  variants.
- The repo consistently treats the optical ring pair plus hub as the primary
  concept-validation lane.
- The wand is explicitly framed as a hedge path, not a second primary program.

**Pass criteria**

- The primary claim is: "A user with limited wrist mobility can browse, click,
  drag, and scroll on everyday surfaces without host-side remapping software."
- The active lanes are frozen in writing: optical ring pair + hub for novelty,
  wand for productization hedge.
- No P1 / Pro / OCR feature is described as blocking for the first proof.

**Red flags**

- The story keeps shifting between ring pair, wand, OCR stylus, air mouse, and
  broader controller family.
- Success cannot be stated in one sentence.
- The first product depends on features that are still unproven or optional.

**If this gate fails**

- Stop feature expansion.
- Rewrite scope docs and README language before doing more hardware or firmware
  work.

---

## Gate 1 — Single-Ring Human Control Loop

**Question:** Is one ring actually usable by a real person before composition?

**Must prove**

- One ring can move cursor, single-click, double-click, and click-drag.
- The ring stays indexed on the finger under realistic click and drag force.
- The user can don, doff, and tolerate it for a real session without heroic
  compensation.

**Pass criteria**

- A tester can complete a basic browsing task with one ring: open a link, close
  a tab or window, and select text.
- On at least one best-case rigid surface and one accessibility-relevant rigid
  surface, the ring is **PASS** on the surface protocol's key metrics:
  latency `< 20 ms`, dropout rate `< 2%`, cumulative jitter `< 5 px`, mean click
  displacement `< 2 px`, click failure count `0 / 50`.
- On additional rigid surfaces in the intended use envelope, results are at
  worst **MARGINAL**, not **FAIL**.
- Click suppression works well enough that clicking does not visibly jump the
  cursor off target.

**Red flags**

- The user must press unnaturally hard, flatten the finger unnaturally, or
  consciously counter-rotate the device to use it.
- The ring shifts position during click or drag.
- Filtering is so aggressive that it hides drift only by destroying control.
- The ring is only usable in a demo posture on one ideal surface.

**Nice to prove**

- BLE-configurable DPI and dead-zone tuning materially improve control for
  different users.

**Deferred**

- Second ring workflow
- Gesture system
- Companion app UX

**If this gate fails**

- Rework shell geometry, click mechanics, retention, or sensor mounting before
  doing more hub or app work.

---

## Gate 2 — Ring Package Closure

**Question:** Can the ring package close without violating geometry, repair, or
RF constraints?

**Must prove**

- The real package fits sensor, battery, BLE module, antenna keep-out, charging
  hardware, and shell structure inside the height budget.
- The optical focal distance or ball contact geometry survives real pressure and
  shell tolerances.
- The shell remains serviceable and non-hermetic.

**Pass criteria**

- A real stackup or physical mockup with representative components fits within
  the documented `~10 mm` finger-to-surface budget.
- The antenna keep-out and module placement work without violating
  [REGULATORY-PRESCAN.md](REGULATORY-PRESCAN.md).
- The shell can be opened for battery replacement and reclosed without
  destructive rework.
- Optical variants maintain focal distance mechanically instead of relying on
  user technique.

**Red flags**

- The package only closes if the antenna keep-out is compromised.
- The design requires glue-sealed, non-serviceable assembly.
- Battery swelling, pressure release, or replacement is not physically
  accommodated.
- Tolerance stack is so tight that sensor performance depends on print luck.

**Nice to prove**

- Parametric CAD covers finger circumference and angle cleanly from one design.

**Deferred**

- Nail-mount miniaturization
- Rigid-flex optimization for future variants

**If this gate fails**

- Do not push ring complexity further.
- Make the wand the primary productization path until the ring package is
  credible.

---

## Gate 3 — Safety, Power, and RF Reality

**Question:** Does the actual body-worn electrical system survive contact with
physics, skin, and radio constraints?

**Must prove**

- Charging is thermally safe in the real enclosure.
- Measured current draw supports a believable prototype battery life.
- Body-worn BLE connectivity works at the low-power settings needed to stay out
  of expensive SAR trouble.

**Pass criteria**

- Charging at `50 mA` stays below the documented thermal limits:
  cell `< 45 deg C`, skin-contact shell `< 48 deg C`, no reliance on user
  restraint to remain safe.
- Low-voltage cutoff, charge disable, and fault handling work in enclosure-level
  tests.
- Measured active / idle / deep-sleep current is recorded and published.
- Prototype battery life is consistent with the repo's stated expectations, or a
  deliberate architecture change is triggered immediately.
- BLE link is stable ring-to-host and ring-to-hub at `0 dBm`, or there is a
  defensible reason to accept the SAR/cost consequence of higher power.

**Red flags**

- Actual battery life collapses below one workday.
- Charging approaches thermal limits even at the specified `20 kohm` resistor.
- BLE only works reliably by pushing TX power above the easy exemption path.
- Deep-sleep or reconnect behavior is too fragile for a wearable.

**Nice to prove**

- Diagnostic telemetry for battery health, charge faults, and reconnect issues.

**Deferred**

- Formal certification testing
- Consumer-generation MCU migration

**If this gate fails**

- Change the electrical architecture before more compliance, app, or enclosure
  polish.

---

## Gate 4 — Two-Ring Composition

**Question:** Does the novel part of the concept work as a dependable product,
not just a protocol demo?

**Must prove**

- Two identical rings plus hub complete the core mouse workflow.
- The host sees one standard HID mouse with no host-side remapping software.
- Disconnect, reconnect, and bond-loss paths never strand the user in a broken
  state.

**Pass criteria**

- A tester can complete the full workflow through the hub: move, left-click,
  right-click, scroll, and click-drag.
- The host sees one mouse and requires no input interception or helper app.
- Role assignment persists across reconnects and remains understandable to the
  user.
- Disconnect and bond-reset handling satisfies the accessibility invariants in
  [MULTI-RING-PROTOCOL.md](MULTI-RING-PROTOCOL.md): no stuck buttons, no stale
  resurrection, no unsafe role confusion.

**Red flags**

- The user must think constantly about which ring is which for basic tasks.
- Scroll is technically present but practically worse than keyboard or trackpad
  alternatives.
- Any disconnect can leave a stuck button or broken role state.
- Multi-ring only works in an ideal lab setup and not as a repeatable daily
  workflow.

**Nice to prove**

- Atomic role swapping
- Minimal serial configuration path

**Deferred**

- Third and fourth ring roles
- Rich gesture tables
- Companion app orchestration

**If this gate fails**

- Do not force multi-ring into v1.
- Ship a single-device product first or pivot the first product to the wand.

---

## Gate 5 — Buildability and Supply-Chain Resilience

**Question:** Can this be built twice without surprise respins or unbuyable
parts?

**Must prove**

- Every P0 line item is either multi-source or has an explicit footprint-safe
  fallback.
- Known BOM caveats are corrected before schematic freeze.
- The assembly path matches the realities of low-volume builds.

**Pass criteria**

- The optical ring and hub BOMs are reviewed against
  [VENDOR-VERIFICATION.md](VENDOR-VERIFICATION.md) before PCB lock.
- Core risky parts have an explicit answer:
  `ESP32-C3-MINI-1-N4X` or equivalent successor is planned,
  optical sensor fallback path is documented,
  false "drop-in alt" claims are removed from BOM notes.
- Cells, sensors, domes, charge ICs, LDOs, and connectors are sourceable in
  prototype quantity from named vendors with a second path where needed.
- PCB footprints reflect reality rather than optimistic assumptions about
  pin-compatible substitutes.

**Red flags**

- A blocking part is only buyable through one sketchy channel with no fallback.
- The BOM still claims non-pin-compatible parts are drop-ins.
- A board respin would be required the moment a listed alternate is used.
- The build depends on hand-tuned sourcing luck rather than a repeatable BOM.

**Nice to prove**

- A small pilot assembly quote or mock build confirms manufacturability.

**Deferred**

- Volume cost-down work
- Second-generation board consolidation

**If this gate fails**

- Revise BOMs and footprints before fabrication.
- Do not call the design repairable or multi-source until it actually is.

---

## Gate 6 — First Product Decision and Claim Honesty

**Question:** Which thing is the first product, and what claims are true enough
to publish?

**Must prove**

- Exactly one first product path is chosen.
- User-facing claims match tested surfaces, battery life, and workflow reality.
- Assembly and disassembly are documented, not implied.

**Pass criteria**

- The repo and public-facing copy pick one first product story:
  ring pair + hub **only if** Gates 1-4 are clean, otherwise wand first.
- Surface compatibility claims are limited to tested results from
  [SURFACE-TEST-PROTOCOL.md](SURFACE-TEST-PROTOCOL.md).
- Replacement path exists for battery, shell, and wear parts.
- README, build spec, and outreach language speak with one voice about what is
  real now versus later.

**Red flags**

- The marketing story still depends on Pro, IMU, OCR, or other deferred work.
- The docs imply glass, skin, or tremor success before those claims are tested.
- Repairability is asserted without actual disassembly instructions.
- The project presents a family vision as if it were a finished first product.

**Nice to prove**

- Minimal companion app for DPI, role assignment, and firmware update only.

**Deferred**

- OCR camera
- Piezo + haptic click
- IMU hybrid tracking
- Nail mount, bracelet, tethered sensor
- Optical-on-ball Pro ring

**If this gate fails**

- Freeze outward-facing claims and narrow them to what the bench truth supports.

---

## Pivot Rules

- If Gate 1 fails: stop hub and app expansion. Fix human control first.
- If Gate 2 fails: make the wand the first productization path.
- If Gate 3 fails: change the electrical architecture before more polish.
- If Gate 4 fails: do not ship multi-ring as v1 just because it is the novel
  idea.
- If Gate 5 fails: do not freeze schematics or claim multi-source resilience.
- If Gate 6 fails: reduce the story until it matches tested reality.

The intended outcome is not "prove everything at once." It is to prevent
downstream work from laundering upstream uncertainty into false confidence.
