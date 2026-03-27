# Battery Safety and Charging Specification

Comprehensive safety analysis for LiPo cells in the PowerFinger ring form
factor. This document specifies required protections, thermal constraints, charge
rate limits, and firmware safety responsibilities.

References:
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)

---

## 1. Cell Specification

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Chemistry | Lithium polymer (LiPo), pouch cell | Only chemistry that fits ring height budget |
| Nominal voltage | 3.7V | Standard single-cell LiPo |
| Capacity | 80-100 mAh | Constrained by ring height budget (~10mm) |
| Charge termination | 4.2V | Standard Li-ion upper limit |
| Discharge cutoff | 3.0V (cell-level) / 3.2V (firmware cutoff) | Firmware cuts off 200mV above cell protection to preserve cycle life |
| Max charge rate | 0.5C (40-50 mA) | See Section 5 thermal analysis |
| Required protection | Integrated PCM (protection circuit module) | Mandatory. See Section 4 |

---

## 2. Regulatory Landscape — IEC 62133-2

IEC 62133-2:2017 ("Secondary lithium cells and batteries for use in portable
applications — Safety requirements") is the primary international standard for
lithium battery safety.

### Applicability to sub-100mAh cells

IEC 62133-2 does **not** exempt cells by capacity. There is no minimum energy
threshold below which the standard ceases to apply. All lithium-ion and
lithium-polymer cells intended for portable applications fall within scope,
including sub-100mAh pouch cells.

However, certain practical realities at this scale:

- **IEC 62133-2 Clause 7.2.1 (External short circuit):** The test applies a
  short through 80 +/- 20 milliohm resistance and observes for fire or
  explosion. Sub-100mAh cells pass this readily because maximum short-circuit
  current is limited by internal resistance (~100-300 milliohm for cells this
  small). Peak short-circuit current for an 80mAh cell is typically 0.3-0.8A,
  producing 0.03-0.2W of internal heating — insufficient for thermal runaway
  in most cases.

- **IEC 62133-2 Clause 7.2.2 (Impact/crush):** Applies to all cells. Pouch
  cells at this scale deform without energetic failure in most cases, but this
  is not guaranteed — internal short circuits from separator puncture can still
  occur.

- **IEC 62133-2 Clause 7.3.5 (Forced internal short circuit):** Applies to all
  cells. This is the hardest test to pass and the one most relevant to a
  wearable ring that may be subjected to crushing force.

- **IEC 62133-2 Clause 8 (Battery management system requirements):** Requires
  overcharge protection, over-discharge protection, and overcurrent protection
  for all battery packs. This requirement is met by the cell's integrated PCM
  plus the TP4054 charge controller.

### IEC 62368-1 — Surface temperature limits

IEC 62368-1:2018 Clause 9.1.2 (Table 42) specifies maximum surface temperatures
for equipment in contact with skin:

| Contact type | Metal surface | Non-metal surface |
|--------------|--------------|-------------------|
| Continuous contact (wearable) | 43 deg C | 48 deg C |
| Short-term contact (10 min) | 48 deg C | 56 deg C |

The PowerFinger ring has a **non-metal shell** (3D-printed POM/nylon), making
the 48 deg C limit applicable for continuous contact. However, the battery and
PCB are separated from skin by only 1-2mm of shell material with poor thermal
insulation. In practice, **treat 48 deg C as the hard limit at any internal
surface that can transfer heat to skin.**

### UN 38.3 — Transport testing

UN Manual of Tests and Criteria Part III Section 38.3 applies to all lithium
cells for transport classification. Sub-100mAh cells typically qualify as
"contained in equipment" (Section II of PI 967) or "packed with equipment"
(Section I of PI 966), which have less onerous documentation requirements. Cells
must still pass T.1 through T.8 altitude/thermal/vibration/shock/short-circuit
tests. Use cells from manufacturers who provide UN 38.3 test summary
documentation.

---

## 3. TP4054 Charge Controller Analysis

The TP4054 (SOT-23-5) is a linear charge controller for single-cell Li-ion/LiPo
batteries. It manages constant-current / constant-voltage (CC/CV) charging with
termination detection.

### Pin functions

| Pin | Function |
|-----|----------|
| VCC | Input supply (USB 5V) |
| BAT | Battery connection |
| GND | Ground |
| PROG | Charge current programming (via resistor to GND) |
| CHRG | Open-drain charge status output |

### Charge current programming

The TP4054 sets charge current by:

    I_charge = V_PROG / R_PROG

where V_PROG = 1000 mV (internal reference, per datasheet).

| RPROG | Charge current | C-rate on 80mAh | C-rate on 100mAh |
|-------|---------------|-----------------|------------------|
| 2 kohm | 500 mA | 6.25C | 5.0C |
| 5 kohm | 200 mA | 2.5C | 2.0C |
| 10 kohm | 100 mA | 1.25C | 1.0C |
| 20 kohm | 50 mA | 0.625C | 0.5C |
| 30 kohm | 33 mA | 0.41C | 0.33C |

### Charge termination voltage

The TP4054 regulates to **4.2V +/- 1%** (4.158V to 4.242V). Per the datasheet
this is the full-charge float voltage.

The +1% upper bound (4.242V) is acceptable for cells with integrated overcharge
protection, which typically trips at 4.25-4.30V. However, the TP4054's
termination accuracy means some cells may be slightly undercharged (stopping at
4.158V, which represents ~95% state of charge). This is conservative and
acceptable — slightly undercharging improves cycle life.

### Charge termination current

The TP4054 terminates charging when the current in CV phase drops to
approximately **1/10th of the programmed charge current**. With RPROG = 20 kohm
(50 mA charge current), termination occurs at ~5 mA. With RPROG = 10 kohm
(100 mA), termination occurs at ~10 mA.

### Temperature monitoring: NONE

**The TP4054 has no NTC thermistor input.** It cannot monitor cell temperature.
There is no thermal foldback or thermal shutdown based on cell temperature.

The TP4054 does have internal thermal regulation for the **IC itself** — it
reduces charge current if the IC junction temperature exceeds approximately
120 deg C. This protects the IC from damage but does **not** protect the cell.
The IC's thermal limit is far above the cell's safe operating range.

This is the single most significant safety gap in the current design.

### Trickle charge behavior

When the TP4054 detects a deeply discharged cell (BAT voltage below ~2.9V), it
enters trickle charge mode at approximately 1/10th of the programmed current.
For RPROG = 20 kohm, trickle current is ~5 mA. When BAT voltage rises above
~2.9V, the controller transitions to full CC mode.

This behavior is appropriate. Deeply discharged LiPo cells (below 2.5V) may have
copper dendrite formation that creates internal short-circuit paths. The TP4054's
trickle charge behavior provides some protection by limiting current into a
potentially damaged cell. However, cells discharged below 2.0V should be
considered unsafe and discarded — the firmware's 3.2V cutoff plus the cell PCM's
~2.5V cutoff should prevent this condition.

### Reverse battery and short circuit

- **Reverse battery:** The TP4054 datasheet specifies tolerance of reverse
  battery voltage to -300 mV. Beyond this, the IC may be damaged. The cell's
  integrated PCM should prevent full reversal under any normal operating
  condition.

- **BAT pin short to GND:** The TP4054 will attempt to deliver the programmed
  charge current into the short. With RPROG = 20 kohm, this is 50 mA from a 5V
  supply, dissipating ~250 mW in the IC. The IC's internal thermal regulation
  will manage this. Not a fire risk, but the IC will run hot until USB is
  disconnected.

---

## 4. Protection Circuit Requirements

### Do sub-100mAh cells need external protection MOSFETs?

**No, if (and only if) the cell has an integrated protection circuit module
(PCM).**

Most small LiPo pouch cells sold through reputable suppliers (e.g., from
manufacturers like Renata, Varta, Great Power, HHS Lipo, BAK) include an
integrated PCM attached to the cell. This PCM provides:

| Protection | Function | Typical threshold |
|------------|----------|-------------------|
| Overcharge | Disconnects charge path when cell exceeds voltage limit | 4.25-4.30V |
| Over-discharge | Disconnects load when cell drops below voltage limit | 2.4-2.5V |
| Overcurrent (discharge) | Disconnects load on excessive current draw | 1-3A for cells this size |
| Short circuit | Disconnects load on short circuit detection | Trips within 5-20 microseconds |

**The PCM is a hard requirement.** Cells without integrated protection exist
(sometimes called "bare cells" or "unprotected cells") and must NOT be used in
the PowerFinger ring.

### Why not external protection?

Adding external protection MOSFETs (e.g., a DW01A + dual MOSFET) requires:

- 2 additional components (DW01A + FS8205A or equivalent)
- PCB area: ~3mm x 2mm
- Additional routing for the sense resistor or MOSFET drain-source path

For a ring PCB with extreme space constraints, external protection is feasible
but unnecessary if the cell has integrated protection. The integrated PCM is
physically bonded to the cell tabs and provides the same protection functions.

### Decision: require cells with integrated PCM

**All LiPo cells used in PowerFinger devices MUST have an integrated PCM.**

Procurement specification for cell vendors:

- Cell must include integrated protection circuit (PCM/PCB)
- Overcharge protection cutoff: 4.25V-4.30V
- Over-discharge protection cutoff: 2.4V-2.5V
- Short-circuit protection with auto-recovery
- UN 38.3 test summary available
- Pouch cell format, max thickness per variant height budget

Cells without integrated PCM are rejected regardless of cost savings.

---

## 5. Thermal Analysis

This section derives the maximum safe charge rate from first principles.

### Thermal environment

The PowerFinger ring operates in a worst-case thermal environment for a battery:

- **Sealed enclosure.** The 3D-printed shell has no ventilation openings. Heat
  removal is limited to conduction through shell walls and convection from the
  outer shell surface.
- **Skin contact.** The ring is worn on a finger. Body temperature is ~37 deg C,
  raising the ambient baseline by ~15 deg C above room temperature (22 deg C).
  The finger acts as both a heat source and a heat sink — at low power
  dissipation the finger absorbs heat, but the temperature differential is
  small.
- **Small thermal mass.** An 80mAh LiPo pouch cell weighs ~2g. The entire ring
  assembly is ~5-8g. Thermal mass is minimal — temperature rises quickly under
  sustained heat input.
- **No forced airflow.** No fan, no movement-induced airflow worth modeling.

### Heat sources during charging

Two heat sources during charging:

**1. TP4054 IC dissipation (linear regulator loss):**

    P_IC = (V_USB - V_BAT) * I_charge

During CC phase, V_BAT ranges from 3.0V (deeply discharged) to ~4.1V (near
full). V_USB = 5.0V.

| RPROG | I_charge | P_IC at V_BAT=3.0V | P_IC at V_BAT=4.0V |
|-------|---------|--------------------|--------------------|
| 10 kohm | 100 mA | 200 mW | 100 mW |
| 20 kohm | 50 mA | 100 mW | 50 mW |
| 30 kohm | 33 mA | 67 mW | 33 mW |

**2. Cell internal dissipation (I^2 * R_internal):**

Internal resistance for a new 80mAh pouch cell is typically 200-500 milliohm.
Aged cells can reach 1 ohm or higher.

| I_charge | P_cell (R=300mOhm) | P_cell (R=800mOhm, aged) |
|---------|--------------------|-----------------------|
| 100 mA | 3 mW | 8 mW |
| 50 mA | 0.75 mW | 2 mW |
| 33 mA | 0.33 mW | 0.87 mW |

Cell internal dissipation is negligible compared to IC dissipation at these
current levels. The TP4054 is the dominant heat source.

**3. Total heat input:**

| RPROG | I_charge | Total P (worst case, V_BAT=3.0V) |
|-------|---------|-----------------------------------|
| 10 kohm | 100 mA | ~203 mW |
| 20 kohm | 50 mA | ~101 mW |
| 30 kohm | 33 mA | ~67 mW |

### Thermal model

A simplified lumped-parameter model for the ring enclosure:

The ring has approximately 300-500 mm^2 of external surface area (excluding the
finger-contact side, which has restricted convection). Natural convection from a
small warm surface in still air gives a heat transfer coefficient of approximately
h = 10-15 W/(m^2*K).

Effective thermal resistance from IC to ambient (through shell):

    R_th = 1 / (h * A) + R_shell

For A = 400 mm^2 = 4e-4 m^2 and h = 12 W/(m^2*K):

    R_th_convection = 1 / (12 * 4e-4) = 208 K/W

The shell wall adds another ~50-100 K/W depending on material and thickness. Use
R_th_total = 250 K/W as a conservative estimate.

**Temperature rise above ambient:**

| P_total | Delta T (R_th = 250 K/W) | Internal temp (ambient = 37 deg C body contact) |
|---------|--------------------------|-----------------------------------------------|
| 200 mW | 50 deg C | 87 deg C |
| 100 mW | 25 deg C | 62 deg C |
| 67 mW | 17 deg C | 54 deg C |
| 50 mW | 12.5 deg C | 49.5 deg C |

### Interpretation

- **100 mA (RPROG = 10 kohm):** Internal temperature could reach 62-87 deg C.
  This is **unsafe.** LiPo cells should not be charged above 45 deg C (most
  manufacturers specify 0-45 deg C charge range). The shell surface temperature
  would exceed the 48 deg C IEC 62368-1 skin contact limit. **Reject.**

- **50 mA (RPROG = 20 kohm):** Internal temperature reaches ~50-62 deg C in
  worst case. This is **marginal.** It exceeds the cell manufacturer's 45 deg C
  charge limit and may exceed the 48 deg C skin contact limit. Acceptable only
  if the thermal model overestimates (possible — the finger acts as a heat sink,
  and the model assumes worst-case static air). **Requires validation on
  prototype hardware with thermocouple measurements.**

- **33 mA (RPROG = 30 kohm):** Internal temperature reaches ~54 deg C worst
  case. Still above the cell's 45 deg C charge limit but within the range where
  the thermal model's conservatism likely covers the gap. **Acceptable with
  monitoring.**

### Thermal model limitations

This model is conservative (intentionally) because:

1. It ignores the finger as a heat sink. A human finger has blood perfusion that
   can absorb significant heat. At low power levels (< 100 mW), the finger
   likely dominates thermal management.
2. It uses still-air convection. Any hand movement improves convection.
3. It assumes all heat flows through the small ring surface. Some heat conducts
   through the finger contact area.
4. It assumes steady-state. An 80mAh cell at 50 mA charges in ~1.5 hours. The
   thermal mass of the assembly provides some transient buffering.

**These limitations do not change the conclusion for 100 mA.** Even with generous
corrections, 200 mW in a sealed ring is too much heat. The limitations do
suggest that 50 mA may be acceptable in practice, but this must be validated
empirically.

### Charge rate recommendation

**Change RPROG from 10 kohm to 20 kohm.**

| Parameter | 10 kohm (current) | 20 kohm (recommended) |
|-----------|-------------------|----------------------|
| Charge current | 100 mA | 50 mA |
| C-rate on 80mAh | 1.25C | 0.625C |
| C-rate on 100mAh | 1.0C | 0.5C |
| Charge time (CC+CV) | ~50-70 min | ~100-140 min |
| IC dissipation (worst case) | 200 mW | 100 mW |
| Estimated delta T | 50 deg C | 25 deg C |
| Estimated peak internal temp | 87 deg C | 62 deg C |
| Cycle life impact | ~300-500 cycles | ~500-800 cycles |

A 20 kohm RPROG is the minimum acceptable value. If prototype thermal
measurements show surface temperatures approaching 48 deg C during charging, move
to 30 kohm (33 mA, ~2.5 hour charge).

**Charge time of ~2 hours at 50 mA is acceptable for this use case.** Users
charge overnight or during breaks. Nobody needs an 80mAh ring fully charged in
15 minutes.

---

## 6. Thermal Runaway Risk Analysis

### What is thermal runaway?

Thermal runaway in lithium cells is a self-accelerating exothermic reaction
sequence: SEI layer decomposition (~80-120 deg C) leads to anode-electrolyte
reaction (~150 deg C) leads to separator melt/collapse (~130-170 deg C for PE,
~170-200 deg C for ceramic-coated) leads to internal short leads to cathode
decomposition (>200 deg C) leads to electrolyte venting and potential ignition.

### Risk assessment for 80mAh pouch cell

The total stored energy in a fully charged 80mAh cell at 4.2V is:

    E = 0.080 Ah * 3.7V * 3600 s/Ah = 1065 J ~ 1.1 kJ

For comparison:
- A match head contains ~1 kJ
- A AAA cell contains ~5 kJ
- A phone battery (3000mAh) contains ~40 kJ

The energy content is low but not negligible. A full thermal runaway event in an
80mAh cell can produce a small flame, toxic fumes (hydrogen fluoride from LiPF6
electrolyte), and localized burns. Inside a sealed ring on a finger, even a
sub-energetic vent event is a safety concern.

### Failure modes specific to ring form factor

| Failure mode | Likelihood | Severity | Mitigation |
|-------------|------------|----------|------------|
| Overcharge (charger fault) | Low (TP4054 + cell PCM provide two layers) | High (overcharge is the primary cause of thermal runaway) | Cell PCM overcharge cutoff at 4.25-4.30V backs up TP4054 |
| Over-discharge followed by charge | Medium (cell sits discharged for weeks/months) | Medium (dendrite growth creates internal short paths) | Cell PCM prevents discharge below 2.4V; firmware cuts at 3.2V |
| Mechanical crush (worn on finger, hand impact) | Medium (rings contact hard surfaces) | Medium (separator puncture causes internal short) | Shell must provide mechanical protection; use cells with ceramic-coated separator if available |
| Charging a swollen cell | Low (user may not notice inside sealed ring) | High (swelling indicates gas generation from internal degradation) | Shell design should allow visual or tactile detection of swelling |
| High-temperature charging (hot environment + body heat) | Medium (summer, hot car, fever) | Medium (accelerates degradation and reduces safety margin) | Firmware NTC monitoring (see Section 7) |
| Manufacturing defect in cell | Very low (quality-dependent) | High | Source from reputable manufacturers with documented QC |

### Shell design requirements for thermal safety

1. **No fully hermetic seal.** The shell must allow gas venting in a fault
   condition. A fully sealed shell could build pressure during a vent event,
   turning the ring into a small pipe bomb. Use a friction-fit or snap-fit
   design that separates under internal pressure rather than glued/welded seals.

2. **Mechanical protection.** The shell must protect the cell from puncture by
   sharp external objects. Minimum wall thickness of 1.0mm at any point over the
   cell.

3. **Material selection.** Shell material must not contribute additional fuel in
   a fire event. POM (polyoxymethylene/Delrin) is flammable — for prototype
   this is acceptable with the understanding that production shells should use a
   fire-retardant material (PA-FR, PC, or similar). Document this in the
   prototype BOM.

4. **Swelling detection.** If the cell swells, the ring should become noticeably
   tighter on the finger or the shell should visibly deform. A completely rigid
   shell that contains swelling without visible indication is dangerous because
   it masks a degraded cell.

---

## 7. Firmware Safety Requirements

The existing `power_manager.c` implements battery voltage monitoring and low
voltage cutoff at 3.2V. Additional firmware safety measures are required.

### 7.1 Required: NTC temperature monitoring

The TP4054 has no temperature monitoring capability. The firmware must fill this
gap.

**Implementation:** Add an NTC thermistor (10 kohm at 25 deg C, B=3950, 0402
package) bonded to the cell or placed adjacent to the cell on the PCB. Read the
NTC via an ADC channel.

**Thresholds:**

| Condition | Threshold | Action |
|-----------|-----------|--------|
| Normal charging | T < 40 deg C | Continue charging |
| Warm charging | 40 deg C <= T < 45 deg C | Log warning; continue charging |
| Overtemperature (charge) | T >= 45 deg C | **Disable charging** by pulling USB VBUS enable low or signaling charge disable. Resume when T < 40 deg C (5 deg C hysteresis) |
| Overtemperature (discharge/use) | T >= 60 deg C | **Enter deep sleep immediately.** Disconnect all loads. This temperature indicates a fault condition. |
| Cold charging | T < 0 deg C | **Disable charging.** Charging LiPo below 0 deg C causes lithium plating (metallic lithium deposition on anode), which permanently damages the cell and creates internal short-circuit risk. |
| Cold use | T < -20 deg C | Log warning. Operation is degraded but not unsafe. |

**Charge disable mechanism:** The TP4054 does not have an enable pin. To disable
charging via firmware:

- Option A: Place a P-channel MOSFET (e.g., SI2301) on the USB VBUS line before
  the TP4054 VCC pin. GPIO controls the MOSFET gate. This adds one component and
  one GPIO but gives deterministic charge control.
- Option B: Place a load switch (e.g., TPS22918) on VBUS. Same function, cleaner
  but slightly larger.
- Option C: Use the ESP32-C3 to pull the PROG pin high via a GPIO + resistor,
  reducing charge current to near zero. Less clean, may not fully stop charging.

**Recommendation:** Option A (P-channel MOSFET on VBUS). This is the simplest
and most reliable approach. If the firmware hangs or the MCU is in deep sleep,
the MOSFET gate floats high (P-channel off), which **defaults to charge
disabled** — a safe default.

### 7.2 Required: Enhanced voltage monitoring

The current firmware checks VBAT every 60 seconds and acts on low voltage (3.2V
cutoff). Additional voltage-based checks:

| Check | Threshold | Interval | Action |
|-------|-----------|----------|--------|
| Low voltage cutoff | VBAT < 3.2V | 60s (existing) | Enter deep sleep (existing) |
| Critical low voltage | VBAT < 3.0V | Every tick when below 3.2V | Immediate deep sleep; skip normal shutdown sequence |
| Overvoltage detection | VBAT > 4.25V | 10s during charging | Disable charging via MOSFET; log error. Indicates TP4054 fault or cell PCM failure. |
| Voltage drop anomaly | VBAT drops > 200mV between consecutive 60s readings with no change in load | 60s | Log warning. Rapid voltage drop under constant load suggests increased internal resistance (cell degradation) or intermittent internal short. |

### 7.3 Required: Charge state tracking

The firmware should track whether USB power is connected and the charge state:

- **USB detection:** Monitor VBUS presence via GPIO (voltage divider from USB 5V
  to a GPIO with appropriate resistor divider to bring 5V into ESP32-C3's 3.3V
  input range). This is also needed for the charge disable MOSFET control.
- **Charge complete detection:** Monitor the TP4054's CHRG pin (open-drain
  output: low = charging, high-Z = complete or no USB). Connect to a GPIO with
  pull-up resistor.
- **State machine integration:** Add CHARGING and CHARGE_COMPLETE states to the
  ring state machine. During charging, the device should remain awake (not enter
  deep sleep) to maintain temperature monitoring.

### 7.4 Required: Watchdog coverage during charging

The existing watchdog must remain active during charging. If the firmware crashes
during charging, the watchdog should reset the MCU. After reset, the VBUS-gate
MOSFET defaults to charge-disabled (safe state). The firmware re-initializes,
detects USB presence, checks temperature, and re-enables charging only if safe.

### 7.5 Recommended: Cell health logging

Over the device's lifetime, log to NVS (non-volatile storage):

- Total charge cycle count (increment when VBAT crosses from <3.5V to >4.0V
  during a charge session)
- Maximum temperature observed during charging
- Number of overtemperature charge-disable events
- Number of overvoltage events (should be zero; any occurrence indicates a
  hardware fault)

This data is readable via a BLE diagnostic characteristic. It allows the
companion app to warn users when the cell is approaching end of life (e.g.,
after 400 cycles) or has experienced anomalous events.

---

## 8. Charge Cycle and Cell Aging

### Cycle life at different charge rates

| Charge rate | Typical cycle life to 80% capacity | Charge time (80mAh) |
|-------------|-----------------------------------|---------------------|
| 0.5C (40 mA) | 500-800 cycles | ~2-2.5 hours |
| 1.0C (80 mA) | 300-500 cycles | ~1-1.5 hours |
| 1.25C (100 mA) | 250-400 cycles | ~50-70 min |
| 2.0C (160 mA) | 150-300 cycles | ~30-45 min |

At the Standard tier's 3-6 day battery life, users charge 1-2 times per week =
52-104 cycles per year.

| Charge rate | Cycles to 80% | Years to 80% (at 1-2x/week) |
|-------------|--------------|----------------------------|
| 0.5C | 500-800 | 5-15 years |
| 1.0C | 300-500 | 3-10 years |
| 1.25C | 250-400 | 2.5-8 years |

At 0.5C, the cell outlasts any reasonable product lifespan. This reinforces the
recommendation for 20 kohm RPROG.

### Calendar aging

LiPo cells lose capacity over time even without cycling:

- At 25 deg C, 50% state of charge: ~2-4% capacity loss per year
- At 25 deg C, 100% state of charge (4.2V): ~4-8% capacity loss per year
- At 40 deg C, 100% state of charge: ~10-15% capacity loss per year

The ring is stored at body temperature (~37 deg C) during use and room
temperature (~22 deg C) when not worn. The firmware's deep sleep mode leaves the
cell at whatever voltage it was at when sleep was entered. No firmware action is
needed for storage voltage optimization — the cell discharges naturally during
deep sleep (16-75 microamp total system draw), reaching a benign mid-charge
state within weeks.

---

## 9. BOM Impact

The battery safety requirements add the following to the BOM:

| Component | Package | Unit cost (qty 100) | Purpose |
|-----------|---------|-------------------|---------|
| NTC thermistor, 10k, B3950 | 0402 | $0.02 | Cell temperature monitoring |
| Resistor (NTC voltage divider) | 0402 | $0.01 | ADC scaling |
| SI2301 P-ch MOSFET (or equiv) | SOT-23 | $0.03 | Charge enable/disable |
| Resistor (MOSFET gate pull-up) | 0402 | $0.01 | Default-off gate bias |
| Change RPROG from 10k to 20k | 0402 | $0.00 | Lower charge rate |
| **Total added cost** | | **~$0.07** | |

The BOM impact is negligible. These components are small (0402 and SOT-23) and
should fit on the existing PCB without layout changes beyond trace routing.

---

## 10. Summary of Requirements

### Cell requirements (procurement)

1. LiPo pouch cell, 80-100mAh, 3.7V nominal
2. **Integrated PCM required** (overcharge, over-discharge, short-circuit)
3. UN 38.3 test summary available from manufacturer
4. Operating temperature range includes 0-45 deg C charging, -20 to 60 deg C
   discharge

### Hardware requirements (EE)

1. RPROG = 20 kohm (50 mA charge current, 0.5-0.625C)
2. NTC thermistor bonded to cell or adjacent on PCB
3. P-channel MOSFET on VBUS line for firmware charge control
4. USB VBUS detection via voltage divider to GPIO
5. TP4054 CHRG pin connected to GPIO with pull-up

### Firmware requirements (FW)

1. NTC temperature monitoring during charging (Section 7.1 thresholds)
2. Charge disable at T >= 45 deg C, re-enable at T < 40 deg C
3. Charge disable at T < 0 deg C
4. Overvoltage detection (VBAT > 4.25V) with charge disable
5. Critical low voltage immediate shutdown (VBAT < 3.0V)
6. Charge state tracking (USB present, charging, complete)
7. Watchdog active during charging
8. Cell health logging to NVS (cycle count, max temp, fault events)

### Shell requirements (ME)

1. Non-hermetic closure (allows gas venting under pressure)
2. Minimum 1.0mm wall thickness over cell
3. Swelling must be detectable (tactile or visual)
4. Production shells must use fire-retardant material (prototype POM/nylon
   acceptable with documented limitation)

---

## 11. Open Questions for Prototype Validation

These questions can only be answered with prototype hardware:

1. **Actual thermal rise during charging.** Place thermocouples on the cell
   surface, IC surface, and outer shell surface. Measure with RPROG = 20 kohm at
   room temperature and on a finger. If shell surface exceeds 45 deg C, move to
   RPROG = 30 kohm.

2. **NTC placement.** Does the NTC need to be bonded directly to the cell pouch,
   or is PCB-adjacent placement sufficient? On a PCB this small, thermal coupling
   is likely adequate, but this needs verification.

3. **MOSFET charge-disable behavior.** Verify that cutting VBUS to the TP4054
   mid-charge does not cause voltage spikes or latch-up. The TP4054 datasheet
   does not specify behavior when VCC is removed during CC phase.

4. **Shell venting.** Verify that the friction-fit or snap-fit shell separates
   under cell vent pressure. Test with a pressure fixture simulating vent gas
   production.

5. **Deep discharge recovery.** Source a sample cell, discharge to 2.5V (PCM
   cutoff), leave for 1 month, then charge. Verify the TP4054 trickle charges
   correctly and the cell recovers to >95% rated capacity.
