# Defensive Publication — PowerFinger Assistive Input Device Family

This document prepares content for external publication venues that establish
prior art for the PowerFinger design space. The repository itself is a defensive
publication (every commit is a timestamped enabling disclosure), but patent
examiners search Google Scholar, Hackaday.io, and OSHWA registries more readily
than GitHub. These external publications make the prior art discoverable.

References:
- Design matrix: [COMBINATORICS.md](COMBINATORICS.md)
- IP strategy: [IP-STRATEGY.md](IP-STRATEGY.md)
- Patent claim map: [PATENT-CLAIM-MAP.md](PATENT-CLAIM-MAP.md)
- Click mechanisms: [CLICK-MECHANISMS.md](CLICK-MECHANISMS.md)
- Glide system: [GLIDE-SYSTEM.md](GLIDE-SYSTEM.md)
- Prototype spec: [PROTOTYPE-SPEC.md](PROTOTYPE-SPEC.md)
- Consumer tiers: [CONSUMER-TIERS.md](CONSUMER-TIERS.md)
- Power budget: [POWER-BUDGET.md](POWER-BUDGET.md)
- Wand competitive landscape: [WAND-COMPETITIVE.md](WAND-COMPETITIVE.md)
- Firmware roadmap: [FIRMWARE-ROADMAP.md](FIRMWARE-ROADMAP.md)

---

## 1. Hackaday.io Project Page Content

*Ready to post. Copy the text below directly into the Hackaday.io project
creation form.*

---

### Project Title

**PowerFinger — Open-Source Assistive Ring Mouse and Wand Family**

### Tagline

Fingertip rings and pen wands that work as BLE HID mice on any surface, for
anyone, at $9 BOM. Hardware: CERN-OHL-S 2.0. Firmware: MIT.

### Project Description

#### The Problem

Computer pointing devices assume an able-bodied user sitting at a desk with a
flat surface and a free hand that can grip a mouse. For people with limited
mobility, limited grip strength, paralysis, tremor conditions, or no access to a
flat surface, standard mice are partially or completely unusable. Existing
assistive alternatives (head trackers, sip-and-puff controllers, joystick mice)
cost $100--500, are single-purpose, and are proprietary.

There is no pointing device under $25 that works on arbitrary surfaces, fits on
a fingertip, and connects to any computer as a standard HID mouse. PowerFinger
fills that gap.

#### The Solution

PowerFinger is a family of open-source assistive input devices:

**Fingertip rings** that sit on the distal phalanx (fingertip pad) of the middle
finger. The user drags their finger across any surface to move the cursor. Each
ring is a complete BLE HID mouse. Two identical rings compose into a full mouse
system: one ring for cursor + left click, another for scroll + right click. A
USB hub dongle (ESP32-S3) pairs with multiple rings over BLE and presents a
single unified USB HID mouse to the host OS.

**Pen wands** with a ball-and-Hall-effect sensor at the tip. Pen grip, drag on
any surface at any angle (30--70 degrees from horizontal), BLE HID output. The
wand works on glass, fabric, skin, mirrors -- surfaces where optical pen mice
fail.

**Key system characteristics:**
- BLE HID -- works with macOS, Windows, Linux, iOS, Android. No driver, no app
  required for basic function.
- Companion app (optional) for configuration: DPI, button mapping, multi-ring
  role assignment, calibration.
- No cloud dependency. Every device functions fully offline.
- Every component is replaceable. Commodity parts with multiple source vendors.
  No planned obsolescence.

#### Design Space: 576 Variants

The full design matrix spans 576 unique configurations across 6 orthogonal axes:

**Form factors (8):**
- Fingertip ring at 4 sensor angles (0, 15, 30, 45 degrees from horizontal)
- Nail-mount ring (sensor on dorsal fingertip, typing-compatible) at 2 angles
- Wrist bracelet (sensor on ulnar wrist, forearm-slide cursor) in 2 styles
- Tethered sensor (sub-gram sensor dot on fingertip, flex cable to wrist hub)
  with 4 sensor options
- Pen wand with standard tip
- Pen wand with retractable tip

**Sensing mechanisms (5 + hybrids):**
- **S-BALL:** Mechanical ball (5--8mm steel) in socket with 4 roller spindles,
  magnets on rollers, 4 SMD Hall effect sensors (DRV5053). Surface-agnostic.
  ~15--60 DPI equivalent. BOM ~$2.50.
- **S-OLED:** Direct optical LED sensor (PAW3204/ADNS-2080 class). 800--2000
  CPI. Most opaque surfaces, fails on glass. BOM ~$0.50--4.
- **S-VCSL:** Direct laser VCSEL sensor. 2000+ CPI. Wider surface range than
  LED, still fails on glass. BOM ~$3--6.
- **S-OPTB:** Optical sensor (PMW3360 class) aimed at a captive ball, reading
  ball surface texture. Surface-agnostic. 100--12,000 CPI. No rollers. BOM
  ~$6--9.
- **S-IMU:** 6-axis IMU (BMI270/MPU-6050/LSM6DSO). Air mouse -- no surface
  needed. Tracks angular velocity and acceleration. BOM ~$0.50--2.
- **IMU hybrid:** Any surface sensor + IMU. Auto-switches between surface
  tracking (high precision) and air tracking (when lifted) using the optical
  sensor's SQUAL register or Hall baseline shift as lift detection. The most
  accessible configuration.

**Click mechanisms (2 recommended):**
- **Metal snap dome** (Snaptron SQ-series): 0.15--0.2mm tall, 5--10M cycles,
  excellent tactile feel, $0.05--0.15. Default for prototypes.
- **Piezo film + LRA haptic:** Piezoelectric polymer film (150 microns) detects
  finger pressure, linear resonant actuator provides simulated click feedback.
  No moving parts in sensor (infinite lifecycle). Fully sealable ring --
  moisture-proof, debris-proof. Premium option. $1--2.

**Optional capabilities:**
- OCR camera (OV2640 + ESP32-S3): text scanning from books/documents. +$5--8.
- Guidance laser (4mm 650nm Class 1 dot): red dot shows scan/point target.
  +$2--3.
- IMU hybrid mode: surface + air auto-switch. +$0.50--2.

**Connectivity:**
- Single ring: BLE HID direct to host (phone, laptop, tablet).
- Multi-ring: N rings pair with USB hub dongle (ESP32-S3) via BLE. Hub composes
  all ring events into one USB HID mouse report. OS sees one mouse.
- Wand: same BLE HID, works standalone or through hub.

#### Key Innovations Documented as Prior Art

**1. Multi-ring composition via hub dongle.** N fingertip ring mice, each a
BLE HID peripheral, connect to a single USB hub dongle (ESP32-S3 with BLE 5.0
central + USB OTG). The hub implements a role engine that assigns semantic
meaning to each ring's input (cursor, scroll, left click, right click, modifier)
and composes their events into a single USB HID mouse report. The host OS sees
one mouse. Role assignment is configurable: default is first ring = cursor +
left click, second ring = scroll + right click. Any number of rings can compose.
The role engine firmware is at `firmware/hub/components/role_engine/`. The event
composer that merges ring inputs is at
`firmware/hub/components/event_composer/`.

**2. Optical-on-ball ring form factor with high-CPI sensor.** A PMW3360-class
optical sensor aimed at a captive ball inside a fingertip ring shell. The sensor
reads the ball's surface texture as the ball rolls, not the desk surface. This
gives 100--12,000 CPI resolution with full surface agnosticism (the ball touches
the surface, the sensor reads the ball). Unlike the traditional optical ring
mouse that fails on glass, this works on any surface the ball can roll on. The
ball is the only moving part -- no rollers, no encoder wheels. The ring shell is
a 3D-printed parametric design accepting finger circumference and sensor angle
as parameters.

**3. Hall effect ball ring for surface agnosticism at minimal BOM.** A 5--8mm
steel ball in a socket with 4 roller spindles, each carrying a small magnet. 4
SMD Hall effect sensors (DRV5053) detect roller rotation magnetically. Entirely
sealed -- no optical path to contaminate. Surface-agnostic: works on glass,
fabric, skin, any surface the ball can roll on. ~15--60 DPI equivalent, suitable
for cursor navigation with firmware interpolation. BOM ~$2.50 for the sensor
assembly. The DRV5053 sensors must be power-gated via MOSFET during sleep (no
built-in sleep mode, ~3mA continuous draw per sensor). Analysis at
`docs/BALL-DIAMETER.md`.

**4. Adaptive BLE connection interval.** Default 15ms connection interval during
idle and low-motion periods, adaptive switch to 7.5ms during active tracking.
At 7.5ms the ESP32-C3 BLE stack cannot enter light sleep between radio events,
roughly doubling power draw. At 15ms, inter-event light sleep reduces average
current from 15--30mA to 8--15mA. The firmware monitors sensor delta magnitude
and switches intervals dynamically. This provides <20ms end-to-end latency when
needed without the constant power penalty. Analysis at `docs/POWER-BUDGET.md`.

**5. Piezo film + LRA haptic sealed click mechanism.** A thin piezoelectric
polymer film (150 microns) laminated to the inner ring shell detects finger
pressure via ADC. A linear resonant actuator (already in BOM for haptic
feedback) provides simulated click feel -- short pulse on threshold crossing.
The combination enables a fully sealed ring with zero mechanical openings: no
dome travel gap, no debris ingress path, no moisture vulnerability. The piezo
sensor has no wear mechanism (effectively infinite lifecycle). The LRA is rated
for 10M+ actuations. Firmware implements ADC threshold calibration, dead zone
during haptic pulse (suppress cursor deltas during motor vibration), and
configurable press threshold. Analysis at `docs/CLICK-MECHANISMS.md` and
`docs/PIEZO-HAPTIC-ANALYSIS.md`.

**6. Parametric ring sizing for continuous finger circumference range.** The
ring shell is a parametric CAD model that accepts finger circumference (mm) and
sensor angle (degrees) as input parameters. A single design file produces rings
for any finger size -- no discrete S/M/L sizing. This is critical for
accessibility: users with atypical hand sizes (children, users with joint
conditions, etc.) get a device that fits. The parametric model also produces all
4 angle variants (0, 15, 30, 45 degrees) from one design.

**7. Raised rim glide system with replaceable UHMWPE pads.** Optical sensor
variants require 2.4--3.2mm focal distance to the tracking surface. A structural
rim (POM or nylon, integral to the ring shell) sets the geometry. Replaceable
UHMWPE (ultra-high molecular weight polyethylene) pads provide the contact
surface. UHMWPE chosen over PTFE to avoid PFAS regulatory risk. Pad geometry
provides tilt resistance and debris clearance. When pads wear (after years of
use), the user replaces $0.50 pads, not the ring. Full analysis at
`docs/GLIDE-SYSTEM.md`.

**8. Tethered sensor architecture.** The sensing element (<0.5g) lives on the
fingertip. MCU, battery, BLE radio, and click switch live on a wrist module
(5--8g). A 6-conductor flex ribbon cable connects them along the dorsal finger
surface under a silicone sleeve. This separates weight from precision: the
fingertip feels like a bare finger with a sticker, while the wrist carries all
the heavy components. BOM ~$7--10, under the $10 floor target. Battery can be
200--500mAh (weeks of life), antenna is in free air (no detuning from finger
proximity), thermal management is trivial. The firmware is identical to the
self-contained ring; the SPI bus simply runs through a longer cable.

**9. Nail-mount typing-compatible ring.** Electronics module on the dorsal
(nail side) fingertip surface. Finger pad remains exposed for typing. User
curls finger to point, straightens to type. The same finger does both without
removing the device. Requires module height of 3--4mm above nail to clear
adjacent keycaps during typing.

**10. Wrist bracelet pointing device.** Electronics and sensor on the ulnar
wrist via bracelet. Sensor faces the desk. User slides forearm to move cursor.
Vertical-mouse ergonomics (forearm in neutral pronation) with full typing
compatibility (fingers are free). A novel form factor combining surface tracking
with wrist-mounted convenience.

**11. Ball-in-pen-tip wand with Hall effect sensing.** A 5--8mm ball at the
pen tip with Hall effect rotation sensing. Angle-independent: works at 30--70
degree pen angles from horizontal (optical pen mice degrade past ~20 degrees).
Surface-agnostic. BLE HID output. No commercial pen mouse offers surface
agnosticism + angle independence + BLE (no dongle) at this price point ($14
BOM). The ball-in-pen-tip concept has expired prior art dating to 1991
(US5210405A, Matsushita). Analysis at `docs/WAND-COMPETITIVE.md`.

**12. Wand with retractable tip.** Ball or sensor retracts into the pen body
via spring + sleeve mechanism when not in use. Protects the sensing element from
pocket debris. +$2 BOM over standard tip.

#### BOM Estimates

| Variant | BOM | Description |
|---------|-----|-------------|
| R30-OLED-NONE-NONE (P0 ring) | ~$9 | Optical ring, dome click, 30 degrees |
| R30-BALL-NONE-NONE (P1 ring) | ~$11 | Ball+Hall ring, surface-agnostic |
| R30-OPTB-NONE-NONE (Pro ring) | ~$15--17 | Optical-on-ball, piezo sealed click |
| WSTD-BALL-NONE-NONE (wand) | ~$14 | Ball+Hall pen wand |
| USB hub dongle | ~$5--6 | ESP32-S3, BLE central + USB HID |
| Two-ring Standard system | ~$24 | 2x optical ring + hub |
| Two-ring Pro system | ~$40 | 2x optical-on-ball ring + hub |

Full BOM spreadsheets at `hardware/bom/`.

#### Architecture

```
                          HOST COMPUTER
                              |
                         USB HID Mouse
                              |
                     ┌────────────────┐
                     │  Hub Dongle    │
                     │  ESP32-S3      │
                     │  BLE Central   │
                     │  USB OTG       │
                     │  Role Engine   │
                     │  Event Composer│
                     └───┬────────┬───┘
                    BLE  │        │  BLE
              ┌──────────┘        └──────────┐
              │                              │
    ┌─────────────────┐            ┌─────────────────┐
    │  Ring 1         │            │  Ring 2         │
    │  ESP32-C3       │            │  ESP32-C3       │
    │  Sensor + Click │            │  Sensor + Click │
    │  BLE HID        │            │  BLE HID        │
    │  Role: Cursor   │            │  Role: Scroll   │
    │  + Left Click   │            │  + Right Click  │
    └─────────────────┘            └─────────────────┘
```

Each ring also works standalone as a BLE HID mouse paired directly to a host.
The hub is optional for multi-ring composition.

#### Firmware

Written in C using ESP-IDF (Apache 2.0 licensed framework). Source at
`firmware/ring/` and `firmware/hub/`. Key components:

- **BLE HID stack:** NimBLE-based, handles pairing, bonding, bond asymmetry
  recovery, connection interval negotiation.
- **Sensor drivers:** SPI interface to PAW3204 (optical), PMW3360
  (optical-on-ball), DRV5053 (Hall effect). Polling with dead zone filtering and
  calibration on boot.
- **Hub role engine:** Assigns semantic roles to connected rings. Configurable
  via BLE characteristic or companion app.
- **Hub event composer:** Merges X/Y deltas, click states, and scroll events
  from all connected rings into a single USB HID report at the hub's report
  rate.
- **Power management:** Adaptive connection interval (15ms idle, 7.5ms active).
  Light sleep between BLE events. Deep sleep with GPIO wake on motion or click.
  Hall sensor power gating via MOSFET.

#### License

- **Hardware:** CERN Open Hardware Licence Version 2, Strongly Reciprocal
  (CERN-OHL-S 2.0). All schematics, PCB layouts, 3D models, BOMs, and assembly
  instructions.
- **Firmware and software:** MIT License. All ESP-IDF code and companion app.
- The strongly reciprocal hardware license requires anyone who manufactures or
  distributes products based on these designs to publish their complete modified
  source. The patent retaliation clause terminates rights for any licensee who
  initiates patent litigation.

#### Contributing

This project is BDFL-led and accessibility-first. Contributions welcome.
Repository: https://github.com/fluffyrabbot/powerfinger

### Hackaday.io Project Tags

`assistive-technology`, `ble`, `esp32`, `open-hardware`, `mouse`,
`accessibility`, `ring-mouse`, `hid`, `cern-ohl-s`, `input-device`,
`3d-printing`, `pen-mouse`, `hall-effect`, `optical-sensor`

---

## 2. arXiv-Style Technical Note

### Suggested Venue

arXiv preprint, category: cs.HC (Human-Computer Interaction) or eess.SP
(Signal Processing). An arXiv preprint is indexed by Google Scholar and
accessible to USPTO patent examiners.

### Title

**PowerFinger: An Open-Source Assistive Input Device Family Using Fingertip Ring
and Pen Wand Form Factors with Multi-Ring Composition**

### Authors

PowerFinger Contributors (https://github.com/fluffyrabbot/powerfinger)

### Abstract

We present PowerFinger, an open-source family of assistive computer input
devices comprising fingertip rings and pen-shaped wands that function as
surface-agnostic Bluetooth Low Energy (BLE) Human Interface Device (HID)
mice. The design space spans 576 unique hardware configurations across six
orthogonal axes: form factor, sensing mechanism, sensor angle, click mechanism,
optional capabilities (IMU hybrid, OCR camera, guidance laser), and
connectivity mode. We describe a multi-ring composition protocol in which N
fingertip ring peripherals connect via BLE to a USB hub dongle that presents a
single unified mouse to the host operating system, with a configurable role
engine assigning semantic meaning (cursor, scroll, click) to each ring. We
detail six sensing mechanisms -- mechanical ball with Hall effect, direct
optical LED, direct laser VCSEL, optical-on-captive-ball, 6-axis IMU, and
hybrid surface/air -- with tradeoff analysis for resolution, surface
compatibility, power draw, and bill of materials cost. The cheapest viable
ring achieves a $9 BOM using commodity components (ESP32-C3, PAW3204 optical
sensor, metal snap dome switch, 80mAh LiPo) and the Pro variant achieves
surface agnosticism with 100-12,000 CPI resolution at a $17 BOM using a
PMW3360 optical sensor reading a captive ball with piezoelectric sealed click
mechanism. All hardware designs are released under CERN-OHL-S 2.0 (strongly
reciprocal) and all firmware under MIT license. The project is motivated by
the accessibility gap in pointing devices: no existing product under $25 offers
surface-agnostic pointing in a fingertip form factor with standard HID
connectivity.

*(198 words)*

### 1. Introduction

#### 1.1 The Accessibility Gap

Computer pointing devices are designed for a narrow use case: an able-bodied
user sitting at a desk, gripping a mouse with a free hand, sliding it on a flat
opaque surface. For the estimated 1.3 billion people worldwide with some form of
disability (WHO Global Report on Disability, 2011), this assumption fails in
multiple ways. Limited grip strength makes mouse gripping painful or impossible.
Limited shoulder/arm mobility prevents the sweeping motions mice require.
Wheelchair users may lack a suitable flat surface. Users in bed have no desk.
Tremor conditions make precision gripping unreliable.

Existing assistive pointing devices -- head trackers ($150--500), sip-and-puff
controllers ($200--400), joystick mice ($100--300) -- address severe mobility
limitations but are expensive, proprietary, and single-purpose. There is no
assistive pointing device under $25 that uses a fingertip form factor, works on
arbitrary surfaces, and connects as a standard HID device.

#### 1.2 Prior Art

The concept of a finger-worn pointing device is not new. US8648805B2 (Bailen,
2014) describes a fingertip mouse with optical/trackball/laser sensing.
US20130063355A1 (2013) covers a finger-mounted optical cursor sensor.
US20150241976A1 (2015) describes a ring housing with motion sensor and BLE.
Commercial products include the Padrone ring mouse (capacitive, desk-surface
only) and various Chinese-manufactured ring mice ($15--40, optical, poor
build quality). Apple's 2025 trackball Apple Pencil patent (US12,353,649)
covers optical sensing in a pen-tip ball, but is specific to optical flow and
laser speckle -- not Hall effect sensing.

None of the existing patents or products describe: (a) multi-ring composition
via a hub dongle, (b) Hall effect ball sensing in a ring form factor for surface
agnosticism at sub-$3 sensor BOM, (c) optical-on-captive-ball in a ring for
high-CPI surface-agnostic tracking, (d) piezo film sealed click mechanism for
infinite-lifecycle ring construction, (e) hybrid surface/air auto-switching via
SQUAL register monitoring, or (f) the tethered sensor architecture with sub-gram
fingertip element and wrist hub.

The ball-in-pen-tip concept has expired prior art dating to 1991 (US5210405A,
Matsushita). Wireless pen pointing has expired prior art from 1999
(US6633282B1, Monroe) and 2001 (US20010025289A1, Xybernaut, abandoned).

#### 1.3 Contribution

This paper documents the complete PowerFinger design space as an enabling
disclosure. We enumerate 576 hardware configurations with BOM estimates,
describe the system architecture (ring, hub, companion app), detail six sensing
mechanisms with quantitative tradeoff analysis, and specify the multi-ring
composition protocol. The full design files, firmware source, and BOMs are
published under open-source licenses at the project repository.

### 2. System Architecture

#### 2.1 Ring Hardware

Each PowerFinger ring is a self-contained BLE HID mouse. The hardware stack,
from finger pad to tracking surface, comprises: finger contact pad, LiPo cell
(80--100mAh), flex or rigid-flex PCB with ESP32-C3-MINI-1-N4 MCU, sensor module,
click mechanism, and (for optical variants) a raised structural rim with UHMWPE
glide pads maintaining 2.4--3.2mm focal distance.

The ring shell is a 3D-printed parametric design. Two parameters -- finger
circumference (mm) and sensor angle (degrees) -- generate the complete shell
geometry. The sensor angle determines how the ring sits when the finger contacts
a surface. At 30 degrees (the recommended default), the sensor aperture is flush
with the desk surface when the hand rests in its natural position. Four standard
angles are defined: 0, 15, 30, and 45 degrees. All angles use identical
electronics; only the shell geometry changes.

The height budget is approximately 10mm from finger pad to surface, with the
sensor, PCB, and battery occupying 7--8mm above the surface and a 2.5--3mm air
gap (or rim height) below. This constraint drives all component selection: the
ESP32-C3-MINI-1-N4 (13.2 x 12.5 x 2.4mm) is chosen over larger modules, the
LiPo cell is limited to 80--100mAh to fit the available volume, and the click
mechanism must add minimal height (snap domes are 0.15--0.2mm, piezo film is
0.15mm).

#### 2.2 Hub Dongle

The hub dongle is an ESP32-S3 module in a USB-stick form factor. It operates as
a BLE 5.0 central, connecting to multiple PowerFinger peripherals simultaneously,
and as a USB OTG device presenting a single USB HID mouse to the host OS.

The hub contains two key firmware components. The **role engine**
(`firmware/hub/components/role_engine/`) assigns semantic roles to each connected
ring: one ring becomes the cursor source (X/Y deltas mapped to mouse movement),
another becomes the scroll source (X/Y deltas mapped to scroll wheel), and each
ring's click event is mapped to a specific mouse button. Roles are assigned by
connection order (first ring = cursor + button 1, second = scroll + button 2) and
are reconfigurable via BLE characteristic or companion app.

The **event composer** (`firmware/hub/components/event_composer/`) merges events
from all connected rings into a single USB HID mouse report at the host's
polling rate. When multiple rings report simultaneously, the composer applies
the role mapping: cursor deltas from ring 1 become mouse X/Y, cursor deltas
from ring 2 become scroll V/H, click states are OR-combined per their button
assignments. The composed report is a standard USB HID mouse report -- the host
OS requires no driver or special support.

#### 2.3 Companion App

An optional companion application (planned as Flutter cross-platform or PWA)
provides configuration beyond what BLE characteristics offer: visual DPI/
sensitivity adjustment, multi-ring role assignment with drag-and-drop, surface
calibration wizards, firmware OTA updates, and battery status monitoring. The
companion app communicates with rings and hub via BLE GATT. No companion app is
required for basic operation -- a single ring works as a standard BLE HID mouse,
and multi-ring composition works through the hub with default role assignments.

### 3. Sensing Mechanisms

#### 3.1 Direct Optical LED (S-OLED)

An optical mouse sensor (PAW3204DB or ADNS-2080 class) mounted in the ring,
looking down at the tracking surface through an aperture in the ring shell. An
LED provides surface illumination. The sensor captures successive images of the
surface texture and computes X/Y displacement via image correlation.

Resolution: 800--2000 CPI, configurable via SPI register. Power: 1.2--2.3mA
active, 12--75uA sleep (automatic, sensor-managed). Surface compatibility: most
opaque matte and semi-gloss surfaces. Fails on glass (transparent, no texture),
mirrors (specular reflection), and uniform white surfaces (no contrast). BOM:
$0.50--4 depending on sensor grade. The raised rim glide system (documented in
`docs/GLIDE-SYSTEM.md`) maintains the required 2.4--3.2mm focal distance.

#### 3.2 Direct Laser VCSEL (S-VCSL)

Same architecture as S-OLED but with VCSEL (vertical-cavity surface-emitting
laser) illumination instead of LED. The coherent laser light creates speckle
patterns on surfaces that are featureless under LED illumination, extending the
range of trackable surfaces to include glossy and lacquered finishes.

Resolution: 2000+ CPI. Surface compatibility: broader than S-OLED, still fails
on glass and mirrors. BOM: $3--6 (VCSEL module cost). Same focal distance
requirement and glide system.

#### 3.3 Mechanical Ball + Hall Effect (S-BALL)

A 5--8mm steel ball sits in a socket in the ring shell. Four roller spindles
contact the ball and rotate as the ball rolls. Each roller carries a small
magnet. Four SMD Hall effect sensors (DRV5053) detect the magnetic field
changes as the rollers spin, resolving X and Y displacement.

This mechanism is fully surface-agnostic: the ball contacts the surface
directly, and the Hall sensors detect rotation magnetically with no optical
path to contaminate. It works on glass, fabric, skin, mirrors -- any surface
the ball can roll on. Resolution is limited by the magnetic encoding: ~9--36
ticks per revolution of a 5mm ball yields approximately 15--60 DPI equivalent.
Firmware interpolation and acceleration curves compensate for the low native
resolution, making it suitable for cursor navigation but not precision drawing.

Power: ~12mA active (4 sensors at 3mA each), must be power-gated via MOSFET
during sleep. BOM: ~$2.50 for ball + rollers + magnets + 4x DRV5053. Diameter
and material analysis at `docs/BALL-DIAMETER.md`.

#### 3.4 Optical on Captive Ball (S-OPTB)

A PMW3360-class high-performance optical sensor aimed at a captive ball inside
the ring shell. The sensor reads the ball's surface texture as the ball rolls
against the tracking surface, not the desk surface itself. This combines surface
agnosticism (the ball touches the desk) with high resolution (the optical sensor
achieves 100--12,000 CPI on the ball's consistent surface texture).

The ball is the only moving part -- no rollers or encoder wheels. The sensor's
focal distance is internal (sensor to ball surface), independent of the desk
surface. The ring does not require a raised rim or glide pads for this variant.

Power: 16.3--37mA active depending on performance mode, 32--61uA deep rest.
BOM: $6--9 for the PMW3360-class sensor. This is the recommended Pro tier
sensor: surface-agnostic, high resolution, single moving part.

#### 3.5 6-Axis IMU (S-IMU)

A 6-axis inertial measurement unit (BMI270 recommended over MPU-6050 for 5x
power efficiency) tracks the ring's angular velocity and acceleration in free
space. Cursor movement is derived from hand tilt and rotation. No surface
contact is required.

This is the most accessible sensing mode: it works in bed, in a wheelchair
without a tray table, standing, walking -- anywhere. The tradeoff is precision
(gyroscope drift, lower spatial resolution than optical) and the need for
drift compensation algorithms (high-pass filtering, accelerometer gravity
reference, auto-centering after inactivity).

Power: BMI270 draws 685uA full mode, 14uA low-power accelerometer. BOM:
$0.50--2. The sensor angle parameter in the ring shell becomes purely
ergonomic (comfort) rather than functional.

#### 3.6 IMU Hybrid (Surface + Air Auto-Switch)

Any surface-tracking sensor (S-OLED, S-VCSL, S-BALL, S-OPTB) combined with an
IMU. The firmware monitors the surface sensor's quality metric -- the SQUAL
register for optical sensors, or Hall baseline shift for ball sensors -- and
auto-switches between surface tracking (high precision, no drift) and air
tracking (IMU, moderate precision, drift-compensated) with no user action.

This enables a usage pattern where a wheelchair user tracks on an armrest
(surface mode), then lifts their hand to point at a screen across the room
(air mode). The transition is seamless. BOM add: $0.50--2 for the IMU.

### 4. Multi-Ring Composition Protocol

The multi-ring composition protocol enables N BLE HID ring peripherals to
function as a single logical mouse through a USB hub dongle.

Each ring is an independent BLE HID mouse peripheral. It advertises, pairs,
and reports standard HID mouse data (X/Y deltas, button states) over GATT.
The hub dongle is a BLE central that connects to all advertising PowerFinger
devices and subscribes to their HID report notifications.

The role engine on the hub maintains a role table:

| Ring Index | X/Y Role | Button Role |
|-----------|---------|------------|
| 0 (first connected) | Mouse cursor X/Y | Left click (button 1) |
| 1 (second connected) | Scroll wheel V/H | Right click (button 2) |
| 2+ (additional) | Configurable | Configurable |

The event composer runs at the USB HID polling interval (default 8ms, matching
125Hz USB report rate). Each cycle, it reads the latest report from each
connected ring, applies the role mapping, and produces one USB HID mouse
report containing the composed cursor position, scroll values, and button
states.

This architecture is extensible: three rings could provide cursor + scroll +
precision mode switching. Four rings could map to a gamepad. The role
assignments are stored in NVS and reconfigurable via BLE GATT characteristic
or companion app.

### 5. Power Management Strategy

Power management for the ring targets 3--6 days battery life on an 80--100mAh
LiPo cell using ESP32-C3.

The dominant power consumer is the BLE radio. At 7.5ms connection intervals, the
BLE stack cannot enter light sleep between radio events, drawing 15--30mA
continuous. At 15ms intervals, inter-event light sleep reduces this to 8--15mA.
The firmware implements adaptive connection interval switching: 15ms during idle
or low-motion periods, 7.5ms during active tracking (when sensor deltas exceed
a threshold). This provides sub-20ms end-to-end latency during active use without
the constant power penalty.

Sensor power management varies by type. The PAW3204 (S-OLED) has automatic sleep
state transitions: 2.3mA active drops to 75uA within 256ms of no motion, then to
12uA after 61 seconds. No firmware intervention needed. The DRV5053 Hall sensors
(S-BALL) have no sleep mode and must be power-gated via external MOSFET. The
PMW3360 (S-OPTB) has rest modes that bring its 16.3mA active draw down to 32uA.

System-level sleep states:
- **Active:** MCU at 80MHz, sensor polling, BLE connected at 7.5--15ms.
- **Light sleep:** MCU in light sleep between BLE events, sensor in auto-sleep.
- **Deep sleep:** MCU in deep sleep (5--8uA), sensor powered down, BLE
  disconnected. GPIO wake on dome press or sensor motion detect pin.

The LDO (RT9080-33GJ5) is selected for 0.5uA quiescent current, which dominates
the deep sleep power budget. The more common AP2112K at 55uA quiescent would
waste 100x more power in deep sleep.

Consumer product generation should evaluate migration to nRF52840 for 3--5x
battery life improvement. Analysis at `docs/NRF52840-MIGRATION.md`.

### 6. Open Questions and Future Work

**Surface characterization.** Systematic testing of all sensing mechanisms across
a defined surface matrix (wood, glass, fabric, paper, glossy magazine, matte
plastic, skin, mirror) is needed to quantify tracking quality rather than
binary works/fails. A standardized test rig that measures CPI accuracy, jitter,
and maximum tracking speed per surface per sensor type would enable data-driven
tier recommendations.

**IMU fusion algorithms.** The hybrid surface/air mode requires tuning of the
SQUAL-to-IMU transition threshold, IMU drift compensation parameters, and
handoff smoothness. Sensor fusion approaches (complementary filter, Kalman
filter) need comparative evaluation for latency and precision on this specific
hardware.

**Click feel validation.** The piezo film + LRA haptic click is simulated, not
mechanical. User testing with accessibility-focused participants must determine
whether the simulated feel is acceptable, particularly for users with reduced
tactile sensitivity.

**Multi-ring latency.** Composition through the hub adds latency (BLE round-trip
from ring to hub + USB report to host). Measuring and optimizing this path is
critical for the two-ring system to feel responsive.

**nRF52840 migration.** The ESP32-C3 is the prototype MCU. For consumer
production, migration to nRF52840 (Zephyr RTOS, SoftDevice BLE stack) would
yield 3--5x battery life improvement but requires a complete firmware port.

**Companion app.** The configuration companion app (DPI, role assignment,
calibration) is defined but not yet implemented.

---

## 3. OSHWA Certification Materials

### What Is OSHWA Certification

The Open Source Hardware Association (OSHWA) maintains a self-certification
program for open-source hardware. Certified projects receive an OSHWA UID
(e.g., US000123) and are listed in the OSHWA certification directory, which is
searchable by patent examiners and the public. Certification is free and
self-administered.

### Required Documentation Checklist

| Requirement | Description | Status | Repo File(s) |
|-------------|-------------|--------|-------------|
| **Hardware source files** | Schematic, PCB layout, 3D model in editable format (KiCad, FreeCAD, etc.) | Not yet created | `hardware/` — directory exists but contains only BOMs. Schematics, PCB layouts, and CAD models needed. |
| **Bill of materials** | Complete BOM with part numbers, quantities, and sources | Done (3 variants + hub) | `hardware/bom/R30-OLED-NONE-NONE.csv`, `hardware/bom/R30-BALL-NONE-NONE.csv`, `hardware/bom/WSTD-BALL-NONE-NONE.csv`, `hardware/bom/USB-HUB.csv` |
| **Assembly instructions** | Sufficient for someone skilled in the art to build the device | Not yet created | Needed as `hardware/ASSEMBLY.md` or similar |
| **Firmware source code** | Complete, buildable source | In progress | `firmware/ring/`, `firmware/hub/` — hub firmware has BLE central, event composer, role engine. Ring firmware in progress. |
| **Open-source hardware license** | Applied to hardware files | Done | `LICENSE-HARDWARE` (CERN-OHL-S 2.0) |
| **Open-source software license** | Applied to firmware/software | Done | `LICENSE-SOFTWARE` (MIT) |
| **README or project description** | Clear description of what the project is and does | Done | `README.md` |
| **Design rationale** | Explanation of design choices | Done (extensive) | `docs/COMBINATORICS.md`, `docs/CLICK-MECHANISMS.md`, `docs/GLIDE-SYSTEM.md`, `docs/CONSUMER-TIERS.md`, `docs/POWER-BUDGET.md`, `docs/BALL-DIAMETER.md`, `docs/PIEZO-HAPTIC-ANALYSIS.md` |

### OSHWA Self-Certification Form Fields

| Field | Value |
|-------|-------|
| Project name | PowerFinger |
| Project version | 0.1 (prototype) |
| Project URL | https://github.com/fluffyrabbot/powerfinger |
| Hardware license | CERN-OHL-S 2.0 |
| Software license | MIT |
| Documentation license | CERN-OHL-S 2.0 (hardware docs), MIT (firmware docs) |
| Country | (to be filled) |
| Categories | Input devices, Assistive technology, Wearables |
| Description | Open-source assistive fingertip ring mouse and pen wand family. BLE HID, surface-agnostic variants, multi-ring composition. |

### Gaps to Fill Before Certification

1. **Hardware source files (KiCad schematics + PCB layouts).** The BOMs exist
   but no schematic or PCB layout files have been committed. OSHWA requires
   editable hardware source files, not just BOMs. This is the primary blocker.

2. **3D model files (STEP/STL).** The parametric ring shell and wand body CAD
   models are specified (in `docs/PROTOTYPE-SPEC.md`) but not yet created as
   CAD files in the repo.

3. **Assembly instructions.** Build instructions for each prototype variant.
   Can reference the existing design docs for rationale but must include
   step-by-step assembly.

4. **Firmware completeness.** The hub firmware has core components (BLE central,
   event composer, role engine). The ring firmware needs sensor drivers and power
   management implemented as buildable code.

### Certification Strategy

OSHWA certification should be pursued per-variant, not for the entire 576-variant
matrix. Certify:

1. **R30-OLED-NONE-NONE** (P0 optical ring) — first, as soon as schematics and
   CAD exist.
2. **USB Hub Dongle** — companion to the P0 ring.
3. **R30-BALL-NONE-NONE** (P1 ball ring) — second ring variant.
4. **WSTD-BALL-NONE-NONE** (P1 wand) — first wand.

Each certification is independent and creates an additional OSHWA registry entry
searchable by patent examiners.

---

## 4. IP-STRATEGY.md Action Item Review

### Action: "Publish repo under CERN-OHL-S 2.0 + MIT"

**Status: Done.**

Both license files exist in the repository root:
- `LICENSE-HARDWARE` — CERN-OHL-S 2.0
- `LICENSE-SOFTWARE` — MIT

The `CLAUDE.md` project instructions and `docs/IP-STRATEGY.md` document the
dual-license structure. The `docs/PROTOTYPE-SPEC.md` explicitly states "MIT
firmware, CERN-OHL-S 2.0 hardware" in the Fixed Across All Devices section.

**Remaining concern:** Individual source files should carry license headers (a
short SPDX identifier line). The firmware `.c` and `.h` files in `firmware/`
should include `// SPDX-License-Identifier: MIT` at the top. Hardware files
(when created) should include `# SPDX-License-Identifier: CERN-OHL-S-2.0`.
This is not blocking but is best practice for license clarity.

### Action: "Commit detailed design files for P0 variant"

**Status: Partially complete.**

What exists:
- `hardware/bom/R30-OLED-NONE-NONE.csv` — complete BOM for P0 ring
- `hardware/bom/R30-BALL-NONE-NONE.csv` — complete BOM for P1 ring
- `hardware/bom/WSTD-BALL-NONE-NONE.csv` — complete BOM for P1 wand
- `hardware/bom/USB-HUB.csv` — complete BOM for hub dongle
- `docs/PROTOTYPE-SPEC.md` — detailed build spec for all 3 prototypes + hub
- `docs/COMBINATORICS.md` — full 576-variant design matrix
- `docs/CLICK-MECHANISMS.md` — click mechanism analysis and recommendations
- `docs/GLIDE-SYSTEM.md` — glide pad design, materials, failure modes
- `docs/POWER-BUDGET.md` — component-level power estimates
- `docs/CONSUMER-TIERS.md` — Standard/Pro tier definition with BOMs
- `docs/BALL-DIAMETER.md` — ball diameter and material analysis
- `docs/PIEZO-HAPTIC-ANALYSIS.md` — piezo click mechanism analysis
- `docs/BATTERY-SAFETY.md` — battery safety analysis
- `docs/DUAL-FOOTPRINT.md` — dual-footprint PCB strategy
- `docs/VENDOR-VERIFICATION.md` — component sourcing verification
- `docs/REGULATORY-PRESCAN.md` — regulatory compliance pre-scan
- `firmware/hub/` — hub firmware with BLE central, event composer, role engine
- `firmware/ring/` — ring firmware (in progress)

What is needed for full enablement:
- **KiCad schematic** for P0 ring (ESP32-C3 + PAW3204 + dome switch + LiPo +
  TP4054 + RT9080 + USB-C)
- **KiCad PCB layout** for P0 ring (flex or rigid-flex)
- **Parametric 3D model** for ring shell (OpenSCAD or FreeCAD, accepting
  `finger_circumference` and `angle` parameters)
- **Assembly instructions** with diagrams
- **Fabricated prototype** (validates that the design actually works)

**Prior art assessment:** The existing documentation is already a strong
defensive publication. The COMBINATORICS.md file alone enumerates 576 variants
with BOMs and tradeoff analysis -- this is sufficient enabling disclosure for
someone skilled in the art to understand and reproduce each variant. The missing
items (schematics, CAD) are needed for OSHWA certification and for practical
buildability, but the documentation already published establishes prior art for
the design concepts.

### Action: "Publish Hackaday.io project page"

**Status: Content ready.** Section 1 of this document provides complete,
publishable Hackaday.io project page content. To execute:

1. Create account at hackaday.io
2. Create new project
3. Paste the content from Section 1 into the project description
4. Add project tags from the tag list
5. Upload architecture diagram as project image
6. Link to the GitHub repository
7. Publish

The Hackaday.io project will be indexed by Google within days and will appear
in search results for relevant terms (ring mouse, assistive pointing device,
BLE HID, etc.).

### Action: "FTO opinion from patent attorney"

**Status: Blocked by funding ($2--5K).** No action possible without funding.

The FTO scope is defined in `docs/IP-STRATEGY.md`: analyze US8648805B2 (FTM
fingertip mouse) against the ring design, and US12,353,649 (Apple trackball
pencil) against the wand design. The `docs/PATENT-CLAIM-MAP.md` file provides
detailed claim-by-claim analysis that would assist the patent attorney and
potentially reduce billable hours.

### Action: "OSHWA certification"

**Status: Blocked by missing hardware source files.** Section 3 of this document
identifies the gaps. The certification form can be submitted as soon as KiCad
schematics and CAD models exist for the P0 variant. The BOMs, license files,
firmware, and documentation are already sufficient.

### Action: "Evaluate OIN/Unified Patents membership"

**Status: Not actionable yet.** OIN membership requires evaluation of whether
ESP-IDF (FreeRTOS-based, not Linux) qualifies under OIN's Linux System
definition. This is a future-phase activity that should be revisited when the
project has community traction or if a patent threat materializes.

---

## 5. Publication Priority Order

Publication should proceed in this order, based on prior art value per unit of
effort:

1. **Hackaday.io project page** — highest indexing speed (Google indexes within
   days), highest hardware community visibility, zero cost, content is ready now.
   This creates the most immediately discoverable prior art.

2. **arXiv preprint** — indexed by Google Scholar and patent databases. Requires
   completing the paper (the outline and key content in Section 2 make this
   largely mechanical). Higher credibility with patent examiners than a project
   page.

3. **OSHWA certification** — creates an entry in the OSHWA certification
   directory (searched by patent examiners). Blocked by hardware source files
   but should be filed immediately when those exist.

4. **Instructables / WikiHow build guide** — broad public reach, different
   audience than Hackaday.io. Lower priority but creates additional indexed
   disclosure.

Each publication venue creates an independent, dated, publicly accessible
record. Redundancy is intentional: if one venue disappears, the others persist.
The GitHub repository with its commit history provides the most granular
timestamping but is the least searchable by patent examiners.

---

## 6. Novel Combinations Inventory for Prior Art

The following specific combinations are documented for explicit prior art value.
Each represents a non-obvious intersection of form factor, sensing, and
capability that could be the subject of a future patent application by a third
party:

| # | Combination | What Makes It Novel | Repo Evidence |
|---|------------|-------------------|--------------|
| 1 | Multi-ring composition: N BLE HID ring mice → 1 USB mouse via hub | No prior art for composing multiple wearable HID peripherals into a single logical mouse via role engine | `firmware/hub/components/role_engine/`, `firmware/hub/components/event_composer/`, `docs/COMBINATORICS.md` |
| 2 | Optical-on-captive-ball in fingertip ring (PMW3360 + ball) | High-CPI surface-agnostic ring without rollers or encoder wheels | `docs/COMBINATORICS.md` (S-OPTB), `docs/CONSUMER-TIERS.md` (Pro tier) |
| 3 | Hall effect ball ring with power-gated DRV5053 sensors | Surface-agnostic ring at ~$2.50 sensor BOM with magnetic rather than optical rotation detection | `docs/COMBINATORICS.md` (S-BALL), `docs/BALL-DIAMETER.md`, `docs/POWER-BUDGET.md` |
| 4 | Piezo film + LRA haptic for sealed ring click | Infinite-lifecycle click mechanism enabling fully sealed wearable with no mechanical openings | `docs/CLICK-MECHANISMS.md` (Pattern D), `docs/PIEZO-HAPTIC-ANALYSIS.md` |
| 5 | SQUAL-based auto-switch between surface optical and IMU air tracking | Hybrid pointing mode using optical sensor quality register as lift detection trigger | `docs/COMBINATORICS.md` (IMU Hybrid Mode section) |
| 6 | Tethered sensor: sub-gram fingertip dot + wrist hub via flex ribbon | Split architecture where sensing is on finger (<0.5g) and compute is on wrist (5--8g) | `docs/COMBINATORICS.md` (Tethered Sensor section) |
| 7 | Parametric ring shell with continuous finger circumference sizing | CAD model that produces any ring size from one design, avoiding discrete S/M/L | `docs/PROTOTYPE-SPEC.md`, `docs/COMBINATORICS.md` |
| 8 | UHMWPE glide pads replacing PTFE for PFAS regulatory risk avoidance | Deliberate material selection for regulatory future-proofing in an assistive device | `docs/GLIDE-SYSTEM.md` |
| 9 | Adaptive BLE connection interval (15ms idle ↔ 7.5ms active) for ring mouse | Dynamic interval switching based on sensor delta magnitude to balance latency and power | `docs/POWER-BUDGET.md`, `docs/FIRMWARE-ROADMAP.md` |
| 10 | Nail-mount ring with typing compatibility | Sensor on dorsal fingertip, pad exposed for typing, curl to point | `docs/COMBINATORICS.md` (Nail Mount section) |
| 11 | Wrist bracelet with surface-tracking sensor | Forearm-slide cursor with vertical-mouse ergonomics and full typing compatibility | `docs/COMBINATORICS.md` (Wrist Bracelet section) |
| 12 | Ball-in-pen-tip wand with Hall effect sensing (angle-independent, surface-agnostic) | Pen mouse that works at 30--70 degree angles on any surface via magnetic ball rotation sensing | `docs/WAND-COMPETITIVE.md`, `docs/PROTOTYPE-SPEC.md` (Prototype 3) |
| 13 | Retractable ball tip in pen wand | Spring + sleeve mechanism retracts ball sensor into pen body for pocket carry | `docs/COMBINATORICS.md` (W-RET section) |
| 14 | Ring + wand + hub as unified assistive input system | Complete input device ecosystem: fingertip for precision, pen for writing surfaces, hub for composition | `docs/COMBINATORICS.md`, `docs/PROTOTYPE-SPEC.md` |
| 15 | OCR camera ring (S-OLED + C-CAM + L-DOT) | Finger-worn OCR scanner: optical pointing + camera + guidance laser in ring form factor | `docs/COMBINATORICS.md` (optional capabilities) |
| 16 | IMU-only ring as surface-free air mouse for wheelchair users | Ring form factor providing cursor control with no surface dependency, specific accessibility use case | `docs/COMBINATORICS.md` (S-IMU section) |

This inventory is not exhaustive -- the full 576-variant matrix in
`docs/COMBINATORICS.md` constitutes the complete prior art disclosure. These 16
entries highlight the combinations with the highest probability of being targeted
by third-party patent applications.
