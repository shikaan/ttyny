Generating World with LLMs
---

## Preconditions
The context already contains a conversation where you clarified the story you 
want and why you want it that way.


## Prompt

```md
Using the attached json schema, write a story for my game. Ask me clarifying 
questions along the way to make sure the story is coherent and fun to play.

Keep these constraints into account
- Do not assume the player knows the story: scatter hints of what needs to be 
  done in different places
- Every item has only meaningful state changes. Picking up (TAKE) or dropping 
  (DROP) MUST NOT change its state to "held" or "on the ground"
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
  transitions from initial state index 0 to their final state (dual triggers). 
  If the item has only one state, no transitions are defined.
- Provide individual items_t for each location, then aggregate into 
  all_locations and all_objects.
- Victory check must be evaluated before failure (success prioritized if 
  simultaneous).
- Keep object and state names short; avoid ambiguous gerunds that imply an 
  action rather than a state.
- If a location cannot be reached (e.g., A -> B, but B does not go back to A) 
  there needs to be a justification (falling from a wall, a closed door...)
- When an item can be used, the transition and the state need to add to the 
  story; for example, give hints on where to go and what the aim of the story is
- Tell the player what they are supposed to do through states and descriptions.
  Overshare, make sure the player knows what they are doing.
- Objects that carry clues and direction are readable and theit state
  descriptions will be reported verbatim to the user. Make them nice ro read: if
  it's a log, format them like a log ([timestamp] message), if it's a document
  format it like a document (# title\n rest of message). In any case, make it 
  personal - so the user can realate to it
```
