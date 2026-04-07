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
