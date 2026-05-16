# SPDX-License-Identifier: MIT

class SelfTestFixtures
  class FixtureTree
    PATHS = [
      "variants",
      "hardware/contracts",
      "hardware/bom",
      "hardware/ring/R30-OLED-NONE-NONE/MANIFEST.md",
      "hardware/ring/R30-OLED-NONE-NONE/STACKUP-VERIFY.md",
      "hardware/ring/R30-OLED-NONE-NONE/cad/r30_oled_none_none_shell_blank.scad",
      "hardware/ring/R30-OLED-NONE-NONE/kicad/INTERFACE-CONTRACT.md",
      "hardware/ring/R30-OLED-NONE-NONE/kicad/PLACEMENT-CONSTRAINTS.md",
      "hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch",
      "hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb",
      "hardware/ring/R30-OLED-NONE-NONE/kicad/BOM-BLOCK-MAP.md",
      "docs/R30-OLED-FIRMWARE-CONFIG.md",
      "docs/DRIVER-HARDWARE-CONTRACT.md",
      "firmware/ring/sdkconfig.defaults",
      "firmware/ring/sdkconfig.defaults.r30_oled_none_none",
      "firmware/ring/components/sensors/paw3204.c",
      "firmware/ring/components/sensors/include/sensor_interface.h",
      "firmware/ring/components/click/include/click_interface.h",
    ].freeze

    def initialize(repo_root, paths: PATHS)
      @repo_root = repo_root
      @paths = paths
    end

    def copy_to(tmp)
      @paths.each do |rel|
        src = File.join(@repo_root, rel)
        dst = File.join(tmp, rel)
        FileUtils.mkdir_p(File.dirname(dst))
        FileUtils.cp_r(src, dst)
      end
    end
  end

  Fixture = Struct.new(:name, :expected_error, :mutation, keyword_init: true) do
    def mutate(root)
      mutation.call(root)
    end
  end

  def self.run(repo_root)
    new(repo_root).run
  end

  def initialize(repo_root)
    @fixture_tree = FixtureTree.new(repo_root)
  end

  def run
    Dir.mktmpdir("powerfinger-contract-check-") do |tmp|
      @fixture_tree.copy_to(tmp)
      ContractCheck.new(tmp).run
    end

    fixtures.each do |fixture|
      Dir.mktmpdir("powerfinger-contract-check-") do |tmp|
        @fixture_tree.copy_to(tmp)
        fixture.mutate(tmp)
        begin
          ContractCheck.new(tmp).run
        rescue SystemExit => e
          raise "#{fixture.name}: expected failure, got exit 0" if e.status == 0
        rescue ContractError => e
          unless e.message.include?(fixture.expected_error)
            raise "#{fixture.name}: expected #{fixture.expected_error.inspect}, got #{e.message.inspect}"
          end
          puts "ok: self-test rejected #{fixture.name}"
          next
        end
        raise "#{fixture.name}: expected failure, checker passed"
      end
    end

    puts "ok: contract checker self-test fixtures passed"
  end

  private

  def fixtures
    [
      fixture(
        "missing contract reference",
        "variant contracts.sensor points at missing file",
        lambda do |root|
          mutate_yaml(root, ContractCheck::VARIANT_REL) do |data|
            data["contracts"]["sensor"] = "hardware/contracts/missing-sensor.yaml"
          end
        end,
      ),
      fixture(
        "pin drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["interfaces"]["optical_sensor_shared_path"]["pins"]["sclk"]["mcu_resource"] = "GPIO14"
          end
        end,
      ),
      fixture(
        "ADC channel drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["interfaces"]["battery_and_charge_safety"]["vbat_sense"]["mcu_resource"] = "ADC1_CH2/GPIO0"
          end
        end,
      ),
      fixture(
        "firmware symbol drift",
        "firmware_symbol expected CONFIG_POWERFINGER_DOME_PIN",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["interfaces"]["click"]["primary_snap_dome"]["firmware_symbol"] = "CONFIG_POWERFINGER_ALT_DOME_PIN"
          end
        end,
      ),
      fixture(
        "contract identity drift",
        "board id/path mismatch",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["id"] = "board.r30-rigid-p1"
          end
        end,
      ),
      fixture(
        "BOM target drift",
        "BOM target row must include ~$9",
        lambda do |root|
          mutate_text(root, "hardware/bom/R30-OLED-NONE-NONE.csv") do |text|
            text.sub("~$9\",", "~$10\",")
          end
        end,
      ),
      fixture(
        "BOM anchor drift",
        "BOM U2 field Value expected PAW3204DB-TJ3L",
        lambda do |root|
          mutate_text(root, "hardware/bom/R30-OLED-NONE-NONE.csv") do |text|
            text.sub("PAW3204DB-TJ3L,8-pin optical", "PAW3204DB-ALT,8-pin optical")
          end
        end,
      ),
      fixture(
        "inline BOM anchor drift",
        "variant BOM anchors must live in component_contract, not inline",
        lambda do |root|
          mutate_yaml(root, ContractCheck::VARIANT_REL) do |data|
            data["bom"]["active_component_anchors"] = {
              "U2" => { "function" => "motion_sensor" },
            }
          end
        end,
      ),
      fixture(
        "substitution viability drift",
        "BOM U2 notes expected rejected substitute YS8205",
        lambda do |root|
          mutate_text(root, "hardware/bom/R30-OLED-NONE-NONE.csv") do |text|
            text.sub("YS8205 is NOT a standalone sensor", "YS8205 maybe usable")
          end
        end,
      ),
      fixture(
        "charge substitution drift",
        "BOM U3 notes expected rejected substitute TP4056",
        lambda do |root|
          mutate_text(root, "hardware/bom/R30-OLED-NONE-NONE.csv") do |text|
            text.sub("TP4056 requires SOP-8 footprint (NOT drop-in)", "TP4056 is a production alternate")
          end
        end,
      ),
      fixture(
        "false firmware behavior claim",
        "paw3204_reset_firmware_consumer non-claim must be false",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["non_claims"]["paw3204_reset_firmware_consumer"] = true
          end
        end,
      ),
      fixture(
        "CHRG_STAT GPIO allocation drift",
        "CHRG_STAT must not have MCU resource",
        lambda do |root|
          mutate_yaml(root, ContractCheck::BOARD_REL) do |data|
            data["interfaces"]["battery_and_charge_safety"]["chrg_stat"]["mcu_resource"] = "GPIO2"
          end
        end,
      ),
      fixture(
        "envelope mismatch",
        "variant max height must match enclosure acceptance",
        lambda do |root|
          mutate_yaml(root, ContractCheck::VARIANT_REL) do |data|
            data["physical"]["finger_to_surface_height_mm_target_max"] = 11
          end
        end,
      ),
      fixture(
        "generated sdkconfig drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          mutate_text(root, ContractCheck::SDKCONFIG_REL) do |text|
            text.sub("CONFIG_POWERFINGER_DOME_PIN=8", "CONFIG_POWERFINGER_DOME_PIN=9")
          end
        end,
      ),
    ]
  end

  def fixture(name, expected_error, mutation)
    Fixture.new(name: name, expected_error: expected_error, mutation: mutation)
  end

  def mutate_yaml(root, rel, &block)
    path = File.join(root, rel)
    data = YAML.load_file(path)
    block.call(data)
    File.write(path, data.to_yaml)
  end

  def mutate_text(root, rel, &block)
    path = File.join(root, rel)
    File.write(path, block.call(File.read(path)))
  end
end
