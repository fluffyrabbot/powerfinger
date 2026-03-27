# Patent Claim Map — PowerFinger P0 Designs

> **Disclaimer:** This document is NOT a legal opinion. It is a structured
> engineering analysis of patent claim elements against the PowerFinger P0
> design, intended to reduce patent attorney hours during formal
> freedom-to-operate review. Claim interpretation, the doctrine of equivalents,
> prosecution history estoppel, and other legal doctrines may alter any
> conclusion below. Engage qualified patent counsel before making business
> decisions.

## Reference Designs Analyzed

**PowerFinger Standard Ring (P0):**
- Ring worn on middle fingertip pad area, 30-degree sensor angle
- PAW3204 optical sensor illuminates surface through raised rim (UHMWPE glide pads)
- Metal snap dome click mechanism
- BLE HID mouse protocol (ESP32-C3 MCU)
- Two-ring system via USB hub dongle: one ring for cursor, one for scroll
- No base station / no docking

**PowerFinger Pro Ring (P0):**
- PMW3360 reading captive ball (fully sealed, surface-independent)
- Piezo + LRA click mechanism
- Otherwise same architecture as Standard

**PowerFinger Wand (P0):**
- Pen-shaped, ball at tip with 4x DRV5053 Hall effect sensors
- Magnetic sensing (not optical)
- BLE HID, works at any pen angle 30-70 degrees

---

## US8648805B2 — "Fingertip mouse and base" (Bailen / FTM Computer Products, 2014)

**Status:** Granted. **Closest match to PowerFinger ring.**

### Claim 1 (Independent) — System Claim

> A computer peripheral system comprising:
> (a) a finger-worn tracking device adapted to be worn on a finger of a user,
> the finger-worn tracking device comprising a first tracking mechanism
> configured to generate movement information for use in controlling operation
> of a graphical user interface of a computing system, the operation of the
> graphical user interface including at least movement of a cursor on the
> graphical user interface based on the movement information, the finger-worn
> tracking device also comprising a transceiver configured to wirelessly
> transmit the movement information via a radio frequency communication channel
> for receipt by the computing system, wherein the finger-worn tracking device
> includes a tracking device housing having a finger holding portion that is
> configured to receive and to be worn by at least a portion of a user's
> finger; and
> (b) a base device that includes a docking station that is configured and
> sized to receive at least a portion of the finger-worn tracking device and
> temporarily secure the finger-worn tracking device to the base device when the
> finger-worn tracking device is not in use on a user's finger, wherein the
> docking station is further configured to allow a user to remove the
> temporarily secured finger-worn tracking device from the base device to enable
> use of the finger-worn tracking device on a user's finger in a stand-alone
> mode, wherein the base device is configured to be operated as a table-top
> mouse-type tracking device wherein, in response to movement in connection with
> the base device, movement information is generated and transmitted for use in
> controlling operation of the graphical user interface, the operation including
> at least movement of the cursor on the graphical user interface,
> wherein the finger-worn tracking device is adapted to be operated in at least
> the stand-alone mode for controlling the operation of the graphical user
> interface, and wherein the finger-worn tracking device is adapted and
> configured such that, while operating in the stand-alone mode, the finger-worn
> tracking device generates and transmits the movement information independently
> of the base device.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 1a | "finger-worn tracking device adapted to be worn on a finger of a user" | PowerFinger ring is worn on the middle fingertip. | **Yes** |
| 1b | "first tracking mechanism configured to generate movement information for use in controlling operation of a graphical user interface... including at least movement of a cursor" | PAW3204 optical sensor generates X/Y delta movement data transmitted as HID mouse reports to move cursor. | **Yes** |
| 1c | "transceiver configured to wirelessly transmit the movement information via a radio frequency communication channel" | ESP32-C3 BLE radio transmits HID reports wirelessly. | **Yes** |
| 1d | "tracking device housing having a finger holding portion that is configured to receive and to be worn by at least a portion of a user's finger" | Ring housing with sizing inserts holds onto fingertip. | **Yes** |
| 1e | "a base device that includes a docking station that is configured and sized to receive at least a portion of the finger-worn tracking device and temporarily secure the finger-worn tracking device to the base device when not in use" | **PowerFinger has no base device and no docking station.** The USB hub dongle is a passive BLE receiver, not a docking station that physically receives the ring. | **No** |
| 1f | "docking station is further configured to allow a user to remove the temporarily secured finger-worn tracking device from the base device to enable use... in a stand-alone mode" | No docking station exists. | **No** |
| 1g | "base device is configured to be operated as a table-top mouse-type tracking device wherein, in response to movement in connection with the base device, movement information is generated and transmitted" | PowerFinger's USB dongle is a passive receiver only. It does not function as a table-top mouse. | **No** |
| 1h | "finger-worn tracking device is adapted to be operated in at least the stand-alone mode... generates and transmits the movement information independently of the base device" | The ring always operates independently (there is no base device to depend on). This element is met trivially, but is moot given that elements 1e-1g are not practiced. | N/A (moot) |

### Overall Assessment — Claim 1

**PowerFinger does NOT practice all elements of Claim 1.** The claim requires a "computer peripheral system" that includes both a finger-worn device AND a base device functioning as a docking station and table-top mouse. PowerFinger has no base device, no docking station, and no table-top mouse mode. Three essential elements (1e, 1f, 1g) are absent from the PowerFinger design.

This is the only independent claim in US8648805B2. All 25 claims depend on Claim 1. Because the independent claim is not practiced, **no claim of this patent is practiced by PowerFinger.**

### Risk Notes

- The base device / docking station requirement is structural and clearly absent. This is a strong design-around.
- Dependent claim 25 narrows to Bluetooth specifically, and dependent claim 3 narrows the tracking mechanism to optical sensor. These would apply if Claim 1 were practiced, but they are not independently assertable.
- If FTM were to obtain a continuation patent removing the base device limitation, that would need fresh analysis. Monitor continuation filings.

---

## US20130063355A1 — "Mouse with a finger triggered sensor"

**Status:** Published application. Claims 1-30 canceled; claims 31-47 pending (as of publication). Check whether this ever issued as a granted patent.

### Claim 31 (Independent)

> A computer input device comprising:
> (a) a device movement tracking mechanism for tracking the movement of the
> computer input device;
> (b) one or more buttons capable of being triggered by being depressed by a
> finger;
> (c) a sensor having a first state and a second state, capable of being
> triggered by one or more fingers of a user to change from the first state to
> the second state or from the second state to the first state; and
> (d) a control circuit for detecting changes of states of the sensor and
> detecting movement signals generated by the device movement tracking
> mechanism, wherein the control circuit generates a left button down signal
> upon detecting a change of the sensor from the first state to the second state
> followed by at least one movement signal within a predefined time period.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 31a | "device movement tracking mechanism for tracking the movement of the computer input device" | PAW3204 optical sensor tracks ring movement relative to surface. | **Yes** |
| 31b | "one or more buttons capable of being triggered by being depressed by a finger" | Metal snap dome click mechanism is a button depressed by the thumb. | **Yes** |
| 31c | "a sensor having a first state and a second state, capable of being triggered by one or more fingers of a user to change from the first state to the second state or from the second state to the first state" | This element describes a separate sensor (distinct from the buttons in 31b) with binary states triggered by a finger. PowerFinger does not have a separate binary-state finger-triggered sensor beyond its click button. The click button itself could be read as this sensor, but the claim lists "buttons" and "sensor" as separate elements. | **Uncertain** |
| 31d | "control circuit... generates a left button down signal upon detecting a change of the sensor from the first state to the second state followed by at least one movement signal within a predefined time period" | PowerFinger generates left button down signals from the snap dome press unconditionally (not contingent on subsequent movement within a time window). The firmware does not implement the "state change + movement within predefined time period" logic that this claim requires. | **No** |

### Claim 40 (Independent)

> A computer input device comprising:
> (a) a device movement tracking mechanism for tracking the movement of the
> computer input device;
> (b) one or more buttons capable of being triggered by being depressed by a
> finger;
> (c) a sensor having a first state and a second state, capable of being
> triggered by one or more fingers of a user to change from the first state to
> the second state or from the second state to the first state; and
> (d) a control circuit for detecting changes of states of the sensor, wherein
> the control circuit generates a sensor-activated message upon detecting a
> change of the sensor from the first state to the second state followed by a
> change of the sensor from the second state to the first state within a
> predefined time period.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 40a | "device movement tracking mechanism" | Same as 31a. | **Yes** |
| 40b | "one or more buttons" | Same as 31b. | **Yes** |
| 40c | "sensor having a first state and a second state" | Same analysis as 31c. | **Uncertain** |
| 40d | "control circuit generates a sensor-activated message upon detecting a change... from the first state to the second state followed by a change... from the second state to the first state within a predefined time period" | This describes a toggle-and-return-within-timeout gesture detection pattern. PowerFinger's click mechanism generates immediate button events on press/release without time-window gating logic. | **No** |

### Claim 41 (Independent)

> A computer input device comprising:
> (a) a touch-sensitive pad located on a substantially horizontal surface of
> the computer input device; and
> (b) a sensor located adjacent the touch-sensitive pad on a substantially
> vertical surface of the computer input device, the sensor being capable of
> being triggered by one or more fingers of a user while using the touch pad.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 41a | "touch-sensitive pad located on a substantially horizontal surface of the computer input device" | PowerFinger is a ring, not a device with horizontal and vertical surfaces bearing a touchpad. No touch-sensitive pad exists. | **No** |
| 41b | "sensor located adjacent the touch-sensitive pad on a substantially vertical surface" | No touchpad, no vertical-surface sensor in this configuration. | **No** |

### Overall Assessment — US20130063355A1

**PowerFinger does NOT practice any independent claim.** Claim 31 and 40 both require specific time-window-gated signal generation logic that PowerFinger does not implement. Claim 41 requires a touchpad form factor entirely different from a ring. Even if the "sensor" element (31c/40c) were construed broadly, the signal-generation logic elements (31d/40d) are definitively not practiced.

Additionally, this appears to be an ungranted application — verify current prosecution status before spending further attorney time.

---

## US20150241976A1 — "Wearable finger ring input device"

**Status:** Published application (2015). Verify whether granted.

### Claim 1 (Independent)

> A control device configured to control a plurality of external devices, said
> control device comprising:
> (a) an exterior housing of a substantially ring shape and configured to be
> wearable on a finger of a user;
> (b) a motion sensor configured to detect a predefined motion of said finger
> and responsive thereto to generate a motion signal;
> (c) control logic coupled to said motion sensor and configured to convert
> said motion signal into digital data;
> (d) a processor coupled to said control logic;
> (e) a memory storing executable instructions that, when executed by said
> processor, cause said processor to perform a method of:
>   - identifying an external device of said plurality of external devices;
>   - processing said digital data; and
>   - generating a control instruction based on said external device and said
>     digital data; and
> (f) a communication circuit coupled to said processor and configured to
> communicate said control instruction for receipt by said external device
> through a communication channel,
> (g) wherein said exterior housing contains said motion sensor, said control
> logic, said processor, said memory and said communication circuit.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 1a | "exterior housing of a substantially ring shape and configured to be wearable on a finger" | PowerFinger ring housing is worn on a finger. | **Yes** |
| 1b | "motion sensor configured to detect a predefined motion of said finger and responsive thereto to generate a motion signal" | PAW3204 detects surface-relative motion. However, the claim language says "predefined motion of said finger" which suggests gesture recognition (detecting specific finger motions like shake, click, slide, circling, bending — see dependent claim 7). PowerFinger's optical sensor tracks continuous X/Y displacement, not predefined gesture motions. | **Uncertain** |
| 1c | "control logic coupled to said motion sensor and configured to convert said motion signal into digital data" | ESP32-C3 reads SPI data from PAW3204 (already digital). Conversion from sensor output to processed data occurs. | **Yes** |
| 1d | "a processor coupled to said control logic" | ESP32-C3 RISC-V processor. | **Yes** |
| 1e | "memory storing executable instructions... identifying an external device of said plurality of external devices; processing said digital data; and generating a control instruction based on said external device and said digital data" | **"Identifying an external device of said plurality of external devices"** is the critical phrase. This requires the ring itself to identify which of multiple possible target devices to control and tailor its output accordingly. PowerFinger transmits standard BLE HID mouse reports — it does not identify or select among multiple external devices. It pairs with one host. The host OS routes input, not the ring. | **No** |
| 1f | "communication circuit configured to communicate said control instruction for receipt by said external device through a communication channel" | ESP32-C3 BLE radio communicates with host. | **Yes** |
| 1g | "exterior housing contains said motion sensor, said control logic, said processor, said memory and said communication circuit" | All electronics are contained within the ring housing. | **Yes** |

### Claim 10 (Independent) — Two-Ring Controller

> A controller configured to control a plurality of external devices, said
> controller comprising:
> (a) a first ring member comprising: a first exterior housing configured in an
> annular shape and wearable on a first finger of a user; and a first sensor
> contained within said first exterior housing and configured to detect a first
> predefined motion of said first finger and responsive thereto to generate a
> first motion signal; and
> (b) a second ring member comprising: a second exterior housing configured in
> an annular shape and wearable on a second finger of said user; and a second
> sensor configured to detect a second predefined motion of said second finger
> and responsive thereto to generate a second motion signal;
> (c) control logic coupled to said second sensor and configured to generate
> digital data based on a combination of said first motion signal and said
> second motion signal;
> (d) a processor coupled to said control logic;
> (e) a memory embodying executable instructions that, when executed by said
> processor, cause said processor to perform a method of: identifying an
> external device that communicates with said controller through a communication
> channel; processing said digital data; and generating a control instruction
> based on said external device and said digital data; and
> (f) a communication circuit coupled to said processor and configured to
> communicate said control instruction to said external device through said
> communication channel,
> (g) wherein said second exterior housing contains said control logic, said
> processor, said memory and said communication circuit.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 10a | "first ring member... annular shape... wearable on a first finger... first sensor... detect a first predefined motion... generate a first motion signal" | PowerFinger two-ring system has a cursor ring on one finger. Same "predefined motion" uncertainty as Claim 1. | **Uncertain** |
| 10b | "second ring member... annular shape... wearable on a second finger... second sensor... detect a second predefined motion... generate a second motion signal" | Scroll ring on another finger. | **Uncertain** |
| 10c | "control logic coupled to said second sensor and configured to generate digital data based on a combination of said first motion signal and said second motion signal" | **PowerFinger's two rings are independent BLE HID devices.** Each transmits its own HID reports to the host via the USB dongle. The dongle does not combine signals from both rings — the host OS receives two independent HID mouse inputs. The claim requires on-device combination of both motion signals. | **No** |
| 10d-e | "processor... identifying an external device... processing said digital data... generating a control instruction based on said external device and said digital data" | Same "identifying an external device" problem as Claim 1 element 1e. PowerFinger does not identify/select among target devices. | **No** |
| 10f | "communication circuit... communicate said control instruction" | BLE radio present. | **Yes** |
| 10g | "second exterior housing contains said control logic, said processor, said memory and said communication circuit" | Each ring is self-contained. | **Yes** |

### Claim 16 (Independent) — Wireless Control System

> A wireless control system comprising:
> (a) a first housing member and a second housing member, wherein said first
> and said second housing members are wearable on a first finger and a second
> finger of a user respectively;
> (b) a first sensor coupled to said first housing member... detect a first
> predefined gesture... generate a first detection signal;
> (c) first control logic... convert said first motion signal to first digital
> data;
> (d) a second sensor coupled to said second housing member... detect a second
> predefined gesture... generate a second detection signal;
> (e) second control logic... convert said second motion signal to second
> digital data;
> (f) a processor coupled to said second control logic;
> (g) a memory storing executable instructions... identifying an external
> device... processing said first digital data and said second digital data...
> generating a control instruction based on said external device and on a
> combination of said first digital data and said second digital data; and
> (h) a communication circuit... wirelessly communicate said control
> instruction to said external device,
> (i) wherein said second sensor, said processor, said memory and said
> communication circuit are contained within said second housing.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 16a | Two housing members wearable on two fingers | Two-ring system fits this. | **Yes** |
| 16b-e | First and second sensors detecting "predefined gestures" with control logic | Same "predefined gesture" uncertainty. PowerFinger tracks continuous motion, not predefined gestures. | **Uncertain** |
| 16f | Processor coupled to second control logic | ESP32-C3 in each ring. | **Yes** |
| 16g | "identifying an external device... generating a control instruction based on said external device and on a combination of said first digital data and said second digital data" | Same two problems: (1) no device identification/selection, and (2) no on-device combination of data from both rings. | **No** |
| 16h | Wireless communication circuit | BLE present. | **Yes** |
| 16i | Second housing contains sensor, processor, memory, communication circuit | Self-contained ring. | **Yes** |

### Overall Assessment — US20150241976A1

**PowerFinger does NOT practice any independent claim (1, 10, or 16).** Every independent claim requires the device to "identify an external device of a plurality of external devices" and generate device-specific control instructions. PowerFinger is a standard BLE HID mouse that pairs with one host and sends generic mouse reports — it does not identify or select among multiple target devices.

Claims 10 and 16 additionally require on-device combination of signals from two rings, which PowerFinger's independent-ring architecture does not do (combination happens at the host OS level).

The "predefined motion/gesture" language is a secondary design difference but is flagged as uncertain because claim construction could potentially read "predefined motion" broadly enough to cover any intentional movement.

### Risk Notes

- Verify prosecution status — if this application was abandoned, it is not enforceable.
- The "plurality of external devices" and "identifying an external device" limitations are strong design-arounds for PowerFinger's single-host BLE HID model.
- If PowerFinger ever adds multi-device switching (common in premium BLE keyboards/mice), this analysis would need revision.

---

## US8125448B2 — "Wearable computer pointing device"

**Status:** Granted (2012).

### Claim 1 (Independent)

> In a computing environment, a system comprising:
> (a) a pointing device configured for wearing on a user's hand, the pointing
> device being generally U-shaped, and having a first part that is configured to
> fit in a user's palm coupled to a second part that is configured to fit behind
> a user's knuckles, the pointing device comprising
> (b) circuitry that senses hand movements, the circuitry including at least
> two gyroscopic sensors including a first sensor that senses pitch and a second
> sensor that senses yaw,
> (c) a magnetometer configured to detect a direction that the pointing device
> is facing,
> (d) a gesture detection mechanism configured to cause the circuitry to detect
> a gesture, output a first data corresponding to performing a first set of
> functions on a first device corresponding to the gesture when the pointing
> device is facing a first direction, and output a second data corresponding to
> performing a second set of functions on a second device corresponding to the
> gesture when the pointing device is facing a second direction, and
> (e) a transmitter that transmits the first data and the second data
> corresponding to the gesture to a host computing device coupled to the
> pointing device; and
> (f) an enable switch comprising a capacitive proximity sensor that is
> positioned on the first part of the pointing device that is configured to fit
> in the user's palm such that the enable switch is actuated while the user's
> hand is closed and a user's fingers are proximate to the capacitive proximity
> sensor and such that the enable switch is not actuated when the user's fingers
> are not proximate to the capacitive proximity sensor,
> (g) and wherein the circuitry transmits the first data and the second data
> corresponding to the sensed hand movements only when the enable switch is
> actuated.

| # | Claim Element | PowerFinger P0 | Practiced? |
|---|---------------|----------------|------------|
| 1a | "pointing device... generally U-shaped... first part that is configured to fit in a user's palm coupled to a second part that is configured to fit behind a user's knuckles" | PowerFinger is a fingertip ring, not a U-shaped palm+knuckle device. Completely different form factor. | **No** |
| 1b | "at least two gyroscopic sensors including a first sensor that senses pitch and a second sensor that senses yaw" | PowerFinger uses an optical sensor (PAW3204) or Hall effect sensors (wand). No gyroscopic sensors. | **No** |
| 1c | "a magnetometer configured to detect a direction that the pointing device is facing" | No magnetometer in any PowerFinger variant. | **No** |
| 1d | "gesture detection mechanism... output a first data corresponding to performing a first set of functions on a first device... when the pointing device is facing a first direction, and output a second data... on a second device... when the pointing device is facing a second direction" | No gesture detection, no direction-based device switching. | **No** |
| 1e | "transmitter that transmits the first data and the second data" | BLE transmitter exists, but the data described in 1d is not generated. | **No** |
| 1f | "enable switch comprising a capacitive proximity sensor... positioned on the first part... fit in the user's palm... actuated while the user's hand is closed" | No capacitive proximity enable switch. No palm-fitting component. | **No** |
| 1g | "circuitry transmits... only when the enable switch is actuated" | No enable switch. | **No** |

### Claim 8 (Independent)

Substantially similar to Claim 1 but uses means-plus-function language for analog-to-digital conversion. Still requires: gyroscopic sensors (pitch + yaw), magnetometer, gesture detection with direction-based device switching, and capacitive proximity enable switch. None of these are present in PowerFinger.

### Claim 15 (Independent)

Adds accelerometer, three gyroscopic sensors (pitch, yaw, roll), and further specifies the U-shaped form factor. Even more elements absent from PowerFinger.

### Overall Assessment — US8125448B2

**PowerFinger does NOT practice any independent claim (1, 8, or 15).** This patent describes a fundamentally different device: a U-shaped palm-and-knuckle wearable using gyroscopic + magnetometer sensing with direction-based multi-device gesture control and a capacitive palm-close enable switch. PowerFinger shares essentially zero structural or functional overlap with these claims. The sensing modality (optical/Hall vs. gyroscope+magnetometer), form factor (fingertip ring vs. U-shaped palm device), and control paradigm (HID mouse vs. direction-based gesture-to-device routing) are all different.

**This patent presents negligible infringement risk.**

---

## US12353649B2 — "Input device with optical sensors" (Apple, 2025)

**Status:** Granted (2025). Wand-relevant.

### Claim 1 (Independent) — System Claim

> A system comprising:
> (a) a first device comprising: a plurality of first sensors configured to
> track displacement in position and angle of the first device relative to a
> non-touch sensitive surface; and first communication circuitry coupled to the
> plurality of first sensors and configured to transmit information from the
> plurality of first sensors to a second device;
> (b) the second device comprising: one or more second sensors configured to
> track a distance between the first device and the non-touch sensitive surface;
> second communication circuitry configured to receive the information from the
> plurality of first sensors; processing circuitry configured to receive the
> displacement in position and angle of the first device from the plurality of
> first sensors and generate content using the displacement in position and
> angle of the first device and the distance between the first device and the
> non-touch sensitive surface; and a display configured to display the content
> generated by the processing circuitry on the non-touch sensitive surface.

| # | Claim Element | PowerFinger P0 (Wand) | Practiced? |
|---|---------------|----------------------|------------|
| 1a-i | "first device comprising: a plurality of first sensors configured to track displacement in position and angle of the first device relative to a non-touch sensitive surface" | PowerFinger wand uses 4x Hall sensors to track ball displacement, which maps to X/Y position. However, the claim says "displacement in position **and angle**" — tracking the angular orientation/tilt of the device itself relative to the surface. The wand's Hall sensors track the ball's rotation (which correlates with tip displacement), not the device's angular orientation relative to the surface. | **Uncertain** |
| 1a-ii | "first communication circuitry coupled to the plurality of first sensors and configured to transmit information from the plurality of first sensors to a second device" | ESP32-C3 BLE radio transmits sensor data to host. | **Yes** |
| 1b-i | "the second device comprising: one or more second sensors configured to track a distance between the first device and the non-touch sensitive surface" | **The "second device" must have sensors that track the distance between the first device (wand/stylus) and the surface.** PowerFinger's host computer (laptop/phone) has no sensors tracking the wand's distance from any surface. This element describes a head-mounted display or similar device with spatial tracking capabilities (see dependent claim 5: "first device comprises a stylus and the second device comprises a head-mounted display device"). | **No** |
| 1b-ii | "second communication circuitry configured to receive the information from the plurality of first sensors" | Host receives BLE data. | **Yes** |
| 1b-iii | "processing circuitry configured to receive the displacement in position and angle... and generate content using the displacement in position and angle... and the distance between the first device and the non-touch sensitive surface" | Host processes mouse input but does not use distance-from-surface data (which is never generated). | **No** |
| 1b-iv | "a display configured to display the content generated by the processing circuitry on the non-touch sensitive surface" | "Display the content... **on the non-touch sensitive surface**" — this describes projection onto a surface (e.g., from a head-mounted display). PowerFinger's host displays content on its own screen, not projected onto the surface the wand touches. | **No** |

### Claim 10 (Independent) — Device Claim

> An electronic device comprising:
> (a) communication circuitry configured to receive information from a
> plurality of first sensors, wherein the first sensors are configured to track
> displacement in position and angle of a first device relative to a non-touch
> sensitive surface;
> (b) one or more second sensors configured to track a distance between the
> first device and the non-touch sensitive surface;
> (c) processing circuitry configured to receive the displacement in position
> and angle of the first device from the plurality of first sensors and generate
> content using the displacement in position and angle of the first device and
> the distance between the first device and the non-touch sensitive surface; and
> (d) a display configured to display the content generated by the processing
> circuitry on the non-touch sensitive surface.

This is the "second device" (receiver/display device) claimed independently. Same analysis applies:

| # | Claim Element | PowerFinger P0 Host | Practiced? |
|---|---------------|---------------------|------------|
| 10a | Communication circuitry receiving sensor info tracking displacement in position and angle | Host receives HID mouse reports. Angle data is not transmitted. | **Uncertain** |
| 10b | "one or more second sensors configured to track a distance between the first device and the non-touch sensitive surface" | Host has no sensors tracking wand-to-surface distance. | **No** |
| 10c | Processing circuitry generating content using position, angle, and distance | Host does not use angle or distance data. | **No** |
| 10d | "display configured to display the content... on the non-touch sensitive surface" | Host displays on its own screen, not projected onto the working surface. | **No** |

### Overall Assessment — US12353649B2

**PowerFinger does NOT practice any independent claim (1 or 10).** This patent describes an AR/VR stylus-and-headset system where: (1) a stylus tracks its own position and angle relative to a surface, (2) a head-mounted display tracks the distance between the stylus and the surface, and (3) content is projected/displayed onto the non-touch surface. PowerFinger is a BLE HID mouse that sends cursor movement to a conventional display. Three fundamental architectural differences make this patent inapplicable:

1. No angle-of-device tracking (PowerFinger tracks ball displacement, not device orientation)
2. No distance-to-surface sensing on the receiving device
3. No content projection onto the working surface

**This patent presents negligible infringement risk for the wand design.**

---

## Summary Matrix

| Patent | Independent Claims | Any Claim Fully Practiced? | Key Non-Practiced Elements | Risk Level |
|--------|-------------------|---------------------------|---------------------------|------------|
| **US8648805B2** (Bailen) | 1 | **No** | Base device / docking station / table-top mouse mode | **Low** — base device requirement is structural and clearly absent |
| **US20130063355A1** | 31, 40, 41 | **No** | Time-window-gated signal generation (31, 40); touchpad form factor (41) | **Low** — different input paradigm; verify prosecution status |
| **US20150241976A1** | 1, 10, 16 | **No** | Multi-device identification/selection; on-device signal combination (10, 16) | **Low** — single-host BLE HID model is fundamentally different; verify prosecution status |
| **US8125448B2** | 1, 8, 15 | **No** | U-shaped palm device; gyroscopes; magnetometer; capacitive enable switch; direction-based device routing | **Negligible** — entirely different device category |
| **US12353649B2** (Apple) | 1, 10 | **No** | Distance-to-surface sensing; content projection onto surface; device angle tracking | **Negligible** — AR/VR stylus+HMD system, not a BLE mouse |

## Design-Around Features (What Keeps PowerFinger Clear)

The following PowerFinger design decisions collectively create separation from the analyzed patent landscape:

1. **No base device / no docking station** — eliminates US8648805B2
2. **Standard BLE HID mouse protocol to single paired host** — eliminates the multi-device identification/selection required by US20150241976A1
3. **Independent two-ring architecture (no on-device signal combination)** — eliminates US20150241976A1 claims 10 and 16
4. **Fingertip ring form factor (not palm/knuckle U-shape)** — eliminates US8125448B2
5. **Optical surface tracking / magnetic ball tracking (not gyroscope+magnetometer)** — eliminates US8125448B2
6. **Immediate button events (no time-window gating)** — eliminates US20130063355A1
7. **Conventional display output (no surface projection, no distance sensing)** — eliminates US12353649B2

## Recommendations for Patent Counsel

1. **US8648805B2 warrants the most attorney time** as the closest match. Confirm that the base device / docking station limitation cannot be read out of Claim 1 under any reasonable construction. Monitor for continuation filings by Bailen/FTM that might drop the base device requirement.

2. **US20150241976A1** — verify prosecution status (granted vs. abandoned). If abandoned, deprioritize. If granted, confirm that "identifying an external device of said plurality of external devices" requires on-device device selection (not merely connecting to any device).

3. **US20130063355A1** — verify prosecution status. The time-window signal generation logic is a clear design-around but confirm claim construction.

4. **US8125448B2 and US12353649B2** are low priority — fundamentally different device architectures with multiple non-practiced elements each.

5. **If PowerFinger ever adds multi-device switching** (e.g., toggle between laptop and tablet), re-analyze US20150241976A1 immediately.
