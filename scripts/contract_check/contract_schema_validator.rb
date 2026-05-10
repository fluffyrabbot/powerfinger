# SPDX-License-Identifier: MIT

class ContractSchemaValidator < BaseValidator
  def validate_variant(variant)
    fail!("variant_id must be #{ContractCheck::EXPECTED_VARIANT_ID}") unless variant["variant_id"] == ContractCheck::EXPECTED_VARIANT_ID
    fail!("variant id must be variant.r30-oled-none-none") unless variant["id"] == "variant.r30-oled-none-none"
    fail!("variant status must stay active_validation_lane") unless variant["status"] == "active_validation_lane"
    fail!("variant form_factor must be ring") unless variant["form_factor"] == "ring"

    contract_paths = call(:fetch, variant, %w[contracts], ContractCheck::VARIANT_REL)
    %w[board sensor battery enclosure].each do |key|
      path = contract_paths[key]
      fail!("variant contracts.#{key} is missing") if path.nil? || path.empty?
      call(:expect_file, path, "variant contracts.#{key}")
    end

    call(:fetch, variant, %w[firmware sdkconfig_fragments], ContractCheck::VARIANT_REL).each do |path|
      call(:expect_file, path, "variant firmware sdkconfig fragment")
    end

    selected = call(:fetch, variant, %w[firmware selected_drivers], ContractCheck::VARIANT_REL)
    fail!("variant selected motion sensor must be SENSOR_PAW3204") unless selected["motion_sensor"] == "SENSOR_PAW3204"
    fail!("variant selected click input must be CLICK_SNAP_DOME") unless selected["click_input"] == "CLICK_SNAP_DOME"
    call(:expect_dir, call(:fetch, variant, %w[firmware target], ContractCheck::VARIANT_REL), "variant firmware target")
    call(:expect_file, call(:fetch, variant, %w[bom source], ContractCheck::VARIANT_REL), "variant BOM source")
    call(:expect_file, call(:fetch, variant, %w[bom component_contract], ContractCheck::VARIANT_REL), "variant BOM component contract")
    if call(:fetch, variant, %w[bom], ContractCheck::VARIANT_REL).key?("active_component_anchors")
      fail!("variant BOM anchors must live in component_contract, not inline")
    end
    call(:require_number, call(:fetch, variant, %w[bom], ContractCheck::VARIANT_REL), "target_usd_prototype", ContractCheck::VARIANT_REL)
    call(:require_array, variant, "known_limits", ContractCheck::VARIANT_REL)
    variant.fetch("evidence_sources", []).each { |path| call(:expect_file, path, "variant evidence source") }

    contract_paths
  end

  def validate_contracts(board, sensor, battery, enclosure)
    fail!("board variant must be #{ContractCheck::EXPECTED_VARIANT_ID}") unless board["variant"] == ContractCheck::EXPECTED_VARIANT_ID

    {
      "board" => board,
      "sensor" => sensor,
      "battery" => battery,
      "enclosure" => enclosure,
    }.each do |label, contract|
      call(:require_string, contract, "id", "#{label} contract")
      call(:require_string, contract, "status", "#{label} contract")
      call(:require_string, contract, "class", "#{label} contract")
    end

    call(:require_array, call(:fetch, board, %w[verification], "board contract"), "release_blockers", "board verification")
    call(:require_array, sensor, "known_limits", "sensor contract")
    call(:require_array, battery, "known_limits", "battery contract")
    call(:require_array, enclosure, "known_limits", "enclosure contract")

    fail!("board id/path mismatch") unless board["id"] == "board.r30-rigid-p0"
    fail!("sensor id/path mismatch") unless sensor["id"] == "sensor.paw3204"
    fail!("battery id/path mismatch") unless battery["id"] == "battery.lipo-protected-ring-100mah"
    fail!("enclosure id/path mismatch") unless enclosure["id"] == "enclosure.r30-serviceable-shell-v1"
  end
end
