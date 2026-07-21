<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Factory Reply Evidence Intake

This directory is the source-controlled landing zone for Shenzhen / Seeed
factory replies tied to the current packet:

- Primary path: `USB-HUB` PCB fabrication and assembly quote plus
  connector/enclosure DFM review.
- Optional path: `R30-OLED-NONE-NONE` DFM/pre-fab review annex only, if the
  factory explicitly accepts the annex.

Create one dated directory per factory reply. Do not reuse a directory for a
later thread, revised quote, or second factory contact.

## Scaffold Command

From the repository root:

```sh
scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD
```

If the same reply also includes an accepted R30 DFM/pre-fab annex:

```sh
scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD --include-r30-annex
```

The default directory shape is:

```text
docs/sensors-converge-2026/factory-replies/YYYY-MM-DD-seeed-fusion-propagate-usb-hub-reply/
  README.md
  incoming/
  quote-files/
  source-return/
  USB-HUB-SUBSTITUTIONS.md
  USB-HUB-DFM-ASKS.md
  SOURCE-RETURN-INDEX.md
  R30-OLED-NONE-NONE-DFM-ANNEX/        # only with --include-r30-annex
```

## Capture Rules

- Put raw email exports, screenshots, and attachments in `incoming/`.
- Put quote sheets, invoices, DFM reports, BOM/CPL/POS import notes, and
  payment terms in `quote-files/`.
- Put factory-returned editable source and CAM-only artifacts in
  `source-return/`, then summarize them in `SOURCE-RETURN-INDEX.md`.
- Transcribe proposed substitutions into `USB-HUB-SUBSTITUTIONS.md`.
- Transcribe non-substitution DFM requests into `USB-HUB-DFM-ASKS.md`.
- Reference the dated directory from
  [`../SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`](../SHENZHEN-FACTORY-RESPONSE-CAPTURE.md)
  before changing `docs/REFERENCE-MANUFACTURERS.md` or
  `docs/VENDOR-VERIFICATION.md`.

Quote files prove that a quote or factory answer was received. They do not
prove build yield, realized COGS, physical fit, RF behavior, connector
retention, source compatibility, or independent distributor availability.
