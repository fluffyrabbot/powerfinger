# Pre-Hardware Agentic Research Checklist

Research and spec work an LLM agent can meaningfully complete before hardware
prototypes exist. Ordered by impact: items higher on the list unblock hardware
decisions or reduce rework risk. Items lower on the list are valuable but not
blocking.

**Scope rule:** Every item here is completable with datasheets, published
research, existing repo docs, and web sources — no bench measurements, no
physical prototypes. When an item's next meaningful step requires hardware,
it is out of scope for this checklist.

References:
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- IP landscape: [IP-STRATEGY.md](IP-STRATEGY.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)

---

## Tier 1 — Directly Unblocks Hardware or Firmware

These items remove open questions that block PCB layout, shell design, or
firmware completion on dev boards.

### 1.1 TinyUSB HID Integration (Hub)
- [ ] Write TinyUSB HID device driver for ESP32-S3 hub dongle
- [ ] Implement USB HID report descriptor (match BLE report format)
- [ ] Wire into existing event composer output path
- [ ] Add host-side tests for USB report generation
- [ ] Verify with `firmware/test/` mock HAL

**Why first:** The hub's `usb_hid_mouse.c` is the only stub in the ring-to-PC
data path. Completing it makes the full ring → BLE → hub → USB → OS pipeline
testable on dev boards. Pure software — no custom hardware needed.

**Inputs:** TinyUSB ESP-IDF component docs, existing `event_composer.c`,
existing `hid_report_descriptor.c` (BLE side).

### 1.2 Dual-Footprint PCB Feasibility Study
- [ ] Compare PAW3204 and PMW3360 pinouts, footprints, voltage domains
- [ ] Analyze shared vs. divergent routing (SPI bus, motion pin, power rail)
- [ ] Determine whether a single PCB can accommodate both via component stuffing
- [ ] Document constraints an EE must respect if dual-footprint is feasible
- [ ] If infeasible, document the minimum delta between two PCB variants

**Why:** CONSUMER-TIERS.md open question #3. Directly affects whether the EE
designs one board or two. Answering this before schematic capture saves a
potential PCB respin.

**Inputs:** PAW3204DB datasheet, PMW3360DM datasheet (PMS0058 Rev 1.50),
existing BOMs (`R30-OLED-NONE-NONE.csv`, Pro tier BOM in CONSUMER-TIERS.md).

### 1.3 PMW3360 Ball Diameter Optimization
- [ ] Model focal distance geometry for ball diameters 4mm–8mm
- [ ] Map valid focal distances against 10mm ring height budget
- [ ] Determine minimum ball diameter for usable SQUAL at finger-pressure force
- [ ] Cross-reference with shell geometry constraints (sensor cavity volume)
- [ ] Write recommendation with tradeoff analysis

**Why:** CONSUMER-TIERS.md open question #2. The ball diameter constrains the
ring shell CAD, the sensor cavity dimensions, and the glide system. The EE/ME
needs this number before starting the Pro ring shell design.

**Inputs:** PMW3360DM datasheet (focal distance, SQUAL vs. surface quality),
PROTOTYPE-SPEC.md height budget, GLIDE-SYSTEM.md rim geometry.

---

## Tier 2 — Reduces Rework Risk

These items prevent expensive surprises during or after prototyping.

### 2.1 Battery Safety and Charging Spec
- [ ] Research IEC 62133-2 requirements for sub-100mAh LiPo cells
- [ ] Analyze TP4054 charge termination behavior and protection requirements
- [ ] Determine whether BOM cells need external protection MOSFETs or if
      integrated cell protection is sufficient
- [ ] Document thermal runaway risk in a ring form factor (no ventilation,
      skin contact, small thermal mass)
- [ ] Specify charge rate limits for the ring's thermal envelope
- [ ] Write safety spec section for PROTOTYPE-SPEC.md

**Why:** LiPo on a finger. The current BOM specifies TP4054 + battery but
doesn't address protection circuitry, charge termination accuracy, or thermal
constraints in a sealed ring. Getting this wrong is a safety issue, not just a
rework issue.

**Inputs:** TP4054 datasheet, IEC 62133-2 standard summary, LiPo cell
datasheets from BOM suppliers, ring shell thermal constraints.

### 2.2 Regulatory Pre-Scan (FCC / CE / BLE)
- [ ] Determine what ESP32-C3-MINI-1-N4 module pre-certification covers
- [ ] Identify what testing is still required for a finished product
      (intentional radiator, unintentional emissions from PCB)
- [ ] Map FCC Part 15 Subpart C requirements to the ring design
- [ ] Map CE RED (EU Radio Equipment Directive) requirements
- [ ] Estimate certification cost and timeline for a single SKU
- [ ] Identify design constraints that affect certification (antenna keep-out
      zones, ground plane requirements, shielding)

**Why:** Certification requirements impose PCB layout constraints. Learning
these after the PCB is designed means a respin. Module pre-certification covers
the radio, but the finished product still needs testing for unintentional
emissions from the PCB traces, battery, and motor (LRA in Pro tier).

**Inputs:** ESP32-C3-MINI-1-N4 module certification documents (Espressif),
FCC Part 15 Subpart C, CE RED 2014/53/EU, existing ring BOM.

### 2.3 Piezo Click + Haptic Dead Zone Interaction Analysis
- [ ] Research piezo film (PVDF) press-detection thresholds and response curves
- [ ] Analyze DRV2605L waveform library: which waveforms produce minimum
      mechanical coupling to an adjacent optical sensor
- [ ] Model whether LRA vibration at typical haptic amplitudes would register
      as motion on PAW3204 or PMW3360 at their respective SQUAL thresholds
- [ ] Determine optimal `HAPTIC_SUPPRESS_MS` window from first principles
- [ ] Document whether the Pro tier needs a longer dead zone than Standard

**Why:** CONSUMER-TIERS.md open questions #1 and #5. The piezo click is the
Pro tier's highest UX risk. If LRA vibration causes cursor jump that can't be
suppressed by dead zone timing alone, the Pro tier needs a different click
mechanism or a mechanical isolation strategy.

**Inputs:** PVDF piezo film datasheets, DRV2605L datasheet and application
notes, PAW3204/PMW3360 SQUAL documentation, CLICK-MECHANISMS.md.

### 2.4 Claim-Level Patent Mapping
- [ ] Pull full claim text for US8648805B2 (closest ring patent)
- [ ] Map each independent claim element to PowerFinger P0 design
- [ ] Identify which elements PowerFinger practices vs. does not practice
- [ ] Repeat for US20130063355A1, US20150241976A1, US8125448B2
- [ ] Repeat for wand patents (US12353649, expired/abandoned patents)
- [ ] Produce a claim chart suitable for patent attorney review
- [ ] Estimate cost savings for FTO opinion prep

**Why:** IP-STRATEGY.md identifies the patents but doesn't map claims to the
design. A structured claim chart cuts patent attorney hours significantly
($200–400/hr saved). The closest patent (US8648805B2) has had zero enforcement
in 12 years, but "probably fine" is not a defensible IP position. The FTO
opinion itself is blocked on funding — this item structures the work so the
attorney conversation is cheaper when it happens.

**Inputs:** Patent full text (Google Patents / USPTO), IP-STRATEGY.md,
COMBINATORICS.md design descriptions.

### 2.5 Multi-Source Vendor Verification
- [ ] For each line item in all 4 BOMs (`R30-OLED`, `R30-BALL`, `WSTD-BALL`,
      `USB-HUB`), verify availability from at least 2 distributors
- [ ] Identify any single-source components
- [ ] For single-source parts, find drop-in alternates or document the risk
- [ ] Verify that alternate parts listed in BOM "Alternatives" columns are
      actually in stock and pin-compatible

**Why:** CLAUDE.md hard rule: "Every component must be replaceable. Use
commodity parts with multiple source vendors." The BOMs list alternates in a
column, but no one has verified those alternates are real (in stock, same
pinout, same voltage). Discovering a single-source part after PCB layout is
a respin.

**Inputs:** Existing BOM CSVs in `hardware/bom/`, distributor APIs (DigiKey,
Mouser, LCSC).

### 2.6 nRF52840 Migration Spec
- [ ] Compare Zephyr BLE HID vs. nRF Connect SDK BLE HID APIs
- [ ] Map current NimBLE API surface to Zephyr/nRF equivalents
- [ ] Identify which of the 9 HAL interfaces (`hal_ble.h`, `hal_spi.h`,
      `hal_gpio.h`, `hal_adc.h`, `hal_timer.h`, `hal_sleep.h`,
      `hal_storage.h`, `hal_ota.h`, `hal_types.h`) port cleanly vs. need
      rewriting
- [ ] Document SPI, GPIO, ADC, timer, sleep, storage, OTA API differences
- [ ] Estimate firmware migration scope (files changed, LOC delta)
- [ ] Identify any HAL design changes that would ease migration if made now

**Why:** POWER-BUDGET.md concludes ESP32-C3 can't hit consumer battery targets.
The HAL abstraction layer was designed for this migration, but no one has
verified the abstraction actually covers the nRF52840's API surface. Finding
a HAL gap now (while the HAL is young) costs a small refactor. Finding it
after 9 firmware phases costs a rewrite.

**Inputs:** Current HAL headers (`firmware/ring/components/hal/include/`),
Zephyr BLE HID samples, nRF Connect SDK BLE HID samples, nRF52840 datasheet.

---

## Tier 3 — Valuable But Not Blocking

These items improve the project but don't gate hardware or firmware decisions.

### 3.1 Accessibility User Research Synthesis
- [ ] Compile published research on ring-form input devices for motor-impaired
      users (spinal cord injury, ALS, cerebral palsy, essential tremor)
- [ ] Document grip force ranges and finger dexterity profiles for target users
- [ ] Identify relevant tremor filtering algorithms from assistive tech literature
- [ ] Map findings to ring shell geometry parameters and firmware filter settings
- [ ] Identify user populations where ring mouse is contraindicated (document
      honestly per CLAUDE.md surface-agnosticism-is-a-spectrum rule)

**Why:** The project's accessibility-first rule is a hard constraint. Published
research can inform ring sizing, click force thresholds, and firmware filtering
parameters before the first user test.

**Inputs:** PubMed, IEEE Xplore, RESNA conference proceedings, AAATE
proceedings, existing assistive device studies (LipSync, FlipMouse, Glassouse).

### 3.2 Surface Compatibility Test Protocol
- [ ] Define quantitative metrics: tracking accuracy (px/mm linearity), jitter
      (px RMS at rest), dropout rate (% lost frames), latency (ms end-to-end)
- [ ] Specify test fixtures (consistent finger pressure, controlled speed)
- [ ] Define pass/fail criteria per metric per surface type
- [ ] Cover all 6 CLAUDE.md surfaces: wood, glass, fabric, paper, glossy
      magazine, matte plastic
- [ ] Add surfaces relevant to accessibility use: wheelchair tray (vinyl,
      powder-coated metal), hospital bed rail, skin, clothing

**Why:** The moment hardware arrives, you need a test protocol. Writing it now
means testing starts on day 1, not day 3. The accessibility surfaces (wheelchair
tray, bed rail) are not in CLAUDE.md's list but are the actual use environment
for the target users.

**Inputs:** Optical mouse sensor test methodologies (PixArt application notes),
HID compliance test procedures, existing surface compatibility tables in
COMBINATORICS.md.

### 3.3 Companion App Architecture (Web Serial)
- [ ] Design the configuration data model: role assignment, DPI curves,
      dead zone parameters, gesture mapping
- [ ] Specify the BLE GATT characteristic schema for configuration read/write
- [ ] Define the Web Serial protocol for hub configuration
- [ ] Wireframe the configuration UI (role assignment, per-ring settings)
- [ ] Document platform support matrix (Web Serial browser support, fallback
      strategies for Safari/iOS)

**Why:** Phase 5 of FIRMWARE-ROADMAP.md. The GATT characteristic schema affects
firmware (it defines the BLE config service), so designing it now prevents a
BLE service refactor later. The UI itself is not blocking.

**Inputs:** FIRMWARE-ROADMAP.md Phase 5 description, Web Serial API spec,
BLE GATT specification, existing `hal_ble_esp.c` GAP/GATT logic.

### 3.4 Multi-Ring Composition Protocol Spec
- [ ] Formally specify role assignment protocol (cursor, scroll, modifier,
      custom)
- [ ] Define behavior for: ring disconnect during drag, ring reconnect with
      stale role, two rings claiming same role, role reassignment while in use
- [ ] Specify state machine for role negotiation between hub and companion app
- [ ] Write edge case tests against existing `event_composer.c` and
      `role_engine.c`

**Why:** The hub firmware implements basic role assignment, but the protocol
for dynamic reassignment and conflict resolution is implicit in code, not
specified. Formalizing it now prevents protocol drift as features are added.

**Inputs:** Existing `role_engine.c`, `event_composer.c`, hub `ble_central.c`,
FIRMWARE-ROADMAP.md Phase 4 and 5 descriptions.

### 3.5 Defensive Publication Preparation
- [ ] Draft Hackaday.io project page content (establishes dated public
      disclosure)
- [ ] Draft arXiv-style technical note covering the sensing mechanisms,
      form factors, and multi-ring paradigm
- [ ] Prepare OSHWA certification application materials
- [ ] Review IP-STRATEGY.md action items and complete all that don't require
      funding

**Why:** IP-STRATEGY.md lists 6 action items, most of which are blocked only
on someone doing the writing. The defensive publication value is time-sensitive
— earlier disclosure creates stronger prior art.

**Inputs:** IP-STRATEGY.md, COMBINATORICS.md (the design matrix *is* the
defensive publication — it just needs to be in a public, timestamped venue).

---

## Diminishing Returns Boundary

The following are explicitly **out of scope** for agentic pre-hardware work:

- **Sensor tuning parameters** — requires bench measurements, not datasheets
- **Actual power measurements** — POWER-BUDGET.md estimates are the ceiling;
  real numbers require hardware
- **Ring shell CAD** — requires EE/ME with mechanical design skills and
  iterative fit testing
- **OTA update edge cases** — speculative architecture for firmware that hasn't
  shipped on real devices
- **Further doc polish** — the existing docs are already unusually thorough;
  additional wordsmithing has near-zero marginal value
- **Outreach execution** — community posts, emails, conference submissions are
  valuable but are not research/spec work
- **Kconfig flag design** — trivial to add when the firmware phase arrives;
  designing them in advance couples to decisions not yet made
