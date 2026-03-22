# PowerFinger

**Open-source assistive input devices. $9 BOM. Build one for anyone who needs it.**

PowerFinger is a family of wearable and handheld pointing devices designed from
first principles to be cheap, accessible, repairable, and surface-agnostic.
Every design file — schematics, PCB layouts, 3D models, firmware, companion
app — is published under CERN-OHL-S 2.0 (strongly reciprocal open hardware
license).

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

- **Radical affordability.** $9–$20 BOM. A 3D printer and a soldering iron is
  all you need.
- **Universal design.** Every device works for able-bodied users at their desk
  AND for users with specific accessibility needs. No separate "disability"
  product line.
- **Surface agnosticism.** Works on a desk, a lap, a tray table, glass, fabric,
  a wall, your jeans. No special surface, pad, or receiver required.
- **Composability.** One ring is a mouse. Two rings are a mouse + scroll wheel.
  Three rings are a configurable control surface. The system grows with the
  user's needs.
- **Repairability.** Every part is replaceable. 3D-print a new shell, swap a
  battery, reflash firmware. No planned obsolescence.
- **Privacy.** Fully offline operation. No cloud, no telemetry, no account. Your
  movement data stays on your device.

## Form Factors

### Fingertip Ring

A ring worn on the distal phalanx (fingertip pad) of the middle finger. Drag
your finger across any surface to move the cursor. Available in multiple sensor
angles optimized for different hand positions.

### Hefty Pen (Wand)

A pen-shaped device with a tracking element at the tip. Hold it like a pen, drag
it across any surface — desk, tray table, wall, your knee. A surface-agnostic
tablet without the tablet.

### Multi-Ring System

Multiple independent rings compose into a modular control surface. Each finger
is an input channel. The configuration is user-defined.

See [docs/COMBINATORICS.md](docs/COMBINATORICS.md) for the full matrix of form
factors, sensing mechanisms, and optional capabilities.

## Sensing Mechanisms

| Mechanism | Surface Agnostic? | Moving Parts | BOM | Resolution |
|-----------|-------------------|-------------|-----|-----------|
| Mechanical ball + Hall effect | Yes — anything | Yes | ~$2 | Low (9–36 ticks/rev) |
| Direct optical (LED) | Most surfaces | None | ~$0.50–4 | Good (800–2000 CPI) |
| Direct laser (VCSEL) | More surfaces | None | ~$2–5 | Good (2000+ CPI) |
| Optical on captive ball | Yes — anything | Ball only | ~$5–8 | Excellent (12K CPI) |

## Optional Capabilities

- **OCR camera** — scan text from physical books/documents, send to companion
  app for AI-powered explanation, translation, or text-to-speech
- **Guidance laser** — red dot on the surface shows exactly where you're
  pointing/scanning

## Quick Start

*Coming soon.* Phase 1 prototyping in progress. See
[docs/COMBINATORICS.md](docs/COMBINATORICS.md) for build options and
[docs/IP-STRATEGY.md](docs/IP-STRATEGY.md) for the patent landscape.

## Project Status

**Pre-prototype.** Architecture and combinatorial design space documented.
Hardware prototyping begins when funding permits.

## License

Hardware designs: [CERN-OHL-S 2.0](https://opensource.org/license/cern-ohl-s)
(strongly reciprocal)

Firmware and software: MIT

This is a defensive choice. The CERN-OHL-S patent retaliation clause means
anyone using these designs who sues anyone else over them automatically loses
their license. The reciprocal requirement means anyone manufacturing products
from these designs must publish their complete source. See
[docs/IP-STRATEGY.md](docs/IP-STRATEGY.md) for full rationale.

## Why "PowerFinger"?

Because controlling a computer should require nothing more than the power of a
single finger. And because it's just corny enough to remember.
