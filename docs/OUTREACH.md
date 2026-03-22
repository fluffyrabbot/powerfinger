# PowerFinger Outreach — Collaborator & Community Targets

This document tracks organizations, communities, and adjacent projects relevant
to finding EE/ME collaborators, accessibility testers, and visibility for the
PowerFinger project.

---

## Tier 1: Direct Fit — Submit Project / Find Collaborators

### Makers Making Change (MMC)
- **URL:** https://www.makersmakingchange.com/
- **What:** Neil Squire program. 200+ open-source assistive device designs.
  Volunteer network of makers, EEs, MEs. Explicit pathway for submitting new
  device designs — describe the need, get matched with engineers who build it.
- **Why:** They're specifically looking for volunteers who can do electronics,
  soldering, and CAD. PowerFinger fits their model exactly: open-source,
  assistive, cheap BOM, 3D-printable. Their community includes people who have
  built BLE assistive devices before.
- **Action:** Create account, submit PowerFinger as a design challenge targeting
  R30-OLED-NONE-NONE. Get matched with an EE who has done ESP32 + 3D-printed
  enclosure work. They'll bring an end user into the process early.
- **Priority:** Start here.

### ATMakers
- **URL:** https://atmakers.org/
- **What:** Bridges maker community with people who have severe physical and
  cognitive challenges. Smaller than MMC but more hardware-focused, more willing
  to experiment with novel form factors.
- **Why:** Their ethos ("solving problems in assistive technology using the
  skills and tools of the maker community") is almost verbatim our README.
- **Action:** Reach out, present the project, ask for collaborator matching.

### GOAT — Grassroots Open Assistive Tech
- **URL:** https://www.openassistivetech.org/
- **What:** Curates, preserves, and freely shares assistive technology designs
  under open licenses. Supports disabled people in making their designs
  available.
- **Why:** Good for visibility and connecting with the disability community.
  Would likely help amplify the project.
- **Action:** Submit PowerFinger for inclusion in their catalogue.

---

## Tier 2: Community Visibility — Discovery & Recruitment

### Open Assistive
- **URL:** https://openassistive.org/
- **What:** Directory/catalogue of open assistive technology projects (hardware
  and software).
- **Why:** Listing puts PowerFinger in front of people already building in this
  space. Someone who built a mouth-controlled mouse or eye-tracking system might
  see a ring mouse and want to contribute.
- **Action:** List the project once P0 prototype exists.

### e-NABLE
- **URL:** https://enablingthefuture.org/
- **What:** 50,000+ volunteers across 100+ countries. Originally 3D-printed
  prosthetic hands. Massive network with deep experience in 3D printing for
  body-worn devices, sizing across anatomy, community-driven manufacturing.
- **Why:** Primarily ME-focused (mechanical, 3D printing) rather than EE — but
  that's exactly half of what we need. The parametric ring shell (angle, finger
  sizing) is their wheelhouse.
- **Action:** Post the project, specifically recruit for ring shell CAD work.

### Hackaday.io
- **URL:** https://hackaday.io/
- **What:** Where EEs who build weird things for fun hang out. Projects get
  featured on the Hackaday front page.
- **Why:** A $9 ring mouse with a corny name and an accessibility mission is
  exactly the kind of project that gets Hackaday featured — worth more than any
  cold outreach. Also serves as a defensive publication venue (indexed,
  timestamped, searchable by patent examiners).
- **Action:** Create project page with full build instructions for P0. Publish
  simultaneously with GitHub repo going public.

### MIT Assistive Technology Club / ATHack
- **URL:** https://assistivetech.mit.edu/
- **What:** Annual hackathon pairing engineering students with people with
  disabilities. Students often seek longer-term projects after the hackathon.
- **Why:** Enthusiastic, technically capable people specifically motivated by
  accessibility. Come with built-in connections to end users.
- **Action:** Present PowerFinger at next ATHack as a continuing project.

---

## Tier 3: Events

### Open Source Assistive Technology Hackathon
- **URL:** https://www.eventbrite.com/e/open-source-assistive-technology-hackathon-tickets-1984064378967
- **What:** Hosted by GitHub in partnership with NV Access and accessibility
  organizations.
- **Action:** Present PowerFinger, recruit contributors.

### TOM (Tikkun Olam Makers) Makeathons
- **URL:** https://tomglobal.org/
- **What:** Cornell-partnered makeathons connecting maker teams with "need
  knowers" — people with disabilities who deeply understand the problem space.
  Global events, 2–3 day prototyping sprints.
- **Action:** Attend with P0 prototype, get direct feedback from users with
  motor disabilities.

---

## Tier 4: Adjacent Projects — Potential Technical Collaborators

### AsTeRICS / FlipMouse
- **URL:** https://www.asterics.eu/
- **What:** Open framework for assistive input. FlipMouse is a finger/mouth
  controlled computer input device. Their EE contributors have solved BLE HID +
  custom sensor + accessibility UX.
- **Why:** The closest existing project to PowerFinger's firmware needs. Their
  BLE HID implementation is battle-tested. They've already hit and solved the
  pairing/bonding/reconnection pitfalls across OSes.
- **Action:** Reach out directly. They might be interested in a new form factor,
  or at minimum can advise on BLE pairing pitfalls.

### LipSync
- **URL:** https://www.makersmakingchange.com/s/product/lipsync/01tJR000000698fYAA
- **What:** Mouth-controlled mouse, open-source, built through Makers Making
  Change. BLE HID on a custom board.
- **Why:** The firmware engineering (BLE HID on custom hardware) is directly
  transferable to PowerFinger.
- **Action:** Study their BLE implementation. Contact contributors.

### Ploopy
- **URL:** https://ploopy.co/
- **What:** Open-source trackball mice. Solved optical-sensor-on-ball mechanism
  at production scale. Runs QMK firmware.
- **Why:** If we pursue the optical-on-captive-ball variant (S-OPTB), Ploopy's
  engineering is directly relevant. Their community might be interested in the
  ring form factor.
- **Action:** Engage when/if we move to the S-OPTB sensing approach.

### Eyewriter
- **What:** Open-source eye-tracking system, originally built for a graffiti
  artist with ALS. Cheap hardware (PS3 camera, IR LEDs, sunglasses).
- **Why:** Pioneering example of open-source assistive input. Community
  understands the intersection of cheap hardware + accessibility + open design.
- **Action:** Reference as inspiration, connect with alumni of the project.

---

## Outreach Priority Order

1. **Makers Making Change** — submit design challenge, get matched with EE/ME
2. **Hackaday.io** — publish project page for visibility + defensive publication
3. **FlipMouse/AsTeRICS team** — direct outreach for BLE HID firmware guidance
4. **Open Assistive + GOAT** — list for discovery
5. **e-NABLE** — recruit for ring shell mechanical design
6. **ATHack / TOM Makeathon** — present at next event with working prototype

---

## What We're Looking For

### EE (Electrical Engineer)
- ESP32-C3/S3 experience (or similar BLE SoC)
- BLE HID implementation (pairing, bonding, reconnection across OSes)
- Low-power design (sleep/wake, battery management)
- Flex PCB layout for the ring form factor
- Sensor integration (optical mouse sensors, Hall effect)

### ME (Mechanical Engineer) / Industrial Designer
- Parametric CAD (OpenSCAD, FreeCAD, or Fusion 360) for the ring shell
- 3D printing for body-worn devices (tolerances, material selection, comfort)
- Finger anatomy sizing (S/M/L or continuous parametric)
- Standoff geometry for optical sensor focal distance

### Accessibility Tester
- Someone with limited wrist/hand mobility who currently uses a mouse
  alternative (trackball, head mouse, eye tracking, switch)
- Willing to try prototypes and give honest feedback
- Ideally involved from Phase 1 so design decisions are informed by real needs

### Firmware Developer
- ESP-IDF and FreeRTOS experience
- BLE stack debugging (NimBLE preferred)
- Comfort with embedded C, interrupt-driven sensor polling
- Ideally experience with HID report descriptors
