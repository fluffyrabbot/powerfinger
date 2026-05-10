#!/usr/bin/env ruby
# SPDX-License-Identifier: MIT
# PowerFinger - Contract drift checker for the active R30 lane.

require "csv"
require "fileutils"
require "optparse"
require "tmpdir"
require "yaml"

class ContractCheck
  EXPECTED_VARIANT_ID = "R30-OLED-NONE-NONE"
  VARIANT_REL = "variants/r30-oled-none-none.yaml"
  BOARD_REL = "hardware/contracts/board.r30-rigid-p0.yaml"
  SDKCONFIG_REL = "firmware/ring/sdkconfig.defaults.r30_oled_none_none"
  INTERFACE_REL = "hardware/ring/R30-OLED-NONE-NONE/kicad/INTERFACE-CONTRACT.md"
  MANIFEST_REL = "hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md"

  attr_reader :repo_root

  def initialize(repo_root)
    @repo_root = File.expand_path(repo_root)
  end

  def run
    check_required_entrypoints

    variant = load_yaml(VARIANT_REL)
    sdkconfig_values = read_sdkconfig(full(SDKCONFIG_REL))
    schema = ContractSchemaValidator.new(self)

    contract_paths = schema.validate_variant(variant)

    board = load_contract(contract_paths["board"])
    sensor = load_contract(contract_paths["sensor"])
    battery = load_contract(contract_paths["battery"])
    enclosure = load_contract(contract_paths["enclosure"])

    schema.validate_contracts(board, sensor, battery, enclosure)
    validate_cross_contracts(variant, board, sensor, battery, enclosure, contract_paths)
    validate_sdkconfig(variant, board, sdkconfig_values)
    validate_packet_text(variant)
    validate_bom(variant)

    puts "ok: variant, hardware contracts, BOM, manifest, interface, and sdkconfig bindings are consistent"
    puts "ok: interface contract still names the active R30 pin and non-claim seams"
  end

  def self.self_test(repo_root)
    SelfTestFixtures.run(repo_root)
  end

  def print_sdkconfig_profile
    check_required_entrypoints

    variant = load_yaml(VARIANT_REL)
    contract_paths = fetch(variant, %w[contracts], VARIANT_REL)
    board = load_contract(contract_paths["board"])

    print generated_sdkconfig_profile(variant, board)
  end

  private

  def check_required_entrypoints
    [VARIANT_REL, BOARD_REL, SDKCONFIG_REL, INTERFACE_REL].each do |path|
      expect_file(path, "required file")
    end
  end

  def validate_cross_contracts(variant, board, sensor, battery, enclosure, contract_paths)
    CrossContractValidator.new(self).validate(variant, board, sensor, battery, enclosure, contract_paths)
  end

  def validate_sdkconfig(variant, board, sdkconfig_values)
    SdkconfigValidator.new(self).validate(variant, board, sdkconfig_values)
  end

  def validate_packet_text(variant)
    PacketTextValidator.new(self).validate(variant)
  end

  def validate_bom(variant)
    ComponentBomValidator.new(self).validate(variant)
  end

  def generated_sdkconfig_profile(variant, board)
    SdkconfigProfileGenerator.new(self).render(variant, board)
  end

  def load_contract(path)
    expect_file(path, "contract")
    YAML.load_file(full(path))
  end

  def load_yaml(path)
    YAML.load_file(full(path))
  end

  def full(path)
    File.join(repo_root, path)
  end

  def expect_file(path, label)
    fail!("#{label} points at missing file: #{path}") unless path.is_a?(String) && File.file?(full(path))
  end

  def expect_dir(path, label)
    fail!("#{label} points at missing directory: #{path}") unless path.is_a?(String) && Dir.exist?(full(path))
  end

  def fetch(hash, path, label)
    path.reduce(hash) do |node, key|
      fail!("#{label}: missing #{path.join(".")}") unless node.is_a?(Hash) && node.key?(key)
      node[key]
    end
  end

  def require_string(hash, key, label)
    value = hash[key]
    fail!("#{label}: #{key} must be a non-empty string") unless value.is_a?(String) && !value.empty?
    value
  end

  def require_number(hash, key, label)
    value = hash[key]
    fail!("#{label}: #{key} must be numeric") unless value.is_a?(Numeric)
    value
  end

  def require_array(hash, key, label, min: 1)
    value = hash[key]
    fail!("#{label}: #{key} must be an array") unless value.is_a?(Array)
    fail!("#{label}: #{key} must contain at least #{min} item(s)") if value.length < min
    value
  end

  def require_range(hash, key, label)
    value = hash[key]
    fail!("#{label}: #{key} must have numeric min/max") unless value.is_a?(Hash) &&
      value["min"].is_a?(Numeric) && value["max"].is_a?(Numeric)
    fail!("#{label}: #{key}.min must be <= max") unless value["min"] <= value["max"]
    value
  end

  def read_sdkconfig(path)
    values = {}
    File.readlines(path, chomp: true).each do |line|
      next if line.empty? || line.start_with?("#")
      key, value = line.split("=", 2)
      values[key] = value if key && value
    end
    values
  end

  def require_sdkconfig_value(values, symbol, expected)
    actual = values[symbol]
    fail!("sdkconfig missing #{symbol}=#{expected}") unless actual == expected
  end

  def fail!(message)
    raise ContractError, message
  end
end

require_relative "contract_check/base_validator"
require_relative "contract_check/contract_schema_validator"
require_relative "contract_check/sdkconfig_profile_generator"
require_relative "contract_check/cross_contract_validator"
require_relative "contract_check/board_interface_validator"
require_relative "contract_check/packet_text_validator"
require_relative "contract_check/sdkconfig_validator"
require_relative "contract_check/component_bom_validator"
require_relative "contract_check/self_test_fixtures"

class ContractError < StandardError; end

options = {
  repo_root: File.expand_path("..", __dir__),
  self_test: false,
  print_sdkconfig_profile: false,
}

OptionParser.new do |parser|
  parser.banner = "Usage: scripts/contract_check.rb [--repo-root PATH] [--self-test] [--print-sdkconfig-profile]"
  parser.on("--repo-root PATH", "Repository root to validate") { |path| options[:repo_root] = path }
  parser.on("--self-test", "Run fixture-based negative tests") { options[:self_test] = true }
  parser.on("--print-sdkconfig-profile", "Print the generated R30 sdkconfig profile") { options[:print_sdkconfig_profile] = true }
end.parse!

begin
  if options[:self_test] && options[:print_sdkconfig_profile]
    raise ContractError, "--self-test and --print-sdkconfig-profile are mutually exclusive"
  elsif options[:self_test]
    ContractCheck.self_test(File.expand_path(options[:repo_root]))
  elsif options[:print_sdkconfig_profile]
    ContractCheck.new(options[:repo_root]).print_sdkconfig_profile
  else
    ContractCheck.new(options[:repo_root]).run
  end
rescue ContractError => e
  warn "error: #{e.message}"
  exit 1
end
