<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# R30-OLED-NONE-NONE Disassembly Baseline

This document defines the non-destructive teardown path the first optical ring
hardware drop must preserve.

## Safety First

- Disconnect USB power before opening the ring.
- If the cell is swollen, hot, punctured, or smells of solvent, stop and treat
  it as a battery incident rather than a normal repair.
- Do not pry against the pouch cell.

## Intended Removal Order

1. Remove glide pads or exterior trim pieces that block the primary service seam.
2. Open the shell using its designed seam, not by cutting or cracking the body.
3. Isolate and remove the battery before high-heat rework near the charger or
   USB connector.
4. Remove the PCB or module assembly as a unit where possible.
5. Remove the optical sensor and lens stack only after their orientation and gap
   features are documented.
6. Remove the dome click element without destroying the shell seat.

## Replacement Expectations

- Battery replacement must not require shell destruction.
- Glide pads must be replaceable wear items.
- Service-pad or fixture-interface repair must not require discarding the
  entire shell.
- Sensor and lens must remain field-replaceable even if fine-pitch rework is
  required.

## Failure Conditions

If teardown requires cutting the shell, peeling a battery from permanent epoxy,
or sacrificing the sensor mount to reach the cell, the design fails the current
repairability baseline.
