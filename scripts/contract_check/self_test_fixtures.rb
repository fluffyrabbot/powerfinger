# SPDX-License-Identifier: MIT

class SelfTestFixtures
  def self.run(repo_root)
    new(repo_root).run
  end

  def initialize(repo_root)
    @repo_root = repo_root
  end

  def run
    Dir.mktmpdir("powerfinger-contract-check-") do |tmp|
      copy_fixture_tree(tmp)
      ContractCheck.new(tmp).run
    end

    fixtures.each do |name, expected_error, mutate|
      Dir.mktmpdir("powerfinger-contract-check-") do |tmp|
        copy_fixture_tree(tmp)
        mutate.call(tmp)
        begin
          ContractCheck.new(tmp).run
        rescue SystemExit => e
          raise "#{name}: expected failure, got exit 0" if e.status == 0
        rescue ContractError => e
          unless e.message.include?(expected_error)
            raise "#{name}: expected #{expected_error.inspect}, got #{e.message.inspect}"
          end
          puts "ok: self-test rejected #{name}"
          next
        end
        raise "#{name}: expected failure, checker passed"
      end
    end

    puts "ok: contract checker self-test fixtures passed"
  end

  private

  def fixtures
    [
      [
        "missing contract reference",
        "variant contracts.sensor points at missing file",
        lambda do |root|
          path = File.join(root, ContractCheck::VARIANT_REL)
          data = YAML.load_file(path)
          data["contracts"]["sensor"] = "hardware/contracts/missing-sensor.yaml"
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "pin drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          path = File.join(root, ContractCheck::BOARD_REL)
          data = YAML.load_file(path)
          data["interfaces"]["optical_sensor_shared_path"]["pins"]["sclk"]["mcu_resource"] = "GPIO14"
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "ADC channel drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          path = File.join(root, ContractCheck::BOARD_REL)
          data = YAML.load_file(path)
          data["interfaces"]["battery_and_charge_safety"]["vbat_sense"]["mcu_resource"] = "ADC1_CH2/GPIO0"
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "firmware symbol drift",
        "firmware_symbol expected CONFIG_POWERFINGER_DOME_PIN",
        lambda do |root|
          path = File.join(root, ContractCheck::BOARD_REL)
          data = YAML.load_file(path)
          data["interfaces"]["click"]["primary_snap_dome"]["firmware_symbol"] = "CONFIG_POWERFINGER_ALT_DOME_PIN"
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "contract identity drift",
        "board id/path mismatch",
        lambda do |root|
          path = File.join(root, ContractCheck::BOARD_REL)
          data = YAML.load_file(path)
          data["id"] = "board.r30-rigid-p1"
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "BOM target drift",
        "BOM target row must include ~$9",
        lambda do |root|
          path = File.join(root, "hardware/bom/R30-OLED-NONE-NONE.csv")
          text = File.read(path).sub("~$9\",", "~$10\",")
          File.write(path, text)
        end,
      ],
      [
        "BOM anchor drift",
        "BOM U2 field Value expected PAW3204DB-TJ3L",
        lambda do |root|
          path = File.join(root, "hardware/bom/R30-OLED-NONE-NONE.csv")
          text = File.read(path).sub("PAW3204DB-TJ3L,8-pin optical", "PAW3204DB-ALT,8-pin optical")
          File.write(path, text)
        end,
      ],
      [
        "inline BOM anchor drift",
        "variant BOM anchors must live in component_contract, not inline",
        lambda do |root|
          path = File.join(root, ContractCheck::VARIANT_REL)
          data = YAML.load_file(path)
          data["bom"]["active_component_anchors"] = {
            "U2" => { "function" => "motion_sensor" },
          }
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "substitution viability drift",
        "BOM U2 notes expected rejected substitute YS8205",
        lambda do |root|
          path = File.join(root, "hardware/bom/R30-OLED-NONE-NONE.csv")
          text = File.read(path).sub("YS8205 is NOT a standalone sensor", "YS8205 maybe usable")
          File.write(path, text)
        end,
      ],
      [
        "charge substitution drift",
        "BOM U3 notes expected rejected substitute TP4056",
        lambda do |root|
          path = File.join(root, "hardware/bom/R30-OLED-NONE-NONE.csv")
          text = File.read(path).sub("TP4056 requires SOP-8 footprint (NOT drop-in)", "TP4056 is a production alternate")
          File.write(path, text)
        end,
      ],
      [
        "false firmware behavior claim",
        "paw3204_reset_firmware_consumer non-claim must be false",
        lambda do |root|
          path = File.join(root, ContractCheck::BOARD_REL)
          data = YAML.load_file(path)
          data["non_claims"]["paw3204_reset_firmware_consumer"] = true
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "envelope mismatch",
        "variant max height must match enclosure acceptance",
        lambda do |root|
          path = File.join(root, ContractCheck::VARIANT_REL)
          data = YAML.load_file(path)
          data["physical"]["finger_to_surface_height_mm_target_max"] = 11
          File.write(path, data.to_yaml)
        end,
      ],
      [
        "generated sdkconfig drift",
        "generated sdkconfig profile drift",
        lambda do |root|
          path = File.join(root, ContractCheck::SDKCONFIG_REL)
          text = File.read(path).sub("CONFIG_POWERFINGER_DOME_PIN=8", "CONFIG_POWERFINGER_DOME_PIN=9")
          File.write(path, text)
        end,
      ],
    ]
  end

  def copy_fixture_tree(tmp)
    paths = [
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
    ]

    paths.each do |rel|
      src = File.join(@repo_root, rel)
      dst = File.join(tmp, rel)
      FileUtils.mkdir_p(File.dirname(dst))
      FileUtils.cp_r(src, dst)
    end
  end
end
