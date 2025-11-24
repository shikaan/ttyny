Failure modes
---

## Design Goals

The very reason I started developing this engine is that I wanted to create an
experience like Zork, but without the limitations of what the developer of the
story thought of beforehand.

In other words, I would like to provide a gaming experience such that there will
be no error cases: the player should be able to try _everything_ and 
_everything_ should be possible.

However, we want to give the designers tools to design stories that make sense:
they should be able to think about feelings they want to convey, worlds that
players should explore, and quests to complete. 

All these things should be driven by numbers and state transitions, as they
need to be deterministic to give designers power over that.

## Failures in the game

Currently there is a parser that extracts the action and the target of a given
action.

Failures can occur for the following reasons:
  1. action is not recognised
  2. target is not recognised
  3. (action, target) is valid, but requirements are not met
  4. input is invalid

The first occurs because the action is something we don't support. The second
occurs because the target is not recognised as one in the list of available
targets (e.g., item not in the room). 

Both can still happen due to genuine mistakes in the parser though (i.e., the 
model does not recognise) the input words as matching targets or actions.

The third is straightforward: the game designer decides when an action is 
allowed.

The fourth is where the magic is: we should progressively reduce the likelyhood
of falling into 4.

## Challenges

### Actions

Actions can be unrecognised, in which case the game should build some narrative
around the action as though it occurred, but it should change nothing in the
game state. 

This is a fine line to walk: the risk is setting expectations in the player by
describing something, even if it's purely for narrative purposes.

### Objects

When the object is not available though can be for three reasons:
* the object is genuinely not there
* the object is there but there is a synonym problem (the parser doesn't
understand it's the same thing)
* the object was hallucinated in a description and it feels like it should be
available

This last type of error is weird
