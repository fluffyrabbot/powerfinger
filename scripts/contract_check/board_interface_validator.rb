# SPDX-License-Identifier: MIT

class BoardInterfaceValidator < BaseValidator
  PIN_BINDINGS = {
    %w[interfaces optical_sensor_shared_path pins sclk mcu_resource] => "GPIO4",
    %w[interfaces optical_sensor_shared_path pins sclk firmware_symbol] => "CONFIG_POWERFINGER_SENSOR_SCLK_PIN",
    %w[interfaces optical_sensor_shared_path pins sdio_mosi mcu_resource] => "GPIO5",
    %w[interfaces optical_sensor_shared_path pins sdio_mosi firmware_symbol] => "CONFIG_POWERFINGER_SENSOR_SDIO_PIN",
    %w[interfaces optical_sensor_shared_path pins ncs_reserved_for_pmw3360 mcu_resource] => "GPIO6",
    %w[interfaces optical_sensor_shared_path pins ncs_reserved_for_pmw3360 firmware_symbol] => "CONFIG_POWERFINGER_SENSOR_NCS_PIN",
    %w[interfaces optical_sensor_shared_path pins miso_reserved_for_pmw3360 mcu_resource] => "GPIO7",
    %w[interfaces optical_sensor_shared_path pins miso_reserved_for_pmw3360 firmware_symbol] => "CONFIG_POWERFINGER_SENSOR_MISO_PIN",
    %w[interfaces click primary_snap_dome mcu_resource] => "GPIO8",
    %w[interfaces click primary_snap_dome firmware_symbol] => "CONFIG_POWERFINGER_DOME_PIN",
    %w[interfaces battery_and_charge_safety vbat_sense mcu_resource] => "ADC1_CH0/GPIO0",
    %w[interfaces battery_and_charge_safety vbat_sense firmware_symbol] => "CONFIG_POWERFINGER_VBAT_ADC_CHANNEL",
    %w[interfaces battery_and_charge_safety cell_ntc mcu_resource] => "ADC1_CH1/GPIO1",
    %w[interfaces battery_and_charge_safety cell_ntc firmware_symbol] => "CONFIG_POWERFINGER_NTC_ADC_CHANNEL",
    %w[interfaces battery_and_charge_safety vbus_detect mcu_resource] => "GPIO3",
    %w[interfaces battery_and_charge_safety vbus_detect firmware_symbol] => "CONFIG_POWERFINGER_VBUS_DETECT_PIN",
    %w[interfaces battery_and_charge_safety charge_enable mcu_resource] => "GPIO10",
    %w[interfaces battery_and_charge_safety charge_enable firmware_symbol] => "CONFIG_POWERFINGER_CHARGE_ENABLE_PIN",
    %w[interfaces battery_and_charge_safety chrg_stat firmware_symbol] => "none",
  }.freeze

  def validate(board, sdkconfig_values, generator)
    validate_pin_bindings(board)
    validate_sdkconfig_pin_values(board, sdkconfig_values, generator)
    validate_non_production_claims(board)
  end

  private

  def validate_pin_bindings(board)
    PIN_BINDINGS.each do |path, expected|
      actual = call(:fetch, board, path, ContractCheck::BOARD_REL)
      fail!("board #{path.join(".")} expected #{expected}, got #{actual.inspect}") unless actual == expected
    end
  end

  def validate_sdkconfig_pin_values(board, sdkconfig_values, generator)
    PIN_BINDINGS.each do |path, expected|
      next unless path.last == "firmware_symbol"
      next if expected == "none" || !sdkconfig_values.key?(expected)

      resource = call(:fetch, board, path[0...-1] + ["mcu_resource"], ContractCheck::BOARD_REL)
      value = generator.value_for_symbol_resource(expected, resource)
      call(:require_sdkconfig_value, sdkconfig_values, expected, value) if value
    end
  end

  def validate_non_production_claims(board)
    reset_claimed = call(:fetch, board, %w[interfaces local_sensor_bringup_pads sensor_nreset production_behavior_claimed], ContractCheck::BOARD_REL)
    motion_claimed = call(:fetch, board, %w[interfaces local_sensor_bringup_pads sensor_motion_n production_behavior_claimed], ContractCheck::BOARD_REL)
    chrg_stat_claimed = call(:fetch, board, %w[interfaces battery_and_charge_safety chrg_stat production_behavior_claimed], ContractCheck::BOARD_REL)

    fail!("board must not claim PAW3204 reset production behavior") unless reset_claimed == false
    fail!("board must not claim PAW3204 motion-wake production behavior") unless motion_claimed == false
    fail!("board must not claim CHRG_STAT production behavior") unless chrg_stat_claimed == false
  end
end
