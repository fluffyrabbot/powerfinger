import test from "node:test";
import assert from "node:assert/strict";

import {
    buildGetGesturesCommand,
    buildGetRingDiagnosticsCommand,
    buildGetRingSettingsCommand,
    buildForgetRingCommand,
    buildSetGestureCommand,
    buildSetRingDeadZoneDistanceCommand,
    buildSetRingDeadZoneTimeCommand,
    buildSetRingDpiCommand,
    buildSetRoleCommand,
    buildSwapRolesCommand,
    parseHubInfoResponse,
    parseGesturesResponse,
    parseProtocolResponse,
    parseRingInfoResponse,
    parseRingDiagnosticsResponse,
    parseRingSettingsResponse,
    parseRingsResponse,
    parseRolesResponse,
} from "../web/protocol.mjs";

test("parseProtocolResponse handles successful multi-line replies", () => {
    const response = parseProtocolResponse(
        "+ fw=0.1.0\n+ hw=DEVBOARD-S3\n+ rings=2\nOK\n",
    );

    assert.equal(response.ok, true);
    assert.equal(response.dataLines.length, 3);
    assert.equal(response.statusLine, "OK");
});

test("parseProtocolResponse handles protocol errors", () => {
    const response = parseProtocolResponse("ERR 404 unknown_mac\n");

    assert.equal(response.ok, false);
    assert.equal(response.errorCode, 404);
    assert.equal(response.errorMessage, "unknown_mac");
});

test("parseHubInfoResponse extracts key fields", () => {
    const info = parseHubInfoResponse(
        "+ fw=0.1.0\n+ hw=DEVBOARD-S3\n+ rings=2\n+ max_rings=4\n+ usb_poll_ms=1\n+ scan_policy=1\nOK\n",
    );

    assert.deepEqual(info, {
        firmwareRevision: "0.1.0",
        hardwareRevision: "DEVBOARD-S3",
        connectedRings: "2",
        maxRings: "4",
        usbPollMs: "1",
        scanPolicy: "1",
    });
});

test("parseRolesResponse returns normalized role entries", () => {
    const roles = parseRolesResponse(
        "+ aa:bb:cc:dd:ee:01 cursor\n+ AA:BB:CC:DD:EE:02 SCROLL\nOK\n",
    );

    assert.deepEqual(roles, [
        { mac: "AA:BB:CC:DD:EE:01", role: "CURSOR" },
        { mac: "AA:BB:CC:DD:EE:02", role: "SCROLL" },
    ]);
});

test("parseRingsResponse includes live connection status", () => {
    const rings = parseRingsResponse(
        "+ AA:BB:CC:DD:EE:01 CURSOR connected\n+ AA:BB:CC:DD:EE:02 SCROLL disconnected\nOK\n",
    );

    assert.deepEqual(rings, [
        { mac: "AA:BB:CC:DD:EE:01", role: "CURSOR", connected: true },
        { mac: "AA:BB:CC:DD:EE:02", role: "SCROLL", connected: false },
    ]);
});

test("parseRingInfoResponse extracts ring snapshot", () => {
    const ringInfo = parseRingInfoResponse(
        "+ mac=AA:BB:CC:DD:EE:01\n+ role=CURSOR\n+ connected=1\nOK\n",
    );

    assert.deepEqual(ringInfo, {
        mac: "AA:BB:CC:DD:EE:01",
        role: "CURSOR",
        connected: true,
    });
});

test("parseRingSettingsResponse extracts live tuning values", () => {
    const settings = parseRingSettingsResponse(
        "+ mac=AA:BB:CC:DD:EE:01\n+ dpi_multiplier=20\n+ dead_zone_time_ms=75\n+ dead_zone_distance=12\n+ firmware_version=0.1.0\nOK\n",
    );

    assert.deepEqual(settings, {
        mac: "AA:BB:CC:DD:EE:01",
        dpiMultiplier: 20,
        deadZoneTimeMs: 75,
        deadZoneDistance: 12,
        firmwareVersion: "0.1.0",
    });
});

test("parseRingDiagnosticsResponse extracts live battery and health state", () => {
    const diagnostics = parseRingDiagnosticsResponse(
        "+ mac=AA:BB:CC:DD:EE:01\n"
        + "+ battery_pct=50\n"
        + "+ battery_mv=3700\n"
        + "+ ring_state=CONNECTED_IDLE\n"
        + "+ sensor_state=READY\n"
        + "+ bond_state=RESTORED\n"
        + "+ connected=1\n"
        + "+ calibration_valid=1\n"
        + "+ conn_param_rejected=0\n"
        + "+ conn_interval_1_25ms=12\n"
        + "+ diagnostics_version=1\n"
        + "OK\n",
    );

    assert.deepEqual(diagnostics, {
        mac: "AA:BB:CC:DD:EE:01",
        batteryPct: 50,
        batteryMv: 3700,
        ringState: "CONNECTED_IDLE",
        sensorState: "READY",
        bondState: "RESTORED",
        connected: true,
        calibrationValid: true,
        connParamRejected: false,
        connInterval125Ms: 12,
        diagnosticsVersion: 1,
    });
});

test("parseGesturesResponse extracts supported gesture mappings", () => {
    const gestures = parseGesturesResponse(
        "+ 0x01 0x02 cursor+scroll=back\n"
        + "+ 0x02 0x00 cursor+modifier=disabled\n"
        + "OK\n",
    );

    assert.deepEqual(gestures, [
        {
            triggerId: "0x01",
            actionId: "0x02",
            triggerLabel: "Cursor + Scroll simultaneous click",
            actionLabel: "Back",
            description: "cursor+scroll=back",
        },
        {
            triggerId: "0x02",
            actionId: "0x00",
            triggerLabel: "Cursor + Modifier simultaneous click",
            actionLabel: "Disabled",
            description: "cursor+modifier=disabled",
        },
    ]);
});

test("command builders normalize and validate arguments", () => {
    assert.equal(
        buildSetRoleCommand("aa:bb:cc:dd:ee:01", "scroll"),
        "SET_ROLE AA:BB:CC:DD:EE:01 SCROLL",
    );
    assert.equal(
        buildSwapRolesCommand("aa:bb:cc:dd:ee:01", "AA:BB:CC:DD:EE:02"),
        "SWAP_ROLES AA:BB:CC:DD:EE:01 AA:BB:CC:DD:EE:02",
    );
    assert.equal(
        buildForgetRingCommand("aa:bb:cc:dd:ee:03"),
        "FORGET_RING AA:BB:CC:DD:EE:03",
    );
    assert.equal(
        buildGetGesturesCommand(),
        "GET_GESTURES",
    );
    assert.equal(
        buildGetRingSettingsCommand("aa:bb:cc:dd:ee:01"),
        "GET_RING_SETTINGS AA:BB:CC:DD:EE:01",
    );
    assert.equal(
        buildGetRingDiagnosticsCommand("aa:bb:cc:dd:ee:01"),
        "GET_RING_DIAGNOSTICS AA:BB:CC:DD:EE:01",
    );
    assert.equal(
        buildSetGestureCommand("0x01", "0x03"),
        "SET_GESTURE 0x01 0x03",
    );
    assert.equal(
        buildSetRingDpiCommand("aa:bb:cc:dd:ee:01", "20"),
        "SET_RING_DPI AA:BB:CC:DD:EE:01 20",
    );
    assert.equal(
        buildSetRingDeadZoneTimeCommand("aa:bb:cc:dd:ee:01", 75),
        "SET_RING_DEAD_ZONE_TIME AA:BB:CC:DD:EE:01 75",
    );
    assert.equal(
        buildSetRingDeadZoneDistanceCommand("aa:bb:cc:dd:ee:01", 12),
        "SET_RING_DEAD_ZONE_DISTANCE AA:BB:CC:DD:EE:01 12",
    );
});

test("ring tuning builders reject blank required values", () => {
    assert.throws(
        () => buildSetRingDpiCommand("aa:bb:cc:dd:ee:01", ""),
        /required/i,
    );
});
