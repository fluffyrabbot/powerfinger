# SPDX-License-Identifier: MIT

class SdkconfigValidator < BaseValidator
  def validate(variant, board, sdkconfig_values)
    generator = SdkconfigProfileGenerator.new(@check)
    generated_profile = generator.render(variant, board)
    checked_in_profile = File.read(call(:full, ContractCheck::SDKCONFIG_REL))
    unless checked_in_profile == generated_profile
      fail!("generated sdkconfig profile drift; regenerate #{ContractCheck::SDKCONFIG_REL} from active variant + board contract")
    end

    generator.values(variant, board).each do |symbol, value|
      call(:require_sdkconfig_value, sdkconfig_values, symbol, value)
    end

    BoardInterfaceValidator.new(@check).validate(board, sdkconfig_values, generator)
  end
end
