# SPDX-License-Identifier: MIT

class SdkconfigProfileGenerator
  def initialize(check)
    @check = check
  end

  def values(variant, board)
    {
      selected_motion_sensor_symbol(variant) => "y",
      selected_click_symbol(variant) => "y",
      "CONFIG_POWERFINGER_SENSOR_SCLK_PIN" => gpio_number(fetch(board, %w[interfaces optical_sensor_shared_path pins sclk mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_SENSOR_SDIO_PIN" => gpio_number(fetch(board, %w[interfaces optical_sensor_shared_path pins sdio_mosi mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_SENSOR_NCS_PIN" => gpio_number(fetch(board, %w[interfaces optical_sensor_shared_path pins ncs_reserved_for_pmw3360 mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_DOME_PIN" => gpio_number(fetch(board, %w[interfaces click primary_snap_dome mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_WAKE_GPIO_MASK" => wake_mask_for_gpio(fetch(board, %w[interfaces click primary_snap_dome mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_VBAT_ADC_CHANNEL" => adc_channel(fetch(board, %w[interfaces battery_and_charge_safety vbat_sense mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_NTC_ADC_CHANNEL" => adc_channel(fetch(board, %w[interfaces battery_and_charge_safety cell_ntc mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_VBUS_DETECT_PIN" => gpio_number(fetch(board, %w[interfaces battery_and_charge_safety vbus_detect mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_CHARGE_ENABLE_PIN" => gpio_number(fetch(board, %w[interfaces battery_and_charge_safety charge_enable mcu_resource], ContractCheck::BOARD_REL)),
      "CONFIG_POWERFINGER_HALL_POWER_PIN" => "-1",
    }
  end

  def value_for_symbol_resource(symbol, resource)
    case symbol
    when /_ADC_CHANNEL\z/
      adc_channel(resource)
    else
      gpio_number(resource)
    end
  end

  def render(variant, board)
    profile_values = values(variant, board)
    lines = [
      "# SPDX-License-Identifier: MIT",
      "# PowerFinger R30-OLED-NONE-NONE ring board profile.",
      "#",
      "# Layer this after sdkconfig.defaults when building firmware for the active",
      "# optical ring board. The generic sdkconfig.defaults remains the dev-board /",
      "# Phase 0 baseline; this fragment binds the first-board hardware contract.",
      "",
      "# Active optical sensor and click path.",
      "#{selected_motion_sensor_symbol(variant)}=y",
      "# CONFIG_SENSOR_NONE is not set",
      "# CONFIG_SENSOR_PMW3360 is not set",
      "# CONFIG_SENSOR_BALL_HALL is not set",
      "#{selected_click_symbol(variant)}=y",
      "# CONFIG_CLICK_NONE is not set",
      "# CONFIG_CLICK_PIEZO_LRA is not set",
      "",
      "# R30 optical sensor and dome pin map.",
      "CONFIG_POWERFINGER_SENSOR_SCLK_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_SENSOR_SCLK_PIN")}",
      "CONFIG_POWERFINGER_SENSOR_SDIO_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_SENSOR_SDIO_PIN")}",
      "CONFIG_POWERFINGER_SENSOR_NCS_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_SENSOR_NCS_PIN")}",
      "CONFIG_POWERFINGER_DOME_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_DOME_PIN")}",
      "CONFIG_POWERFINGER_WAKE_GPIO_MASK=#{profile_values.fetch("CONFIG_POWERFINGER_WAKE_GPIO_MASK")}",
      "",
      "# Active-board battery and charge-safety path.",
      "CONFIG_POWERFINGER_VBAT_ADC_CHANNEL=#{profile_values.fetch("CONFIG_POWERFINGER_VBAT_ADC_CHANNEL")}",
      "CONFIG_POWERFINGER_NTC_ADC_CHANNEL=#{profile_values.fetch("CONFIG_POWERFINGER_NTC_ADC_CHANNEL")}",
      "CONFIG_POWERFINGER_VBUS_DETECT_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_VBUS_DETECT_PIN")}",
      "CONFIG_POWERFINGER_CHARGE_ENABLE_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_CHARGE_ENABLE_PIN")}",
      "",
      "# R30-OLED has no Hall rail. Keep the ring runtime from driving GPIO9.",
      "CONFIG_POWERFINGER_HALL_POWER_PIN=#{profile_values.fetch("CONFIG_POWERFINGER_HALL_POWER_PIN")}",
      "",
      "CONFIG_POWERFINGER_DEVICE_NAME=\"PowerFinger R30\"",
      "",
    ]
    lines.join("\n")
  end

  private

  def call(method_name, *args)
    @check.send(method_name, *args)
  end

  def fetch(hash, path, label)
    call(:fetch, hash, path, label)
  end

  def selected_motion_sensor_symbol(variant)
    symbol = fetch(variant, %w[firmware selected_drivers motion_sensor], ContractCheck::VARIANT_REL)
    call(:fail!, "variant selected motion sensor must be SENSOR_PAW3204") unless symbol == "SENSOR_PAW3204"
    "CONFIG_#{symbol}"
  end

  def selected_click_symbol(variant)
    symbol = fetch(variant, %w[firmware selected_drivers click_input], ContractCheck::VARIANT_REL)
    call(:fail!, "variant selected click input must be CLICK_SNAP_DOME") unless symbol == "CLICK_SNAP_DOME"
    "CONFIG_#{symbol}"
  end

  def adc_channel(resource)
    channel = resource.to_s[/\bADC1_CH(\d+)\b/, 1]
    call(:fail!, "could not derive ADC channel from #{resource.inspect}") unless channel
    channel
  end

  def wake_mask_for_gpio(resource)
    gpio = gpio_number(resource)
    call(:fail!, "could not derive wake GPIO from #{resource.inspect}") unless gpio
    (1 << gpio.to_i).to_s
  end

  def gpio_number(resource)
    resource.to_s[/\bGPIO(\d+)\b/, 1]
  end
end
