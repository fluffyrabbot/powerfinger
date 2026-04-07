# Companion App

The companion app is optional configuration software, not a runtime dependency.

## Current Repo Status

- No Flutter, web, desktop, or mobile app scaffold is checked in yet.
- The current deliverables are protocol and architecture docs plus the hub-side
  control surface the future app will talk to.
- The hub firmware already exposes the text companion protocol over USB CDC.
- Implemented hub commands today: `GET_HUB_INFO`, `GET_ROLES`, `SET_ROLE`,
  `SWAP_ROLES`, `ROLE_SWAP`, and `FORGET_RING`.

## Current Scope

- No host-side input interception
- No cloud dependency
- No workflow automation dependency
- No blocking role in P0 validation

## What To Read First

- `docs/COMPANION-APP-ARCH.md` - architecture, transport, and UX sketch
- `docs/MULTI-RING-PROTOCOL.md` - hub command contract
- `firmware/hub/components/companion_cdc/` - USB CDC transport
- `firmware/hub/components/companion_session/` - line-oriented session layer
- `firmware/hub/components/companion_protocol/` - command parser and responses

## Expected Future Responsibilities

- Role reassignment UI
- Sensitivity and dead-zone tuning
- Firmware update UX
- Minimal diagnostics once hardware telemetry exists

See `docs/COMPANION-APP-ARCH.md` for the deferred architecture sketch.
