Create a world for my text adventure engine in `/assets/` using the existing API
in `/src/world.h`. 

The adventure should be in the spirit of zork: should be about exploring and
collecting.

Follow these guidelines:

- Every item has only meaningful state changes. Picking up (TAKE) or dropping 
(DROP) MUST NOT change its state.
- Do NOT use state names like "held", "carried", "taken", "collected", 
"in pocket".
- No generic placeholder object names like "Weapon" or "Item". Use specific 
nouns: "Stage Knife", "Access Log", "Mixer Panel", etc.
- Large, fixed, or infrastructural objects (cameras, consoles, whiteboards, 
terminals, door frames) should be non-collectible.
- Intangible or purely informational sources (emails, logs, displays) are 
non-collectible; they change state only by EXAMINE or USE and remain in place.
- Ensure collectibility is coherent: do not mark obviously immovable things as 
collectible.
- Readable textual/document/display items (papers, profiles, reports, emails, 
logs, boards) MUST define both ACTION_TYPE_EXAMINE and ACTION_TYPE_USE 
transitions from initial state index 0 to their final state (dual triggers). If 
the item has only one state, no transitions are defined.
- Buffer counts must match: number of state strings = maximum referenced state 
index + 1; transitions reference valid indices.
- All per-location items_t buffers must have length == total item count 
(pad with NULL).
- Provide individual items_t for each location, then aggregate into 
all_locations and all_objects.
- Provide at least 2 independent victory condition sets (A, B, ...). Each 
set: 2–3 specific item final states.
- Victory if ANY one set is fully satisfied.
- Victory check must be evaluated before failure (success prioritized if 
simultaneous).
- No external includes beyond ../src/world.h.
- Keep object and state names short; avoid ambiguous gerunds that imply an 
action rather than a state.
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
- If a location cannot be reached (e.g., A -> B, but B does not go back to A) 
there needs to be a justification (falling from a wall, a closed door...)
- When an item can be used, the transition and the state need to add to the 
story; for example, give hints on where to go and what the aim of the story is
- Tell the player what they are supposed to do through states and descriptions.
Overshare, make sure the player knows what they are doing.
- Objects that carry clues and direction are `readable` and theit state
descriptions will be reported verbatim to the user. Make them nice ro read: if
it's a log, format them like a log ([timestamp] message), if it's a document
format it like a document (# title\n rest of message). In any case, make it 
personal - so the user can realate to it


Quick Action Checklist (author final pass):

- Theme set and contemporary? (No fantasy artifacts)
- Total items count consistent across: all_objects, per-location buffers, inventory length
- Collectible traits only on portable physical items
- Non-collectible traits on fixed hardware / intangible info sources
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
