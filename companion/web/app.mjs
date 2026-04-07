import {
    SERIAL_OPTIONS,
    SUPPORTED_GESTURE_ACTIONS,
    SUPPORTED_GESTURE_TRIGGERS,
    SUPPORTED_ROLES,
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
    parseGesturesResponse,
    parseHubInfoResponse,
    parseProtocolResponse,
    parseRingDiagnosticsResponse,
    parseRingInfoResponse,
    parseRingSettingsResponse,
    parseRingsResponse,
} from "./protocol.mjs";

const COMMAND_TIMEOUT_MS = 4000;

const state = {
    port: null,
    reader: null,
    writer: null,
    readLoopPromise: null,
    readBuffer: "",
    disconnecting: false,
    pendingResponse: null,
    hubInfo: null,
    roles: [],
    gestures: {},
    ringSettings: {},
    ringDiagnostics: {},
};

const elements = {
    connectButton: document.querySelector("#connect-button"),
    refreshButton: document.querySelector("#refresh-button"),
    supportHint: document.querySelector("#support-hint"),
    connectionState: document.querySelector("#connection-state"),
    statusNote: document.querySelector("#status-note"),
    hubInfoGrid: document.querySelector("#hub-info-grid"),
    roleCards: document.querySelector("#role-cards"),
    swapForm: document.querySelector("#swap-form"),
    swapMacA: document.querySelector("#swap-mac-a"),
    swapMacB: document.querySelector("#swap-mac-b"),
    swapButton: document.querySelector("#swap-button"),
    gestureForm: document.querySelector("#gesture-form"),
    gestureGrid: document.querySelector("#gesture-grid"),
    saveGesturesButton: document.querySelector("#save-gestures-button"),
    commandForm: document.querySelector("#command-form"),
    commandInput: document.querySelector("#command-input"),
    sendButton: document.querySelector("#send-button"),
    responseStatus: document.querySelector("#response-status"),
    responseOutput: document.querySelector("#response-output"),
    transcriptOutput: document.querySelector("#transcript-output"),
    quickButtons: Array.from(document.querySelectorAll("[data-command]")),
};

function setPillState(element, text, tone) {
    element.textContent = text;
    element.className = "";
    element.classList.add(
        element.id === "response-status" ? "response-status" : "meta-pill",
        tone,
    );
}

function setStatusNote(message, tone = "meta-pill-neutral") {
    setPillState(elements.statusNote, message, tone);
}

function setConnectionState(message, tone = "meta-pill-neutral") {
    setPillState(elements.connectionState, message, tone);
}

function setResponseStatus(message, tone = "response-status-neutral") {
    setPillState(elements.responseStatus, message, tone);
}

function appendTranscript(direction, line) {
    const stamp = new Date().toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit",
    });
    const prefix = direction === "out" ? ">>" : "<<";
    const nextLine = `[${stamp}] ${prefix} ${line}`;

    if (elements.transcriptOutput.textContent.includes("Session transcript will appear here.")) {
        elements.transcriptOutput.textContent = nextLine;
    } else {
        elements.transcriptOutput.textContent += `\n${nextLine}`;
    }

    elements.transcriptOutput.scrollTop = elements.transcriptOutput.scrollHeight;
}

function setBusy(disabled) {
    const connected = Boolean(state.port);

    elements.refreshButton.disabled = disabled || !connected;
    elements.commandInput.disabled = disabled || !connected;
    elements.sendButton.disabled = disabled || !connected;
    elements.swapButton.disabled = disabled || !connected || state.roles.length < 2;
    elements.swapMacA.disabled = disabled || !connected || state.roles.length < 2;
    elements.swapMacB.disabled = disabled || !connected || state.roles.length < 2;
    elements.saveGesturesButton.disabled = disabled || !connected;

    for (const button of elements.quickButtons) {
        button.disabled = disabled || !connected;
    }

    elements.gestureGrid.querySelectorAll("[data-gesture-select]").forEach((control) => {
        control.disabled = disabled || !connected;
    });

    elements.roleCards.querySelectorAll("[data-apply-role], [data-forget-ring], [data-inspect-ring], [data-load-settings], [data-load-diagnostics], [data-save-settings], [data-role-select], [data-setting-input]").forEach((control) => {
        control.disabled = disabled || !connected;
    });
}

function updateConnectionUi() {
    const connected = Boolean(state.port);
    elements.connectButton.textContent = connected ? "Disconnect Hub" : "Connect Hub";
    elements.refreshButton.disabled = !connected;
    elements.commandInput.disabled = !connected;
    elements.sendButton.disabled = !connected;
    elements.saveGesturesButton.disabled = !connected;

    if (!connected) {
        setConnectionState("Disconnected", "meta-pill-neutral");
    }

    renderGestureControls();
    renderSwapSelectors();
    renderRoleCards();
}

function renderHubInfo() {
    const values = state.hubInfo ?? {
        firmwareRevision: "—",
        hardwareRevision: "—",
        connectedRings: "—",
        maxRings: "—",
        usbPollMs: "—",
        scanPolicy: "—",
    };

    const stats = [
        ["Firmware", values.firmwareRevision],
        ["Hardware", values.hardwareRevision],
        ["Connected Rings", values.connectedRings],
        ["Capacity", values.maxRings],
        ["USB Poll", values.usbPollMs === "—" ? "—" : `${values.usbPollMs} ms`],
        ["Scan Policy", values.scanPolicy],
    ];

    elements.hubInfoGrid.innerHTML = stats
        .map(
            ([label, value]) => `
                <div class="stat-card">
                    <dt>${label}</dt>
                    <dd>${value}</dd>
                </div>
            `,
        )
        .join("");
}

function pruneCachedRingData() {
    const nextSettings = {};
    const nextDiagnostics = {};

    for (const entry of state.roles) {
        if (state.ringSettings[entry.mac]) {
            nextSettings[entry.mac] = state.ringSettings[entry.mac];
        }
        if (entry.connected && state.ringDiagnostics[entry.mac]) {
            nextDiagnostics[entry.mac] = state.ringDiagnostics[entry.mac];
        }
    }

    state.ringSettings = nextSettings;
    state.ringDiagnostics = nextDiagnostics;
}

function formatConnectionIntervalMs(connInterval125Ms) {
    const milliseconds = Number(connInterval125Ms) * 1.25;

    if (!Number.isFinite(milliseconds)) {
        return "—";
    }
    if (Number.isInteger(milliseconds)) {
        return `${milliseconds} ms`;
    }

    return `${milliseconds.toFixed(2).replace(/0+$/, "").replace(/\.$/, "")} ms`;
}

function describeDiagnosticsStatus(diagnostics) {
    if (!diagnostics) {
        return "";
    }

    const parts = [
        `${diagnostics.batteryPct}%`,
        `${diagnostics.batteryMv} mV`,
        formatConnectionIntervalMs(diagnostics.connInterval125Ms),
    ];

    return parts.join(" · ");
}

function describeDiagnosticsFlags(diagnostics) {
    const parts = [
        diagnostics.calibrationValid ? "calibrated" : "calibration pending",
        diagnostics.connParamRejected ? "conn params rejected" : "conn params accepted",
        diagnostics.connected ? "session connected" : "session disconnected",
    ];

    return parts.join(" · ");
}

function renderRoleCards() {
    if (state.roles.length === 0) {
        const message = state.port
            ? "No persisted role assignments are stored on the hub yet."
            : "Connect the hub, then refresh to load known rings.";

        elements.roleCards.innerHTML = `<article class="empty-state">${message}</article>`;
        return;
    }

    elements.roleCards.innerHTML = state.roles
        .map((entry) => {
            const settings = state.ringSettings[entry.mac] ?? null;
            const diagnostics = state.ringDiagnostics[entry.mac] ?? null;
            const options = SUPPORTED_ROLES.map((role) => {
                const selected = role === entry.role ? " selected" : "";
                return `<option value="${role}"${selected}>${role}</option>`;
            }).join("");

            const tuningStatus = settings
                ? `Loaded live tuning${settings.firmwareVersion ? ` · FW ${settings.firmwareVersion}` : ""}`
                : entry.connected
                    ? "Load tuning to read the ring's live BLE settings."
                    : "Reconnect this ring to inspect or change tuning.";
            const diagnosticsStatus = diagnostics
                ? `Loaded live telemetry · ${describeDiagnosticsStatus(diagnostics)}`
                : entry.connected
                    ? "Load battery and diagnostics readback from the ring."
                    : "Reconnect this ring to inspect battery and diagnostics.";
            const diagnosticsBody = diagnostics
                ? `
                            <div class="ring-telemetry-grid">
                                <div class="telemetry-stat">
                                    <span>Battery</span>
                                    <strong>${diagnostics.batteryPct}% · ${diagnostics.batteryMv} mV</strong>
                                </div>
                                <div class="telemetry-stat">
                                    <span>Ring state</span>
                                    <strong>${diagnostics.ringState}</strong>
                                </div>
                                <div class="telemetry-stat">
                                    <span>Sensor path</span>
                                    <strong>${diagnostics.sensorState}</strong>
                                </div>
                                <div class="telemetry-stat">
                                    <span>Bond state</span>
                                    <strong>${diagnostics.bondState}</strong>
                                </div>
                                <div class="telemetry-stat">
                                    <span>Connection interval</span>
                                    <strong>${formatConnectionIntervalMs(diagnostics.connInterval125Ms)}</strong>
                                </div>
                                <div class="telemetry-stat">
                                    <span>Flags</span>
                                    <strong>${describeDiagnosticsFlags(diagnostics)}</strong>
                                </div>
                            </div>
                        `
                : `<p class="ring-telemetry-empty">No live diagnostics loaded for this ring yet.</p>`;

            return `
                <article class="ring-card" data-mac="${entry.mac}">
                    <div class="ring-card-header">
                        <div>
                            <h3>Known ring</h3>
                            <p class="mac-label">${entry.mac}</p>
                        </div>
                        <div class="ring-card-status">
                            <span class="ring-badge">${entry.role}</span>
                            <span class="ring-badge ${entry.connected ? "" : "ring-badge-muted"}">
                                ${entry.connected ? "Connected" : "Disconnected"}
                            </span>
                        </div>
                    </div>

                    <label class="field">
                        <span>Assigned role</span>
                        <select data-role-select="${entry.mac}">
                            ${options}
                        </select>
                    </label>

                    <section class="ring-settings-panel">
                        <div class="ring-settings-header">
                            <div>
                                <h4>Live tuning</h4>
                                <p>${tuningStatus}</p>
                            </div>
                            <button
                                class="secondary-button"
                                type="button"
                                data-load-settings="${entry.mac}"
                                ${entry.connected ? "" : "disabled"}
                            >
                                Load Tuning
                            </button>
                        </div>

                        <div class="ring-settings-grid">
                            <label class="field">
                                <span>DPI multiplier</span>
                                <input
                                    type="number"
                                    min="1"
                                    max="255"
                                    step="1"
                                    value="${settings?.dpiMultiplier ?? ""}"
                                    placeholder="${entry.connected ? "10 = 1.0x" : "Ring offline"}"
                                    data-setting-input="dpiMultiplier"
                                    data-setting-mac="${entry.mac}"
                                    ${entry.connected ? "" : "disabled"}
                                >
                            </label>

                            <label class="field">
                                <span>Dead zone time (ms)</span>
                                <input
                                    type="number"
                                    min="0"
                                    max="2000"
                                    step="1"
                                    value="${settings?.deadZoneTimeMs ?? ""}"
                                    placeholder="${entry.connected ? "50" : "Ring offline"}"
                                    data-setting-input="deadZoneTimeMs"
                                    data-setting-mac="${entry.mac}"
                                    ${entry.connected ? "" : "disabled"}
                                >
                            </label>

                            <label class="field">
                                <span>Dead zone distance</span>
                                <input
                                    type="number"
                                    min="0"
                                    max="255"
                                    step="1"
                                    value="${settings?.deadZoneDistance ?? ""}"
                                    placeholder="${entry.connected ? "10" : "Ring offline"}"
                                    data-setting-input="deadZoneDistance"
                                    data-setting-mac="${entry.mac}"
                                    ${entry.connected ? "" : "disabled"}
                                >
                            </label>
                        </div>
                    </section>

                    <section class="ring-settings-panel ring-telemetry-panel">
                        <div class="ring-settings-header">
                            <div>
                                <h4>Battery + diagnostics</h4>
                                <p>${diagnosticsStatus}</p>
                            </div>
                            <button
                                class="secondary-button"
                                type="button"
                                data-load-diagnostics="${entry.mac}"
                                ${entry.connected ? "" : "disabled"}
                            >
                                Load Telemetry
                            </button>
                        </div>

                        ${diagnosticsBody}
                    </section>

                    <div class="ring-card-actions">
                        <button class="secondary-button" type="button" data-inspect-ring="${entry.mac}">
                            Inspect
                        </button>
                        <button
                            class="secondary-button"
                            type="button"
                            data-save-settings="${entry.mac}"
                            ${entry.connected ? "" : "disabled"}
                        >
                            Save Tuning
                        </button>
                        <button class="secondary-button" type="button" data-apply-role="${entry.mac}">
                            Apply Role
                        </button>
                        <button class="secondary-button danger-button" type="button" data-forget-ring="${entry.mac}">
                            Forget Ring
                        </button>
                    </div>
                </article>
            `;
        })
        .join("");

    elements.roleCards.querySelectorAll("[data-apply-role]").forEach((button) => {
        button.addEventListener("click", () => handleSetRole(button.dataset.applyRole));
    });

    elements.roleCards.querySelectorAll("[data-forget-ring]").forEach((button) => {
        button.addEventListener("click", () => handleForgetRing(button.dataset.forgetRing));
    });

    elements.roleCards.querySelectorAll("[data-inspect-ring]").forEach((button) => {
        button.addEventListener("click", () => handleInspectRing(button.dataset.inspectRing));
    });

    elements.roleCards.querySelectorAll("[data-load-settings]").forEach((button) => {
        button.addEventListener("click", () => handleLoadRingSettings(button.dataset.loadSettings));
    });

    elements.roleCards.querySelectorAll("[data-load-diagnostics]").forEach((button) => {
        button.addEventListener("click", () => handleLoadRingDiagnostics(button.dataset.loadDiagnostics));
    });

    elements.roleCards.querySelectorAll("[data-save-settings]").forEach((button) => {
        button.addEventListener("click", () => handleSaveRingSettings(button.dataset.saveSettings));
    });
}

function renderSwapSelectors() {
    const placeholder = `<option value="">Select a ring</option>`;
    const options = state.roles
        .map((entry) => `<option value="${entry.mac}">${entry.mac} · ${entry.role} · ${entry.connected ? "connected" : "disconnected"}</option>`)
        .join("");

    elements.swapMacA.innerHTML = placeholder + options;
    elements.swapMacB.innerHTML = placeholder + options;
    elements.swapMacA.disabled = !state.port || state.roles.length < 2;
    elements.swapMacB.disabled = !state.port || state.roles.length < 2;
    elements.swapButton.disabled = !state.port || state.roles.length < 2;
}

function renderGestureControls() {
    elements.gestureGrid.innerHTML = SUPPORTED_GESTURE_TRIGGERS
        .map((trigger) => {
            const selectedAction = state.gestures[trigger.id] ?? "0x00";
            const options = SUPPORTED_GESTURE_ACTIONS.map((action) => {
                const selected = action.id === selectedAction ? " selected" : "";
                return `<option value="${action.id}"${selected}>${action.label}</option>`;
            }).join("");

            return `
                <label class="field gesture-field">
                    <span>${trigger.label}</span>
                    <select data-gesture-select="${trigger.id}" ${state.port ? "" : "disabled"}>
                        ${options}
                    </select>
                </label>
            `;
        })
        .join("");

    elements.saveGesturesButton.disabled = !state.port;
}

function setLastResponse(response) {
    if (!response) {
        setResponseStatus("Idle", "response-status-neutral");
        elements.responseOutput.textContent = "No response yet.";
        return;
    }

    if (response.ok) {
        setResponseStatus("OK", "response-status-success");
    } else {
        setResponseStatus(`ERR ${response.errorCode}`, "response-status-danger");
    }
    elements.responseOutput.textContent = response.rawText;
}

function ensureWebSerialSupport() {
    if ("serial" in navigator) {
        setPillState(
            elements.supportHint,
            "Web Serial available. Use Chrome or Edge on localhost.",
            "meta-pill-success",
        );
        return true;
    }

    setPillState(
        elements.supportHint,
        "This browser does not expose Web Serial. Use Chrome or Edge on localhost.",
        "meta-pill-danger",
    );
    elements.connectButton.disabled = true;
    return false;
}

async function connectToHub() {
    if (!ensureWebSerialSupport()) {
        return;
    }

    const port = await navigator.serial.requestPort();
    await port.open(SERIAL_OPTIONS);

    state.port = port;
    state.writer = port.writable.getWriter();
    state.disconnecting = false;
    state.readBuffer = "";
    state.readLoopPromise = startReadLoop();

    setConnectionState("Connected", "meta-pill-success");
    setStatusNote("Hub connected. Loading snapshot…", "meta-pill-success");
    updateConnectionUi();
    await refreshAll();
}

function rejectPendingResponse(reason) {
    if (!state.pendingResponse) {
        return;
    }

    clearTimeout(state.pendingResponse.timeoutId);
    state.pendingResponse.reject(reason);
    state.pendingResponse = null;
}

async function disconnectFromHub(reason = "Disconnected.") {
    if (!state.port) {
        updateConnectionUi();
        return;
    }

    state.disconnecting = true;
    rejectPendingResponse(new Error(reason));

    if (state.reader) {
        try {
            await state.reader.cancel();
        } catch (error) {
            // Ignore cancellation races during unplug or reload.
        }
    }

    if (state.readLoopPromise) {
        try {
            await state.readLoopPromise;
        } catch (error) {
            // Read-loop cleanup should not block UI teardown.
        }
    }

    if (state.writer) {
        try {
            state.writer.releaseLock();
        } catch (error) {
            // Ignore lock-release races during teardown.
        }
    }

    try {
        await state.port.close();
    } catch (error) {
        // Ignore close races after physical unplug.
    }

    state.port = null;
    state.reader = null;
    state.writer = null;
    state.readLoopPromise = null;
    state.readBuffer = "";
    state.hubInfo = null;
    state.roles = [];
    state.gestures = {};
    state.ringSettings = {};
    state.ringDiagnostics = {};
    state.disconnecting = false;

    updateConnectionUi();
    renderHubInfo();
    renderSwapSelectors();
    renderRoleCards();
    setLastResponse(null);
    setConnectionState("Disconnected", "meta-pill-neutral");
    setStatusNote(reason, "meta-pill-neutral");
}

async function startReadLoop() {
    const decoder = new TextDecoder();
    state.reader = state.port.readable.getReader();

    try {
        while (state.port && !state.disconnecting) {
            const { value, done } = await state.reader.read();
            if (done) {
                break;
            }

            if (value) {
                handleIncomingText(decoder.decode(value, { stream: true }));
            }
        }

        const tail = decoder.decode();
        if (tail) {
            handleIncomingText(tail);
        }
    } catch (error) {
        if (!state.disconnecting) {
            setStatusNote(`Serial read failed: ${error.message}`, "meta-pill-danger");
        }
    } finally {
        try {
            state.reader.releaseLock();
        } catch (error) {
            // Ignore duplicate release during teardown.
        }
        state.reader = null;
    }
}

function handleIncomingText(chunk) {
    state.readBuffer += chunk;

    while (state.readBuffer.includes("\n")) {
        const newlineIndex = state.readBuffer.indexOf("\n");
        const line = state.readBuffer.slice(0, newlineIndex).replace(/\r$/, "");
        state.readBuffer = state.readBuffer.slice(newlineIndex + 1);

        if (line.length === 0) {
            continue;
        }

        appendTranscript("in", line);
        collectResponseLine(line);
    }
}

function collectResponseLine(line) {
    if (!state.pendingResponse) {
        return;
    }

    state.pendingResponse.lines.push(line);

    if (/^OK(?:\s+.*)?$/.test(line) || /^ERR\s+\d+\s+.+$/.test(line)) {
        clearTimeout(state.pendingResponse.timeoutId);
        const rawText = `${state.pendingResponse.lines.join("\n")}\n`;
        const response = parseProtocolResponse(rawText);
        state.pendingResponse.resolve(response);
        state.pendingResponse = null;
    }
}

async function sendCommand(command) {
    if (!state.port || !state.writer) {
        throw new Error("Connect to a hub before sending commands.");
    }
    if (state.pendingResponse) {
        throw new Error("Another command is still waiting for a response.");
    }

    const normalized = command.trim();
    if (!normalized) {
        throw new Error("Command cannot be empty.");
    }

    appendTranscript("out", normalized);
    const payload = new TextEncoder().encode(`${normalized}\n`);

    const responsePromise = new Promise((resolve, reject) => {
        const timeoutId = setTimeout(() => {
            if (!state.pendingResponse) {
                return;
            }

            state.pendingResponse = null;
            reject(new Error(`Timed out waiting for response to: ${normalized}`));
        }, COMMAND_TIMEOUT_MS);

        state.pendingResponse = {
            lines: [],
            resolve,
            reject,
            timeoutId,
        };
    });

    try {
        await state.writer.write(payload);
    } catch (error) {
        rejectPendingResponse(error);
        throw error;
    }

    return responsePromise;
}

async function refreshHubInfo() {
    const response = await sendCommand("GET_HUB_INFO");
    setLastResponse(response);

    if (!response.ok) {
        throw new Error(`${response.errorCode} ${response.errorMessage}`);
    }

    state.hubInfo = parseHubInfoResponse(response);
    renderHubInfo();
}

async function refreshRoles() {
    const response = await sendCommand("GET_RINGS");
    setLastResponse(response);

    if (!response.ok) {
        throw new Error(`${response.errorCode} ${response.errorMessage}`);
    }

    state.roles = parseRingsResponse(response);
    pruneCachedRingData();
    renderRoleCards();
    renderSwapSelectors();
}

async function refreshGestures() {
    const response = await sendCommand(buildGetGesturesCommand());
    setLastResponse(response);

    if (!response.ok) {
        throw new Error(`${response.errorCode} ${response.errorMessage}`);
    }

    const nextGestures = {};
    for (const entry of parseGesturesResponse(response)) {
        nextGestures[entry.triggerId] = entry.actionId;
    }

    for (const trigger of SUPPORTED_GESTURE_TRIGGERS) {
        if (!nextGestures[trigger.id]) {
            nextGestures[trigger.id] = "0x00";
        }
    }

    state.gestures = nextGestures;
    renderGestureControls();
}

async function refreshAll() {
    setBusy(true);

    try {
        await refreshHubInfo();
        await refreshRoles();
        await refreshGestures();
        setStatusNote("Snapshot refreshed from the hub.", "meta-pill-success");
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

function getRingSettingInput(mac, settingName) {
    return elements.roleCards.querySelector(
        `[data-setting-input="${settingName}"][data-setting-mac="${mac}"]`,
    );
}

async function loadRingSettings(mac, announce = true) {
    const response = await sendCommand(buildGetRingSettingsCommand(mac));
    setLastResponse(response);

    if (!response.ok) {
        throw new Error(`${response.errorCode} ${response.errorMessage}`);
    }

    state.ringSettings[mac] = parseRingSettingsResponse(response);
    renderRoleCards();

    if (announce) {
        setStatusNote(`Loaded live tuning for ${mac}.`, "meta-pill-success");
    }
}

async function loadRingDiagnostics(mac, announce = true) {
    const response = await sendCommand(buildGetRingDiagnosticsCommand(mac));
    setLastResponse(response);

    if (!response.ok) {
        throw new Error(`${response.errorCode} ${response.errorMessage}`);
    }

    state.ringDiagnostics[mac] = parseRingDiagnosticsResponse(response);
    renderRoleCards();

    if (announce) {
        setStatusNote(`Loaded battery and diagnostics for ${mac}.`, "meta-pill-success");
    }
}

async function runCommand(command, successMessage) {
    setBusy(true);

    try {
        const response = await sendCommand(command);
        setLastResponse(response);

        if (!response.ok) {
            throw new Error(`${response.errorCode} ${response.errorMessage}`);
        }

        if (successMessage) {
            setStatusNote(successMessage, "meta-pill-success");
        }

        await refreshAll();
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleSetRole(mac) {
    const select = elements.roleCards.querySelector(`[data-role-select="${mac}"]`);
    if (!select) {
        return;
    }

    const command = buildSetRoleCommand(mac, select.value);
    await runCommand(command, `Updated ${mac} to ${select.value}.`);
}

async function handleForgetRing(mac) {
    const shouldForget = window.confirm(
        `Forget ${mac}? This removes the persisted role assignment from the hub.`,
    );
    if (!shouldForget) {
        return;
    }

    const command = buildForgetRingCommand(mac);
    await runCommand(command, `Forgot ${mac}.`);
}

async function handleInspectRing(mac) {
    setBusy(true);

    try {
        const response = await sendCommand(`GET_RING_INFO ${mac}`);
        setLastResponse(response);

        if (!response.ok) {
            throw new Error(`${response.errorCode} ${response.errorMessage}`);
        }

        const ringInfo = parseRingInfoResponse(response);
        if (ringInfo.connected) {
            await loadRingDiagnostics(mac, false);
            const diagnostics = state.ringDiagnostics[mac];
            setStatusNote(
                `${ringInfo.mac} is connected as ${ringInfo.role}. Battery ${describeDiagnosticsStatus(diagnostics)}.`,
                "meta-pill-success",
            );
            return;
        }

        setStatusNote(
            `${ringInfo.mac} is ${ringInfo.connected ? "connected" : "disconnected"} as ${ringInfo.role}.`,
            "meta-pill-success",
        );
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleLoadRingSettings(mac) {
    setBusy(true);

    try {
        await loadRingSettings(mac);
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleLoadRingDiagnostics(mac) {
    setBusy(true);

    try {
        await loadRingDiagnostics(mac);
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleSaveRingSettings(mac) {
    const dpiInput = getRingSettingInput(mac, "dpiMultiplier");
    const deadZoneTimeInput = getRingSettingInput(mac, "deadZoneTimeMs");
    const deadZoneDistanceInput = getRingSettingInput(mac, "deadZoneDistance");
    if (!dpiInput || !deadZoneTimeInput || !deadZoneDistanceInput) {
        setStatusNote("Could not find the tuning fields for that ring.", "meta-pill-danger");
        return;
    }

    let commands = [];
    try {
        commands = [
            buildSetRingDpiCommand(mac, dpiInput.value),
            buildSetRingDeadZoneTimeCommand(mac, deadZoneTimeInput.value),
            buildSetRingDeadZoneDistanceCommand(mac, deadZoneDistanceInput.value),
        ];
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
        return;
    }

    setBusy(true);

    try {
        for (const command of commands) {
            const response = await sendCommand(command);
            setLastResponse(response);

            if (!response.ok) {
                throw new Error(`${response.errorCode} ${response.errorMessage}`);
            }
        }

        await loadRingSettings(mac, false);
        setStatusNote(`Saved live tuning for ${mac}.`, "meta-pill-success");
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleSwap(event) {
    event.preventDefault();

    try {
        const command = buildSwapRolesCommand(
            elements.swapMacA.value,
            elements.swapMacB.value,
        );
        await runCommand(command, "Swapped the selected ring roles.");
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    }
}

async function handleSaveGestures(event) {
    event.preventDefault();

    const commands = [];
    try {
        for (const trigger of SUPPORTED_GESTURE_TRIGGERS) {
            const select = elements.gestureGrid.querySelector(
                `[data-gesture-select="${trigger.id}"]`,
            );
            if (!select) {
                throw new Error(`Could not find the gesture control for ${trigger.label}.`);
            }
            commands.push(buildSetGestureCommand(trigger.id, select.value));
        }
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
        return;
    }

    setBusy(true);

    try {
        for (const command of commands) {
            const response = await sendCommand(command);
            setLastResponse(response);

            if (!response.ok) {
                throw new Error(`${response.errorCode} ${response.errorMessage}`);
            }
        }

        await refreshGestures();
        setStatusNote("Updated the hub gesture mappings.", "meta-pill-success");
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

async function handleManualCommand(event) {
    event.preventDefault();

    setBusy(true);
    try {
        const response = await sendCommand(elements.commandInput.value);
        setLastResponse(response);

        if (response.ok) {
            setStatusNote("Command completed.", "meta-pill-success");
        } else {
            setStatusNote(
                `Hub returned ${response.errorCode} ${response.errorMessage}.`,
                "meta-pill-warning",
            );
        }
    } catch (error) {
        setStatusNote(error.message, "meta-pill-danger");
    } finally {
        setBusy(false);
    }
}

function bindEvents() {
    elements.connectButton.addEventListener("click", async () => {
        if (state.port) {
            await disconnectFromHub("Disconnected by user.");
            return;
        }

        try {
            setStatusNote("Waiting for Web Serial device selection…", "meta-pill-warning");
            await connectToHub();
        } catch (error) {
            setStatusNote(error.message, "meta-pill-danger");
        }
    });

    elements.refreshButton.addEventListener("click", refreshAll);
    elements.swapForm.addEventListener("submit", handleSwap);
    elements.gestureForm.addEventListener("submit", handleSaveGestures);
    elements.commandForm.addEventListener("submit", handleManualCommand);

    for (const button of elements.quickButtons) {
        button.addEventListener("click", async () => {
            elements.commandInput.value = button.dataset.command ?? "";
            await handleManualCommand(new Event("submit"));
        });
    }

    if ("serial" in navigator && typeof navigator.serial.addEventListener === "function") {
        navigator.serial.addEventListener("disconnect", async (event) => {
            if (event.target === state.port) {
                await disconnectFromHub("Hub disconnected.");
            }
        });
    }
}

function init() {
    ensureWebSerialSupport();
    bindEvents();
    renderHubInfo();
    renderRoleCards();
    renderGestureControls();
    renderSwapSelectors();
    setLastResponse(null);
    updateConnectionUi();
}

init();
