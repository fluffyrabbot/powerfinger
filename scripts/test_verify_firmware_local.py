#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Exercise verifier failure boundaries without SDK, CAD, or network access."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("powerfinger_verifier", REPO / "scripts/verify_firmware_local.py")
VERIFIER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFIER)
MANIFEST = {
    "version": "v6.1", "commit": "fff9895c82d744c7237be8847347bdd1b07c6643",
    "hub_dependencies_lock_sha256": hashlib.sha256(b"fixture lock\n").hexdigest(),
    "targets": ["esp32c3", "esp32s3"],
}
FAKE_KICAD = r'''#!/usr/bin/env python3
import json, os, sys
if "-o" not in sys.argv:
    print("10.0.6")
    raise SystemExit(0)
source = next(arg for arg in sys.argv if arg.endswith((".kicad_sch", ".kicad_pcb")))
kind = "erc" if "sch" in sys.argv else "drc"
packet = "hub" if "usb_hub" in source else "ring"
with open(os.environ["PF_KICAD_LOG"], "a") as log:
    log.write(packet + ":" + kind + "\n")
scenario = os.environ.get("PF_KICAD_SCENARIO", "clean")
output = sys.argv[sys.argv.index("-o") + 1]
if scenario == "no_report":
    raise SystemExit(0)
if scenario == "malformed":
    with open(output, "w") as stream:
        stream.write("not json")
    raise SystemExit(0)
if kind == "erc":
    data = {"sheets": [{"violations": []}]}
    if scenario == "empty_sheets": data = {"sheets": []}
    if scenario == "malformed_sheet": data = {"sheets": ["bad"]}
    if scenario == "missing_violations": data = {"sheets": [{}]}
    if scenario == "findings": data = {"sheets": [{"violations": [{"severity": "error"}]}]}
else:
    data = {"violations": [], "unconnected_items": [], "schematic_parity": []}
    if scenario == "missing_drc": data.pop("schematic_parity")
    if scenario == "findings": data["violations"].append({"severity": "error"})
with open(output, "w") as stream: json.dump(data, stream)
if scenario == "exit_nonzero": raise SystemExit(7)
'''


class FixtureTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo = Path(self.temp.name) / "repo"
        self.repo.mkdir()
        (self.repo / "scripts").mkdir()
        for name in ("verify_firmware_local.py", "verify-firmware-local.sh"):
            shutil.copy2(REPO / "scripts" / name, self.repo / "scripts" / name)
        (self.repo / "toolchains").mkdir()
        (self.repo / "toolchains/esp-idf-local.json").write_text(json.dumps(MANIFEST))
        (self.repo / "firmware/hub").mkdir(parents=True)
        (self.repo / "firmware/hub/dependencies.lock").write_bytes(b"fixture lock\n")
        for sources in VERIFIER.PACKETS.values():
            for source in sources:
                path = self.repo / source
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("fixture")
        self.bin = Path(self.temp.name) / "bin"
        self.bin.mkdir()
        for name, actual in (("python3", sys.executable), ("git", shutil.which("git")), ("bash", shutil.which("bash"))):
            (self.bin / name).symlink_to(actual)
        executable = self.bin / "kicad-cli"
        executable.write_text(FAKE_KICAD)
        executable.chmod(0o755)
        for command in (
            ["init", "-q"], ["config", "user.email", "test@example.invalid"],
            ["config", "user.name", "Verifier Tests"],
            ["add", "scripts", "toolchains", "firmware", "hardware"],
            ["-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null", "commit", "-qm", "fixture"],
        ):
            subprocess.run(["git", *command], cwd=self.repo, check=True, capture_output=True)
        self.report = self.repo / "report.json"

    def run_verifier(self, *arguments, scenario="clean"):
        env = os.environ.copy()
        env.update({
            "PATH": str(self.bin) + ":/usr/bin:/bin",
            "PF_KICAD_SCENARIO": scenario,
            "PF_KICAD_LOG": str(self.repo / "kicad.log"),
            "POWERFINGER_IDF_ROOT": str(self.repo / "absent-sdk"),
        })
        self.report.unlink(missing_ok=True)
        result = subprocess.run(
            [str(self.repo / "scripts/verify-firmware-local.sh"), *arguments, "--report", str(self.report)],
            cwd=self.repo, env=env, text=True, capture_output=True, timeout=30,
        )
        report = json.loads(self.report.read_text()) if self.report.exists() else None
        return result, report

    def assert_failed(self, result, report):
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIsNotNone(report, result.stdout + result.stderr)
        self.assertEqual(report["result"], "fail")
        self.assertTrue(any(stage["status"] == "failed" for stage in report["stages"]))

    def test_clean_hardware_runs_both_packets(self):
        result, report = self.run_verifier("--hardware-only")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(report["result"], "pass")
        self.assertEqual(set((self.repo / "kicad.log").read_text().splitlines()), {"ring:erc", "ring:drc", "hub:erc", "hub:drc"})
        self.assertFalse(any(stage["name"].startswith("sdk/") for stage in report["stages"]))

    def test_missing_kicad_fails(self):
        (self.bin / "kicad-cli").unlink()
        result, report = self.run_verifier("--hardware-only")
        self.assert_failed(result, report)
        self.assertTrue(any("kicad-cli" in stage["reason"] for stage in report["stages"]))

    def test_missing_sources_fail_but_other_packet_runs(self):
        for source in VERIFIER.PACKETS["USB-HUB"]:
            (self.repo / source).unlink()
        result, report = self.run_verifier("--hardware-only")
        self.assert_failed(result, report)
        self.assertEqual(sum("missing KiCad input" in stage["reason"] for stage in report["stages"]), 2)
        self.assertIn("ring:erc", (self.repo / "kicad.log").read_text())

    def test_bad_reports_and_findings_never_pass(self):
        for scenario in ("malformed", "empty_sheets", "malformed_sheet", "missing_violations", "missing_drc", "findings", "exit_nonzero"):
            with self.subTest(scenario=scenario):
                result, report = self.run_verifier("--hardware-only", scenario=scenario)
                self.assert_failed(result, report)
                self.assertTrue(any("USB-HUB/drc" in stage["name"] for stage in report["stages"]))

    def test_stale_report_cannot_mask_no_output(self):
        result, _ = self.run_verifier("--hardware-only")
        self.assertEqual(result.returncode, 0)
        result, report = self.run_verifier("--hardware-only", scenario="no_report")
        self.assert_failed(result, report)
        self.assertTrue(any("No such file" in stage["reason"] for stage in report["stages"]))

    def test_report_only_is_never_verification(self):
        result, report = self.run_verifier("--hardware-report-only")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(report["result"], "fail")
        self.assertTrue(any(stage["status"] == "report-only" for stage in report["stages"]))

    def test_contradictory_flags_are_rejected(self):
        for arguments in (("--fast", "--firmware-only"), ("--all", "hub"), ("--doctor", "--with-kicad"), ("--hardware-only", "ring"), ("--ring-profile", "r30-oled-none-none", "hub")):
            with self.subTest(arguments=arguments):
                result, _ = self.run_verifier(*arguments)
                self.assertEqual(result.returncode, 2, result.stdout + result.stderr)

    def test_default_all_and_generic_plans(self):
        for arguments, expected in (
            (("--firmware-only",), ["ring", "hub"]),
            (("--firmware-only", "--all"), ["ring", "ring-generic", "pen", "puck", "hub"]),
            (("--firmware-only", "ring-generic"), ["ring-generic"]),
        ):
            with self.subTest(arguments=arguments):
                result, report = self.run_verifier(*arguments)
                self.assert_failed(result, report)  # No SDK installed in the fixture.
                self.assertEqual(report["selected"]["configurations"], expected)
                self.assertEqual(sum(stage["status"] == "skipped" for stage in report["stages"]), len(expected))

    def test_missing_or_invalid_manifest_produces_failure_report(self):
        manifest = self.repo / "toolchains/esp-idf-local.json"
        manifest.write_text("{")
        result, report = self.run_verifier("--firmware-only")
        self.assert_failed(result, report)
        manifest.unlink()
        result, report = self.run_verifier("--firmware-only")
        self.assert_failed(result, report)


class ExecutionTests(unittest.TestCase):
    def test_interruption_replaces_previous_pass_with_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "toolchains").mkdir()
            (root / "toolchains/esp-idf-local.json").write_text(json.dumps(MANIFEST))
            report = root / "report.json"
            report.write_text('{"result":"pass"}')

            def interrupt(_names):
                self.assertEqual(json.loads(report.read_text())["result"], "running")
                raise KeyboardInterrupt

            with patch.object(VERIFIER, "__file__", str(root / "scripts/verify_firmware_local.py")), patch.object(VERIFIER, "probe", return_value="fixture"), patch.object(VERIFIER, "tool_versions", side_effect=interrupt):
                self.assertEqual(VERIFIER.main(["--hardware-only", "--report", str(report)]), 1)
            result = json.loads(report.read_text())
            self.assertEqual(result["result"], "fail")
            self.assertIn("interrupted", result["stages"][-1]["reason"])

    def test_timeout_kills_child_process(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            marker = root / "survived"
            child = f"import time; from pathlib import Path; time.sleep(0.6); Path({str(marker)!r}).touch()"
            parent = f"import subprocess,sys,time; subprocess.Popen([sys.executable,'-c',{child!r}]); time.sleep(5)"
            runner = VERIFIER.Runner(root, root)
            self.assertFalse(runner.stage("timeout", [sys.executable, "-c", parent], timeout=0.2))
            self.assertEqual(runner.stages[0]["status"], "failed")
            time.sleep(0.7)
            self.assertFalse(marker.exists(), "child process survived verifier timeout")

    def test_owned_config_reset_and_unowned_rejection(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            build = VERIFIER.prepare_build(root, root / "build", "ring")
            (build / "sdkconfig").write_text("CONFIG_SENSOR_NONE=y")
            VERIFIER.prepare_build(root, root / "build", "ring")
            self.assertFalse((build / "sdkconfig").exists())
            (build / ".powerfinger-build.json").unlink()
            sentinel = build / "sdkconfig"
            sentinel.write_text("user config")
            with self.assertRaises(ValueError):
                VERIFIER.prepare_build(root, root / "build", "ring")
            self.assertEqual(sentinel.read_text(), "user config")

    def test_idf_activation_preserves_arguments_and_tools_path(self):
        with tempfile.TemporaryDirectory() as directory:
            sdk = Path(directory)
            (sdk / "export.sh").write_text('echo activation-banner >&2\nexport PF_ACTIVATED=yes\n')
            code = "import json,os,sys; print(json.dumps([os.environ['PF_ACTIVATED'],os.environ['IDF_TOOLS_PATH'],sys.argv[1:]]))"
            command = VERIFIER.idf_command(sdk, sdk / "tools with spaces", [sys.executable, "-c", code, "arg with spaces"])
            value = json.loads(VERIFIER.probe(command))
            self.assertEqual(value, ["yes", str(sdk / "tools with spaces"), ["arg with spaces"]])
            (sdk / "export.sh").write_text('echo activation-failed >&2\nreturn 1\n')
            with self.assertRaisesRegex(ValueError, "activation-failed"):
                VERIFIER.probe(command)

    def test_configuration_failure_skips_build_and_aggregates(self):
        class FakeRunner(VERIFIER.Runner):
            def stage(self, name, command, **kwargs):
                self.commands.append((name, command))
                return self.note(name, "failed", "fixture configure failure")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for project in ("ring", "hub"):
                path = root / "firmware" / project
                path.mkdir(parents=True)
                (path / "sdkconfig.defaults").write_text("fixture")
            (root / "firmware/ring/sdkconfig.defaults.r30_oled_none_none").write_text("overlay")
            (root / "firmware/hub/dependencies.lock").write_bytes(b"fixture lock\n")
            runner = FakeRunner(root, root / "reports")
            runner.commands = []
            with patch.dict(os.environ, {"POWERFINGER_IDF_BUILD_ROOT": str(root / "build")}):
                VERIFIER.run_firmware(runner, root, MANIFEST, (root / "sdk", root / "tools"), ["ring", "hub"])
            self.assertEqual([name for name, _ in runner.commands], ["firmware/ring/configure", "firmware/hub/configure"])
            self.assertEqual(sum(stage["status"] == "skipped" for stage in runner.stages), 2)
            ring_command = runner.commands[0][1]
            build_arg = ring_command[ring_command.index("-B") + 1]
            self.assertEqual(Path(build_arg), root.resolve() / "build" / MANIFEST["commit"] / "verify" / "r30-oled-none-none")
            defaults = next(str(item) for item in ring_command if str(item).startswith("-DSDKCONFIG_DEFAULTS="))
            self.assertIn("sdkconfig.defaults;", defaults)
            self.assertTrue(defaults.endswith("sdkconfig.defaults.r30_oled_none_none"))


if __name__ == "__main__":
    unittest.main()
