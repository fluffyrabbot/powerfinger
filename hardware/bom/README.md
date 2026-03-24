# Bills of Materials

Pre-schematic BOMs for P0 and P1 prototype variants. These are intent documents
— they specify target components and acceptable alternatives. When KiCad
schematics exist, the EE generates authoritative BOMs from the schematic and
these files become reference.

## Files

| File | Variant | BOM Target | Priority |
|------|---------|-----------|----------|
| `R30-OLED-NONE-NONE.csv` | Optical ring (30°, dome click) | ~$9 | P0 |
| `R30-BALL-NONE-NONE.csv` | Ball+Hall ring (30°, dome click) | ~$11 | P1 |
| `WSTD-BALL-NONE-NONE.csv` | Ball+Hall wand (standard tip) | ~$14 | P1 |
| `USB-HUB.csv` | USB hub dongle (BLE central + USB HID) | ~$5–6 | P0 |

## Format

Standard CSV. Columns follow OSHWA/KiCad convention:

`Ref, Qty, Description, Value, Package, Manufacturer, MPN, Supplier, Supplier PN, Unit Cost, Notes`

- **Unit Cost** is at prototype quantities (1–10 units), not volume pricing.
- **Notes** column lists acceptable alternatives and constraints.
- Supplier part numbers are provided where known; `—` means "search by MPN."
- All components must be sourceable from multiple vendors (project hard rule).

## Suppliers Referenced

| Supplier | URL | Notes |
|----------|-----|-------|
| LCSC | https://lcsc.com | Best for SMD components, ships from China |
| DigiKey | https://digikey.com | US/EU stock, fast shipping, higher prices |
| Mouser | https://mouser.com | Similar to DigiKey |
| AliExpress | https://aliexpress.com | Cheapest for modules, sensors, batteries. Slow shipping. |
| McMaster-Carr | https://mcmaster.com | Raw materials (UHMWPE sheet, metal tube, steel balls) |
| JLCPCB | https://jlcpcb.com | PCB fabrication + assembly |
| PCBWay | https://pcbway.com | PCB fabrication, good for flex PCB |
