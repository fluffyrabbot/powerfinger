<!-- SPDX-License-Identifier: MIT -->
# R30-OLED Firmware Config Contract

This is the board-specific firmware binding for the active
`R30-OLED-NONE-NONE` optical ring. It bridges the packet-level KiCad interface
contract and the ring firmware Kconfig settings.

The generic `firmware/ring/sdkconfig.defaults` remains the dev-board baseline.
Do not put first-board-only hardware assumptions there. Use the R30 fragment
through the local verifier when building firmware for the active optical board:

```bash
scripts/verify-firmware-local.sh --ring-profile r30-oled-none-none
```

The verifier configures `build-idf/r30-oled-none-none/sdkconfig`, checks the
resolved values below, and only then builds the ring profile.

## Config Binding

| Board contract | Firmware setting | Value | Notes |
| --- | --- | --- | --- |
| Active sensor | `CONFIG_SENSOR_PAW3204` | `y` | Selects the PAW3204 bit-banged optical driver. |
| Deferred sensors | `CONFIG_SENSOR_NONE`, `CONFIG_SENSOR_PMW3360`, `CONFIG_SENSOR_BALL_HALL` | not set | Keeps the active board out of fake-motion, Pro, and Hall modes. |
| Primary click | `CONFIG_CLICK_SNAP_DOME` | `y` | Uses the active-low dome GPIO driver. |
| Deferred click | `CONFIG_CLICK_NONE`, `CONFIG_CLICK_PIEZO_LRA` | not set | Keeps P0 on dome click, not no-click or haptic click. |
| Sensor clock | `CONFIG_POWERFINGER_SENSOR_SCLK_PIN` | `4` | Matches `SENSOR_SCLK` in the R30 interface contract. |
| Sensor data | `CONFIG_POWERFINGER_SENSOR_SDIO_PIN` | `5` | Matches PAW3204 bidirectional `SENSOR_SDIO`. |
| Sensor chip select reserve | `CONFIG_POWERFINGER_SENSOR_NCS_PIN` | `6` | PMW3360-only reserve; harmless for PAW3204 but kept aligned with the packet. |
| Primary dome | `CONFIG_POWERFINGER_DOME_PIN` | `8` | GPIO8 remains the click and wake source for this board. |
| Wake mask | `CONFIG_POWERFINGER_WAKE_GPIO_MASK` | `256` | `1 << 8`, matching the dome GPIO. |
| Battery ADC | `CONFIG_POWERFINGER_VBAT_ADC_CHANNEL` | `0` | Binds `VBAT_SENSE` to `ADC1_CH0` / GPIO0. |
| Cell NTC ADC | `CONFIG_POWERFINGER_NTC_ADC_CHANNEL` | `1` | Binds `NTC_SENSE` to `ADC1_CH1` / GPIO1. |
| VBUS detect | `CONFIG_POWERFINGER_VBUS_DETECT_PIN` | `3` | Binds resistor-divided `VBUS_DETECT` to GPIO3. |
| Charge gate | `CONFIG_POWERFINGER_CHARGE_ENABLE_PIN` | `10` | Binds `CHARGE_EN` to GPIO10 through the logic-safe gate driver. |
| Hall rail | `CONFIG_POWERFINGER_HALL_POWER_PIN` | `-1` | R30-OLED has no Hall rail; firmware must not drive GPIO9. |
| Charger status | none | none | `CHRG_STAT` is a pulled-up local status/test pad only, not firmware-consumed behavior. |
| BLE name | `CONFIG_POWERFINGER_DEVICE_NAME` | `PowerFinger R30` | Identifies the active board profile during pairing. |

## Rules

1. A firmware build using this fragment proves Kconfig selection only. It does
   not prove optical focal distance, click displacement, thermal behavior, RF
   margin, or charge safety.
2. Any change to GPIOs, ADC channels, sensor family, click family, or charge
   safety nets must update this file, the fragment, and the R30 KiCad interface
   contract together.
3. Do not claim `CHRG_STAT` firmware behavior until a real Kconfig symbol and
   production consumer exist.
4. If an ADNS-2080-class fallback replaces PAW3204, treat it as a new board
   profile unless it preserves the exact protocol, voltage, pin, and lens
   assumptions.
