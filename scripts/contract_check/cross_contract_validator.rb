# SPDX-License-Identifier: MIT

class CrossContractValidator < BaseValidator
  def validate(variant, board, sensor, battery, enclosure, contract_paths)
    validate_identity(board, sensor, enclosure)
    validate_board(board)
    validate_sensor(sensor)
    validate_battery(battery)
    validate_enclosure(enclosure)
    validate_compatibility(variant, board, sensor, enclosure, contract_paths)
    validate_non_claims(variant, board)
  end

  private

  def validate_identity(board, sensor, enclosure)
    fail!("sensor primary_variant must match active variant") unless sensor["primary_variant"] == ContractCheck::EXPECTED_VARIANT_ID
    fail!("enclosure variant must match active variant") unless enclosure["variant"] == ContractCheck::EXPECTED_VARIANT_ID
    fail!("board source_packet must match active packet") unless board["source_packet"] == "hardware/ring/R30-OLED-NONE-NONE"
  end

  def validate_board(board)
    %w[source_packet schematic_source pcb_source].each do |key|
      path = board[key]
      key == "source_packet" ? call(:expect_dir, path, "board #{key}") : call(:expect_file, path, "board #{key}")
    end
    call(:expect_file, call(:fetch, board, %w[mcu antenna_keepout packet_reference], "board contract"), "board antenna keepout reference")
    call(:expect_dir, call(:fetch, board, %w[mcu firmware_target], "board contract"), "board firmware target")
  end

  def validate_sensor(sensor)
    call(:expect_file, call(:fetch, sensor, %w[firmware driver], "sensor contract"), "sensor firmware driver")
    call(:expect_file, call(:fetch, sensor, %w[firmware public_api], "sensor contract"), "sensor public API")
    unless call(:fetch, sensor, %w[firmware kconfig_symbol], "sensor contract") == "CONFIG_SENSOR_PAW3204"
      fail!("sensor kconfig symbol must match selected driver")
    end
    call(:require_range, call(:fetch, sensor, %w[mechanical], "sensor contract"), "optical_gap_mm", "sensor mechanical")
    call(:require_array, call(:fetch, sensor, %w[surface_scope], "sensor contract"), "claimed", "sensor surface_scope")
    call(:require_array, call(:fetch, sensor, %w[surface_scope], "sensor contract"), "not_claimed", "sensor surface_scope")
  end

  def validate_battery(battery)
    call(:expect_file, call(:fetch, battery, %w[acceptance fit_evidence_required], "battery contract"), "battery fit evidence")
    call(:fetch, battery, %w[variant_bindings r30_oled_none_none], "battery contract").each do |key, value|
      call(:expect_file, value, "battery variant binding #{key}") if %w[cad_keepout_source bom_source].include?(key)
    end
    fail!("battery must require protected cells") unless call(:fetch, battery, %w[electrical integrated_pcm_required], "battery contract") == true
    fail!("battery must reject soldered cells") unless call(:fetch, battery, %w[connector soldered_cell_directly_to_board_allowed], "battery contract") == false
    fail!("battery must reject destructive shell entry") unless call(:fetch, battery, %w[mechanical destructive_shell_entry_allowed], "battery contract") == false
  end

  def validate_enclosure(enclosure)
    call(:expect_file, enclosure["source"], "enclosure source")
    %w[board battery sensor].each do |key|
      call(:expect_file, call(:fetch, enclosure, %w[interfaces] + [key, "contract"], "enclosure contract"), "enclosure #{key} contract")
    end
    call(:expect_file, call(:fetch, enclosure, %w[fit_coupons evidence_target], "enclosure contract"), "enclosure fit coupon evidence")
    call(:require_range, call(:fetch, enclosure, %w[acceptance], "enclosure contract"), "optical_gap_mm", "enclosure acceptance")
    unless call(:fetch, enclosure, %w[acceptance shell_reopenable_after_full_assembly], "enclosure contract") == "required"
      fail!("enclosure must require reopenable shell")
    end
  end

  def validate_compatibility(variant, board, sensor, enclosure, contract_paths)
    fail!("variant board contract path mismatch") unless contract_paths["board"] == call(:fetch, enclosure, %w[interfaces board contract], "enclosure contract")
    fail!("variant sensor contract path mismatch") unless contract_paths["sensor"] == call(:fetch, enclosure, %w[interfaces sensor contract], "enclosure contract")
    fail!("variant battery contract path mismatch") unless contract_paths["battery"] == call(:fetch, enclosure, %w[interfaces battery contract], "enclosure contract")
    fail!("board compatible sensor missing active sensor") unless call(:fetch, board, %w[compatible_contracts sensors], "board contract").include?(contract_paths["sensor"])
    fail!("board compatible battery missing active battery") unless call(:fetch, board, %w[compatible_contracts batteries], "board contract").include?(contract_paths["battery"])

    fail!("variant physical pcb_size_mm must match board physical board_size_mm") unless call(:fetch, variant, %w[physical pcb_size_mm], ContractCheck::VARIANT_REL) == call(:fetch, board, %w[physical board_size_mm], "board contract")
    fail!("variant optical gap must match sensor contract") unless call(:fetch, variant, %w[physical optical_gap_mm], ContractCheck::VARIANT_REL) == call(:fetch, sensor, %w[mechanical optical_gap_mm], "sensor contract")
    fail!("variant optical gap must match enclosure acceptance") unless call(:fetch, variant, %w[physical optical_gap_mm], ContractCheck::VARIANT_REL) == call(:fetch, enclosure, %w[acceptance optical_gap_mm], "enclosure contract")
    fail!("variant max height must match enclosure acceptance") unless call(:fetch, variant, %w[physical finger_to_surface_height_mm_target_max], ContractCheck::VARIANT_REL) == call(:fetch, enclosure, %w[acceptance finger_to_surface_height_mm_max], "enclosure contract")
    fail!("variant sensor angle must match enclosure default") unless call(:fetch, variant, %w[physical sensor_angle_deg], ContractCheck::VARIANT_REL) == call(:fetch, enclosure, %w[parameters sensor_angle_deg default], "enclosure contract")
    fail!("variant finger circumference must match enclosure default") unless call(:fetch, variant, %w[physical finger_circumference_mm_default], ContractCheck::VARIANT_REL) == call(:fetch, enclosure, %w[parameters finger_circumference_mm default], "enclosure contract")
  end

  def validate_non_claims(variant, board)
    variant_non_claims = call(:fetch, variant, %w[non_claims], ContractCheck::VARIANT_REL)
    board_non_claims = call(:fetch, board, %w[non_claims], ContractCheck::BOARD_REL)

    [
      ["glass_surface_support", variant_non_claims],
      ["chrg_stat_firmware_consumer", variant_non_claims],
      ["paw3204_reset_firmware_consumer", variant_non_claims],
      ["paw3204_motion_wake_firmware_consumer", variant_non_claims],
      ["imu_haptic_or_premium_gesture_silicon_present", variant_non_claims],
      ["chrg_stat_firmware_consumer", board_non_claims],
      ["paw3204_reset_firmware_consumer", board_non_claims],
      ["paw3204_motion_wake_firmware_consumer", board_non_claims],
    ].each do |key, source|
      fail!("#{key} non-claim must be false") unless source[key] == false
    end

    chrg_stat = call(:fetch, board, %w[interfaces battery_and_charge_safety chrg_stat], ContractCheck::BOARD_REL)
    fail!("CHRG_STAT must stay local test only") unless chrg_stat["local_test_only"] == true
    fail!("CHRG_STAT must not have MCU resource") unless chrg_stat["mcu_resource"] == "none"
    fail!("CHRG_STAT must not have firmware symbol") unless chrg_stat["firmware_symbol"] == "none"
    fail!("variant/board CHRG_STAT non-claim mismatch") unless variant_non_claims["chrg_stat_firmware_consumer"] == board_non_claims["chrg_stat_firmware_consumer"]
    fail!("variant/board PAW3204 reset non-claim mismatch") unless variant_non_claims["paw3204_reset_firmware_consumer"] == board_non_claims["paw3204_reset_firmware_consumer"]
    fail!("variant/board PAW3204 motion non-claim mismatch") unless variant_non_claims["paw3204_motion_wake_firmware_consumer"] == board_non_claims["paw3204_motion_wake_firmware_consumer"]
  end

end
