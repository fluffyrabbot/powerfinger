<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# PowerFinger Conformance Criteria

**Plain language. No legal teeth. Self-attestation only.**

## Disclaimer (read first)

> Inclusion in any future PowerFinger compatible-devices registry indicates
> only that the listed vendor self-attested to passing the conformance
> scripts in `conformance/` against the listed firmware version. The
> PowerFinger project makes no warranty, express or implied, regarding the
> safety, fitness for purpose, accessibility, or merchantability of any
> third-party device. Listings do not constitute endorsement. Each vendor is
> solely responsible for their product's compliance with applicable laws and
> regulations, including medical-device, accessibility, and radio
> regulations in each jurisdiction of sale.

This disclaimer is repeated at the top of any future
`compatible-devices/README.md` and is required verbatim in every attestation
YAML.

---

## What "Conformance" Means Here

This document defines what makes a derivative device *functionally
compatible* with the PowerFinger reference design. It is **not** a
certification, **not** a quality mark, **not** a legal instrument, and
**not** an endorsement.

It exists for one reason: to give downstream productizers, accessibility
reviewers, and end users a published, citable criterion they can use to
distinguish "actually works with the rest of the ecosystem" from "claims to
work with the rest of the ecosystem." Self-attestation is the whole
mechanism. There is no certifying body — not the maintainers, not OSHWA,
not anyone else.

Anyone may run the conformance checks. Anyone may publish results. The
project's own variants (`R30-OLED-NONE-NONE`, `USB-HUB`, etc.) will run the
same checks and publish the same results.

For the affirmative third-party openness signal, derivatives should pursue
[OSHWA Certified](https://certification.oshwa.org/) status separately. OSHWA
Certified is what users will look for; conformance attestation is what
ecosystem participants will check before relying on a device for hub
composition or companion-app integration.

---

## Conformance Categories

Four categories. A device that passes all four can claim
"PowerFinger-compatible" honestly. A device passing fewer should claim
honestly which categories it covers.

### Category 1 — BLE HID Mouse Profile Compliance

**What this checks:** Standard Bluetooth HID Mouse profile compliance, so
the device works as a BLE HID mouse with any host (macOS, Windows, Linux,
iOS, Android) without driver or app installation.

**How to validate:** Use the upstream Bluetooth SIG profile test toolset
(PTS, https://www.bluetooth.com/develop-with-bluetooth/qualification-listing/qualification-test-tools/profile-tuning-suite/).
This category is the standard "is it a real BLE HID mouse" check; we do not
replicate or supplant the Bluetooth SIG validation.

**Self-attestation requirement:** Vendor declares which Bluetooth profile
test results pass and links to the test report (private to the vendor is
fine — public is not required).

### Category 2 — Hub Composition Protocol Compatibility

**What this checks:** The device participates correctly in the PowerFinger
hub composition protocol so it can be composed with other PowerFinger
devices through the USB hub dongle into a single USB HID mouse.

**The protocol:** See `docs/MULTI-RING-PROTOCOL.md`. The hub's
expectations of a peripheral are: standard BLE HID mouse advertising,
manufacturer-specific advertising data identifying the device as a
PowerFinger-protocol participant (so the hub does not pair with arbitrary
BLE mice), correct GATT report descriptor for cursor + button + scroll, and
correct response to the role-assignment characteristic.

**How to validate:** Run `conformance/hub-composition.{sh,py}` against the
device with a reference hub (the project's `firmware/hub` build is the
reference). The script connects, exercises pairing, role assignment,
disconnection, reconnection, and bond-asymmetry recovery. Output is a
pass/fail report.

**Status of the test script:** **Not yet written.** The script will live in
`conformance/hub-composition.*` when implemented. Activation criterion:
when the first third-party productizer ships and asks how to attest.

### Category 3 — Companion App Handshake Compatibility

**What this checks:** The device exposes the GATT characteristics the
companion app expects for configuration (DPI/sensitivity, role assignment,
firmware OTA-update endpoint, calibration commands, battery level reporting).

**The protocol:** See `docs/COMPANION-APP-ARCH.md`. The companion app's
expectations of a peripheral are documented as a GATT service tree with
required and optional characteristics.

**How to validate:** Run `conformance/companion-handshake.{sh,py}` against
the device. The script connects, walks the GATT tree, and validates that
required characteristics are present, readable/writable as specified, and
respond correctly to a sequence of probe commands.

**Status of the test script:** **Not yet written.** Same activation
criterion as Category 2.

### Category 4 — Accessibility Validation Protocol

**What this checks:** The device meets minimum accessibility usability
thresholds — operable for users with limited grip strength, tremor, limited
finger flexion range, and other mobility-relevant conditions the project's
target user populations include.

**The protocol:** **Not yet written.** This is a known doc gap.
`docs/SURFACE-TEST-PROTOCOL.md` is the closest existing analog, but it
covers *surface* compatibility (does the sensor work on glass / fabric /
skin?), not *accessibility scenario* compatibility (is the click force low
enough for tenodesis pinch? is the tremor filter tunable to 4–12Hz? does
sustained operation cause user fatigue?). The accessibility scenario
protocol is a follow-up scoping job that needs an author with accessibility
testing expertise. See the open-question list in
`docs/scoping/LICENSE-REVISION-SCOPE.md` §4.3 for the framing.

**Self-attestation requirement, in the absence of the protocol:** Vendor
declares which user populations they have tested with and reports raw
results. Honest "we have not tested with X population" is acceptable and
encouraged. Overclaiming is the failure mode to avoid.

**This category is the most important one and the least mature.** That is
not ideal but it is honest. The project's own reference variants will not
claim Category 4 conformance until the protocol exists and they pass it.

---

## Self-Attestation Format (Reserved for Future Activation)

When the first third-party productizer is ready to attest, they will open a
PR adding a YAML file to `compatible-devices/<vendor>-<product>.yml` in
this repo. The YAML schema (defined when activation happens) will require:

- Vendor name and contact
- Product name and version
- Hardware revision identifier
- Firmware version tested
- For each conformance category: pass / partial / fail / not-attested
- Test log URL (vendor-hosted is fine)
- Verbatim copy of the disclaimer at the top of this document
- Date of attestation

Merge of the PR by maintainers is a confirmation that the attestation YAML
is well-formed and the disclaimer is present. **Merge is not validation
that the attestation is true.** Maintainers do not run the tests against
third-party hardware.

**The `compatible-devices/` directory does not exist today.** It is
reserved for activation when a third-party productizer is ready to attest.
This is intentional per the scoping decision in
`docs/scoping/LICENSE-REVISION-SCOPE.md` §1.3 — building a registry before
there is something to register is premature scaffolding.

---

## Where the Conformance Suite Lives (When Written)

`conformance/` at the repository root, sibling to `firmware/`, `hardware/`,
`companion/`, and `trial-scripts/`. The directory does not exist yet —
creating it requires the test scripts to exist, and the scripts do not
exist yet.

Why this location:

- Most discoverable.
- Distinct from `trial-scripts/` (KiCad utility scripts) and `scripts/`
  (developer/build helpers).
- Implies project-wide scope (firmware + hub + companion + hardware), not
  any single sub-component.

The conformance suite, when written, will be MIT-licensed (consistent with
`firmware/` and `companion/`) so vendors can integrate test execution into
their internal CI without share-alike obligations.

---

## What This Document Is Not

It is not:

- A certification mark. The project does not operate one. See
  `docs/NAMING-AND-COMPATIBILITY.md`.
- A trademark. The project name is descriptive. Same reference.
- A substitute for OSHWA Certified. Pursue
  [OSHWA Certified](https://certification.oshwa.org/) for the third-party
  openness signal.
- A regulatory framework. Vendors remain solely responsible for compliance
  with their jurisdiction's medical-device, accessibility, and radio
  regulations.
- A guarantee of safety, fitness, or quality of any third-party device.
- An endorsement of any third-party device.

---

## Cross-References

- [`docs/NAMING-AND-COMPATIBILITY.md`](NAMING-AND-COMPATIBILITY.md) —
  descriptive-name posture and bad-faith response framework.
- [`docs/MULTI-RING-PROTOCOL.md`](MULTI-RING-PROTOCOL.md) — hub composition
  protocol details.
- [`docs/COMPANION-APP-ARCH.md`](COMPANION-APP-ARCH.md) — companion app
  protocol details.
- [`docs/SURFACE-TEST-PROTOCOL.md`](SURFACE-TEST-PROTOCOL.md) — surface
  compatibility test protocol (existing, distinct from Category 4).
- [`docs/IP-STRATEGY.md`](IP-STRATEGY.md) — IP defense strategy.
- [`docs/scoping/LICENSE-REVISION-SCOPE.md`](scoping/LICENSE-REVISION-SCOPE.md)
  — scoping doc that motivated this file.
- OSHWA certification: https://certification.oshwa.org/

---

*This document is part of the PowerFinger documentation set, licensed under
CC-BY-SA 4.0. It is not legal advice. The project consists of unincorporated
contributors, not a legal entity.*
