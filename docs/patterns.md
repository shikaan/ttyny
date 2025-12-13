Patterns
---

Writing a story for ttyny can be a challenging task. The engine and the medium
come with limitations that the writer needs to be aware of to create a fun
little adventure.

This collection of patterns is an unsorted list of ways to achieve certain
behaviours that are common, but may not be straightforward to express in ttyny.

## Locked Rooms

To prevent access to a room, you need to add a transition to the location object
with the `"move"` action and set requirements that aren't met. As soon as the
requirements are met, the room will be unlocked.

```jsonc
{
  "name": "Locked Chamber",
  "type": "location",
  "descriptions": [
    "describe here the room from the outside and why it's locked", // state 0 - locked
    "describe here what's in the room, the usual location description" // state 1 - unlocked
  ],
  "transitions": [
    {
      "actions": ["move"],
      "target": "Locked Chamber",
      "from": 0,
      "to": 1,
      "requirements": {
        "inventory": null,
        "locations": null,
        "turns": null,
        "current_location": null,
        "items": ["Door.1"]  // Door must be in state 1 (open)
      }
    }
  ],
  "items": [],
  "exits": ["Previous Room"]
}
```

## Unreacheable Items

Items that cannot be reached until a blocking item reaches a certain state can be modeled like this

```jsonc
{
  "name": "Unreacheable Item",
  "type": "item",
  "descriptions": [
    "describe here why the item is unreacheable",   // state 0 - unreacheable
    "describe here that the item is now reacheable" // state 1 - reacheable (i.e., can be taken)
  ],
  "transitions": [
    {
      "actions": ["take"],
      "target": "Unreacheable Item.0",
      "from": 0,
      "to": 1,
      "requirements": {
        "inventory": null,
        "items": ["Blocking Item.1"], // When Blocking Item is in state 1, this transition goes through
        "locations": null,
        "turns": null,
        "current_location": null
      }
    }
  ],
  "collectible": true,  // Make sure the item is collectible!
  "readable": false
}
```
