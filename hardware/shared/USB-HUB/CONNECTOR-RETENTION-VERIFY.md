<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Connector Retention Verification

Fill this document with first-board mechanical evidence for the direct-plug hub.

## Connector Load Path

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Connector insertion/removal load is shared by PCB + enclosure | Required | | |
| Connector strain is not carried solely by solder joints | Required | | |
| Normal host insertion does not twist or pry the enclosure open | Required | | |

## Serviceability

| Check | Required Outcome | Status | Notes |
|-------|------------------|--------|-------|
| Enclosure can reopen without destroying PCB retention | Required | | |
| Boot/reset access remains usable after enclosure install | Required | | |
| Connector style leaves enough clearance for adjacent ports | Required | | |

## Sign-Off

- Update `MANIFEST.md` with the current closure state
- If any row is red, do not start secondary hub or accessory hardware
