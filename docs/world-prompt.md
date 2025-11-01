Create a new world header for my text adventure engine in path zork/assets/<world_name>.h using the existing API in src/world.h. Follow these strict rules:

World Theme:
- Theme: [INSERT THEME HERE, e.g. “abandoned biotech lab”, “subway evacuation”, etc.]
- Contemporary or near-future. No fantasy tropes (no magic, runes, dragons, golems, torches, enchanted items, etc.).

Objects and States:
- Every item has only meaningful state changes. Picking up (take) or dropping must NOT change its state.
- Do NOT use any state names like “held”, “carried”, “taken”, “collected”, “in pocket”.
- Prefer 1–3 semantic states per item (e.g. “unused” -> “activated”, “sealed” -> “opened”).
- If an item does not transform via use/examine, give it a single state (length 1) and no transitions.
- Only these actions may cause transitions: USE, EXAMINE (and possibly MOVE for location-specific transitions).
- Avoid implicit interpretation traps: do not include items whose names imply actions like “Torch” (use “Work Light (off/on)” only if necessary and explicit).
- No item combination puzzles. Each objective is independent.

Transitions:
- TAKE and DROP must never appear in transitions unless they restore a reversible environmental state (generally avoid; default is no transition).
- Each transition struct uses ACTION_TYPE_* constants exactly as seen in existing worlds (ACTION_TYPE_USE, ACTION_TYPE_EXAMINE, ACTION_TYPE_MOVE, etc.).
- States arrays and transitions buffers must match counts; each object’s current_state index starts at the first state.

Buffers and Layout:
- All item arrays for locations must have length == total items count, even if many NULL.
- Provide individual items_t buffers per location, then aggregate all_locations and all_objects buffers at the end.
- Locations have exactly one state unless a location-based puzzle requires a change (e.g. “sealed” -> “unsealed”).
- Keep naming consistent and human-readable; descriptions are comma-separated adjectives (no sentences).

Victory Condition:
- Implement a digest function that returns GAME_STATE_VICTORY only when a defined set of target items reach their final states (list them in comments).
- Example pattern: scan badge, archive drive, open exit, etc.
- Victory check should prioritize success over failure if simultaneous.

Failure Condition (include this):
- Add a timed failure: After a triggering event (e.g. examining a “Security Console” or “Tripped Sensor”) start a turn countdown (e.g. 10 turns). Store the trigger turn as a static variable in the digest function. If time expires before victory, return GAME_STATE_DEAD.
- Only one such timed failure trigger.

Inventory:
- Create player inventory buffer sized to total items; initial used = 0.
- Collectible items have trait bit 0 set (0b00000001). Non-collectibles keep traits = 0.

Comments:
- At top, add narrative summary, explicit victory requirements, and failure timer note.
- For each item that changes state, add a concise comment describing its purpose.

Constraints:
- No external includes beyond ../src/world.h.
- Keep object/state names short, lowercase or simple title case (e.g. “Access Badge”, “Exit Door”, “Sensor Array”).
- Avoid ambiguous verbs in names (“Processor” is fine; “Activator” might confuse).
- No combining or cross-item dependency logic inside digest—just direct state checks.

Output Format:
- Return only the header file content in one code block using the path-based fenced format:
