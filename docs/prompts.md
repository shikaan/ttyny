## LFM2-1.2B-Q4_k_m.gguf

### Full Example

```
You write 2 sentences. Use 'you' to address the reader.
LOCATION: Kitchen (smelly,sinister) [dark]
ITEMS: Lamp (old,oil) [off]
EXITS: Cellar, Garden
OUTPUT: You are in a dark room. You don't see much except for an old oil lamp.
Too dangerous to proceed with lights off.

ACTION: light lamp

LOCATION: Kitchen (smelly,sinister) [bright]
ITEMS: Lamp (old,oil) [on]
EXITS: Cellar, Garden
You light the lamp and the room brightens, revealing two doors: one to the
outside and a smaller one, probably leading to a basement.

ACTION: go outside

LOCATION: Garden (moonlit,average) [-]
ITEMS: Mailbox (red) [closed]
EXITS: Street, Kitchen
The moonlit garden is pretty average. A driveway to the street features a red
mailbox on the side. The kitchen door is behind you.
```

Problem: it picks up context from the prompt and ignores the story

### Explicit Example Marks

```
You write 2 sentences. Use 'you' to address the reader.
## EXAMPLE START
LOCATION: Kitchen (smelly,sinister) [dark]
ITEMS:
 - Lamp (old,oil) [off]
EXITS:
 - Cellar
 - Garden
OUTPUT: You are in a dark room. You don't see much except for an old oil lamp.
Too dangerous to proceed with lights off.
## EXAMPLE END
```
