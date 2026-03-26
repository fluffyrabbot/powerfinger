# PowerFinger

**Open-source assistive input devices. Under $25 BOM. Build one for anyone who
needs it.**

PowerFinger is a family of wearable and handheld pointing devices designed for
accessibility, repairability, and surface agnosticism. Every design file —
schematics, PCB layouts, 3D models, firmware, companion app — is published under
CERN-OHL-S 2.0 (strongly reciprocal open hardware license).

## Philosophy

Computer input is a solved problem for people who can sit at a desk and grip a
mouse. For everyone else — people with limited wrist mobility, repetitive strain
injuries, tremors, amputations, arthritis, or simply no desk — it isn't.

Existing assistive input devices cost $100–$350 and are closed, proprietary, and
fragile. When the vendor disappears, the device becomes e-waste and the user
loses their interface to the digital world.

PowerFinger treats input devices as **prosthetics for digital agency**. The more
redundant ways people have to control things the way they find most natural, the
better as a common good. We design for:

- **Affordability.** $9–$25 BOM depending on variant. The cheapest ring is under
  $10 in components. A complete two-ring mouse system is ~$24.
- **Universal design.** Every device works for able-bodied users at their desk
  AND for users with specific accessibility needs. No separate "disability"
  product line.
- **Surface agnosticism as a spectrum.** The optical ring works on most opaque
  surfaces (not glass). The ball+Hall variants work on any surface including
  glass, fabric, and skin. The IMU variant needs no surface at all. Each
  variant's limitations are documented honestly — see
  [docs/COMBINATORICS.md](docs/COMBINATORICS.md).
- **Composability.** One ring is a mouse. Two rings are a mouse + scroll wheel.
  Three rings are a configurable control surface. The system grows with the
  user's needs.
- **Repairability.** Every part is replaceable. 3D-print a new shell, swap a
  battery, reflash firmware. No planned obsolescence.
- **Privacy.** Fully offline operation. No cloud, no telemetry, no account. Your
  movement data stays on your device.

## Form Factors

### Ring

Every ring has a tracking sensor + a click switch + BLE. One ring is a mouse.
Two identical rings are a **complete mouse replacement**: cursor + left click on
one finger, scroll + right click on the other. Roles are assigned in software,
not hardware.

```
Middle finger (Ring 1):  move cursor + left click
Index finger  (Ring 2):  scroll      + right click
```

Everything else — drag, double-click, zoom, middle click, modifiers — is
software-defined combinations of those four primitives.

| Variant | BOM | Surface | Resolution |
|---------|-----|---------|------------|
| Optical ring (P0) | ~$9 | Most opaque surfaces (not glass) | 800–2000 CPI |
| Ball+Hall ring (P1) | ~$11 | Any surface including glass | ~15–60 DPI |
| Optical-on-ball ring (Pro) | ~$17 | Any surface including glass | Up to 12,000 CPI |

A USB hub dongle (~$6) composes two or more rings into a single USB HID mouse
that works on any OS without drivers or a companion app.

### Wand

A pen-shaped BLE HID pointing device with a ball+Hall sensor at the tip. Hold it
like a pen, drag it across any surface — desk, glass, tray table, blanket, wall,
your knee. ~$14 BOM.

No existing product fills this space. Commercial pen mice ($35–150) use optical
sensors that fail on glass and degrade past ~20 degrees of pen tilt. Assistive
styluses only work on touchscreens. The wand works on any surface, at any
natural pen angle (30–70 degrees), over BLE HID with no dongle required.

Pen grip reduces forearm pronation from ~42 degrees (flat mouse) to ~28 degrees
— clinically significant for carpal tunnel and RSI. An aluminum or stainless
steel body provides natural weight stabilization for tremor (same principle as
weighted assistive pens).

See [docs/WAND-COMPETITIVE.md](docs/WAND-COMPETITIVE.md) for the competitive
landscape, accessibility case, and patent analysis.

### Other Mountings

The electronics module is invariant; the harness adapts it to any body position.
The ring shell is just one harness. Others: toe ring, knuckle strap, wrist
bracelet, prosthetic mount, headband. Same PCB, same firmware, different
physical mounting. The companion app assigns meaning.

See [docs/COMBINATORICS.md](docs/COMBINATORICS.md) for the full design space.

## Sensing Mechanisms

| Mechanism | Surface Agnostic? | Moving Parts | BOM | Resolution |
|-----------|-------------------|-------------|-----|-----------|
| Mechanical ball + Hall effect | Yes — anything | Yes | ~$2 | Low (9–36 ticks/rev) |
| Direct optical (LED) | Most opaque surfaces | None | ~$0.50–4 | Good (800–2000 CPI) |
| Direct laser (VCSEL) | More surfaces than LED | None | ~$2–5 | Good (2000+ CPI) |
| Optical on captive ball | Yes — anything | Ball only | ~$5–8 | Excellent (12K CPI) |

## Optional Capabilities

- **OCR camera** — scan text from physical books/documents, send to companion
  app for local OCR (Tesseract/PaddleOCR), translation, or text-to-speech.
  Requires companion app; pointing function works without it.
- **Guidance laser** — red dot on the surface shows exactly where you're
  pointing/scanning

## Quick Start

*Coming soon.* See [docs/COMBINATORICS.md](docs/COMBINATORICS.md) for what to
build first and [docs/PROTOTYPE-SPEC.md](docs/PROTOTYPE-SPEC.md) for the EE/ME
build spec.

## Project Status

**Pre-prototype.** Architecture, design space, power budget, and competitive
analysis documented. Hardware prototyping next. See
[docs/FIRMWARE-ROADMAP.md](docs/FIRMWARE-ROADMAP.md) for the firmware build
order — every phase runs on a $3 ESP32-C3 dev board before prototype hardware
arrives.

## License

Hardware designs: [CERN-OHL-S 2.0](https://opensource.org/license/cern-ohl-s)
(strongly reciprocal)

Firmware and software: MIT

The CERN-OHL-S patent retaliation clause means anyone using these designs who
sues anyone else over them automatically loses their license. The reciprocal
requirement means anyone manufacturing products from these designs must publish
their complete source. See [docs/IP-STRATEGY.md](docs/IP-STRATEGY.md) for full
rationale.

## Why "PowerFinger"?

Controlling a computer should require nothing more than a single finger. The
name is corny. It sticks.
