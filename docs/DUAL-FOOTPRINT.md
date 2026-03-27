# Dual-Footprint PCB Feasibility Study

Can a single ring PCB accommodate both the PAW3204DB-TJ3L (Standard tier) and the
PMW3360DM-T2QU (Pro tier) via component stuffing options?

This document provides a concrete pin-by-pin, voltage-domain, and footprint
analysis. The verdict is at the end.

References:
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)

---

## 1. Pin-by-Pin Comparison

### PAW3204DB-TJ3L (8-Pin DIP-Style Package)

| Pin | Name    | Function                            | Direction     |
|-----|---------|-------------------------------------|---------------|
| 1   | NRESET  | Active-low reset (internal pull-up) | Input         |
| 2   | SDIO    | Bidirectional serial data           | Bidirectional |
| 3   | SCLK    | Serial clock                        | Input (MCU drives) |
| 4   | VLED    | LED anode power supply              | Power         |
| 5   | LED_GND | LED cathode return                  | Power (ground)|
| 6   | GND     | Digital ground                      | Power         |
| 7   | VDD     | Digital power (2.1-3.6V)            | Power         |
| 8   | NC      | No connect / exposed pad            | --            |

**Interface summary:** 2-wire bit-banged serial. SCLK is unidirectional clock.
SDIO is bidirectional data -- the MCU drives it for writes, tri-states and reads
it back for reads. This is NOT SPI. There is no chip select. There is no
separate MISO. The protocol is PixArt-proprietary, bit-banged via GPIO. Total
MCU pins consumed: 2 (GPIO4 for SCLK, GPIO5 for SDIO in the current Kconfig).

### PMW3360DM-T2QU (16-Pin QFN Package)

| Pin | Name       | Function                                  | Direction     |
|-----|------------|-------------------------------------------|---------------|
| 1   | NCS        | SPI chip select (active low)              | Input         |
| 2   | MISO       | SPI master-in, slave-out                  | Output        |
| 3   | MOSI       | SPI master-out, slave-in                  | Input         |
| 4   | SCLK       | SPI clock                                 | Input         |
| 5   | MOT        | Motion interrupt (active low, open drain) | Output        |
| 6   | AG         | Analog ground                             | Power         |
| 7   | AVDD       | Analog supply (1.8V nominal)              | Power         |
| 8   | VDDIO      | Digital I/O supply (1.9V nominal)         | Power         |
| 9   | DG         | Digital ground                            | Power         |
| 10  | NRESET     | Active-low reset                          | Input         |
| 11  | LED_A      | Laser/LED anode drive                     | Output        |
| 12  | LED_C_GND  | Laser/LED cathode/ground                  | Power         |
| 13  | VDD_LED    | LED driver supply                         | Power         |
| 14-16 | GND/EP  | Exposed pad / ground                      | Power         |

**Interface summary:** Standard 4-wire SPI (SCLK, MOSI, MISO, NCS) plus a
motion interrupt output (MOT). Total MCU pins consumed: 5 (SCLK, MOSI, MISO,
NCS, MOT interrupt). The MOT pin is optional but strongly recommended -- without
it, the MCU must poll the motion register, which prevents optimal sleep.

### Side-by-Side Signal Comparison

| Function             | PAW3204                 | PMW3360                  | Shareable? |
|----------------------|------------------------|--------------------------|------------|
| Clock to sensor      | SCLK (GPIO bit-bang)   | SCLK (SPI hardware)      | **No** (1) |
| Data MCU-to-sensor   | SDIO (bidirectional)   | MOSI (unidirectional)    | **No** (2) |
| Data sensor-to-MCU   | SDIO (same pin)        | MISO (separate pin)      | **No** (2) |
| Chip select          | None (not used)        | NCS (required)           | N/A        |
| Motion interrupt     | None (polled via SDIO) | MOT (active-low, open drain) | N/A   |
| Reset                | NRESET                 | NRESET                   | **Yes**    |
| LED drive            | VLED + LED_GND         | LED_A + LED_C_GND + VDD_LED | Partial (3) |
| Digital power        | VDD (2.1-3.6V)        | VDDIO (1.9V)             | **No** (4) |
| Analog power         | N/A (single domain)   | AVDD (1.8V)              | N/A        |
| Ground               | GND                    | AG + DG + EP             | **Yes**    |

Notes:
1. PAW3204 SCLK is bit-banged GPIO; PMW3360 SCLK is driven by the SPI hardware
   peripheral. The same GPIO pin (GPIO4) could theoretically serve both roles --
   configured as plain GPIO output for PAW3204 or as SPI CLK function for
   PMW3360 -- but the electrical characteristics differ. SPI CLK idles at a
   defined level per mode; GPIO bit-bang toggles manually. The pin can be
   **physically shared** on the PCB with firmware selecting the mode at boot.
2. The PAW3204 SDIO pin is bidirectional on a single wire. The PMW3360 has
   separate MOSI and MISO. A single PCB trace cannot serve both roles. The SDIO
   pad must connect to one MCU GPIO; MOSI and MISO must connect to two different
   MCU GPIOs (or SPI peripheral pins). This is the fundamental pin incompatibility.
3. Both sensors drive an LED/laser, but the PMW3360 has a three-pin LED circuit
   (VDD_LED supply, LED_A anode driver, LED_C ground) versus the PAW3204's
   two-pin circuit (VLED, LED_GND). The LED/lens assembly is sensor-specific
   anyway, so these traces necessarily diverge.
4. The PAW3204 runs at 2.1-3.6V on a single rail. The PMW3360 requires 1.8V
   analog and 1.9V digital -- neither of which is 3.3V. See voltage analysis
   below.

---

## 2. Voltage Domain Analysis

### PAW3204 Power Architecture

```
Battery (3.7-4.2V)
  |
  +-- RT9080 LDO --> 3.3V rail
        |
        +-- ESP32-C3 VDD (3.3V)
        +-- PAW3204 VDD (3.3V, within 2.1-3.6V spec)
        +-- PAW3204 VLED (3.3V, through current-limit resistor)
```

Single 3.3V rail. No additional regulators. The PAW3204 GPIO interface runs at
VDD level (3.3V), matching the ESP32-C3 GPIO levels directly. Zero voltage
translation needed.

### PMW3360 Power Architecture

```
Battery (3.7-4.2V)
  |
  +-- RT9080 LDO --> 3.3V rail
  |     |
  |     +-- ESP32-C3 VDD (3.3V)
  |     +-- PMW3360 VDD_LED (LED driver supply, 3.3V acceptable)
  |
  +-- Additional LDO #1 --> 1.8V rail
  |     |
  |     +-- PMW3360 AVDD (1.8V analog supply)
  |
  +-- Additional LDO #2 or combined --> 1.9V rail
        |
        +-- PMW3360 VDDIO (1.9V digital I/O supply)
```

The PMW3360 requires **two additional voltage domains** beyond the existing 3.3V
rail:

| Rail   | Voltage | Tolerance      | Current Draw        | Notes                        |
|--------|---------|----------------|---------------------|------------------------------|
| AVDD   | 1.8V    | +/- 5% (1.71-1.89V) | ~5 mA active   | Analog imaging supply        |
| VDDIO  | 1.9V    | +/- 5% (1.81-2.0V)  | ~2 mA active   | Digital I/O level            |
| VDD_LED| 1.6-2.4V typical | Application-dependent | ~20 mA peak | LED/laser driver |

**Critical problem: VDDIO sets the I/O voltage.** At 1.9V, the PMW3360's SPI
output high level (MISO, MOT) will be ~1.9V. The ESP32-C3's GPIO input high
threshold at 3.3V VDD is ~2.48V (0.75 * VDD per datasheet). A 1.9V signal does
not meet this threshold. This requires one of:

- **Level shifter** on MISO and MOT lines (adds 2 components, ~$0.10, board area)
- **Running VDDIO at 3.3V** -- the PMW3360 datasheet specifies VDDIO at 1.9V
  nominal. Some community projects (QMK keyboard trackballs, Ploopy) run VDDIO
  from 3.3V successfully, but this is out of spec and voids the datasheet
  guarantees. At minimum it increases digital power draw. For a prototype this is
  acceptable; for a consumer product it is a risk.
- **Separate 1.9V LDO for VDDIO** with level translators -- the correct
  engineering solution but adds cost and board area.

**Practical recommendation:** Run VDDIO at 3.3V for the prototype (matching
community practice in QMK/Ploopy projects), use a single additional LDO for
the 1.8V AVDD rail only. This avoids level shifting entirely. Flag as a known
out-of-spec condition to revisit for consumer product.

### Voltage Domain Component Delta

| Component                     | Needed for PAW3204? | Needed for PMW3360? | Cost     |
|-------------------------------|---------------------|---------------------|----------|
| RT9080-33GJ5 (3.3V LDO)      | Yes                 | Yes                 | $0.15    |
| Additional 1.8V LDO (AVDD)   | No                  | **Yes**             | ~$0.15   |
| AVDD decoupling caps (1uF+0.1uF) | No             | **Yes**             | ~$0.02   |
| VDDIO decoupling caps        | No                  | **Yes** (even if run at 3.3V) | ~$0.02 |

The PMW3360 variant requires one additional LDO and associated decoupling. On a
dual-footprint PCB, the 1.8V LDO pads would be present but depopulated (left
unstuffed) on Standard builds.

---

## 3. Footprint Overlay Comparison

### Package Dimensions

| Parameter              | PAW3204DB-TJ3L         | PMW3360DM-T2QU          |
|------------------------|------------------------|--------------------------|
| Package type           | 8-pin DIP (through-hole capable, SMD variant available) | 16-pin QFN (SMD only) |
| Body dimensions        | ~5.0 x 4.0 mm          | ~4.0 x 4.0 mm (body), 5.1 x 5.1 mm (land pattern) |
| Pin count              | 8                       | 16                        |
| Pin pitch              | 1.27 mm (SOP variant)  | 0.5 mm (QFN)            |
| Height (package only)  | ~1.5 mm (SOP)          | ~0.9 mm (QFN)           |
| Mounting               | Reflow or hand solder   | Reflow only (exposed pad)|
| Lens/clip interface    | Proprietary clip mount  | Proprietary clip mount   |

### Physical Footprint Comparison

The two sensors have comparable body sizes (~4x4 to 5x5 mm) but radically
different pad geometries:

- **PAW3204 (SOP-8):** 8 pads on two sides, 1.27mm pitch, wide pad spacing.
  Traces route out from two sides. Relatively simple to hand-rework.
- **PMW3360 (QFN-16):** 16 pads on four sides plus exposed ground pad, 0.5mm
  pitch. Traces route out from all four sides. Requires reflow; not
  hand-solderable without hot air.

A dual-footprint approach would place **overlapping** or **adjacent** land
patterns for both packages. The two strategies:

#### Strategy A: Overlapping Footprints (Same Location)

Place both pad patterns at the same board location, with the sensor aperture
(optical window) at the same position. Populate one or the other.

**Problem:** The PAW3204 SOP-8 pads and the PMW3360 QFN-16 pads physically
conflict. The QFN exposed ground pad occupies the center of the footprint where
SOP pads would route. The 1.27mm SOP pads are wider than the 0.5mm QFN pads. A
combined pattern would create solder bridges, tombstoning risk, and DRC
violations. **Not feasible without a creative staggered layout that compromises
both footprints.**

#### Strategy B: Adjacent Footprints (Side by Side)

Place both footprints on the PCB, spaced apart, with independent routing to a
shared MCU connection point. Populate one, leave the other empty.

**Problem:** The ring PCB has severe area constraints. The sensor must be
positioned at the optical aperture -- the point where the lens assembly focuses
on the surface. There is one such point per ring, defined by the mechanical
design. Two adjacent sensor footprints means the unused one occupies dead board
area directly adjacent to the optical window. On a flex PCB that wraps around a
fingertip, this is roughly 10mm x 5mm of wasted area -- significant when total
usable PCB area is approximately 20mm x 12mm (constrained by finger
circumference and the 7-8mm height budget).

**Assessment:** Adjacent footprints are technically feasible but waste ~25% of
the available PCB area. On a flex PCB at $2-5 prototype cost, the area increase
could push cost toward the upper end of the range, and the mechanical team must
accommodate the wider flex segment at the sensor location.

#### Strategy C: Shared Aperture with Breakout Pads

Place the optical aperture at a fixed location. Route a short flex tail from the
aperture to a standardized pad array. Attach the sensor-specific daughter board
(PAW3204 or PMW3360 on a small rigid PCB segment) to this pad array.

This is effectively a modular connector approach. The "daughter board" is a
~6mm x 6mm rigid PCB segment carrying the sensor, its decoupling caps, and (for
PMW3360) the 1.8V LDO. The main flex PCB carries the MCU, battery, charging,
BLE antenna, and click mechanism.

**Assessment:** Cleanest electrical solution. Adds one flex-to-rigid interface
(the daughter board connection), which is a reliability concern for a wearable
device. Cost of the daughter board is ~$0.50-1.00 at prototype quantity (small
2-layer rigid PCB). Adds ~1mm height if the daughter board stacks on the main
flex.

---

## 4. Routing Analysis: Shared vs. Divergent Traces

### MCU Pin Allocation (ESP32-C3, Current Kconfig)

| GPIO | PAW3204 Assignment | PMW3360 Assignment | Shared? |
|------|--------------------|--------------------|---------|
| 4    | SCLK (bit-bang)    | SCLK (SPI peripheral) | **Same physical pin, different driver mode** |
| 5    | SDIO (bidirectional) | MOSI               | **Same physical pin, incompatible function** |
| 6    | *(not used by PAW3204 -- no NCS)* | NCS   | PMW3360 only |
| 7    | *(not used)*       | MISO               | PMW3360 only |
| 8    | Snap dome click    | Snap dome / piezo ADC | Shared (click, not sensor) |
| --   | *(not needed)*     | MOT (interrupt)    | PMW3360 only; needs one more GPIO |

**Pin 5 conflict analysis:** In the PAW3204 configuration, GPIO5 is SDIO -- a
bidirectional data pin that the MCU alternately drives and reads. In the PMW3360
configuration, GPIO5 is MOSI -- a unidirectional output from MCU to sensor.
The PMW3360 also needs MISO on a separate pin (GPIO7). A dual-footprint PCB
must route GPIO5 to:

- The PAW3204 SDIO pad (when PAW3204 is stuffed)
- The PMW3360 MOSI pad (when PMW3360 is stuffed)

These are electrically compatible since both are driven by the MCU in the
outbound direction. The conflict is in the inbound direction: PAW3204 reads
data back on the same pin (GPIO5 as input), while PMW3360 sends data back on
MISO (GPIO7). On a dual-footprint PCB:

- GPIO5 trace routes to both the PAW3204 SDIO pad and the PMW3360 MOSI pad.
  When PAW3204 is populated, GPIO5 does bidirectional serial. When PMW3360 is
  populated, GPIO5 does MOSI only. The unstuffed sensor's pad is floating (no
  load, acceptable).
- GPIO7 trace routes to the PMW3360 MISO pad only. When PAW3204 is populated,
  GPIO7 is unused (configure as input with pull-down to avoid floating).

**This works.** GPIO4 and GPIO5 can be physically shared between both sensors
on the same PCB traces. GPIO6 and GPIO7 are PMW3360-only traces that simply
have no destination when PAW3204 is stuffed.

### Trace Routing Summary

| Trace             | PAW3204 Build   | PMW3360 Build   | Shared Copper? |
|-------------------|-----------------|-----------------|----------------|
| GPIO4 -> SCLK    | Bit-bang clock   | SPI CLK         | **Yes**        |
| GPIO5 -> SDIO/MOSI | Bidirectional serial | SPI MOSI | **Yes**        |
| GPIO6 -> NCS     | Unused (floating)| SPI chip select | PMW3360 only   |
| GPIO7 -> MISO    | Unused           | SPI MISO        | PMW3360 only   |
| GPIOx -> MOT     | Not present      | Motion interrupt | PMW3360 only   |
| GPIOx -> NRESET  | Sensor reset     | Sensor reset    | **Yes**        |
| 3.3V -> VDD      | Sensor power     | VDD_LED + VDDIO (if run at 3.3V) | **Yes** |
| 1.8V -> AVDD     | Not present      | Analog power    | PMW3360 only   |
| GND              | Sensor ground    | Sensor grounds  | **Yes**        |
| LED anode         | VLED (3.3V rail) | LED_A (driven)  | **No** (different circuit) |
| LED ground        | LED_GND          | LED_C_GND       | **Yes** (both to GND) |

Shared traces: 5 (SCLK, SDIO/MOSI, NRESET, 3.3V, GND).
PMW3360-only traces: 4 (NCS, MISO, MOT, 1.8V AVDD).
Divergent traces: 1 (LED anode drive circuit).

---

## 5. Bill of Materials Delta

Components present on a dual-footprint PCB that are only populated for one
variant:

### Standard Build (PAW3204 Stuffed)

| Component              | Populated? | Notes                          |
|------------------------|------------|--------------------------------|
| PAW3204DB-TJ3L         | **Yes**    | 8-pin SOP                     |
| PAW3204 lens/clip      | **Yes**    | Sensor-specific optic          |
| PAW3204 decoupling cap | **Yes**    | 0.1uF on VDD                  |
| PMW3360DM-T2QU         | No         | QFN pads present, unpopulated  |
| PMW3360 lens/clip      | No         |                                |
| 1.8V LDO (AVDD)       | No         | Pads present, unpopulated      |
| AVDD decoupling caps   | No         | Pads present, unpopulated      |
| VDDIO decoupling cap   | No         | Pads present, unpopulated      |
| MOT pull-up resistor   | No         | Pad present, unpopulated       |

### Pro Build (PMW3360 Stuffed)

| Component              | Populated? | Notes                          |
|------------------------|------------|--------------------------------|
| PAW3204DB-TJ3L         | No         | SOP pads present, unpopulated  |
| PAW3204 lens/clip      | No         |                                |
| PMW3360DM-T2QU         | **Yes**    | 16-pin QFN                    |
| PMW3360 lens/clip      | **Yes**    | Sensor-specific optic          |
| 1.8V LDO (AVDD)       | **Yes**    | e.g., RT9080-18GJ5 or AP7333  |
| AVDD decoupling caps   | **Yes**    | 1uF + 0.1uF                   |
| VDDIO decoupling cap   | **Yes**    | 0.1uF (even if run at 3.3V)   |
| MOT pull-up resistor   | **Yes**    | 10k to VDDIO                  |

Depopulated components cost nothing in BOM but cost board area. The
unpopulated pads for the other sensor plus its supporting components consume
approximately 8-10mm^2 of PCB area.

---

## 6. Lens/Clip Assembly Incompatibility

This is the mechanical blocker that the electrical analysis alone does not
capture. Both sensors require a sensor-specific lens and clip assembly mounted
directly above the sensor IC:

| Parameter           | PAW3204                        | PMW3360                        |
|---------------------|--------------------------------|--------------------------------|
| Lens part           | PixArt-specified or compatible  | PixArt ADNS-6190-002 or equivalent |
| Focal distance      | 2.4-3.2mm from surface         | ~2.4mm from surface (ball-read variant uses different optic) |
| Clip mounting       | Snaps onto 8-pin package body  | Snaps onto QFN package body    |
| Optical window      | ~3mm diameter                  | ~2mm diameter                  |
| LED/laser position  | Integrated with clip           | Integrated with clip           |

The lens clips are physically keyed to the sensor package. An 8-pin SOP clip
does not fit a 16-pin QFN, and vice versa. **The optical assembly above the
sensor is entirely sensor-specific.** This means the shell aperture and lens
mount geometry are also sensor-specific.

On a dual-footprint PCB, the shell must accommodate either clip. If the sensors
are at the same PCB location (overlapping footprints), the aperture hole must be
large enough for the larger clip, with each clip variant using an adapter ring or
friction fit. If the sensors are at adjacent locations, the shell needs two
aperture options (one blocked off per build). Neither is elegant.

---

## 7. Verdict

**A single dual-footprint PCB is technically feasible but is not the recommended
approach.**

### What Works

- **Signal routing is shareable.** GPIO4 (SCLK) and GPIO5 (SDIO/MOSI) can
  serve both sensors from the same traces, with firmware selecting bit-bang vs.
  SPI mode at boot. GPIO6, GPIO7, and a MOT interrupt pin are PMW3360-only
  traces that add 3 copper runs and occupy minimal routing area.
- **Power routing is additive.** The PMW3360 needs one additional LDO and 3
  decoupling caps beyond what PAW3204 requires. These pads can be present and
  depopulated on Standard builds.
- **Shared ground plane.** Both sensors ground to the same copper pour.
- **Shared MCU, BLE, battery, charging, click.** Everything except the sensor
  area is identical between builds.

### What Does Not Work

- **Overlapping footprints are not feasible.** The SOP-8 and QFN-16 pad
  geometries physically conflict. Center ground pad of the QFN collides with
  SOP trace routing. Pad pitches differ by 2.5x (1.27mm vs 0.5mm). Combined
  land pattern violates manufacturability DRC rules for any standard fab.
- **Adjacent footprints waste 25% of scarce board area.** The ring flex PCB
  has approximately 20mm x 12mm usable area. Two sensor footprints plus their
  decoupling and LED circuits occupy ~10mm x 10mm total. Only one is populated
  per build. This dead area is in the most constrained zone of the PCB (the
  sensor end, which must fit within the 7-8mm height budget with lens clip
  above).
- **Lens/clip assemblies are mechanically incompatible.** The shell aperture
  and mechanical mounting differ per sensor. A dual-footprint PCB does not
  escape this -- the shell still needs sensor-specific geometry.
- **Cost savings are marginal.** The flex PCB cost driver is area and layer
  count, not NRE. A dual-footprint PCB that is 25% larger to accommodate both
  sensor zones costs more per unit than two smaller single-purpose PCBs.

### Recommendation: Two PCB Variants with Maximum Shared Design

Instead of one dual-footprint PCB, design two PCB variants that share as much
as possible:

| PCB Region              | Standard PCB         | Pro PCB              | Shared? |
|-------------------------|----------------------|----------------------|---------|
| MCU + antenna area      | Identical            | Identical            | **Yes** |
| BLE antenna keepout     | Identical            | Identical            | **Yes** |
| Battery pads + charging | Identical            | Identical            | **Yes** |
| LDO (3.3V)             | Identical            | Identical            | **Yes** |
| Click input area        | Dome GPIO pad        | Piezo ADC + LRA PWM  | **No**  |
| Sensor zone             | PAW3204 SOP-8 pads   | PMW3360 QFN-16 pads  | **No**  |
| Sensor decoupling       | 1x 0.1uF             | 3x caps + 1.8V LDO  | **No**  |
| GPIO4 trace (SCLK)     | To sensor SCLK       | To sensor SCLK       | **Yes** |
| GPIO5 trace (data)     | To sensor SDIO       | To sensor MOSI       | **Yes** |
| GPIO6 trace (NCS)      | Not routed           | To sensor NCS        | Pro only|
| GPIO7 trace (MISO)     | Not routed           | To sensor MISO       | Pro only|
| MOT interrupt trace    | Not routed           | To sensor MOT        | Pro only|
| USB-C connector        | Identical            | Identical            | **Yes** |
| Ground plane           | Identical            | Identical            | **Yes** |

### Minimum Delta Between Variants

The two PCBs differ in exactly one zone: the sensor area (~8mm x 8mm at the
optical end of the flex). Everything else -- MCU, antenna, battery, charging,
USB-C, LDO, ground plane, and the main signal bus -- is identical copper.

**EE workflow:** Design the Standard PCB first. To create the Pro variant:

1. Delete the PAW3204 SOP-8 footprint and its decoupling cap.
2. Place the PMW3360 QFN-16 footprint at the same optical aperture location.
3. Add the 1.8V LDO (SOT-23-5) and its decoupling caps adjacent to the sensor.
4. Add VDDIO decoupling cap.
5. Route GPIO6 to NCS, GPIO7 to MISO, and one spare GPIO to MOT.
6. Adjust the LED drive circuit (PMW3360 LED_A is driven differently than
   PAW3204 VLED).
7. Update the click area if the Pro variant uses piezo + LRA instead of dome.
8. Run DRC. The rest of the board is unchanged.

This is a 30-60 minute layout modification, not a from-scratch redesign. The
schematic delta is ~8 components. The Gerber files diverge only in the sensor
zone. Both variants can be panelized on the same flex PCB production run if the
fab supports mixed panels (common at prototype quantity).

### Cost Impact

| Item                          | Single Dual-Footprint | Two Separate PCBs     |
|-------------------------------|----------------------|------------------------|
| PCB area (flex)               | ~25% larger (wasted pads) | Minimal per variant |
| NRE (PCB tooling)             | 1x                   | 2x (but delta is small for flex) |
| Per-unit PCB cost (prototype) | $3.00-6.00 (larger)  | $2.00-5.00 each (smaller) |
| Assembly complexity           | Higher (must track which pads to populate) | Lower (each board has only its own pads) |
| BOM management                | One PCB SKU, two stuff lists | Two PCB SKUs, each with one stuff list |

At prototype quantities (10-50 units), the NRE difference between one and two
flex PCB designs is ~$50-100 (one additional tooling charge). The per-unit
savings from smaller, simpler boards likely offset this. At production
quantities (1000+), two smaller PCBs are unambiguously cheaper than one
oversized dual-footprint.

---

## 8. Upgrade Path Implications

The CONSUMER-TIERS.md document specifies that a Standard-to-Pro upgrade should
be possible without full device replacement. With two separate PCBs, an upgrade
requires replacing:

1. **The sensor-end PCB section** (if rigid-flex with a separable rigid sensor
   island) or the **entire flex PCB** (if the flex is one continuous piece).
2. **The lens/clip assembly.**
3. **The shell** (sealed vs vented).
4. **Populating the 1.8V LDO** (or it comes pre-populated on the Pro PCB).

If the flex PCB is designed as rigid-flex with a separable sensor island
connected via a flex tail, the upgrade reduces to swapping a ~8mm x 8mm rigid
board segment. This is the strongest argument for the daughter-board approach
(Strategy C above) -- not as a dual-footprint, but as a modular interface that
enables field upgrade.

**Recommendation for upgrade support:** Design the flex PCB with a standardized
6-pin flex connector at the sensor end:

| Pin | Signal    | Notes                              |
|-----|-----------|------------------------------------|
| 1   | 3.3V      | Main power rail                    |
| 2   | GND       | Ground                             |
| 3   | GPIO4     | SCLK (bit-bang or SPI)             |
| 4   | GPIO5     | SDIO / MOSI                        |
| 5   | GPIO6     | NCS (PMW3360) / NC (PAW3204)       |
| 6   | GPIO7     | MISO (PMW3360) / NC (PAW3204)      |

The MOT interrupt and NRESET can share pins 5/6 via the daughter board's own
routing, or a wider 8-pin connector can be used to carry them separately. The
sensor daughter board carries the sensor IC, its lens clip, the LED circuit,
the 1.8V LDO (if PMW3360), and all decoupling caps.

This connector adds ~$0.20 and one reliability point. Whether it is worth it
depends on whether upgrade-in-field is a real user story or a theoretical nice-
to-have. For prototyping, it is not needed -- two separate full PCBs are simpler.
For a consumer product, it enables the CONSUMER-TIERS.md upgrade promise.

---

## 9. Summary Table

| Question                                      | Answer                          |
|-----------------------------------------------|---------------------------------|
| Can both sensors share the same MCU GPIO pins? | Yes, for SCLK and data-out. PMW3360 needs 3 additional GPIOs. |
| Can both sensors share the same power rail?    | No. PMW3360 needs an additional 1.8V LDO. |
| Can both sensors share the same PCB footprint? | No. SOP-8 and QFN-16 pads physically conflict. |
| Can both sensors share the same lens/clip?     | No. Lens clips are keyed to package geometry. |
| Is a dual-footprint (adjacent) PCB feasible?   | Technically yes, but wastes ~25% of PCB area. |
| Is a dual-footprint PCB cost-effective?        | No. Two smaller PCBs are cheaper at any quantity. |
| What is the minimum delta between two PCBs?    | ~8mm x 8mm sensor zone: footprint, LED circuit, decoupling, 1.8V LDO. |
| What is shared between two PCBs?               | MCU, antenna, battery, charging, USB-C, 3.3V LDO, ground, GPIO4/5 routing (~75% of copper). |
| Is field upgrade possible?                     | Yes, if a modular flex connector is used at the sensor boundary. |
| Recommended approach?                          | Two PCB variants with shared base design. Optional daughter-board connector for consumer product upgrade path. |
