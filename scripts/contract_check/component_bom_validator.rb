# SPDX-License-Identifier: MIT

class ComponentBomValidator < BaseValidator
  def validate(variant)
    bom_path = call(:fetch, variant, %w[bom source], ContractCheck::VARIANT_REL)
    component_contract_path = call(:fetch, variant, %w[bom component_contract], ContractCheck::VARIANT_REL)
    component_contract = call(:load_yaml, component_contract_path)

    fail!("component contract variant must match active variant") unless component_contract["variant"] == ContractCheck::EXPECTED_VARIANT_ID
    fail!("component contract source_bom must match variant BOM source") unless component_contract["source_bom"] == bom_path

    bom = CSV.read(call(:full, bom_path), headers: true)
    bom_values = bom.each_with_object({}) do |row, values|
      ref = row["Ref"]
      values[ref] = row if ref && !ref.empty?
    end

    anchors = call(:fetch, component_contract, %w[active_component_anchors], component_contract_path)
    anchors.each do |ref, contract|
      row = bom_values[ref]
      fail!("BOM missing #{ref}") unless row
      validate_anchor(ref, contract, row)
    end

    validate_target_cost(variant, bom)
  end

  private

  def validate_anchor(ref, contract, row)
    {
      "description" => "Description",
      "value" => "Value",
      "package" => "Package",
    }.each do |contract_key, csv_key|
      next unless contract.key?(contract_key)

      expected = contract[contract_key].to_s
      actual = row[csv_key].to_s
      fail!("BOM #{ref} field #{csv_key} expected #{expected}") unless actual == expected
    end

    call(:require_string, contract, "function", "BOM #{ref} anchor")
    contract.fetch("required_notes", []).each do |needle|
      fail!("BOM #{ref} notes expected #{needle}") unless row["Notes"].to_s.include?(needle)
    end
    validate_substitutions(ref, contract, row)
  end

  def validate_substitutions(ref, contract, row)
    substitutions = contract.fetch("substitutions", {})
    notes = row["Notes"].to_s

    substitutions.fetch("viable", []).each do |substitution|
      id = call(:require_string, substitution, "id", "BOM #{ref} viable substitution")
      call(:require_array, substitution, "acceptance", "BOM #{ref} viable substitution")
      fail!("BOM #{ref} notes expected viable substitute #{id}") unless notes.include?(id)
    end

    substitutions.fetch("rejected", []).each do |substitution|
      id = call(:require_string, substitution, "id", "BOM #{ref} rejected substitution")
      required_note = call(:require_string, substitution, "required_note", "BOM #{ref} rejected substitution")
      call(:require_string, substitution, "reason", "BOM #{ref} rejected substitution")
      fail!("BOM #{ref} notes expected rejected substitute #{id}") unless notes.include?(id)
      fail!("BOM #{ref} notes expected rejected substitute #{id}") unless notes.include?(required_note)
    end
  end

  def validate_target_cost(variant, bom)
    target_cost = call(:fetch, variant, %w[bom target_usd_prototype], ContractCheck::VARIANT_REL)
    total_rows = bom.select { |row| row["Ref"].to_s.empty? && row["Description"].to_s.include?("TARGET BOM") }
    fail!("BOM missing TARGET BOM row") if total_rows.empty?
    unless total_rows.any? { |row| row["Unit Cost (1-10)"].to_s.include?("~$#{target_cost}") }
      fail!("BOM target row must include ~$#{target_cost}")
    end
  end

end
