<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Ring, separate switch, and puck bench comparison

Use this worksheet to decide whether changing the click location or removing the finger band improves pointing for a particular user. An engineering partner can build and qualify the fixture remotely. A person who uses assistive input supplies the usability evidence. A successful operator demonstration establishes fixture function only.

## Build one interchangeable fixture

| Condition | Motion input | Primary click | What the comparison tells us |
|---|---|---|---|
| A | Optical head in a 30° finger ring | Downward dome on the ring | Performance with the pointing finger carrying both actions |
| B | Same head, ring, mass and feet as A | A normally open switch at the participant's chosen site | A versus B tests relocation of the click. Leave the dome in place, electrically disconnected. |
| C | Same optical head on a free-standing, concave fingertip sled | Same switch, site and actuation as B | B versus C tests the combined consequences of retention, bearing the weight and finding the device. Record the sled mass and geometry. |

The split-pod R30 CAD is an off-body fit fixture. Use a separately fitted, removable band for participant trials. Measure its mass and contact points.

Use a removable optical head with the selected sensor, matched lens and powered emitter. Retain the head against hard stops. Keep the optical axis normal to the test plane and use the same feet and measured working distance. A passive sled with room for the head is sufficient. Record the dimensions if the existing board makes it larger than the proposed PowerPuck. This comparison does not require a second radio, a finished puck, a 3.5 mm jack on the hub, or new gesture firmware.

Use a known working pointing controller with raw motion reports and an active-low, pulled-up primary switch input. Connect **one** dry contact at a time between that input and its ground. For a passive 3.5 mm mono switch, confirm with a continuity meter that tip and sleeve close only during actuation, and connect those two contacts to the input and ground. Make the change with fixture power off. Keep the same controller and direct HID path in A, B and C. Route the cable with slack and strain relief on the fixed support. Record switch model, force if measured, support and placement. Use the same switch mechanism for A and B where practical. If using a participant's own switch, record that A versus B compares both placement and switch feel.

For the R30 board the dome input is GPIO8. The C3-DevKitM-1 has an onboard RGB LED on that pin. A dev-board fixture needs its own pin assignment, real optical and dome drivers, and a qualified power profile. The generic firmware uses simulated motion by default. A known working donor optical mouse with its existing lens/illumination and a switch breakout can answer the **form-factor comparison** if the R30 optics are unavailable. Label all donor runs by model and keep them outside R30 qualification. Use bench power with the cell removed and the battery input supplied within its specified range if using an R30 fixture. Never put an unqualified charging assembly on a participant. [First-test equipment and board differences](FIRST-TEST-SHOPPING-LIST.md)

## Qualify the fixture before inviting a participant

1. Photograph the head installed in both carriers. Record the controller and firmware revision, sensor and lens IDs, working distance, masses, host/OS, display refresh, scaling, pointer gain, acceleration, CPI, click suppression and HID logger. Preserve these settings across conditions. Disable other pointing devices for the recorded trials where practical.
2. With the device off the hand, establish signed X and Y on forward, reverse and diagonal strokes. Check a stationary surface, a lift and a set-down. On each switch confirm that press, sustained hold and release produce one intended button sequence. Confirm a disconnect while held releases the host button after link loss. Stop and repair the fixture if any of these fail.
3. Use a wood or matte desk and a representative rigid tray. Identify each specimen and mounting. Apply the existing surface protocol for the stationary jitter, controlled motion/dropout and end-to-end latency trials. Record raw HID counts separately from on-screen pixels. A motion-only HID logger may omit zero reports. Use the prescribed report cadence or independently instrument the sample stream before assigning a dropout percentage. Mark a metric unmeasured if the instrumentation cannot resolve it. [Surface test methods and scoring](SURFACE-TEST-PROTOCOL.md)
4. Secure the work surface and remove sharp edges and exposed contacts. Let the participant choose the finger, side, switch location, arm support and assistance they normally use. Make the power disconnect reachable by the operator. Stop on request, discomfort, pressure marks, or an inability to remove the ring. Record an incomplete condition as an outcome. Keep measurements and any consented recordings local under a participant code.

## Run the comparison

Use an offline document with selectable text, a link, and a draggable item with a marked drop target. Mark a home position for each device. Give each participant practice with each arrangement. Rotate A–B–C, B–C–A and C–A–B across participants. For one participant, repeat the most different pair in reverse order after rest. Record the actual order, breaks and any adjustment. Start a new row if support or settings change.

For each condition on each surface:

1. From the resting position, acquire the pointer, open the link, double-click a word, select a specified phrase, drag the marked item to its target and release it. Lift and reposition at the edge of comfortable travel, then resume pointing. Return to typing or the participant's usual resting activity, then reacquire the device. Repeat the sequence three times. Record task completions, misses, extra presses, unintended scrolling/motion, dropped or stuck drags, help, re-grips and whether the person had to flatten or counter-rotate their finger.
2. Run the 50-click displacement trial from the surface protocol. Save down/up and cursor coordinates, the initial reference position and a pre-press reference for each attempt so cumulative drift and press-induced motion can be distinguished. Count every intended attempt, including missed, extra and off-target events. Record rest-induced activations separately. Use a rest or split blocks if requested. Record the actual denominator for an incomplete block.
3. Ask which actions felt controllable, whether the person could let the hand rest, and whether they would choose this arrangement for that task. Record their words with permission. Separate independent use, preferred assistance and operator intervention.

### Return sheet

Copy one row per participant, condition, surface and settings combination. Attach a manifest linking the raw event log, fixture photo, task sheet and any latency recording by run ID.

| Run / order | Condition / surface | Independent or assisted / tasks completed of 3 sequences | Missed, extra, off-target / attempts | Mean displacement px / rest activations | Drift px / dropout % / latency ms | Dropped or stuck drags / re-grips | Comfort, preference, evidence IDs |
|---|---|---|---|---|---|---|---|
| | | | | | | | |

Use the project's Gate 1 thresholds on both rigid surfaces for R30 evidence: latency <20 ms, dropout <2%, cumulative jitter <5 px, mean click displacement <2 px and zero click failures in 50 attempts. Preserve incomplete and failed results. A donor run or a favorable secondary condition cannot qualify the integrated ring. [Gate 1 acceptance](GO-NO-GO-RUBRIC.md#gate-1--single-ring-human-control-loop)

**Decision.** If B repeatedly improves clicks or dragging over A with comparable tracking, prioritize support for an independently placed switch. If C also improves control or allows rest or independent acquisition that B prevents, prioritize a puck trial for that user. If A performs as well and is preferred, keep the self-contained dome. If all three lose tracking on the same surface, fix the optical head or surface interface first. Keep individual preferences visible when results differ. Record unassisted results on bare surfaces separately from any repeat using a matte tile.
