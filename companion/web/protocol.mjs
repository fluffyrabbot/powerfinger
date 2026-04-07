export const SERIAL_OPTIONS = Object.freeze({
    baudRate: 115200,
    dataBits: 8,
    stopBits: 1,
    parity: "none",
    flowControl: "none",
});

export const SUPPORTED_ROLES = Object.freeze([
    "CURSOR",
    "SCROLL",
    "MODIFIER",
]);

export const SUPPORTED_GESTURE_TRIGGERS = Object.freeze([
    Object.freeze({ id: "0x01", label: "Cursor + Scroll simultaneous click" }),
    Object.freeze({ id: "0x02", label: "Cursor + Modifier simultaneous click" }),
    Object.freeze({ id: "0x03", label: "Scroll + Modifier simultaneous click" }),
    Object.freeze({ id: "0x04", label: "All three simultaneous click" }),
]);

export const SUPPORTED_GESTURE_ACTIONS = Object.freeze([
    Object.freeze({ id: "0x00", label: "Disabled" }),
    Object.freeze({ id: "0x01", label: "Middle click" }),
    Object.freeze({ id: "0x02", label: "Back" }),
    Object.freeze({ id: "0x03", label: "Forward" }),
]);

export const RING_SETTINGS_LIMITS = Object.freeze({
    dpiMultiplier: Object.freeze({ min: 1, max: 255 }),
    deadZoneTimeMs: Object.freeze({ min: 0, max: 2000 }),
    deadZoneDistance: Object.freeze({ min: 0, max: 255 }),
});

export const RING_DIAGNOSTICS_LIMITS = Object.freeze({
    batteryPct: Object.freeze({ min: 0, max: 100 }),
    batteryMv: Object.freeze({ min: 0, max: 65535 }),
    connInterval125Ms: Object.freeze({ min: 0, max: 65535 }),
    diagnosticsVersion: Object.freeze({ min: 1, max: 255 }),
});

export function normalizeMac(mac) {
    if (typeof mac !== "string") {
        throw new TypeError("MAC address must be a string.");
    }

    const normalized = mac.trim().toUpperCase();
    if (!/^[0-9A-F]{2}(?::[0-9A-F]{2}){5}$/.test(normalized)) {
        throw new Error(`Invalid MAC address: ${mac}`);
    }

    return normalized;
}

export function normalizeRole(role) {
    if (typeof role !== "string") {
        throw new TypeError("Role must be a string.");
    }

    const normalized = role.trim().toUpperCase();
    if (!SUPPORTED_ROLES.includes(normalized)) {
        throw new Error(`Invalid role: ${role}`);
    }

    return normalized;
}

export function normalizeInteger(value, label, { min, max }) {
    if (typeof value === "string" && value.trim() === "") {
        throw new Error(`${label} is required.`);
    }

    const numeric = Number(value);
    if (!Number.isInteger(numeric)) {
        throw new Error(`${label} must be an integer.`);
    }
    if (numeric < min || numeric > max) {
        throw new Error(`${label} must be between ${min} and ${max}.`);
    }

    return numeric;
}

export function buildSetRoleCommand(mac, role) {
    return `SET_ROLE ${normalizeMac(mac)} ${normalizeRole(role)}`;
}

export function buildSwapRolesCommand(macA, macB) {
    const normalizedA = normalizeMac(macA);
    const normalizedB = normalizeMac(macB);

    if (normalizedA === normalizedB) {
        throw new Error("Swap roles requires two different MAC addresses.");
    }

    return `SWAP_ROLES ${normalizedA} ${normalizedB}`;
}

export function buildForgetRingCommand(mac) {
    return `FORGET_RING ${normalizeMac(mac)}`;
}

export function buildGetRingSettingsCommand(mac) {
    return `GET_RING_SETTINGS ${normalizeMac(mac)}`;
}

export function buildGetRingDiagnosticsCommand(mac) {
    return `GET_RING_DIAGNOSTICS ${normalizeMac(mac)}`;
}

export function buildGetGesturesCommand() {
    return "GET_GESTURES";
}

function normalizeGestureId(value, label, supportedIds) {
    if (typeof value !== "string") {
        throw new TypeError(`${label} must be a string.`);
    }

    const trimmed = value.trim();
    const normalized = /^0x[0-9a-f]{2}$/i.test(trimmed)
        ? `0x${trimmed.slice(2).toUpperCase()}`
        : trimmed.toUpperCase();
    if (!supportedIds.includes(normalized)) {
        throw new Error(`Invalid ${label.toLowerCase()}: ${value}`);
    }

    return normalized;
}

export function normalizeGestureTriggerId(triggerId) {
    return normalizeGestureId(
        triggerId,
        "Gesture trigger",
        SUPPORTED_GESTURE_TRIGGERS.map((trigger) => trigger.id),
    );
}

export function normalizeGestureActionId(actionId) {
    return normalizeGestureId(
        actionId,
        "Gesture action",
        SUPPORTED_GESTURE_ACTIONS.map((action) => action.id),
    );
}

export function buildSetGestureCommand(triggerId, actionId) {
    return `SET_GESTURE ${normalizeGestureTriggerId(triggerId)} ${normalizeGestureActionId(actionId)}`;
}

export function buildSetRingDpiCommand(mac, dpiMultiplier) {
    const value = normalizeInteger(
        dpiMultiplier,
        "DPI multiplier",
        RING_SETTINGS_LIMITS.dpiMultiplier,
    );
    return `SET_RING_DPI ${normalizeMac(mac)} ${value}`;
}

export function buildSetRingDeadZoneTimeCommand(mac, deadZoneTimeMs) {
    const value = normalizeInteger(
        deadZoneTimeMs,
        "Dead zone time",
        RING_SETTINGS_LIMITS.deadZoneTimeMs,
    );
    return `SET_RING_DEAD_ZONE_TIME ${normalizeMac(mac)} ${value}`;
}

export function buildSetRingDeadZoneDistanceCommand(mac, deadZoneDistance) {
    const value = normalizeInteger(
        deadZoneDistance,
        "Dead zone distance",
        RING_SETTINGS_LIMITS.deadZoneDistance,
    );
    return `SET_RING_DEAD_ZONE_DISTANCE ${normalizeMac(mac)} ${value}`;
}

export function parseProtocolResponse(rawText) {
    if (typeof rawText !== "string") {
        throw new TypeError("Protocol response must be a string.");
    }

    const lines = rawText
        .replace(/\r\n/g, "\n")
        .replace(/\r/g, "\n")
        .split("\n")
        .filter((line) => line.length > 0);

    if (lines.length === 0) {
        throw new Error("Protocol response was empty.");
    }

    const statusLine = lines.at(-1);
    const okMatch = /^OK(?:\s+(.*))?$/.exec(statusLine);
    if (okMatch) {
        return {
            ok: true,
            statusLine,
            okMessage: okMatch[1] ?? "",
            dataLines: lines.slice(0, -1),
            rawLines: lines,
            rawText: lines.join("\n"),
        };
    }

    const errMatch = /^ERR\s+(\d+)\s+(.+)$/.exec(statusLine);
    if (!errMatch) {
        throw new Error(`Malformed terminal status line: ${statusLine}`);
    }

    return {
        ok: false,
        statusLine,
        errorCode: Number(errMatch[1]),
        errorMessage: errMatch[2],
        dataLines: lines.slice(0, -1),
        rawLines: lines,
        rawText: lines.join("\n"),
    };
}

export function parseKeyValueDataLines(dataLines) {
    if (!Array.isArray(dataLines)) {
        throw new TypeError("Data lines must be an array.");
    }

    const values = {};
    for (const line of dataLines) {
        const match = /^\+\s+([^=]+)=(.*)$/.exec(line);
        if (!match) {
            throw new Error(`Expected key=value data line, got: ${line}`);
        }
        values[match[1].trim()] = match[2].trim();
    }

    return values;
}

export function parseHubInfoResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Hub info response was not successful.");
    }

    const fields = parseKeyValueDataLines(parsed.dataLines);
    return {
        firmwareRevision: fields.fw ?? "—",
        hardwareRevision: fields.hw ?? "—",
        connectedRings: fields.rings ?? "—",
        maxRings: fields.max_rings ?? "—",
        usbPollMs: fields.usb_poll_ms ?? "—",
        scanPolicy: fields.scan_policy ?? "—",
    };
}

export function parseRolesResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Roles response was not successful.");
    }

    return parsed.dataLines.map((line) => {
        const match = /^\+\s+([0-9A-F:]{17})\s+([A-Z]+)$/i.exec(line);
        if (!match) {
            throw new Error(`Expected role entry, got: ${line}`);
        }

        return {
            mac: normalizeMac(match[1]),
            role: normalizeRole(match[2]),
        };
    });
}

export function parseRingsResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Rings response was not successful.");
    }

    return parsed.dataLines.map((line) => {
        const match = /^\+\s+([0-9A-F:]{17})\s+([A-Z]+)\s+(connected|disconnected)$/i.exec(line);
        if (!match) {
            throw new Error(`Expected ring summary entry, got: ${line}`);
        }

        return {
            mac: normalizeMac(match[1]),
            role: normalizeRole(match[2]),
            connected: match[3].toLowerCase() === "connected",
        };
    });
}

export function parseRingInfoResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Ring info response was not successful.");
    }

    const fields = parseKeyValueDataLines(parsed.dataLines);
    return {
        mac: normalizeMac(fields.mac ?? ""),
        role: normalizeRole(fields.role ?? ""),
        connected: fields.connected === "1",
    };
}

export function parseRingSettingsResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Ring settings response was not successful.");
    }

    const fields = parseKeyValueDataLines(parsed.dataLines);
    return {
        mac: normalizeMac(fields.mac ?? ""),
        dpiMultiplier: normalizeInteger(
            fields.dpi_multiplier ?? Number.NaN,
            "DPI multiplier",
            RING_SETTINGS_LIMITS.dpiMultiplier,
        ),
        deadZoneTimeMs: normalizeInteger(
            fields.dead_zone_time_ms ?? Number.NaN,
            "Dead zone time",
            RING_SETTINGS_LIMITS.deadZoneTimeMs,
        ),
        deadZoneDistance: normalizeInteger(
            fields.dead_zone_distance ?? Number.NaN,
            "Dead zone distance",
            RING_SETTINGS_LIMITS.deadZoneDistance,
        ),
        firmwareVersion: String(fields.firmware_version ?? "—"),
    };
}

export function parseRingDiagnosticsResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Ring diagnostics response was not successful.");
    }

    const fields = parseKeyValueDataLines(parsed.dataLines);
    return {
        mac: normalizeMac(fields.mac ?? ""),
        batteryPct: normalizeInteger(
            fields.battery_pct ?? Number.NaN,
            "Battery percentage",
            RING_DIAGNOSTICS_LIMITS.batteryPct,
        ),
        batteryMv: normalizeInteger(
            fields.battery_mv ?? Number.NaN,
            "Battery millivolts",
            RING_DIAGNOSTICS_LIMITS.batteryMv,
        ),
        ringState: String(fields.ring_state ?? "UNKNOWN"),
        sensorState: String(fields.sensor_state ?? "UNKNOWN"),
        bondState: String(fields.bond_state ?? "UNKNOWN"),
        connected: fields.connected === "1",
        calibrationValid: fields.calibration_valid === "1",
        connParamRejected: fields.conn_param_rejected === "1",
        connInterval125Ms: normalizeInteger(
            fields.conn_interval_1_25ms ?? Number.NaN,
            "Connection interval",
            RING_DIAGNOSTICS_LIMITS.connInterval125Ms,
        ),
        diagnosticsVersion: normalizeInteger(
            fields.diagnostics_version ?? Number.NaN,
            "Diagnostics version",
            RING_DIAGNOSTICS_LIMITS.diagnosticsVersion,
        ),
    };
}

export function parseGesturesResponse(response) {
    const parsed = typeof response === "string" ? parseProtocolResponse(response) : response;
    if (!parsed?.ok) {
        throw new Error("Gestures response was not successful.");
    }

    return parsed.dataLines.map((line) => {
        const match = /^\+\s+(0x[0-9A-F]{2})\s+(0x[0-9A-F]{2})(?:\s+(.+))?$/i.exec(line);
        if (!match) {
            throw new Error(`Expected gesture entry, got: ${line}`);
        }

        const triggerId = normalizeGestureTriggerId(match[1]);
        const actionId = normalizeGestureActionId(match[2]);
        const trigger = SUPPORTED_GESTURE_TRIGGERS.find((entry) => entry.id === triggerId);
        const action = SUPPORTED_GESTURE_ACTIONS.find((entry) => entry.id === actionId);

        return {
            triggerId,
            actionId,
            triggerLabel: trigger?.label ?? triggerId,
            actionLabel: action?.label ?? actionId,
            description: String(match[3] ?? ""),
        };
    });
}
