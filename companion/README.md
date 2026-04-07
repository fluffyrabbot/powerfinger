# Companion App

The companion app is optional configuration software, not a runtime dependency.

## Current Repo Status

- A minimal dependency-free Web Serial client now lives in `companion/web/`.
- It talks to the hub's existing USB CDC transport from Chrome or Edge on
  `localhost`.
- Implemented hub commands today: `GET_HUB_INFO`, `GET_ROLES`, `GET_RINGS`,
  `GET_RING_INFO`, `SET_ROLE`, `SWAP_ROLES`, `ROLE_SWAP`, and `FORGET_RING`.
- No Flutter, mobile, or packaged desktop app scaffold is checked in yet.

## Current Scope

- No host-side input interception
- No cloud dependency
- No workflow automation dependency
- No blocking role in P0 validation

## Run It Locally

```bash
scripts/serve-companion-local.sh
```

Then open `http://127.0.0.1:4173` in Chrome or Edge and connect to the hub over
Web Serial.

## What The Scaffold Covers

- Hub snapshot via `GET_HUB_INFO`
- Known-ring list with live connected/disconnected state via `GET_RINGS`
- Per-ring snapshot via `GET_RING_INFO`
- Per-ring reassignment via `SET_ROLE`
- Two-ring swap via `SWAP_ROLES`
- Ring removal via `FORGET_RING`
- Raw command console plus transcript for protocol debugging

## Current Gaps

- No direct BLE mode for single-ring tuning
- No ring-settings relay UI (`SET_RING_*` style commands do not exist yet)
- No OTA UX
- No packaged installable app

## What To Read First

- `docs/COMPANION-APP-ARCH.md` - architecture, transport, and UX sketch
- `docs/MULTI-RING-PROTOCOL.md` - hub command contract
- `companion/web/` - static local app scaffold
- `firmware/hub/components/companion_cdc/` - USB CDC transport
- `firmware/hub/components/companion_session/` - line-oriented session layer
- `firmware/hub/components/companion_protocol/` - command parser and responses

## Expected Future Responsibilities

- Role reassignment UI
- Sensitivity and dead-zone tuning
- Firmware update UX
- Minimal diagnostics once hardware telemetry exists

See `docs/COMPANION-APP-ARCH.md` for the deferred architecture sketch.
