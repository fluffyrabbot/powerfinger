<!-- SPDX-License-Identifier: CC-BY-SA-4.0 -->
# Floor Playbook — Networking for the First-Timer

> **Status: deferred for the 2026 show.** This is the pitch-mode playbook.
> The current recommended posture for Sensors Converge 2026 is *soft contact
> only* — see [`SOFT-CONTACT-MODE.md`](SOFT-CONTACT-MODE.md). Use this doc
> when all four readiness conditions are met:
> 1. There is a working ring or wand that demos cleanly.
> 2. There is a one-pager you'd hand to a stranger without explanation.
> 3. The GitHub README is presentable on a 2-minute first read.
> 4. At least one consumer-tier BOM has validated power numbers.
>
> Until then, this doc is reference for a later event.

Self-facing prep for working a B2B trade-show floor solo, with no prior
trade-show experience. Re-read on the plane and the morning of day 1 of
whichever future event triggers pitch mode.

The goal of this doc is to lower decision overhead at the booth. If you've
internalized the playbook, you don't have to think about *how* to engage —
only *what* to ask.

## Three things to internalize before walking in

1. **The booth wants you to come over.** It exists to be approached. The booth
   staff are paid to talk to strangers. A trade-show floor is the only
   environment in B2B where cold approach is socially correct. There is
   nothing impolite about walking up; there is something impolite about
   hovering near a booth without engaging.

2. **You are not asking for a favor.** You're carrying a credible open-source
   reference design that uses their parts (or could). A silicon vendor's job
   is to win designs; you are a small but real possible design. Even the
   smallest design wins create reference content their sales team uses to
   close bigger ones. You are not pitching, you are exchanging.

3. **Most conversations will be short and inconclusive. That's correct.** The
   shape of a successful day is not "five long deep partnerships discussed"
   but "twelve five-minute conversations, three of which led to named contacts
   and parts samples." Two-minute conversations are the unit, not exceptions.

## What to bring (physical)

- **The artifact.** A 3D-printed ring shell on your finger, even non-functional,
  is worth more than any printed material. Wear it. People will ask.
- **40–60 one-pagers.** One side, no fluff. BOM total, power numbers, surface
  table, license, GitHub URL, your email. Print on slightly heavy stock so
  they don't feel disposable. Carry in a flat folder, not folded in a pocket.
- **Your own business cards.** Even if you don't have a company. Just name,
  email, project name, GitHub URL. ~$25 from any online printer. Cards make
  you legible; their absence makes you confusing.
- **A small notebook + pen.** Not a phone. Notes on a phone look like
  texting; notes in a notebook look like attention. Write the booth name and
  contact on each page; one page per conversation.
- **A reusable water bottle.** Floors are dehydrating; the water from booth
  fridges runs out by mid-afternoon.
- **Comfortable shoes.** Non-negotiable. Expect 8–12 km of walking per day
  on hard concrete.
- **A bag with a single shoulder strap.** Two-strap backpacks make booth
  approach awkward; tote bags overflow. A messenger bag or single-strap
  works.
- **Phone charger battery + cable.** Show floor outlets are precious.

## What to wear

Aim for "engineer who might be in charge of a small team." Not a suit.
Not a hoodie. The default that fits both:

- Collared shirt or a clean technical pullover (no logos that distract).
- Dark pants or dark jeans without rips. Belt.
- Closed shoes. Sneakers are fine if they look intentional.
- Visible badge clipped at chest height, not on a hip lanyard.

Avoid: company swag from previous jobs, ironic t-shirts, shorts, sandals,
anything that says "I am a hobbyist." You may *be* a hobbyist; you don't
need to advertise.

## When to arrive

- **Day 1, 30 minutes after doors open.** First-thing-in-the-morning is when
  booth staff are still doing setup; mid-morning is when they're warmed up
  and not yet exhausted.
- **Avoid the keynote hour.** Show floor empties out and booth staff are
  watching the speaker stream. Bad time to engage.
- **Last 90 minutes of each day are golden.** Booth staff are tired, less
  busy, and less in pitch mode. Real conversations happen.
- **Don't try to do everything in one day.** Tier S in day 1, Tier A in day 2,
  Tier B as drive-bys.

## The approach

Walking up to a booth cold is the part newbies freeze on. The mechanic:

1. **Walk straight at the booth, not around the perimeter.** Hovering reads
   as uncertainty.
2. **Make eye contact with one staff member.** The least-busy one is fine.
3. **Stand at a comfortable distance — about 1m / 3ft** — and wait for them
   to acknowledge you. They will. They are paid to.
4. **Smile and say your opener.**

That's it. There's no special move. The booth is a transaction surface; you
are a participant.

If the booth is busy:
- Stand a step back, look at the demos, wait for a staffer to free up. Make
  eye contact; they'll wave you in when ready.
- If five minutes pass and no one frees up, leave a one-pager on the table
  with a note ("Re: open-source assistive BLE HID, would like 5 min — [your
  email]") and come back later.
- Don't interrupt a conversation in progress.

## Opening lines (memorize one of these)

Pick one and stick with it for the whole show. Variation makes you waste
mental energy.

- **"Hi — I'm working on an open-source assistive input device. I'd love five
  minutes if you have it."**
- **"Hi — I'm looking at [their product line] for a low-power wearable BLE
  HID design. Are you the right person to ask, or is there an FAE here?"**
- **"Hi — quick question about [specific part number]. Got a minute?"**

The third opener is the strongest because it signals you've done homework.
The first opener is the safest because it works on every booth without
modification.

What to **avoid** as an opener:
- "Can I have one of those pens?" — marks you as a swag-collector and ends
  the conversation before it starts.
- "Tell me about your products." — signals you haven't prepared.
- "I have a startup that's going to revolutionize..." — every booth has heard
  this from twenty people; staff disengage automatically.

## The 30-second pitch (memorize this verbatim)

Once they say "sure, what's the project," deliver this:

> "PowerFinger is an open-source family of assistive input devices — wearable
> rings and a wand-style stylus that work as BLE HID mice. Sub-$25 BOM, fully
> offline, designed for users with limited mobility but useful for everyone.
> Hardware's CERN-OHL-S, firmware's MIT. I'm pre-prototype, evaluating silicon
> partners for the consumer tier. Wanted to ask about [specific thing]."

That's it. About 25 seconds spoken. Practice it out loud five times before the
show. The fluency matters more than the content.

The last sentence — "wanted to ask about [specific thing]" — is the bridge to
the actual ask. Do not skip it. The pitch without an ask is a monologue.

## The 2-minute pitch (only if they ask for more)

If they say "tell me more," add (in this order):

1. **The problem:** "Existing assistive input is $100–350, closed source, dies
   when the vendor disappears."
2. **The wedge:** "Two rings + a USB hub gives mouse + scroll on any surface,
   no host-side software. Pro tier with optical-on-ball works on glass, fabric,
   skin."
3. **Why this booth:** the specific reason their silicon is interesting (the
   per-vendor docs in this directory have these reasons by name).
4. **The ask, repeated:** what you want from them, concretely.

Total: ~90 seconds. Stop talking. Wait for their response.

## Reading the booth — who actually has authority

Booths typically have three kinds of people:

1. **Booth babysitters / contractors.** Often hired through the show; can
   demo the products but can't make decisions. Friendly, generic answers.
2. **Sales / FAE staff from the company.** Real employees, usually
   regional. They have authority to send dev kits, name follow-up contacts,
   and forward a project to relevant teams.
3. **Engineering / marketing leads.** Usually rotate through; not always
   present. The most valuable contacts.

How to tell them apart:

- **Badge color / ribbon** sometimes indicates seniority — note the patterns.
- **Knowledge depth.** A contractor reads the data sheet; an FAE answers
  questions the data sheet doesn't cover.
- **Question deflection.** "Let me grab someone who can answer that" =
  contractor or junior. "That's a good question, the answer is..." =
  the right person.

If your first contact is a babysitter, politely ask: **"Is there an
applications engineer here today, or someone working on [specific topic]?
I'd like to ask one technical question."** This is a normal request; they
will fetch someone or point.

## The ask — when and how

The ask comes after the pitch. Be specific. Each per-vendor doc in this
directory has a "minimum viable" ask and a "stretch" ask. Lead with the
minimum viable; if they're engaged, escalate to the stretch.

Examples:

- "Could I get an STM32WBA Discovery board sample mailed to me?"
- "Who in your developer-relations team handles open-source reference
  designs?"
- "Is there an app note for piezo-film signal conditioning at sub-100µA
  current you'd point me to?"

What you're avoiding:

- Vague asks ("how can we work together?") — these end with "email our
  partnerships inbox," which is a dead letter office.
- Premature stretch asks ("can we be in your Partner Program?") — kills the
  conversation if they can't say yes immediately.
- Asks they have to escalate to make ("can you sponsor my prototype?") —
  not a five-minute-booth-conversation ask. Save for written follow-up.

## Capturing the contact

Three things to leave with:

1. **Their business card or badge scan.** If they offer a card, take it. If
   they only offer a badge scan, accept it (their CRM will email you).
2. **A name and email written into your notebook**, even if you have the
   card. Cards get lost; notebooks don't.
3. **A short note on what they said and what you promised to send.** "Said
   they'll send WBA dev kit. I promised one-pager + GitHub link by Friday."

Give them your one-pager. Hand it over physically. Saying "here's a one-pager"
and putting it in their hands is a stronger commitment device than "I'll email
you info."

## The exit — how to leave gracefully

The hardest move for a newbie. You're done; they're polite; the conversation
is dragging because neither of you wants to be the one to end it. Solutions:

- **The clean exit:** "Great, I have what I need. I'll follow up Friday with
  [the specific thing]. Thanks for the time." Step back, walk away.
- **The deflected exit:** "I should let you talk to other folks. Thanks again."
  Works if the booth is busy.
- **The resource exit:** "Could I grab one of those datasheets / dev kit
  flyers / stickers before I go?" gives you a reason to physically pivot
  away.

Do not linger past the natural end. Two-minute conversations are correct.

## Energy management

The floor will exhaust you in ways that feel unrelated to physical exertion.
Three reasons:

- **Cognitive switching cost.** Each booth is a new context, a new partial-
  product, a new pitch variant. Your brain runs hot.
- **Background noise.** Ambient conversation volume is loud and continuous.
  Underrated source of fatigue.
- **Performance posture.** Smiling and being engaged is real work. By
  3pm of day 1 you will be drained.

Counters:

- **Take breaks every 90 minutes.** Sit down for 15 minutes off the floor.
  Drink water. Eat something with protein. Don't power through.
- **Schedule the easy booths after lunch.** Tier B drive-bys at 2pm when
  energy is lowest. Save Tier S for morning and last-90-minutes slots.
- **Don't drink at the receptions on day 1.** Day 2 is fine. Day 1 you need
  the morning brain.
- **Sleep before the show.** Not the cliché it sounds.

## Newbie mistakes — numbered list to re-read

1. **Trying to "network" without a specific ask.** Wasted conversations.
   Always have a concrete ask in mind.
2. **Failing to ask for a contact.** Conversation goes well, you walk away
   with no email, you can't follow up. Always get the email or the card.
3. **Pitching the *project* instead of the *ask*.** They don't need to fall
   in love with PowerFinger to send you a dev kit. Make the ask the bridge.
4. **Using filler words to fill silence.** Silence after the pitch is them
   thinking. Let it sit. Newbies fill silence with weakening language ("I
   mean, it's still early, just an idea, kind of...") that destroys the
   pitch's credibility.
5. **Asking for too much too early.** A first conversation should not end
   with "can you give us $50k"; it should end with "can you mail me a dev
   kit."
6. **Ignoring badge ribbons.** Speaker, sponsor, exhibitor, press — these
   matter. Note them and adjust accordingly.
7. **Failing to take notes during the conversation.** You'll forget the
   details by the next booth. Notebook open, pen visible, write while they
   talk.
8. **Saying you'll follow up "next week."** "Friday" or "Tuesday" is a real
   commitment; "next week" is a euphemism for "never." Pick a real day.
9. **Wearing the badge on a lanyard at hip height.** No one can read it.
   Clip to chest.
10. **Spending more than 10 minutes at any single booth.** If a conversation
    runs long, take it off the floor: "Could we grab coffee tomorrow morning?"
    Booth staff have to move other visitors through.

## Same-night ritual

Every night of the show, before bed, sit down for 30 minutes and:

1. **Triage your business cards / notebook pages.** Sort into: must-follow-up,
   maybe, and discard.
2. **Send "nice to meet" emails to Tier S contacts the same night.** Just two
   sentences each. Reference the specific thing you discussed. Attach the
   one-pager. The same-day email is the difference between "I remember you"
   and "who?"
3. **Update this directory.** Log what each booth said in the per-vendor doc
   under a "post-show notes" section.

The same-night ritual is the difference between a productive show and a pile
of business cards that decays into nothing over the following month.

## Mental reframe for the day

Each conversation is small. Each conversation is also independent. A bad
conversation costs you nothing. A good conversation gains you a contact and
maybe a dev kit. The expected value of approaching a booth is positive.

You are not networking. You are doing reconnaissance for a small open-source
project that has a real story. The story sells itself once you give it a
chance to be heard. Your job is to give it that chance, twelve times.

## See also

- [`README.md`](README.md) — triage and floor-walk order.
- Per-vendor docs — specific asks, conversation cues, risks for each booth.
- [`../OUTREACH.md`](../OUTREACH.md) — the broader partnership pipeline that
  Sensors Converge feeds into.
