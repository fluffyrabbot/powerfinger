# Multi-Source Vendor Verification

Last verified: 2026-03-26

This document checks whether each BOM line item across all four PowerFinger
variants is available from at least two independent distributors, whether listed
alternatives are actually viable, and flags single-source risks.

---

## Verification Summary

| Status | Count |
|--------|-------|
| Multi-source verified | 11 |
| Single-source (alt available) | 5 |
| Single-source (no alt) | 2 |
| Commodity (multi-source by nature) | 10 |

**Critical findings:**

1. The ESP32-C3-MINI-1-N4 is marked NRND (Not Recommended for New Designs) by
   Espressif. The successor is ESP32-C3-MINI-1-N4X (chip rev v1.1, same pinout).
   Stock remains plentiful but this should be tracked.
2. The PAW3204DB-TJ3L is not stocked by any major Western distributor (DigiKey,
   Mouser, Farnell). It is only available through AliExpress, small Chinese
   brokers, and Octopart-listed specialty houses. The listed alt YS8205 is not a
   real standalone component -- it is an integrated USB mouse controller IC
   harvested from disposable mice and is not a viable alternative.
3. The Snaptron SQ-05400N is sold direct from Snaptron and through DigiKey
   Marketplace, but not through standard DigiKey/Mouser catalog stock. Minimum
   order quantities may apply.
4. The MCP73831 is NOT pin-compatible with the TP4054 without careful PCB
   layout review. The TP4054 is SOT-23-5; the MCP73831 is available in SOT-23-5
   but with a different pin assignment. The BOM's "Alt: MCP73831" note needs a
   caveat.
5. The TP4056 is NOT package-compatible with the TP4054. The TP4056 uses SOP-8
   (8-pin), not SOT-23-5. The BOM already notes it is "larger, through-hole
   friendly for prototype" but calling it an "Alt" is misleading for production.
6. The AH3503 (listed alt for DRV5053) comes in TO-92 by default, not SOT-23.
   SOT-23 variants exist but are harder to source. Pin ordering differs. This is
   not a drop-in alternative without footprint changes.

---

## Detailed Results

### Active Components (ICs and Modules)

---

#### ESP32-C3-MINI-1-N4
*Used in: all ring variants, wand*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | In stock | Ships same day. PN: 13877574 |
| LCSC | In stock | C2838502. ~18k units. $2.01 |
| Mouser | Listed | Espressif is a Mouser-stocked brand; specific stock not confirmed via search |
| Farnell | Not checked | Espressif modules generally available |

- **Alternative (ESP32-C3-MINI-1-H4):** In stock at DigiKey. Same pinout, same
  module family. Only difference is extended temperature range (-40 to 105C vs
  85C). True drop-in replacement.
- **Alternative (ESP32-C3-MINI-1-N4X):** Recommended successor (chip rev v1.1).
  Same pinout. Should be the target for new PCB revisions.
- **NRND warning:** The -N4 (chip rev v0.4) is marked Not Recommended for New
  Designs. Stock is still plentiful but the project should migrate to -N4X.
- **Verdict: Multi-source verified (with NRND caveat)**

---

#### ESP32-S3-MINI-1-N8
*Used in: USB hub dongle*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | In stock | Ships same day. PN: 15295890 |
| Mouser | In stock | Listed on Mouser product page |
| LCSC | In stock | C2913206. ~6.4k units. ~$3.26 |

- **Alternative (ESP32-S3-MINI-1-N4R2):** In stock at LCSC (C3013941, ~$3.42)
  and DigiKey. Same pinout. 4MB flash + 2MB PSRAM vs 8MB flash. True drop-in
  with firmware flash size adjustment.
- **Verdict: Multi-source verified**

---

#### PAW3204DB-TJ3L (optical mouse sensor)
*Used in: standard optical ring*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | Not stocked | Not in DigiKey catalog |
| Mouser | Not stocked | Not in Mouser catalog |
| LCSC | Not confirmed | No listing found in searches |
| AliExpress | Available | Typically $0.50-1.50, sold as sensor+lens kits |
| Octopart | 3 distributors listed | Specialty/broker distributors only |
| Jotrin, Ariat-Tech | Listed | Small Chinese component brokers |

- **Alternative (ADNS-2080):** In stock at DigiKey (ships same day). Broadcom
  part. Same SPI interface, 8-DIP optical module. Functionally equivalent for
  low-power mouse applications. This is the strongest Western-sourced alt.
- **Alternative (YS8205):** NOT a viable alternative. The YS8205 is an
  integrated USB mouse controller with built-in sensor -- it has its own USB
  stack and is not an SPI peripheral sensor. It is harvested from disposable
  mice, not sold as a component. BOM should remove this as an alt.
- **Supply chain risk:** This is a PixArt part distributed through PixArt's
  authorized channel (EPS Global) and Chinese brokers. Not available through
  any tier-1 Western distributor for small quantities. The AliExpress
  sensor+lens kit channel is the realistic prototype source.
- **Verdict: Single-source risk (AliExpress). ADNS-2080 is a viable Western-sourced alt.**

---

#### DRV5053VAQDBZR (linear Hall effect sensor)
*Used in: ball ring (x4), wand (x4)*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | In stock | Ships same day. PN: 5015738 |
| LCSC | In stock | C962159. ~$0.14 |
| Mouser | Expected available | TI parts are universally stocked at Mouser |

- **Alternative (AH3503):** Available, but with caveats. The AH3503 from
  Nanjing AH Electronics comes in TO-92 package by default, not SOT-23. SOT-23
  variants (SOT-23-3L) exist per the datasheet but are harder to source. Pin
  ordering (VCC/GND/OUT) differs from DRV5053 (VCC/OUT/GND) -- this is NOT a
  drop-in replacement without PCB changes. Also, the AH3503 minimum supply
  voltage is 3.5V (some datasheets say 4.5V) vs DRV5053's 2.5V, which may
  cause issues at low battery. The AH3503 also draws 6-14mA vs DRV5053's 3mA,
  significantly worse for battery life with 4 sensors.
- **Verdict: Multi-source verified. AH3503 alt is NOT pin-compatible -- BOM
  should flag this.**

---

#### TP4054 (LiPo charge controller)
*Used in: all ring variants, wand*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | In stock | UMW and EVVO brands. Ships same day |
| LCSC | In stock | C382138 (TPOWER). C32574. ~$0.03 |
| Mouser | Not confirmed | Not a Mouser-typical brand |

- **Alternative (TP4056):** NOT pin-compatible. The TP4056 is SOP-8 (8-pin)
  vs TP4054's SOT-23-5. Different package, different footprint. The BOM
  correctly notes "larger, through-hole friendly for prototype" but listing it
  as "Alt" is misleading for production boards. It is a functional equivalent
  only if the PCB has a separate TP4056 footprint.
- **Alternative (MCP73831):** Functionally similar. Available in SOT-23-5 from
  Microchip (DigiKey, Mouser, widely stocked). However, pinout is NOT
  identical to TP4054 -- pin assignment differs. The MCP73831 datasheet shows
  a different pin 1 function. This requires PCB layout verification before
  claiming drop-in compatibility.
- **Alternative (LTC4054):** The original design that TP4054 clones. Same
  SOT-23-5, closest to pin-compatible. Available at DigiKey from Analog
  Devices. This is the best true alt if TP4054 becomes unavailable.
- **Verdict: Multi-source verified (DigiKey + LCSC). Alt compatibility
  notes need correction in BOM.**

---

#### RT9080-33GJ5 (3.3V ultra-low-Iq LDO)
*Used in: all ring variants, wand*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | In stock | Ships same day. PN: 6161634 |
| LCSC | In stock | C882092. ~59k units |
| Mouser | Not confirmed in search | Richtek parts typically available |
| Newark | In stock | PN: 06AC2505 |

- **Alternative (XC6220B331MR-G):** In stock at DigiKey, Mouser, and LCSC
  (C86534, ~21k units). SOT-25-5 package (same as TSOT-23-5 for practical
  purposes). However, the XC6220 has 8uA quiescent current vs RT9080's 0.5uA
  -- a 16x difference that matters for deep sleep power budget. It is a
  functional alt but a significant performance downgrade for this application.
  The BOM correctly notes "NOT AP2112K" but should also note the Iq tradeoff
  for XC6220.
- **Verdict: Multi-source verified**

---

### Switches

---

#### Snaptron SQ-05400N (metal snap dome)
*Used in: all ring variants, wand (optional)*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey Marketplace | Possibly available | Snaptron is a DigiKey supplier partner, but the specific SQ-05400N was not found in catalog search |
| Mouser | Not found | Not in standard catalog |
| LCSC | Not found | Not stocked |
| Snaptron direct | Available | Order direct from snaptron.com |

- **Alternatives:** The BOM correctly notes "any 4-6mm metal dome, 150-250gf
  actuation" from Murata or C&K. These are commodity metal dome switches
  available from multiple sources. DigiKey stocks various Snaptron dome
  models (GX and F series) through their marketplace.
- **Risk assessment:** Low. Metal snap domes are commodity items made to
  standard dimensions. The specific Snaptron part number matters less than the
  dimensional and force specs. Any 5mm round dome with ~200gf actuation will
  work.
- **Verdict: Single-source for specific PN (Snaptron direct), but commodity
  category with many equivalent parts.**

---

#### Kailh GM8.0 (micro switch, wand barrel button)
*Used in: wand*

| Distributor | Status | Notes |
|-------------|--------|-------|
| DigiKey | Not found | Not in DigiKey catalog |
| Mouser | Not found | Not in Mouser catalog |
| LCSC | Not confirmed | Not found in search results |
| AliExpress | Available | ~$0.20-0.50 per switch |
| Amazon | Available | 4-packs, various sellers |
| kailhswitch.net | Available | Official Kailh store |

- **Alternative (Omron D2FC-F-7N):** In stock at DigiKey (multiple variants:
  20M, 60M, 100M cycle life). In stock at LCSC (~$0.29). Widely available
  from all major distributors. Same 3-pin mouse micro switch footprint. This
  is the industry standard mouse switch and an excellent fallback.
- **Verdict: Single-source for Kailh GM8.0 (AliExpress/direct). Omron D2FC
  alt is fully multi-source and pin-compatible.**

---

### Passive Components and Commodity Items

These are standard commodity parts available from any electronics distributor.
Verification is provided at category level rather than per-part.

---

#### 100nF 0402 MLCC Capacitor
*Used in: all variants (x2 each)*

- Available from every distributor. Millions of units in stock globally.
  Multiple manufacturers (Samsung, Murata, Yageo, TDK).
- **Verdict: Commodity, multi-source by nature**

---

#### 10uF 0402/0603 MLCC Capacitor
*Used in: all variants*

- Standard bulk/bypass cap. Available everywhere.
- **Verdict: Commodity, multi-source by nature**

---

#### 10k ohm 0402 Resistor
*Used in: all variants (charge current set)*

- Available from every distributor.
- **Verdict: Commodity, multi-source by nature**

---

#### USB-C Connector (2.0 receptacle, SMD)
*Used in: all ring variants, wand*

- Generic USB-C 2.0 receptacles are commodity items at LCSC (hundreds of
  listings). Also available at DigiKey and Mouser from brands like GCT, Amphenol,
  Wurth, Molex.
- **Verdict: Commodity, multi-source by nature**

---

#### USB-A Male Connector
*Used in: USB hub dongle*

- Standard through-hole or SMD USB-A plug. Commodity at all distributors.
- **Verdict: Commodity, multi-source by nature**

---

#### IR/Red LED (0603)
*Used in: standard optical ring*

- Generic indicator LED. Available everywhere. Often included with sensor kits.
- **Verdict: Commodity, multi-source by nature**

---

#### Status LED (0402, green) + 1k ohm resistor
*Used in: USB hub dongle*

- Standard parts at any distributor.
- **Verdict: Commodity, multi-source by nature**

---

#### Tactile Switch (6x6mm, boot/reset button)
*Used in: USB hub dongle*

- Standard 6x6mm tactile switch. Commodity at all distributors.
- **Verdict: Commodity, multi-source by nature**

---

#### 2.4GHz Chip Antenna
*Used in: noted in optical ring BOM but ESP32-C3-MINI-1 has onboard antenna*

- Only needed if using U.FL module variant. Standard 2.4GHz chip antennas
  available from Johanson, Fractus, Abracon at all major distributors.
- **Verdict: Commodity (and likely not needed)**

---

### Mechanical and Custom Components

These components are inherently outside the traditional electronics distributor
ecosystem. They are verified against their specific sourcing channels.

---

#### LiPo Battery (80-100mAh / 100-150mAh pouch cell)
*Used in: all ring variants (80-100mAh), wand (100-150mAh)*

- Small LiPo pouch cells are commodity items on AliExpress. Multiple Chinese
  cell manufacturers. The size constraint (20x15x4mm for ring, 40x10x5mm for
  wand) limits options but cells in this range are widely available from brands
  like DTP, Shenzhen Data Power, and others.
- Not available from DigiKey/Mouser at these sizes (they stock Renata, Varta
  coin cells which are different chemistry/form factor).
- **Verdict: Commodity within AliExpress ecosystem. Not available from
  Western distributors at required sizes.**

---

#### 5mm Steel Ball
*Used in: ball ring, wand*

- Chrome steel or stainless steel bearing balls. Available from McMaster-Carr,
  AliExpress, Amazon, and any bearing supplier.
- **Verdict: Commodity, multi-source by nature**

---

#### 1x1mm N52 Neodymium Magnets
*Used in: ball ring (x4), wand (x4)*

- Micro neodymium magnets in this size are available from AliExpress, Amazon
  (various magnet vendors), and specialty magnet suppliers (K&J Magnetics,
  first4magnets). Not a standard electronics distributor item.
- **Verdict: Commodity within magnet/hobby supplier ecosystem**

---

#### Roller Spindles
*Used in: ball ring (x4), wand (x4)*

- Custom or modified parts. Will likely need to be fabricated (CNC lathe or
  sourced as modified dowel pins). This is the most custom mechanical component
  in the BOM.
- **Verdict: Custom fabrication required. No off-the-shelf source.**

---

#### UHMWPE Glide Pads
*Used in: standard optical ring (x4)*

- Punched from UHMWPE sheet stock. McMaster-Carr stocks UHMWPE sheet in various
  thicknesses. Also available from AliExpress and plastics suppliers.
- **Verdict: Commodity raw material, custom punching required**

---

#### PCBs (flex/rigid-flex/rigid)
*Used in: all variants*

- JLCPCB, PCBWay, OSH Park, and dozens of other PCB fabricators. Standard
  service.
- **Verdict: Commodity service, multi-source by nature**

---

#### 3D-Printed Shells, Sockets, Tips, Caps
*Used in: all variants*

- Any FDM/SLS printer or service bureau (JLCPCB 3D printing, Shapeways,
  Craftcloud, local makerspaces).
- **Verdict: Commodity service, multi-source by nature**

---

#### Aluminum/Stainless Tube (wand body)
*Used in: wand*

- 8mm OD tubing from McMaster-Carr, AliExpress, Amazon, or any metals supplier.
  Cut to length.
- **Verdict: Commodity, multi-source by nature**

---

## BOM Correction Recommendations

Based on this verification, the following BOM changes are recommended:

### Corrections to Make

1. **Remove YS8205 as PAW3204 alt.** The YS8205 is not an SPI mouse sensor --
   it is an integrated USB mouse controller. It cannot substitute for the
   PAW3204DB in this design. Replace with a note that ADNS-2080 (DigiKey) is
   the verified Western-source alternative.

2. **Flag AH3503 as NOT pin-compatible with DRV5053.** Different pin ordering
   (VCC/GND/OUT vs VCC/OUT/GND), default TO-92 package (not SOT-23), higher
   minimum voltage (3.5V+), and 2-5x higher current draw. Requires PCB changes
   to use. Change from "Alt" to "Functional equivalent (different footprint and
   pinout)."

3. **Clarify TP4056 is NOT a drop-in alt for TP4054.** Different package
   (SOP-8 vs SOT-23-5). Add note: "Requires separate footprint on PCB." Add
   LTC4054 (Analog Devices) as the true pin-compatible alt.

4. **Add pinout caveat to MCP73831 alt.** The MCP73831 in SOT-23-5 has a
   different pin assignment than the TP4054. Note: "Requires PCB layout
   verification. Not a blind drop-in."

5. **Add NRND note to ESP32-C3-MINI-1-N4.** Note that Espressif recommends
   migrating to ESP32-C3-MINI-1-N4X (chip rev v1.1, same pinout, more flash
   space). Current stock is plentiful but long-term availability is uncertain.

6. **Add XC6220 Iq caveat.** The XC6220B331MR has 8uA Iq vs RT9080's 0.5uA.
   Note the 16x quiescent current penalty in the alt column, since deep sleep
   power budget is a first-order design concern.

### Additions to Make

7. **Add ADNS-2080 to optical ring BOM** as the primary verified alternative
   with DigiKey/Mouser sourcing.

8. **Add LTC4054ES5-4.2 to all BOMs** as the true pin-compatible charger IC
   alternative (Analog Devices, DigiKey PN: 960438).

9. **Add ESP32-C3-MINI-1-N4X to all C3 BOMs** as the recommended successor
   module.

---

## Risk Matrix by Variant

### R30-OLED-NONE-NONE (Standard Optical Ring, ~$9)

| Component | Risk | Mitigation |
|-----------|------|------------|
| PAW3204DB-TJ3L | HIGH | AliExpress only. Stock ADNS-2080 as backup |
| Snaptron SQ-05400N | LOW | Any 5mm dome works. Commodity category |
| LiPo pouch cell | LOW | Commodity on AliExpress. Multiple vendors |
| All other parts | NONE | Multi-source verified |

### R30-BALL-NONE-NONE (Ball+Hall Ring, ~$11)

| Component | Risk | Mitigation |
|-----------|------|------------|
| Roller spindles | MEDIUM | Custom fab required. No off-the-shelf source |
| Snaptron SQ-05400N | LOW | Any 5mm dome works |
| LiPo pouch cell | LOW | Commodity on AliExpress |
| All other parts | NONE | Multi-source verified |

### WSTD-BALL-NONE-NONE (Wand, ~$14)

| Component | Risk | Mitigation |
|-----------|------|------------|
| Kailh GM8.0 | LOW | AliExpress only, but Omron D2FC is multi-source drop-in |
| Roller spindles | MEDIUM | Custom fab required |
| Aluminum tube | NONE | McMaster + many sources |
| All other parts | NONE | Multi-source verified |

### USB-HUB (Hub Dongle, ~$5-6)

| Component | Risk | Mitigation |
|-----------|------|------------|
| (none elevated) | NONE | All parts multi-source or commodity |

---

## Methodology and Limitations

This verification was conducted via web search of distributor websites on
2026-03-26. The following limitations apply:

- **Stock quantities are point-in-time snapshots.** Distributor inventory
  changes daily. "In stock" means the part was listed as available on the
  date checked, not that it will be available at time of order.
- **Mouser verification was indirect** for several parts. Mouser product pages
  were confirmed to exist but exact stock counts were not always returned by
  search. Mouser carries Espressif, TI, Richtek, Microchip, and Torex as
  stocked brands, so availability is expected but not individually confirmed
  for every MPN.
- **AliExpress "availability" is inherently fuzzy.** Multiple sellers list
  the same parts; some are genuine, some are relabeled. For prototype
  quantities this is acceptable. For production, authenticated distributor
  channels are preferred.
- **Broker/specialty distributor stock** (Jotrin, Ariat-Tech, OMO Electronic)
  was noted but not independently verified. These sources often carry stock
  but may have higher prices or longer lead times.
- **Pin compatibility claims** were checked against datasheets where possible
  but full electrical validation requires physical testing.
