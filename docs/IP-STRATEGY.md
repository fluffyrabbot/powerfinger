# PowerFinger IP Strategy

## Mission Context

PowerFinger is an accessibility project. Computer input devices are prosthetics
for digital agency. The more redundant ways people have to control things the
way they find most natural, the better as a common good. This project exists to
make that as cheap and accessible as possible.

This IP strategy exists to protect the project's ability to exist, not to
extract rent from it.

---

## License

### Hardware: CERN-OHL-S 2.0 (Strongly Reciprocal)

All hardware designs — schematics, PCB layouts, 3D models, BOM files, assembly
instructions — are licensed under CERN Open Hardware Licence Version 2, Strongly
Reciprocal (CERN-OHL-S 2.0).

This license provides three critical protections:

1. **Reciprocal source disclosure.** Anyone who manufactures or distributes
   products based on these designs must publish their complete modified source.
   This prevents someone from taking the designs, closing them, and selling a
   proprietary version.

2. **Patent retaliation clause.** If any licensee initiates patent litigation
   claiming the covered source or a product constitutes patent infringement,
   all rights granted to them under the license terminate immediately. This
   deters IP plays from anyone within the ecosystem.

3. **Copyleft propagation.** Derivative hardware designs must also be released
   under CERN-OHL-S 2.0. The openness is viral — it cannot be closed.

### Firmware and Software: MIT

All firmware (ESP-IDF code) and companion app code is licensed under MIT. This
is deliberately more permissive than the hardware license because:
- Firmware on its own (without the hardware designs) is not useful for
  competitive closed-source products
- MIT maximizes adoption and contribution from the developer community
- The hardware reciprocity requirement is the load-bearing protection

---

## Patent Landscape

### Known Relevant Patents

| Patent | Title | Holder | Year | Threat | Enforcement History |
|--------|-------|--------|------|--------|-------------------|
| US8648805B2 | "Fingertip mouse and base" | Michael P. Bailen / FTM Computer Products | 2014 | **Closest match** — finger-worn optical/trackball/laser device | **Zero litigation in 12 years.** RPX Insight shows 0 cases filed, 0 asserted, 0 as plaintiff. Multiple commercial ring mice ship without enforcement. |
| US20130063355A1 | "Mouse with a finger triggered sensor" | Individual inventor | 2013 | Finger-mounted optical sensor for cursor | No known enforcement |
| US20150241976A1 | "Wearable finger ring input device" | Individual inventor | 2015 | Ring housing, motion sensor, BLE | No known enforcement |
| US8125448B2 | "Wearable computer pointing device" | Individual inventor | 2012 | U-shaped finger device with optical sensors | No known enforcement |
| CN102023731A | "Wireless tiny finger-ring mouse" | Chinese filing | 2010 | Ring-form mouse for mobile terminals | No known enforcement |
| Apple (2025) | Trackball Apple Pencil | Apple Inc. | 2025 | Ball-tip pen with optical sensors, any-surface | Apple does not sue open-source hardware makers or small peripheral companies. Their enforcement targets large direct competitors (Samsung, Masimo, Qualcomm). |
| Prolo "Modtouch" | Capacitive touch + IMU ring | Prolo Team | 2025 | Specific to dual-zone thumb trackpad | Different mechanism entirely |

### Assessment

The general concept of "ring + sensor = mouse" is broadly patented. The specific
combination in PowerFinger (middle fingertip pad, drag-to-track, angled sensor
mount, direct optical or mechanical ball) may or may not fall within existing
claims.

**Critical fact:** No patent holder in this space has ever enforced against any
ring mouse product — commercial or open source. FTM Computer Products has zero
litigation in 12 years despite Padrone, Prolo, AirKLC, and Magnima all shipping
products in this space.

**A freedom-to-operate (FTO) opinion from a patent attorney is required before
Phase 2 investment.** Estimated cost: $2–5K. This is gated at Phase 1.5 in the
execution plan.

---

## Defensive Publication Strategy

Every hardware design committed to this repository constitutes a **defensive
publication** — a dated, public, enabling disclosure that creates prior art.

### Why This Matters

A defensive publication prevents anyone from obtaining a patent on the published
design *after* the publication date. It does not invalidate existing patents, but
it blocks future patent applications covering the same ground.

### Requirements for Effective Defensive Publication

For prior art to be maximally useful, it must be:

1. **Enabling.** Someone skilled in the art must be able to build the device from
   the description. Complete schematics, BOM, assembly instructions, and firmware
   source satisfy this requirement.

2. **Publicly accessible.** The repository must be public. Git commit timestamps
   provide verifiable dates.

3. **Searchable by patent examiners.** USPTO examiners search Google Patents,
   Google Scholar, IEEE, and major technical publication databases. A Hackaday.io
   project page or arXiv preprint is more discoverable than a GitHub repo alone.

4. **Specific.** Each publication should describe the exact sensing mechanism,
   form factor, angle, interaction model, and intended use case.

### Publication Checklist

When committing a new design variant:

- [ ] Complete schematic (KiCad or equivalent)
- [ ] Bill of materials with specific part numbers
- [ ] 3D model files (STEP/STL) for all mechanical parts
- [ ] Assembly instructions with photos/diagrams
- [ ] Firmware source code
- [ ] Design rationale explaining *why* this combination of choices
- [ ] Commit message includes: form factor, angle, sensing mechanism, optional
      capabilities, BOM estimate, intended use case (pointing/OCR/accessibility)

### External Publication

In addition to the GitHub repository, publish to at least one of:

- [ ] Hackaday.io project page (high visibility in hardware community)
- [ ] arXiv preprint (indexed by patent offices)
- [ ] OSHWA certification (Open Source Hardware Association)
- [ ] Instructables / WikiHow build guide (broad public reach)

---

## Defensive Alliances

### Open Invention Network (OIN)

OIN provides royalty-free cross-licenses for Linux-related patents among its
members. If PowerFinger's firmware runs on Linux (ESP-IDF is FreeRTOS-based,
not Linux), OIN may not directly apply. However, the companion app (if Linux-
based) could qualify. Evaluate membership when the project has community
traction.

### Unified Patents / Linux Foundation

The Linux Foundation and CNCF partner with Unified Patents for:
- NPE (non-practicing entity) risk analysis
- Patent portfolio analysis
- PATROLL prior art bounty program

Membership is available to open-source projects. Consider when/if a patent
threat materializes.

### EFF (Electronic Frontier Foundation)

The EFF maintains resources for patent troll victims and has directly intervened
in cases involving open-source projects (GNOME vs. Rothschild Patent Imaging).
Contact proactively if threatened.

---

## The Accessibility Shield

PowerFinger's strongest defense is not legal — it's narrative.

This project exists to make computer input accessible to people with limited
mobility. The BOM is $9. The designs are open. Anyone with a 3D printer and a
soldering iron can build one for someone who needs it.

No patent holder has ever survived public scrutiny for suing an open-source
accessibility project. The GNOME case (2019) demonstrated that community
mobilization + institutional support + strong narrative can destroy a patent
outright.

If threatened:
1. Publish the threat publicly with full context
2. Frame accurately: "Patent holder sues open-source accessibility project
   that gives $9 computer mice to people with disabilities"
3. Engage EFF and open-source advocacy organizations
4. Rally the community for prior art searches via PATROLL
5. If the patent is weak, file for inter partes review (IPR) at USPTO

The cost of this defense strategy is near-zero until activated. The deterrent
value is high.

---

## Action Items

| Action | When | Cost | Status |
|--------|------|------|--------|
| Publish repo under CERN-OHL-S 2.0 + MIT | Now | $0 | Pending |
| Commit detailed design files for P0 variant | Phase 1 | $0 | Pending |
| Publish Hackaday.io project page | Phase 1 | $0 | Pending |
| FTO opinion from patent attorney | Phase 1.5 | $2–5K | Blocked by funding |
| OSHWA certification | Phase 2 | $0 (self-cert) | Pending |
| Evaluate OIN/Unified Patents membership | Phase 2+ | $0 | Pending |
