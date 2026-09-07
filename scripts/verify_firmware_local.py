#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run PowerFinger's selected local checks and retain reviewable evidence."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

# Bound tools that can wait on I/O and host tests that should finish promptly.
COMMAND_TIMEOUT = 900
TEST_TIMEOUT = 60
PROBE_TIMEOUT = 30
PROBE_OUTPUT_LIMIT = 65536
PACKETS = {
    "R30-OLED-NONE-NONE": (
        "hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_sch",
        "hardware/ring/R30-OLED-NONE-NONE/kicad/r30_oled_none_none.kicad_pcb",
    ),
    "USB-HUB": (
        "hardware/shared/USB-HUB/kicad/usb_hub.kicad_sch",
        "hardware/shared/USB-HUB/kicad/usb_hub.kicad_pcb",
    ),
}
CONFIGURATIONS = {
    "ring": ("ring", "r30-oled-none-none", "esp32c3"),
    "ring-generic": ("ring", "ring-generic", "esp32c3"),
    "pen": ("pen", "pen", "esp32c3"),
    "puck": ("puck", "puck", "esp32c3"),
    "hub": ("hub", "hub", "esp32s3"),
}
R30_EXPECTED = {
    "CONFIG_SENSOR_PAW3204": "y",
    "CONFIG_CLICK_SNAP_DOME": "y",
    "CONFIG_POWERFINGER_SENSOR_SCLK_PIN": "4",
    "CONFIG_POWERFINGER_SENSOR_SDIO_PIN": "5",
    "CONFIG_POWERFINGER_DOME_PIN": "8",
    "CONFIG_POWERFINGER_WAKE_GPIO_MASK": "256",
    "CONFIG_POWERFINGER_VBAT_ADC_CHANNEL": "0",
    "CONFIG_POWERFINGER_NTC_ADC_CHANNEL": "1",
    "CONFIG_POWERFINGER_VBUS_DETECT_PIN": "3",
    "CONFIG_POWERFINGER_CHARGE_ENABLE_PIN": "-1",
    "CONFIG_POWERFINGER_HALL_POWER_PIN": "-1",
}


def execute(command, output, timeout, cwd=None, env=None):
    """A timed-out build must not leave compiler children using its files."""
    process = subprocess.Popen(
        [str(item) for item in command], cwd=cwd, env=env,
        stdout=output, stderr=subprocess.STDOUT, start_new_session=True,
    )
    try:
        return process.wait(timeout=timeout)
    except BaseException:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass  # The process group already exited.
        finally:
            process.wait()
        raise


def probe(command, cwd=None, env=None):
    """Bound metadata in memory even if a malfunctioning tool prints a lot."""
    with tempfile.TemporaryFile() as output:
        code = execute(command, output, PROBE_TIMEOUT, cwd=cwd, env=env)
        output.seek(0)
        raw = output.read(PROBE_OUTPUT_LIMIT + 1)
    if len(raw) > PROBE_OUTPUT_LIMIT:
        raise ValueError(f"metadata output exceeds limit: {command[0]}")
    value = raw.decode("utf-8", errors="replace").strip()
    if code:
        raise ValueError(f"{command[0]} exited {code}: {value}")
    return value


def digest(path):
    result = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(PROBE_OUTPUT_LIMIT), b""):
            result.update(block)
    return result.hexdigest()


class Runner:
    def __init__(self, repo, report_dir):
        self.repo = Path(repo)
        self.report_dir = Path(report_dir)
        self.logs = self.report_dir / "logs"
        self.logs.mkdir(parents=True, exist_ok=True)
        self.stages = []

    def note(self, name, status, reason, **details):
        self.stages.append({"name": name, "status": status, "reason": reason, **details})
        print(f"[{status.upper():11}] {name}: {reason}", flush=True)
        return status == "passed"

    def stage(self, name, command, env=None, timeout=COMMAND_TIMEOUT, cwd=None):
        log = self.logs / (name.replace("/", "_") + ".log")
        code = None
        started = time.monotonic()
        try:
            with log.open("w", encoding="utf-8") as output:
                output.write(json.dumps([str(item) for item in command]) + "\n")
                output.flush()
                code = execute(command, output, timeout, cwd or self.repo, env)
            status = "passed" if code == 0 else "failed"
            reason = "completed" if code == 0 else f"exit status {code}"
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            status = "failed"
            reason = f"{type(error).__name__}: {error}"
        return self.note(
            name, status, reason, exit_status=code, log=str(log),
            duration_seconds=round(time.monotonic() - started, 3),
        )


def parse_args(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("projects", nargs="*", metavar="CONFIGURATION")
    parser.add_argument("--all", action="store_true", help="all five firmware configurations and default checks")
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--fast", "--host-tests-only", action="store_true", help="verifier, host, contracts, operator and companion tests")
    modes.add_argument("--firmware-only", action="store_true", help="selected firmware builds only")
    modes.add_argument("--hardware-only", "--kicad-only", action="store_true", help="strict KiCad checks only")
    modes.add_argument("--hardware-report-only", action="store_true", help="informational hardware run; never passes verification")
    modes.add_argument("--doctor", action="store_true", help="check default prerequisites without builds")
    parser.add_argument("--with-kicad", "--kicad-strict", action="store_true", help="explicit strict hardware checks (already the default)")
    parser.add_argument("--ring-profile", choices=["r30-oled-none-none"])
    parser.add_argument("--report", type=Path, help="default: build-verification/verification.json")
    selected = parser.parse_args(argv)
    if any(name not in CONFIGURATIONS for name in selected.projects):
        parser.error("unknown configuration; choose " + ", ".join(CONFIGURATIONS))
    if selected.all and (selected.projects or selected.ring_profile):
        parser.error("--all cannot be combined with project/profile selectors")
    if selected.fast or selected.hardware_only or selected.hardware_report_only or selected.doctor:
        if selected.projects or selected.all or selected.ring_profile:
            parser.error("this mode cannot be combined with firmware selectors")
    if selected.with_kicad and (selected.fast or selected.firmware_only or selected.doctor or selected.hardware_report_only):
        parser.error("strict hardware checks cannot be combined with this mode")
    if selected.ring_profile and selected.projects and "ring" not in selected.projects:
        parser.error("--ring-profile requires the ring configuration")
    return selected


def plan(selected):
    hardware_only = selected.hardware_only or selected.hardware_report_only
    firmware_enabled = not (selected.fast or hardware_only or selected.doctor)
    configurations = []
    if firmware_enabled:
        configurations = list(CONFIGURATIONS) if selected.all else list(dict.fromkeys(selected.projects or ["ring", "hub"]))
    return {
        "configurations": configurations,
        "host": not (selected.firmware_only or hardware_only or selected.doctor),
        "sanitizers": not (selected.fast or selected.firmware_only or hardware_only or selected.doctor),
        "hardware": not (selected.fast or selected.firmware_only or selected.doctor),
        "doctor": selected.doctor,
        "report_only": selected.hardware_report_only,
    }


def load_manifest(repo):
    data = json.loads((repo / "toolchains/esp-idf-local.json").read_text())
    if not isinstance(data, dict):
        raise ValueError("manifest must be an object")
    for key, pattern in (("version", r"v\d+\.\d+\.\d+"), ("commit", r"[0-9a-f]{40}"), ("hub_dependencies_lock_sha256", r"[0-9a-f]{64}")):
        if not isinstance(data.get(key), str) or not re.fullmatch(pattern, data[key]):
            raise ValueError(f"invalid manifest field {key}")
    if data.get("targets") != ["esp32c3", "esp32s3"]:
        raise ValueError("manifest targets must match ring and hub targets")
    return data


def sdk_paths(data):
    install = Path(os.environ.get("POWERFINGER_IDF_ROOT", data.get("install_root", "~/.powerfinger-sdk"))).expanduser().resolve()
    tools = Path(os.environ.get("POWERFINGER_IDF_TOOLS_PATH", install / "espressif-tools")).expanduser().resolve()
    return install / f"esp-idf-{data['version']}", tools


def check_sdk(data, sdk):
    if os.environ.get("POWERFINGER_IDF_VERSION") not in (None, data["version"]):
        raise ValueError("POWERFINGER_IDF_VERSION conflicts with the pinned manifest")
    if not (sdk / "export.sh").is_file():
        raise ValueError(f"SDK is missing at {sdk}; run scripts/setup-esp-idf-local.sh")
    if probe(["git", "-C", sdk, "rev-parse", "HEAD"]) != data["commit"]:
        raise ValueError("SDK commit differs from the pinned manifest")
    if probe(["git", "-C", sdk, "status", "--porcelain", "--untracked-files=all"]):
        raise ValueError("SDK checkout has modifications")
    submodules = probe(["git", "-C", sdk, "submodule", "status", "--recursive"])
    if any(line and line[0] in "-+U" for line in submodules.splitlines()):
        raise ValueError("SDK submodules are incomplete or differ from the pinned checkout")


def idf_command(sdk, tools, command):
    # Positional arguments keep paths/data out of shell source text.
    glue = 'set -e; export IDF_PATH="$1" IDF_TOOLS_PATH="$2"; . "$1/export.sh" >/dev/null; shift 2; exec "$@"'
    return ["bash", "-c", glue, "powerfinger-idf", sdk, tools, *command]


def check_lock(repo, data):
    if digest(repo / "firmware/hub/dependencies.lock") != data["hub_dependencies_lock_sha256"]:
        raise ValueError("hub dependencies.lock differs from its pinned hash; review dependency changes")


def tool_versions(names):
    versions = {}
    for name in names:
        try:
            value = probe([name, "version" if name == "kicad-cli" else "--version"])
            if not value:
                raise ValueError("empty version output")
            versions[name] = {"path": shutil.which(name), "version": value.splitlines()[0]}
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            versions[name] = {"error": str(error)}
    return versions


def sdk_preflight(runner, repo, data, tools_record):
    try:
        sdk, tools = sdk_paths(data)
        check_sdk(data, sdk)
        check_lock(repo, data)
        for tool in ("idf.py", "riscv32-esp-elf-gcc", "xtensa-esp32s3-elf-gcc"):
            value = probe(idf_command(sdk, tools, [tool, "--version"]))
            if not value or (tool == "idf.py" and value != f"ESP-IDF {data['version']}"):
                raise ValueError(f"unexpected {tool} version: {value}")
            tools_record[tool] = {"version": value.splitlines()[0], "sdk": str(sdk), "tools_root": str(tools)}
        runner.note("sdk/prerequisites", "passed", "pinned source, dependency lock, IDF and both compilers verified")
        return sdk, tools
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        runner.note("sdk/prerequisites", "failed", str(error))
        return None


def run_host(runner, repo, sanitizer=False):
    label = "host-sanitizers" if sanitizer else "host"
    build = repo / ("build-test-asan" if sanitizer else "build-test")
    if not runner.stage(f"{label}/configure", ["cmake", "-S", repo / "firmware/test", "-B", build, "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", f"-DPOWERFINGER_ENABLE_SANITIZERS={'ON' if sanitizer else 'OFF'}"]):
        runner.note(f"{label}/build", "skipped", "configure failed")
        runner.note(f"{label}/tests", "skipped", "configure failed")
        return
    if not runner.stage(f"{label}/build", ["cmake", "--build", build]):
        runner.note(f"{label}/tests", "skipped", "build failed")
        return
    runner.stage(f"{label}/tests", ["ctest", "--test-dir", build, "--output-on-failure", "--timeout", str(TEST_TIMEOUT)])


def check_r30(config):
    values = {}
    for line in config.read_text().splitlines():
        if "=" in line and not line.startswith("#"):
            key, value = line.split("=", 1)
            values[key] = value
    wrong = [f"{key}: expected {value}, got {values.get(key)}" for key, value in R30_EXPECTED.items() if values.get(key) != value]
    if wrong:
        raise ValueError("; ".join(wrong))


def prepare_build(repo, build_root, configuration):
    project, directory, _ = CONFIGURATIONS[configuration]
    # Only remove configs in a directory explicitly owned by this verifier.
    target = build_root / "verify" / directory
    if target.is_symlink() or target.parent.is_symlink():
        raise ValueError("refusing a symlinked profile directory")
    marker = target / ".powerfinger-build.json"
    ownership = {"project": str((repo / "firmware" / project).resolve()), "configuration": configuration}
    if target.exists() and any(target.iterdir()):
        if marker.is_symlink() or not marker.is_file() or json.loads(marker.read_text()) != ownership:
            raise ValueError(f"refusing to reset an unowned build directory: {target}")
    target.mkdir(parents=True, exist_ok=True)
    marker.write_text(json.dumps(ownership) + "\n")
    for filename in ("sdkconfig", "sdkconfig.old"):
        config = target / filename
        if config.is_symlink():
            raise ValueError(f"refusing to remove symlinked config: {config}")
        config.unlink(missing_ok=True)
    return target


def run_firmware(runner, repo, data, sdk_tools, configurations):
    if not sdk_tools:
        for name in configurations:
            runner.note(f"firmware/{name}", "skipped", "SDK prerequisite failed")
        return
    sdk, tools = sdk_tools
    build_root = Path(os.environ.get("POWERFINGER_IDF_BUILD_ROOT", repo / "build-idf")).expanduser()
    if not build_root.is_absolute():
        build_root = repo / build_root
    build_root = build_root.resolve()
    for name in configurations:
        label = f"firmware/{name}"
        project, _, target = CONFIGURATIONS[name]
        try:
            check_lock(repo, data)
            defaults = [repo / "firmware" / project / "sdkconfig.defaults"]
            if name == "ring":
                defaults.append(repo / "firmware/ring/sdkconfig.defaults.r30_oled_none_none")
            for path in defaults:
                if not path.is_file():
                    raise ValueError(f"missing defaults: {path}")
            build = prepare_build(repo, build_root, name)
            config = build / "sdkconfig"
            options = ["idf.py", "-C", repo / "firmware" / project, "-B", build, f"-DIDF_TARGET={target}", f"-DSDKCONFIG={config}", "-DSDKCONFIG_DEFAULTS=" + ";".join(map(str, defaults))]
            env = os.environ.copy()
            env["IDF_TARGET"] = target
            configured = runner.stage(f"{label}/configure", idf_command(sdk, tools, [*options, "reconfigure"]), env=env)
            check_lock(repo, data)
            if not configured:
                runner.note(f"{label}/build", "skipped", "configure failed")
                continue
            if name == "ring":
                check_r30(config)
                runner.note(f"{label}/profile", "passed", "resolved R30 sensor, click and pin contract matches")
            built = runner.stage(f"{label}/build", idf_command(sdk, tools, [*options, "build"]), env=env)
            check_lock(repo, data)
            if built:
                artifacts = [build / f"powerfinger_{project}.{suffix}" for suffix in ("bin", "elf")]
                hashes = {str(path): digest(path) for path in artifacts}
                runner.note(f"{label}/artifacts", "passed", "firmware artifacts retained", artifacts=hashes, sdkconfig=str(config))
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            runner.note(f"{label}/validation", "failed", str(error))


def finding_count(path, kind):
    data = json.loads(path.read_text())
    if not isinstance(data, dict):
        raise ValueError("KiCad report must be an object")
    if kind == "erc":
        sheets = data.get("sheets")
        if not isinstance(sheets, list) or not sheets:
            raise ValueError("ERC sheets must be a nonempty array")
        arrays = []
        for sheet in sheets:
            if not isinstance(sheet, dict) or not isinstance(sheet.get("violations"), list):
                raise ValueError("every ERC sheet must contain a violations array")
            arrays.append(sheet["violations"])
    else:
        arrays = [data.get(key) for key in ("violations", "unconnected_items", "schematic_parity")]
        if any(not isinstance(value, list) for value in arrays):
            raise ValueError("DRC violations, unconnected_items and schematic_parity must be arrays")
    return sum(len(items) for items in arrays)


def run_hardware(runner, repo):
    for packet, sources in PACKETS.items():
        for kind, source in zip(("erc", "drc"), sources):
            label = f"hardware/{packet}/{kind}"
            report = runner.report_dir / "kicad" / packet / f"{kind}.json"
            try:
                if not (repo / source).is_file():
                    raise ValueError(f"missing KiCad input: {source}")
                report.parent.mkdir(parents=True, exist_ok=True)
                report.unlink(missing_ok=True)
                command = ["kicad-cli", "sch" if kind == "erc" else "pcb", kind, "--severity-all", "--format", "json", "--exit-code-violations", "-o", report, repo / source]
                if kind == "drc":
                    command.append("--schematic-parity")
                command_ok = runner.stage(label, command)
                count = finding_count(report, kind)
                runner.note(f"{label}/report", "passed" if command_ok and count == 0 else "failed", f"{count} findings", artifact=str(report))
            except (OSError, ValueError) as error:
                runner.note(f"{label}/report", "failed", str(error))


def write_report(path, report):
    # Readers see a complete result, including "running" after a new invocation.
    with tempfile.NamedTemporaryFile(mode="w", dir=path.parent, delete=False, encoding="utf-8") as stream:
        temporary = Path(stream.name)
        try:
            json.dump(report, stream, indent=2, sort_keys=True)
            stream.write("\n")
            stream.flush()
            os.replace(temporary, path)
        finally:
            temporary.unlink(missing_ok=True)


def main(argv=None):
    selected = parse_args(argv)
    repo = Path(__file__).resolve().parents[1]
    report_path = (selected.report or repo / "build-verification/verification.json").resolve()
    runner = Runner(repo, report_path.parent)
    selection = plan(selected)
    report = {
        "schema": 1, "result": "running", "selected": selection,
        "stages": runner.stages, "source": {}, "tools": {},
        "started_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    write_report(report_path, report)
    try:
        try:
            report["source"] = {
                "revision": probe(["git", "rev-parse", "HEAD"], cwd=repo),
                "dirty": bool(probe(["git", "status", "--porcelain"], cwd=repo)),
            }
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            runner.note("source", "failed", str(error))
        data = load_manifest(repo)
        report["toolchain"] = data
        names = ["python3", "git", "bash"]
        if selection["host"] or selection["doctor"]:
            names += ["cmake", "ctest", "cc", "ruby", "node"]
        if selection["hardware"] or selection["doctor"]:
            names += ["kicad-cli", "openscad"]
        report["tools"] = tool_versions(names)
        sdk_tools = None
        if selection["configurations"] or selection["doctor"]:
            sdk_tools = sdk_preflight(runner, repo, data, report["tools"])
        if selection["doctor"]:
            for name, value in report["tools"].items():
                if name != "openscad":
                    runner.note(f"doctor/{name}", "failed" if "error" in value else "passed", value.get("error", value.get("version", "available")))
            runner.stage("doctor/ruby-libraries", ["ruby", "-e", "require 'yaml'; require 'csv'"], timeout=PROBE_TIMEOUT)
        if selection["host"]:
            runner.stage("verifier/regressions", [sys.executable, "-m", "unittest", "scripts.test_verify_firmware_local"], timeout=COMMAND_TIMEOUT)
            run_host(runner, repo)
            for name, command in (
                ("contracts", [repo / "scripts/check-contracts-local.sh"]),
                ("operator/factory", [repo / "scripts/scaffold-shenzhen-seeed-factory-reply.py", "--self-test"]),
                ("operator/coupon", [repo / "scripts/ingest-usb-hub-coupon-results.py", "--self-test"]),
                ("companion/protocol", ["node", "--test", repo / "companion/test/protocol.test.mjs"]),
            ):
                runner.stage(name, command)
        if selection["sanitizers"]:
            run_host(runner, repo, sanitizer=True)
        if selection["configurations"]:
            run_firmware(runner, repo, data, sdk_tools, selection["configurations"])
        if selection["hardware"]:
            run_hardware(runner, repo)
        if selection["report_only"]:
            runner.note("hardware/report-only", "report-only", "informational run cannot establish verification")
    except KeyboardInterrupt:
        runner.note("verification", "failed", "interrupted before completion")
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        runner.note("verification", "failed", str(error))
    finally:
        report["result"] = "pass" if runner.stages and all(stage["status"] == "passed" for stage in runner.stages) else "fail"
        report["generated_at"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
        write_report(report_path, report)
        print(f"Report: {report_path}\nResult: {report['result'].upper()}", flush=True)
    return 0 if report["result"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
