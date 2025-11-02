Create a new world header for my text adventure engine in path /assets using the existing API in /src/world.h. Follow these strict rules and extended narrative guidelines:

World Theme:

- Theme: [INSERT THEME HERE]
- Contemporary, grounded. No fantasy tropes (no magic, runes, dragons, golems, torches, enchanted items, etc.).
- If the scenario involves a crime or investigation: clearly identify (in comments) the victim and the player’s role/relationship.

Objects and States:

- Every item has only meaningful state changes. Picking up (TAKE) or dropping (DROP) MUST NOT change its state.
- Do NOT use state names like "held", "carried", "taken", "collected", "in pocket".
- Prefer 1–3 semantic states per item (e.g. "sealed" -> "opened", "encrypted" -> "decoded"). Single-state if it never meaningfully changes.
- Only these actions may cause transitions: ACTION_TYPE_USE, ACTION_TYPE_EXAMINE (and optionally ACTION_TYPE_MOVE for a location or environmental structural change).
- No composite / combination puzzles. Each item’s transitions are independent.
- States must be concise, lower-case or simple title case, comma-free phrases (the engine already separates description vs. state). They should reflect revealed lore or investigative progression, not player possession.
- Final (last) state of investigative clue items should explicitly reveal new information (e.g. culprit identity, motive element, timeline gap) rather than vague text.
- Avoid ambiguous verbs or future tense in state names ("will open", "activating"). Use factual result snapshots ("opened", "decoded threat", "forged signature").
- No generic placeholder object names like "Weapon" or "Item". Use specific nouns: "Stage Knife", "Access Log", "Mixer Panel", etc.
- Large, fixed, or infrastructural objects (cameras, consoles, whiteboards, terminals, door frames) should be non-collectible.
- Intangible or purely informational sources (emails, logs, displays) are non-collectible; they change state only by EXAMINE or USE and remain in place.
- Portable physical artifacts (papers, small tools, handheld devices, samples) can be collectible.
- Ensure collectibility is coherent: do not mark obviously immovable things as collectible.

Transitions:

- DO NOT use ACTION_TYPE_TAKE or ACTION_TYPE_DROP in transitions unless a rare reversible environmental change is logically tied (generally avoid; default = never).
- Each transition struct uses ACTION*TYPE*\* constants exactly as defined (ACTION_TYPE_USE, ACTION_TYPE_EXAMINE, ACTION_TYPE_MOVE).
- Provide only the minimal transitions needed (most clue items are a single one-way transition from index 0 -> 1).
- Readable textual/document/display items (papers, profiles, reports, emails, logs, boards) MUST define both ACTION_TYPE_EXAMINE and ACTION_TYPE_USE transitions from initial state index 0 to their final state (dual triggers). If the item has only one state, no transitions are defined.
- current_state always starts at index 0.
- Buffer counts must match: number of state strings = maximum referenced state index + 1; transitions reference valid indices.

Buffers and Layout:

- All per-location items_t buffers must have length == total item count (pad with NULL).
- Provide individual items_t for each location, then aggregate into all_locations and all_objects.
- Locations normally have exactly one state (describe ambience) unless a meaningful structural change (e.g. "locked" -> "unlocked") is part of the design; if so, use at most 2 states.
- Location descriptions: comma-separated adjectives/short fragments (no full sentences).

Lore & Clue Design (IMPORTANT):

- Include multiple clue items beyond the minimum needed for victory (at least 2 extra optional clues).
- Optional clues still reveal concrete bits of lore (motive fragment, timeline confirmation, tampering evidence) via their final state descriptions.
- Distinguish required vs. optional clues in comments above the digest.
- Final states of required clues must collectively justify victory (identity + motive + opportunity OR alternative verified chain).
- Be specific: reference roles or entities ("manager alone with victim", "forged insurance addendum"), not vague pronouns ("they argued").
- Maintain internal consistency: if a state reveals "planted blood mismatch", no other state should contradict forensic logic.
- Avoid redundancy: each clue adds a distinct facet (timeline, motive, method, cover-up, identity).

Victory Conditions:

- Provide at least 2 independent victory condition sets (A, B, ...). Each set: 2–3 specific item final states.
- Victory if ANY one set is fully satisfied.
- List each set explicitly in comments above the digest function, referencing item names and target final states.
- Do not require optional clue items for victory (they enrich narrative only).
- Victory check must be evaluated before failure (success prioritized if simultaneous).

Failure Condition (Timed):

- Exactly one timed failure trigger.
- Trigger starts when a specific item reaches a designated final state (e.g. using an access log, examining a broadcast panel) or a location transition occurs.
- Store the trigger turn in a static variable inside digest; after N turns (e.g. 10) without victory, return GAME_STATE_DEAD.
- Comment clearly which item/state triggers the countdown and the turn limit.
- Do not tie the timer to collecting an item; tie it to USE/EXAMINE (or MOVE) of a logical object (e.g. accessing sensitive system alerts adversary).
- Failure message (end_game[GAME_STATE_DEAD]) should specify exactly what the player failed to assemble or the narrative consequence of delay.

Inventory:

- Create inventory buffer sized to total items; initial used = 0.
- Collectible items have trait bit 0 set (0b00000001). Non-collectibles keep traits = 0.
- Do not set collectible bit for fixed infrastructure / intangible info sources.

Constraints:

- No external includes beyond ../src/world.h.
- Keep object and state names short; avoid ambiguous gerunds that imply an action rather than a state.
- No cross-item dependency logic inside digest—only direct checks of each item’s current_state.
- No hidden global flags beyond static timer variable(s) for failure.

Digest Function Requirements:

- Comment block must include:
  - Victim identity (if applicable).
  - Player role / relationship.
  - Victory condition sets (items + required final state names).
  - Timed failure trigger and turn limit.
  - Optional clue items list (not required for win).
- Implement the timer logic robustly: only set the trigger turn once.
- Return order: check victory first, then timed failure expiry, else continue.

End Game Messages (world_t.end_game):

- Index 0 (GAME_STATE_CONTINUE): Atmosphere / unresolved status, mention unresolved gaps generically (not victory criteria).
- Index 1 (GAME_STATE_VICTORY): Explicitly enumerate the verified chain causing success (identity, motive, opportunity/method or alternative set).
- Index 2 (GAME_STATE_DEAD): Explicitly state which chain failed to assemble and narrative outcome of delay (e.g. evidence corrupted, suspect escapes, media misdirection).

Naming & Wording Specificity:

- Use concrete role nouns (manager, technician, curator, broker) rather than "someone", "they".
- Replace generic objects: "Device" -> "Audio Recorder", "Weapon" -> "Stage Knife", "Document" -> "Contract Addendum".
- Final clue states should be minimally wordy but specific (e.g. "forged mgr signature", "decoded threat audio", "timeline mgr alone", "planted blood mismatch").

Coherence Checklist (self-verify before finalizing):

1. All fixed hardware / terminals non-collectible.
2. Intangible info (emails, logs, boards) non-collectible.
3. Portable papers / handheld devices collectible (unless a deliberate exception documented).
4. No TAKE/DROP transitions present.
5. Each item with transitions has exactly one forward semantic transition (unless a justified location change).
6. Victory sets reference existing items & final states verbatim.
7. Timed failure only one trigger, timer length documented.
8. Inventory length matches total items; all location item buffers length == total items.
9. No contradictory clue states.
10. End game strings reflect the actual implemented logic.

Output Requirements:

- Provide a single .h file with:
  - State description buffers
  - Transition buffers
  - Item declarations
  - Per-location item buffers (padded to total)
  - Location exit buffers and location objects
  - all_locations and all_objects aggregates
  - Inventory buffer
  - Digest function with detailed comments
  - world_t instance with enriched end_game messages
- Do not include extraneous commentary outside comments in code.

Remember:

- Minimal, explicit, internally consistent, investigation-driven states.
- Rich clue density: more clues than strictly needed for victory, but only some are required.
- Prioritize clarity over obfuscation; the puzzle is sequencing evidence, not guessing parser phrasing.

Quick Action Checklist (author final pass):

- Theme set and contemporary? (No fantasy artifacts)
- Victim & player role commented (if investigative scenario)
- Total items count consistent across: all_objects, per-location buffers, inventory length
- Collectible traits only on portable physical items
- Non-collectible traits on fixed hardware / intangible info sources
- Each item: 1–3 states, meaningful; no possession states
- Transitions only USE / EXAMINE (MOVE only if structural); no TAKE/DROP transitions
- Each transitioning item: exactly one forward transition unless justified
- Required victory sets (≥2) each list 2–3 item final states in comments
- Optional clues listed in comments and NOT required in digest logic
- Timed failure single trigger: item/state clearly commented, timer length set
- Digest: victory evaluated before timer expiry check
- End_game[0/1/2] messages align with unresolved / success chain / failure consequence
- No contradictory lore among final states
- Specific nouns (Stage Knife, Access Log, etc.), no generic “Weapon/Device/Item”
- Final clue states reveal concrete facts (identity, motive, method, cover-up, timeline)
- Location states single (or justified two-state) only
- No extraneous includes or logic beyond direct state checks + timer
