<!-- SPDX-License-Identifier: MIT -->
# First-test shopping list

Refreshed 2026-09-07. USD, US suppliers, before shipping and tax. Listed prices
were checked on that date; stock must be reconfirmed at checkout. Consumable
allowances below are planning estimates, not vendor quotes.

Scope: two optical ring nodes plus one USB hub, following the
[active lane](ACTIVE-LANE-CHECKLIST.md). Start with one node, then add the second.
Development boards support USB-powered BLE/USB bench tests; optical control and
wearable qualification also require the sensor kit and physical ring fixture.

## Buy or reuse for the first bench

| Qty | Item | Price / allowance | Purpose and purchasing status |
| --- | --- | --- | --- |
| 2 | [Espressif ESP32-C3-DEVKITM-1-N4X](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-DEVKITM-1-N4X/25587617) | $8 each; **$16 total** | Two ring-node controllers; listed in stock. Documented headers and Micro-B USB. |
| 1 | [ESP32-S3-DevKitC-1-N8R8, Adafruit 5336](https://www.adafruit.com/product/5336) | **$19.95** | Hub controller, onboard antenna, native USB plus separate UART USB. Listing offers Add to Cart; no stock count provided. |
| 4 | USB data cables, host connector to Micro-B | $12–20 total allowance | Two C3 boards, S3 native USB, and S3 debug UART. Reuse a compatible host hub/adapter if necessary. |
| 1 set | Two breadboards, short jumper wires, two normally-open momentary buttons, resistor/capacitor assortment | $25–40 allowance | Stable bench wiring and click inputs; final resistor values follow the bench wiring plan. |
| 1 | Digital multimeter | Reuse/borrow; otherwise $25–50 allowance | Continuity and supply checks before connection. |
| 3 kits | PAW3204DB-TJ3L sensor with matched lens and emitter | **Quote required** | Two working optical nodes plus one spare. Exact low-quantity complete-kit supply remains unresolved. |

Boards total **$35.95**. With cables and wiring, allow **about $75–100**, assuming
an existing multimeter and host computer. Allow **about $100–150** if a meter is
also needed. Both totals exclude optical kits, printed fixtures, instruments,
shipping and tax.

For the optical quote, specify **PAW3204DB-TJ3L** and ask the supplier to confirm
the complete illumination/lens stack, pinout, supply requirements, dimensions,
minimum order and unit price. The
[manufacturer datasheet hosted by EPS Global](https://www.epsglobal.com/Media-Library/EPSGlobal/Products/files/pixart/PAW3204DB-TJ3L.pdf?ext=.pdf)
identifies the PNLR-012LSI infrared LED lens. Confirm what the kit actually
includes before ordering. The old sensor prices in the
[reference BOM](../hardware/bom/R30-OLED-NONE-NONE.csv) are estimates, not a
verified purchasing route. Other PAW3204 suffixes, generic mouse controller ICs,
ADNS-2080 and PMW3360 require hardware/driver evaluation before substitution.

## Buy or borrow for measurement

| Qty | Item | Checked price | When needed |
| --- | --- | --- | --- |
| 1 | [Nordic PPK2, Adafruit 5048](https://www.adafruit.com/product/5048) | **$99.95**, listed in stock | Active/idle/sleep current qualification. Micro-B cable is extra; reuse one or add a fifth cable. |
| 1 | [SparkFun 8-channel 24 MHz USB logic analyzer, TOL-18627](https://www.sparkfun.com/usb-logic-analyzer-24mhz-8-channel.html) | **$26.95** | Sensor bus and GPIO timing, using PulseView/sigrok. Listing shows conflicting stock/backorder labels; confirm availability or borrow equivalent. USB-C cable and jumper leads included. |
| 1 optional | [Nordic nRF52840 Dongle](https://www.digikey.com/en/products/detail/nordic-semiconductor-asa/NRF52840-DONGLE/9491124) | $11.02, **out of stock at this supplier** | BLE packet diagnosis with [Nordic's sniffer](https://www.nordicsemi.com/Products/Development-tools/nRF-Sniffer-for-Bluetooth-LE). Borrow or locate stock when RF diagnosis needs it. |

PPK2 plus analyzer adds **$126.90**: roughly **$200–225** for the bench and these
two instruments, still excluding sensors/fixtures and assuming a reused meter.
Check the analyzer vendor's ground-pin erratum: the pin next to input 6 may
carry a clock; use a verified ground pin.

Borrow a current-limited supply and contact thermometer for custom-board and
charging bring-up. Keep the USB and profiler power paths explicitly isolated
when powering a DUT from the profiler. Dev-board LEDs, regulator and USB bridge
contribute current; product power claims require an isolated DUT measurement.
GPIO captures and phone video alone do not establish sensor-to-host latency
below 20 ms; that requires a defined, synchronized measurement procedure.

## Gather for optical and accessibility tests

- Wood, glass, fabric, paper, glossy magazine and matte plastic samples.
- A 300 mm ruler, 150 mm digital calipers, tape and marker.
- A phone capable of high-frame-rate video and a stable clamp; reuse equipment.
- A 0.1 g scale and 50/100/200 g test weights for repeatable fixture loading.
- Small samples of 5 mm snap domes around 150/200/250 gf and 0.5 mm UHMWPE glide
  material. Compare click force with users' needs; a bench button does not
  qualify ring ergonomics.
- Printed ring and hub fit coupons, using an existing printer or print service.
  Allow $30–60 for a small fit/consumables batch if basic measuring tools are
  already available; obtain a print quote before treating this as a firm cost.

Use [the surface protocol](SURFACE-TEST-PROTOCOL.md) for measured outcomes,
including surfaces on which optical tracking fails. Generate coupons through
`scripts/generate-r30-ring-fit-coupons.sh` and
`scripts/generate-usb-hub-validation-coupons.sh`; record physical observations in
the existing ring and hub coupon ledgers.

## Resolve before flashing or ordering custom boards

1. **Create a reviewed bench pin profile and wiring sheet.** The C3 DevKitM-1
   [uses GPIO8 for its RGB LED](https://documentation.espressif.com/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitm-1/index.html),
   while the custom R30 profile uses that pin for the dome. Supply/battery/NTC
   sense inputs also need deliberate bench treatment. Do not flash the custom
   R30 profile unchanged onto the dev board with floating sense inputs. A
   SENSOR_NONE/simulated-input harness can exercise protocol behavior while
   optical sourcing is open; human-control qualification needs the real sensor.
2. **Use the S3 native USB port for HID/CDC.** Its UART bridge port is for
   flashing/debugging. Identify both ports and the received board revision in
   the wiring sheet before testing.
3. **Close fit and supplier intake before assembly purchase.** Follow the
   [ring first-board checklist](../hardware/ring/R30-OLED-NONE-NONE/FIRST-BOARD-CHECKLIST.md)
   and [hub first-board checklist](../hardware/shared/USB-HUB/FIRST-BOARD-CHECKLIST.md).
   Request a quote for two assembled rings and one hub, with spare-board pricing
   separately. Derive the assembly order from the current CAD packet;
   [reference CSV BOMs](../hardware/bom/README.md) are not assembly authority.
4. **Select batteries after measured fit and harness definition.** The ring
   calls for protected 80–100 mAh 3.7 V cells, target maximum 20 × 15 × 4 mm,
   documentation and replaceable harnesses per the reference BOM. Match charge
   current to the selected cell under [battery safety](BATTERY-SAFETY.md).
   The routed P0 uses off-board USB/battery service pads and fixture-controlled
   charge VBUS; it has no onboard USB-C receptacle or MCU charge cutoff.

Bench equipment is a reusable development expense. The ring's roughly $9 BOM
target and the project ceilings still need validation against actual component
and assembly quotes; this shopping list does not establish those costs.

## Recommended follow-up

Prepare the dedicated C3/S3 bench profiles, connector-labelled wiring sheet,
and local bring-up record; prepare an exact sensor-kit RFQ for review. Acceptance:
the pin and sense-input audit passes, scoped local verification passes, and the
record distinguishes simulated protocol tests from real optical tests. Once
hardware is connected, record USB HID/CDC enumeration, one-node click/release
and reconnect, then two-node roles and stuck-button recovery in the
[ESP-IDF qualification checklist](ESP-IDF-6.1-QUALIFICATION.md). Keep physical
results pending until measured and the optical purchasing line pending until a
supplier confirms a complete compatible kit and price.
