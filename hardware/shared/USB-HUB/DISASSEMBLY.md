<!-- SPDX-License-Identifier: CERN-OHL-S-2.0 -->
# USB-HUB Disassembly Baseline

This is the teardown baseline for the shared hub accessory.

## Intended Removal Order

1. Remove the service hatch with a non-marring spudger if pad access is needed.
2. Remove the reversible clamp hardware at `MH1` / `MH2`.
3. Open the enclosure at its designed seam.
4. Lift the PCB from the wider body first; do not pry on the USB connector shell
   or use the USB-A nose as a lever.
5. Service the switch, LED, service pads, or connector individually where
   possible.

## Replacement Expectations

- USB connector replacement must not require discarding the enclosure.
- Switch and LED repairs should not require cutting the board out of the shell.
- The enclosure should survive at least one routine service cycle.
- `MH1` / `MH2` clamp hardware should be replaceable without changing the PCB.

## Failure Conditions

If the enclosure must be cracked open or the connector has no practical repair
path short of discarding the board, the design fails this baseline.

If the service hatch cannot expose `EN`, `BOOT_N`, UART, power, and ground pads
without removing the whole board, the enclosure fails the active-lane recovery
baseline.
