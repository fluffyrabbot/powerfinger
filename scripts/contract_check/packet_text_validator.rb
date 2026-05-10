# SPDX-License-Identifier: MIT

class PacketTextValidator < BaseValidator
  def validate(variant)
    manifest = File.read(call(:full, ContractCheck::MANIFEST_REL))
    interface = File.read(call(:full, ContractCheck::INTERFACE_REL))
    target_cost = call(:fetch, variant, %w[bom target_usd_prototype], ContractCheck::VARIANT_REL)

    fail!("manifest must name active variant ID") unless manifest.include?("Variant ID: `#{ContractCheck::EXPECTED_VARIANT_ID}`")
    fail!("manifest must name active BOM target") unless manifest.include?("BOM target: `~$#{target_cost}`")

    %w[
      GPIO4
      GPIO5
      GPIO6
      GPIO7
      GPIO8
      GPIO10
      CONFIG_POWERFINGER_SENSOR_SCLK_PIN
      CONFIG_POWERFINGER_SENSOR_SDIO_PIN
      CONFIG_POWERFINGER_DOME_PIN
      CONFIG_POWERFINGER_CHARGE_ENABLE_PIN
    ].each do |needle|
      fail!("interface contract expected to find '#{needle}' in #{ContractCheck::INTERFACE_REL}") unless interface.include?(needle)
    end
  end

end
