# SPDX-License-Identifier: MIT

class ShenzhenLaneValidator < BaseValidator
  def validate
    validate_quote_packet(read(ContractCheck::SHENZHEN_QUOTE_PACKET_REL))
    validate_first_contact(read(ContractCheck::SHENZHEN_FIRST_CONTACT_REL))
    validate_pairing(read(ContractCheck::SHENZHEN_PAIRING_REL))
    validate_reference_manufacturers(read(ContractCheck::REFERENCE_MANUFACTURERS_REL))
    validate_vendor_verification(read(ContractCheck::VENDOR_VERIFICATION_REL))
    validate_response_capture(read(ContractCheck::SHENZHEN_RESPONSE_CAPTURE_REL))
    validate_usb_hub_bom(read(ContractCheck::USB_HUB_BOM_REL))
    validate_usb_hub_manifest(read(ContractCheck::USB_HUB_MANIFEST_REL))
  end

  private

  def validate_quote_packet(text)
    reject_phrase(
      text,
      "~20x12mm",
      "quote packet must not carry the old compact USB-HUB geometry",
    )
    reject_phrase(
      text,
      "~20 x 12 mm",
      "quote packet must not carry the old compact USB-HUB geometry",
    )
    require_phrase(
      text,
      "Send-now quote path: `USB-HUB` PCB fab/assembly quote plus connector/enclosure DFM review.",
      "quote packet must keep USB-HUB as send-now quote path",
    )
    require_phrase(
      text,
      "Optional annex: `R30-OLED-NONE-NONE` DFM/pre-fab review only.",
      "quote packet must keep R30 as annex-only DFM/pre-fab review",
    )
    require_phrase(
      text,
      "Quote only the send-now hub path unless the factory explicitly accepts the ring annex as a DFM/pre-fab review input.",
      "quote packet must not promote the R30 annex to the send-now quote path",
    )
    require_phrase(
      text,
      "- `docs/sensors-converge-2026/SHENZHEN-FACTORY-RESPONSE-CAPTURE.md`",
      "quote packet must list the factory response-capture form in the USB-HUB send-now packet",
    )
    require_phrase(
      text,
      "keeping quote-only statements separate from verified evidence.",
      "quote packet must keep quote-only statements separate from verified evidence",
    )
    require_phrase(
      text,
      "scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD",
      "quote packet must point first replies at the dated factory-reply scaffold",
    )
    require_phrase(
      text,
      "docs/sensors-converge-2026/factory-replies/",
      "quote packet must name the repo-local factory-reply evidence root",
    )
    require_phrase(
      text,
      "`OUTBOUND-DRAFT.md`, generated from `SHENZHEN-FIRST-CONTACT-TEMPLATE.md` with subject/body placeholders preserved and the optional R30 body section included only in annex-mode exports",
      "quote packet must document the generated outbound draft",
    )
    require_phrase(
      text,
      "`ATTACHMENT-MANIFEST.md`, generated from the same packet-document sections that drive the copied USB-HUB files and optional R30 annex files",
      "quote packet must document the section-driven attachment manifest",
    )
    require_phrase(
      text,
      "fails if the generated attachment manifest drifts from the packet-doc file sections",
      "quote packet must keep attachment-manifest drift guard explicit",
    )
    require_phrase(
      text,
      "PCB: stepped direct USB-A dongle, not an earlier compact placeholder.",
      "quote packet must name the USB-HUB stepped direct-plug board",
    )
    require_phrase(
      text,
      "The packet documents a host-side USB-A nose and wider `54 x 26 mm`",
      "quote packet must name the USB-HUB direct-plug body envelope",
    )
    require_phrase(
      text,
      "no-go for build release until printed host-fit, clamp-alignment, service-hatch reach, adjacent-port clearance, and connector-retention evidence are recorded",
      "quote packet must keep USB-HUB build release gated on measured mechanical evidence",
    )
  end

  def validate_first_contact(text)
    require_phrase(
      text,
      "PowerFinger USB-HUB PCB assembly quote + enclosure/connector DFM review",
      "first contact subject must lead with USB-HUB quote and DFM review",
    )
    require_phrase(
      text,
      "Please quote PCB fabrication and assembly for the current `USB-HUB` source packet.",
      "first contact must ask for USB-HUB PCB fabrication and assembly quote",
    )
    require_phrase(
      text,
      "Current mechanical outline: stepped direct-plug USB-A dongle with a host-side USB-A nose and wider `54 x 26 mm` module/service body.",
      "first contact must state the current USB-HUB stepped direct-plug geometry",
    )
    require_phrase(
      text,
      "Please treat the ring files as DFM/pre-fab review only.",
      "first contact must keep R30 files as DFM/pre-fab review only",
    )
    require_phrase(
      text,
      "Do not quote ring PCB fabrication or assembly from the annex.",
      "first contact must keep R30 annex out of fabrication quote",
    )
    require_phrase(
      text,
      "Optional, only if the factory accepts pre-fab review: `scripts/export-shenzhen-seeed-quote-packet.sh --include-r30-annex`",
      "first contact must keep the R30 annex opt-in",
    )
    require_phrase(
      text,
      "`OUTBOUND-DRAFT.md`: generated subject/body draft with placeholders; annex body text appears only in annex-mode exports",
      "first contact must point to the generated outbound draft",
    )
    require_phrase(
      text,
      "`ATTACHMENT-MANIFEST.md`: generated send list aligned to the packet doc",
      "first contact must point to the generated attachment manifest",
    )
  end

  def validate_pairing(text)
    require_phrase(
      text,
      "Use [`SHENZHEN-SEEED-QUOTE-PACKET.md`](SHENZHEN-SEEED-QUOTE-PACKET.md) as the current starter packet: hub quote first, ring DFM/pre-fab review until the board-house output constraints and physical fit/stackup evidence close.",
      "pairing doc must keep hub quote first and R30 review gated",
    )
    require_phrase(
      text,
      "Use [`SHENZHEN-FIRST-CONTACT-TEMPLATE.md`](SHENZHEN-FIRST-CONTACT-TEMPLATE.md) for the first Shenzhen / Seeed outbound message so the contact starts from the same hub-quote / ring-review boundary as the packet.",
      "pairing doc must tie first contact to the hub-quote / ring-review boundary",
    )
  end

  def validate_reference_manufacturers(text)
    require_phrase(
      text,
      "Candidate for `USB-HUB` first PCB/assembly quote, with `R30-OLED-NONE-NONE` only as a DFM/pre-fab review annex",
      "reference manufacturers must keep Seeed row scoped to USB-HUB quote and R30 annex",
    )
    require_phrase(
      text,
      "**Placeholder. No quote or batch has been run yet.**",
      "reference manufacturers must keep Seeed row placeholder until evidence exists",
    )
    require_phrase(
      text,
      "Use the send-now path for `USB-HUB`; treat ring materials as annex-only DFM/pre-fab review until board-house output constraints and physical fit/stackup evidence close.",
      "reference manufacturers must keep USB-HUB send-now and R30 annex-only guidance",
    )
    require_phrase(
      text,
      "First reply evidence must land under `docs/sensors-converge-2026/factory-replies/` and pass the response-capture update gate before this row changes.",
      "reference manufacturers must keep first factory replies gated through repo-local evidence",
    )
  end

  def validate_vendor_verification(text)
    require_phrase(
      text,
      "Incoming factory quote files, proposed substitutions, DFM asks, and returned source artifacts should first be captured once under",
      "vendor verification must route factory claims through first-reply evidence intake",
    )
    require_phrase(
      text,
      "Factory sourcing claims remain quote-only here until independently checked against distributor or received-sample evidence.",
      "vendor verification must keep factory sourcing claims quote-only until independently checked",
    )
  end

  def validate_response_capture(text)
    require_phrase(
      text,
      "Use this sheet to paste factory answers back into the repo without converting ambiguous email prose into hidden design decisions.",
      "response capture must stay the explicit factory-answer paste-back surface",
    )
    require_phrase(
      text,
      "Primary quote path: `USB-HUB` PCB fabrication and assembly quote, connector/enclosure DFM review, serviceability feedback, and proposed substitutions.",
      "response capture must keep USB-HUB as primary quote path",
    )
    require_phrase(
      text,
      "Optional annex path: `R30-OLED-NONE-NONE` DFM/pre-fab review only, if the factory explicitly accepts the annex.",
      "response capture must keep R30 annex optional and DFM/pre-fab only",
    )
    require_phrase(
      text,
      "`Quoted` means the factory offered price, MOQ, lead-time, availability, DFM, or source-return information. It does not mean verified.",
      "response capture must define Quoted separately from Verified",
    )
    require_phrase(
      text,
      "`Verified` means the repo has direct evidence",
      "response capture must define Verified as repo-backed evidence",
    )
    require_phrase(
      text,
      "Do not update `docs/REFERENCE-MANUFACTURERS.md` from this sheet until a real quote, run, or direct verification exists.",
      "response capture must gate reference-manufacturer updates on real evidence",
    )
    require_phrase(
      text,
      "Quote-vs-verified status",
      "response capture must keep quote-vs-verified status explicit",
    )
    require_phrase(
      text,
      "BDFL decisions are explicit and not inferred from factory wording.",
      "response capture must keep BDFL decisions explicit",
    )
    require_phrase(
      text,
      "scripts/scaffold-shenzhen-seeed-factory-reply.py YYYY-MM-DD",
      "response capture must name the dated factory-reply scaffold command",
    )
    require_phrase(
      text,
      "docs/sensors-converge-2026/factory-replies/YYYY-MM-DD-seeed-fusion-propagate-usb-hub-reply/",
      "response capture must document the dated factory-reply evidence shape",
    )
    require_phrase(
      text,
      "The dated evidence directory is recorded in the response header.",
      "response capture checklist must require the dated evidence directory",
    )
  end

  def validate_usb_hub_bom(text)
    reject_phrase(
      text,
      "~20x12mm",
      "USB-HUB BOM must not preserve the old compact dongle geometry",
    )
    reject_phrase(
      text,
      "~20 x 12 mm",
      "USB-HUB BOM must not preserve the old compact dongle geometry",
    )

    rows = CSV.parse(text, headers: true)
    pcb = row_by_ref(rows, "PCB1")
    enclosure = row_by_ref(rows, "ENCL1")
    switch = row_by_ref(rows, "SW1")

    require_phrase(
      [pcb["Package"], pcb["Notes"]].join(" "),
      "stepped USB-A nose + 54 x 26 mm body",
      "USB-HUB BOM PCB row must name the stepped direct-plug board geometry",
    )
    require_phrase(
      pcb["Notes"],
      "54 x 26 mm module/service body",
      "USB-HUB BOM PCB row must name the source body envelope without implying fit proof",
    )
    require_phrase(
      pcb["Notes"],
      "host fit and adjacent-port clearance remain unmeasured",
      "USB-HUB BOM PCB row must keep host-fit evidence unmeasured",
    )
    require_phrase(
      [enclosure["Package"], enclosure["Notes"]].join(" "),
      "stepped direct-plug shell",
      "USB-HUB BOM enclosure row must not collapse the enclosure to a generic USB stick",
    )
    require_phrase(
      enclosure["Notes"],
      "stepped USB-A nose plus 54 x 26 mm body",
      "USB-HUB BOM enclosure row must match the current stepped body",
    )
    require_phrase(
      enclosure["Notes"],
      "host-fit and adjacent-port clearance remain unmeasured",
      "USB-HUB BOM enclosure row must keep physical fit evidence unmeasured",
    )
    require_phrase(
      switch["Notes"],
      "stepped direct-plug body",
      "USB-HUB BOM switch row must not size controls against the old placeholder envelope",
    )
  end

  def validate_usb_hub_manifest(text)
    reject_phrase(
      text,
      "~20x12mm",
      "USB-HUB manifest must not carry the old compact geometry",
    )
    reject_phrase(
      text,
      "~20 x 12 mm",
      "USB-HUB manifest must not carry the old compact geometry",
    )
    require_phrase(
      text,
      "a host-side USB-A nose, a wider `54 x 26 mm` module/service body",
      "USB-HUB manifest must preserve the stepped direct-plug body description",
    )
    require_phrase(
      text,
      "no physical fit evidence is recorded yet",
      "USB-HUB manifest must stay honest about missing physical fit evidence",
    )
    require_phrase(
      text,
      "Adjacent-port clearance is checked with the stepped USB-A nose and wider body, not assumed from the schematic",
      "USB-HUB manifest must gate adjacent-port clearance on measured evidence",
    )
  end

  def read(path)
    call(:expect_file, path, "Shenzhen lane document")
    File.read(call(:full, path))
  end

  def require_phrase(text, phrase, message)
    fail!(message) unless normalize(text).include?(normalize(phrase))
  end

  def reject_phrase(text, phrase, message)
    fail!(message) if normalize(text).include?(normalize(phrase))
  end

  def row_by_ref(rows, ref)
    row = rows.find { |candidate| candidate["Ref"] == ref }
    fail!("USB-HUB BOM missing #{ref}") unless row
    row
  end

  def normalize(text)
    text.to_s.gsub(/\s+/, " ").strip
  end
end
