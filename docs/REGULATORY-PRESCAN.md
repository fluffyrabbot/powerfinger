# Regulatory Pre-Scan — FCC / CE / Bluetooth SIG

Pre-schematic-capture analysis of certification requirements for the PowerFinger
ring. The goal is to surface every PCB layout constraint and labeling obligation
before the EE begins routing, so no regulatory-driven respin is needed.

References:
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)

---

## 1. Module Pre-Certification Scope

### ESP32-C3-MINI-1-N4

| Certification | ID | Scope |
|---------------|----|-------|
| FCC Part 15 Subpart C | FCC ID: 2AC7Z-ESPC3MINI1 | Intentional radiator (BLE + Wi-Fi TX at 2.4 GHz) |
| IC (Canada) | IC certification on file | Intentional radiator |
| CE RED 2014/53/EU | CE mark on module | Radio (Article 3.2), EMC (Article 3.1b) for the module itself |
| Bluetooth SIG | QDID 243324 | BLE 5.0 controller subsystem qualification |

### ESP32-S3-MINI-1-N8 (Hub)

The hub module carries its own FCC, IC, and CE pre-certifications. The same
analysis below applies to the hub, but the hub is a desktop USB device with no
body-worn or miniaturization constraints, so it is lower-risk. This document
focuses on the ring.

### What Module Certification Covers

The module grant covers the **intentional radiator** — the BLE/Wi-Fi transmitter,
its on-board PCB antenna (3.96 dBi gain), RF front-end, and associated conducted
and radiated emissions from the radio subsystem. The module was tested in a
stand-alone configuration per FCC modular approval requirements.

### What Module Certification Does NOT Cover

1. **Unintentional emissions** from the host PCB (digital switching noise, SPI
   bus to sensor, I2C bus to DRV2605L, LiPo charge circuit, LRA motor drive
   signals).
2. **RF exposure (SAR)** evaluation for the finished product in its intended
   body-worn configuration.
3. **Labeling and user information** obligations for the finished product.
4. **Safety** (LiPo battery, LRA motor) — not part of radio certification.

---

## 2. Conditions for Using Module Certification

The ESP32-C3-MINI-1-N4 has full modular approval (not limited modular approval).
To inherit the module's FCC grant in the finished ring, these conditions must be
met:

### 2.1 Antenna

- The module uses its **on-board PCB antenna**. No external antenna is connected.
- The host product must **not modify, replace, or add antennas**. Any antenna
  change invalidates the module grant and requires a new Part 15 Subpart C
  certification.
- The module's antenna must remain the **only intentional radiator antenna** in
  the product.

### 2.2 Antenna Keep-Out Zone

Per the ESP32-C3-MINI-1 datasheet (Section 10, Module Dimensions) and the
ESP32-C3 Hardware Design Guidelines:

- **No copper pour, traces, or components** within the antenna keep-out zone
  marked in the datasheet mechanical drawing. The keep-out zone extends from the
  antenna end of the module (the end opposite the castellated pads) and covers
  the area above the on-board PCB antenna.
- The datasheet specifies the keep-out zone graphically in Figure 10.1. The zone
  extends approximately **10 mm beyond the module edge** at the antenna end and
  spans the full width of the module.
- **No ground plane** under the antenna area on adjacent PCB layers. Ground plane
  under the antenna degrades gain and detunes the matching, potentially pushing
  radiated emissions out of spec.
- The host PCB should **not extend beyond the antenna end** of the module. If it
  must, the extended area must be copper-free (no pour, no traces) within the
  keep-out zone.

**Ring-specific constraint:** The ring PCB is flex or rigid-flex with very tight
area constraints (~10 mm height budget). The antenna keep-out zone is the single
largest area reservation on the PCB. Route the module so its antenna end faces
outward (away from finger, toward the ring exterior) with no copper in the
keep-out zone. This likely dictates module placement at one end of the flex
substrate.

### 2.3 Ground Plane Under Module Body

- A **continuous ground plane** is required on the PCB layer directly beneath the
  module body (excluding the antenna keep-out zone).
- No splits, slots, or trace routing through the ground plane under the module.
- Ground vias should stitch the ground plane to other layers per Espressif's
  reference design.

### 2.4 Module Labeling

- The module's FCC ID must be **visible** when the module is installed, OR the
  finished product's exterior must display a label stating:
  "Contains FCC ID: 2AC7Z-ESPC3MINI1"
- For the ring, the module will be fully enclosed. See Section 5 (Labeling) for
  how to handle this.

---

## 3. FCC Requirements for the Finished Ring

### 3.1 Part 15 Subpart C — Intentional Radiator

**Covered by module certification.** No additional Subpart C testing or
certification filing is required for the finished ring, provided the conditions
in Section 2 are met (antenna unmodified, keep-out zone respected, ground plane
intact).

The ring inherits the module's FCC grant. The ring does **not** receive its own
FCC ID for the intentional radiator. The ring's labeling references the module's
FCC ID.

### 3.2 Part 15 Subpart B — Unintentional Radiator

**Required for every digital device, regardless of module certification.**

The ring is a Class B digital device (consumer product, residential use). Class B
limits are stricter than Class A (commercial/industrial).

Authorization method: **Supplier's Declaration of Conformity (SDoC)**. No FCC
filing or FCC ID is required for the unintentional radiator portion. The
responsible party (must be a US-based entity) self-declares compliance based on
testing at an accredited lab.

Required testing:
- **Radiated emissions** (47 CFR 15.109): Measured at 3 m or 10 m per ANSI
  C63.4. Limits for Class B devices apply.
- **Conducted emissions** (47 CFR 15.107): Measured on the AC power line. For
  battery-powered devices with no AC connection during normal use, conducted
  emissions testing on the AC line is **not applicable** during normal operation.
  However, if the ring is connected to a USB-C charger during use, conducted
  emissions on the USB-C port may apply.

Sources of unintentional emissions on the ring PCB:
- SPI bus to optical sensor (PAW3204 or PMW3360) — MHz-range clock
- I2C bus to DRV2605L haptic driver (Pro tier) — 400 kHz typical
- LiPo charge controller (TP4054) switching
- LRA motor drive PWM from DRV2605L — can produce broadband harmonics
- USB-C connector (during charging)

### 3.3 RF Exposure (SAR) Evaluation

The ring is a **portable device** (operates within 20 cm of the body — it is worn
on the finger, effectively 0 cm separation).

Per FCC KDB 447498 D04 (General RF Exposure Guidance):

- **1 mW blanket exemption:** Devices with maximum time-averaged output power of
  1 mW or less are exempt from SAR evaluation regardless of separation distance.
- **BLE power levels:** The ESP32-C3 BLE TX power is configurable. At the
  default BLE 5.0 TX power, the module can transmit up to approximately 20 dBm
  (100 mW) peak. However, BLE duty cycle is very low — BLE connection events are
  brief bursts with long idle periods.
- **Time-averaged power calculation:** At a 15 ms connection interval with
  typical BLE event duration (~1-2 ms), the duty cycle is roughly 7-13%. At
  0 dBm (1 mW) TX power, time-averaged power is well below 1 mW. Even at
  higher TX power settings, the BLE duty cycle typically brings the time-averaged
  power below the SAR exemption threshold.

**Recommended approach:**
1. Set maximum BLE TX power in firmware to **0 dBm (1 mW)** or lower. This is
   sufficient for a ring-to-hub link distance of 1-3 m and keeps time-averaged
   power firmly below the 1 mW blanket exemption.
2. Document the time-averaged power calculation per KDB 447498 D04 procedures.
3. If TX power must exceed 0 dBm for range, perform the RF exposure evaluation
   using the duty-cycle method. Full SAR testing (cost: $5,000-$15,000) is
   likely avoidable but depends on the final TX power configuration.

**PCB design implication:** No SAR-driven layout constraints if TX power is
limited to 0 dBm. If higher TX power is needed, the antenna placement relative
to the finger becomes relevant to the SAR evaluation.

---

## 4. CE RED Requirements (EU Market)

### 4.1 Article 3.1(a) — Health and Safety

Harmonized standard: **EN 62368-1** (Audio/video, information and communication
technology equipment — Safety requirements).

Covers:
- **LiPo battery safety:** Overcharge protection, overdischarge protection,
  short-circuit protection, thermal management. The TP4054 charge controller
  provides overcharge protection; the firmware or a protection IC must provide
  overdischarge cutoff.
- **Thermal limits:** Surface temperature of a body-worn device must not cause
  burns. EN 62368-1 specifies touch temperature limits (typically 48 C for
  prolonged skin contact with metal, lower for other materials).
- **LRA motor:** No specific safety concern beyond thermal. The DRV2605L has
  built-in overcurrent protection.

**RF exposure (SAR) under CE:**
- Harmonized standard: **EN 62479** (Assessment of compliance of low-power
  equipment with basic restrictions related to human exposure to electromagnetic
  fields, 10 MHz to 300 GHz).
- EN 62479 provides a simplified calculation method for low-power devices. If the
  maximum output power is below the threshold in Annex A of EN 62479 (20 mW for
  general public exposure at 2.4 GHz per EN 50663), the device is deemed
  compliant without SAR testing.
- BLE at 0 dBm (1 mW) is well below 20 mW. **EN 62479 compliance is achievable
  by calculation alone** — no SAR measurement required.

### 4.2 Article 3.1(b) — Electromagnetic Compatibility

Harmonized standards:
- **EN 301 489-1** (EMC standard for radio equipment — general requirements)
- **EN 301 489-17** (EMC specific conditions for broadband data transmission
  systems operating in the 2.4 GHz band)

These cover:
- Radiated emissions (analogous to FCC Part 15 Subpart B)
- Conducted emissions
- Radiated immunity
- Conducted immunity
- ESD immunity

**Key difference from FCC:** CE EMC testing includes **immunity testing** (how the
device responds to external interference), not just emissions. The ring must
continue to operate correctly when subjected to ESD, radiated RF fields, and
electrical fast transients per EN 301 489-1.

**PCB design implications for immunity:**
- Robust ESD protection on the USB-C port (TVS diode array recommended)
- Decoupling capacitors on all power rails close to ICs
- Continuous ground plane (already required for antenna performance)

### 4.3 Article 3.2 — Effective Use of Radio Spectrum

Harmonized standard: **EN 300 328** (Wideband transmission systems operating in
the 2.4 GHz band).

Covers:
- Transmit power limits
- Frequency stability
- Occupied bandwidth
- Adaptive frequency hopping (BLE uses frequency hopping natively)

**Covered by module certification.** The ESP32-C3-MINI-1 has been tested against
EN 300 328. No additional testing required if the module antenna and RF path are
unmodified.

### 4.4 CE Compliance Path

CE RED compliance uses a **self-declaration** model (Declaration of Conformity,
DoC). No notified body approval is required when all applicable harmonized
standards are used and the device is not in the Article 3.3 category (which
covers additional requirements like emergency services access — not applicable
to a BLE ring).

Steps:
1. Test against EN 300 328, EN 301 489-1/-17, EN 62368-1, EN 62479 at an
   accredited lab.
2. Prepare technical documentation (test reports, circuit diagrams, BOM, user
   manual, risk assessment).
3. Draft and sign the EU Declaration of Conformity.
4. Affix CE mark to the product (see Section 5 for labeling).
5. Appoint an EU Authorized Representative if the manufacturer is outside the EU.

---

## 5. Labeling Requirements

### 5.1 FCC Labeling (47 CFR 15.19)

Required text on the product or packaging:

> This device complies with Part 15 of the FCC rules. Operation is subject to
> the following two conditions: (1) This device may not cause harmful
> interference, and (2) this device must accept any interference received,
> including interference that may cause undesired operation.

Additionally, the product must display:
> Contains FCC ID: 2AC7Z-ESPC3MINI1

**Small device exception (47 CFR 15.19(a)(3)):** When a device is too small to
bear the required label text in 4-point or larger font, and the device does not
have a display capable of showing electronic labels, then:
- The required statements must be placed in the **user manual**.
- The required statements must **also** appear on the **product packaging** or on
  a **removable label** attached to the device.

**Ring application:** A fingertip ring cannot physically bear legible 4-point
text with the full FCC compliance statement and FCC ID. The ring has no display.
Therefore:
1. Place all FCC text in the user manual / quick-start guide.
2. Place all FCC text on the retail packaging.
3. Optionally, include a removable label (hang tag or wrap) on the ring itself.

**Electronic labeling (e-label)** is only available for devices with a built-in
display capable of showing the information. The ring has no display, so e-labeling
is not an option.

### 5.2 FCC User Information (47 CFR 15.21)

The user manual must include:

> Changes or modifications not expressly approved by the party responsible for
> compliance could void the user's authority to operate the equipment.

### 5.3 FCC Interference Notice (47 CFR 15.105)

For a Class B digital device, the user manual must include:

> This equipment has been tested and found to comply with the limits for a Class
> B digital device, pursuant to Part 15 of the FCC Rules. These limits are
> designed to provide reasonable protection against harmful interference in a
> residential installation. This equipment generates, uses and can radiate radio
> frequency energy and, if not installed and used in accordance with the
> instructions, may cause harmful interference to radio communications. However,
> there is no guarantee that interference will not occur in a particular
> installation. If this equipment does cause harmful interference to radio or
> television reception, which can be determined by turning the equipment off and
> on, the user is encouraged to try to correct the interference by one or more
> of the following measures:
> - Reorient or relocate the receiving antenna.
> - Increase the separation between the equipment and receiver.
> - Connect the equipment into an outlet on a circuit different from that to
>   which the receiver is connected.
> - Consult the dealer or an experienced radio/TV technician for help.

### 5.4 CE Labeling

- **CE mark** must appear on the product, packaging, or accompanying documents.
  For a ring too small for the mark, it may appear on the packaging and in the
  user manual.
- The CE mark must be at least **5 mm tall** (per EU Regulation 765/2008).
- The packaging or documentation must include: manufacturer name and address (or
  EU Authorized Representative), product type/model identifier.

### 5.5 IC (Canada) Labeling

Similar small-device provisions apply. The IC certification number for the module
must be referenced:
> Contains IC: [module IC number]

This can be placed in the user manual and on packaging per the small-device
exception.

---

## 6. Bluetooth SIG Qualification

Separate from FCC/CE, the Bluetooth Special Interest Group (SIG) requires
qualification for any product that uses Bluetooth technology and bears the
Bluetooth word mark or logo.

### 6.1 ESP32-C3-MINI-1 Existing Qualification

The ESP32-C3-MINI-1 has Bluetooth SIG QDID 243324, covering the controller
subsystem. The ESP-IDF BLE host stack (NimBLE or Bluedroid) has its own
qualification.

### 6.2 End Product Listing

The ring manufacturer must complete an **End Product Listing** (EPL) via the
Bluetooth SIG Launch Studio, referencing the module's QDID and the host stack
QDID. This declares the product as a qualified Bluetooth end product.

### 6.3 Costs

| Item | Cost |
|------|------|
| Bluetooth SIG membership (annual) | $2,500 (Adopter level) |
| End Product Listing fee | $0 (included with membership if referencing existing QDIDs) |
| Innovation Incentive Program (revenue < $1M/year) | Reduced to $2,500 total for first declaration |

If the product uses only pre-qualified subsystems (module controller + host
stack), no additional Bluetooth testing is required. The EPL is a paperwork
exercise.

---

## 7. PCB Layout Constraints Summary (Pre-Schematic-Capture)

These constraints must be incorporated into the PCB layout from the start.

### 7.1 Antenna Keep-Out Zone (Highest Priority)

| Constraint | Value | Source |
|------------|-------|--------|
| No copper/traces/components in antenna keep-out zone | ~10 mm beyond module antenna end, full module width | ESP32-C3-MINI-1 datasheet Fig. 10.1 |
| No ground plane under antenna area | Must be copper-void on all layers | Espressif HW Design Guidelines |
| Module antenna end should face outward | Away from finger, toward ring exterior | RF performance + SAR |
| Host PCB should not extend past antenna end | Or if it does, must be copper-free in keep-out | Espressif HW Design Guidelines |

### 7.2 Ground Plane

| Constraint | Value | Source |
|------------|-------|--------|
| Continuous ground plane under module body (non-antenna area) | No splits, slots, or trace routing | Espressif HW Design Guidelines |
| Ground via stitching under module | Per Espressif reference design | Espressif HW Design Guidelines |
| Continuous ground plane near RF traces | No slots that could act as slot antennas | EMC best practice |

### 7.3 Motor EMI (Pro Tier — LRA + DRV2605L)

The LRA motor is driven by a PWM signal from the DRV2605L. The motor leads and
drive traces are sources of broadband unintentional emissions.

| Constraint | Rationale |
|------------|-----------|
| Keep LRA motor leads short | Longer leads = larger loop antenna area = more radiated emissions |
| Route motor drive traces away from antenna keep-out zone | Prevent coupling into the BLE antenna |
| Place DRV2605L close to LRA motor | Minimize trace length between driver and motor |
| Add series ferrite bead on motor leads (if emissions fail pre-compliance) | Attenuates high-frequency harmonics from PWM drive |
| Consider local ground pour around DRV2605L with via stitching | Contains return currents from motor drive |
| Decoupling capacitors on DRV2605L VDD, close to pin | Per DRV2605L datasheet recommended layout |

### 7.4 Sensor Bus Routing

| Constraint | Rationale |
|------------|-----------|
| SPI clock to optical sensor: keep traces short, route over continuous ground | SPI clock harmonics are a common source of radiated emissions |
| Series resistor on SPI clock (33-100 ohm) | Slows edges, reduces harmonics without affecting signal integrity at typical SPI rates |
| I2C bus (DRV2605L): keep traces short | 400 kHz is low risk but trace length still matters on flex PCB |

### 7.5 USB-C Port

| Constraint | Rationale |
|------------|-----------|
| TVS diode array on USB-C data lines | ESD protection (required for CE immunity testing) |
| Common-mode choke on USB-C data lines (if space permits) | Reduces conducted emissions on USB cable during charging |
| Place USB-C connector away from module antenna end | Prevent USB noise coupling into antenna |

### 7.6 Battery

| Constraint | Rationale |
|------------|-----------|
| LiPo must have integrated protection circuit (or external protection IC) | EN 62368-1 safety; overdischarge, overcurrent, short-circuit protection |
| Battery traces: wide, short, away from antenna | High peak currents during BLE TX create voltage droop and EMI |

---

## 8. Certification Cost Estimate — Single SKU (Standard Ring)

These are rough cost ranges based on published lab pricing for BLE devices using
pre-certified modules. Actual costs vary by lab and region.

### 8.1 Pre-Compliance Testing

| Item | Estimated Cost |
|------|---------------|
| Radiated emissions pre-scan (informal, accredited lab) | $500 - $1,500 |
| Conducted emissions pre-scan (USB-C charging mode) | $300 - $800 |

Pre-compliance testing is optional but strongly recommended. It identifies
emissions problems before the expensive formal test, when PCB changes are still
cheap.

### 8.2 FCC (US Market)

| Item | Estimated Cost |
|------|---------------|
| Part 15 Subpart B testing (SDoC, Class B, accredited lab) | $2,000 - $5,000 |
| RF exposure evaluation (calculation-based, if TX power <= 0 dBm) | $500 - $1,500 |
| RF exposure SAR testing (if TX power > 0 dBm, full measurement) | $5,000 - $15,000 |
| FCC filing fee | $0 (SDoC has no filing fee) |

**Total FCC (assuming calculation-based RF exposure):** $2,500 - $6,500

### 8.3 CE (EU Market)

| Item | Estimated Cost |
|------|---------------|
| EN 301 489-1/-17 EMC testing (emissions + immunity) | $3,000 - $6,000 |
| EN 62368-1 safety evaluation (LiPo battery focus) | $2,000 - $5,000 |
| EN 62479 RF exposure assessment (calculation) | $500 - $1,000 |
| EN 300 328 radio testing | Covered by module cert (no additional cost) |
| Declaration of Conformity preparation | Internal effort |
| EU Authorized Representative (annual, if needed) | $500 - $2,000/year |

**Total CE:** $6,000 - $14,000

### 8.4 IC (Canada)

| Item | Estimated Cost |
|------|---------------|
| ICES-003 unintentional radiator testing | Often combined with FCC Part 15B test; incremental $500 - $2,000 |

### 8.5 Bluetooth SIG

| Item | Estimated Cost |
|------|---------------|
| SIG Adopter membership | $2,500/year |
| End Product Listing (using pre-qualified QDID) | $0 (included) |

### 8.6 Total Estimate — Single SKU, FCC + CE + IC + BT SIG

| Scenario | Estimated Range |
|----------|----------------|
| Best case (0 dBm TX, no SAR test, clean pre-compliance) | $12,000 - $25,000 |
| Worst case (SAR test required, EMC respin needed) | $25,000 - $50,000 |

The best case assumes TX power is limited to 0 dBm, all tests pass on the first
attempt, and the module certification is fully inherited. The worst case assumes
a PCB respin after pre-compliance and full SAR measurement.

---

## 9. Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Antenna keep-out zone violated due to ring size constraints | Medium | High — requires respin or new FCC cert | Model keep-out zone in CAD before routing; this is the #1 layout constraint |
| LRA motor PWM causes radiated emissions failure | Medium | Medium — fixable with ferrite/shielding | Pre-compliance test early; keep motor leads short; series ferrite bead |
| SPI clock harmonics exceed Class B limits | Low-Medium | Medium — fixable with series resistor | Add series damping resistor on SPI clock in schematic (zero-cost) |
| USB-C conducted emissions during charging | Low | Low — only applies during charging, not normal BLE operation | Common-mode choke + TVS on USB-C lines |
| TX power must exceed 0 dBm for adequate range | Low | High — triggers SAR testing ($5-15K) | Design for 0 dBm; validate range with hub at 1-3 m during prototyping |
| Ring too small for any physical labeling | Certain | Low — solved by packaging/manual labeling per 15.19(a)(3) | Plan packaging and manual text from the start |
| EN 62368-1 LiPo safety concerns | Low | Medium — may need protection IC | Specify LiPo cells with integrated protection circuit; verify TP4054 provides sufficient protection |

---

## 10. Action Items for Schematic Capture

1. **Reserve antenna keep-out zone first.** Before placing any other component,
   mark the keep-out zone from the ESP32-C3-MINI-1 datasheet Fig. 10.1 as a
   board constraint. All other placement works around this.

2. **Set BLE TX power to 0 dBm in firmware config.** This avoids SAR testing
   entirely. Validate that 0 dBm provides adequate range to the hub at 1-3 m
   during prototype bring-up. If range is insufficient, increase in 3 dBm steps
   and re-evaluate RF exposure.

3. **Add series damping resistor (33-100 ohm) on SPI clock** in the schematic.
   Zero BOM cost impact, significant emissions risk reduction.

4. **Add TVS diode array on USB-C data lines** in the schematic. Required for CE
   ESD immunity regardless.

5. **Add decoupling capacitors per DRV2605L and TP4054 datasheets.** Standard
   practice but critical for emissions.

6. **Specify LiPo cell with integrated protection circuit** (overcurrent,
   overdischarge, short-circuit). This satisfies EN 62368-1 battery safety
   requirements without adding a separate protection IC to the BOM.

7. **Plan user manual and packaging text.** FCC 15.19, 15.21, and 15.105
   statements must be drafted before production. CE mark and manufacturer info
   on packaging.

8. **Budget for pre-compliance testing** after the first prototype PCB is
   assembled. A radiated emissions pre-scan ($500-$1,500) before formal testing
   saves a potential $10,000+ respin cycle.
