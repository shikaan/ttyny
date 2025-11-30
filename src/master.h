#pragma once

#include "ai.h"
#include "buffers.h"
#include "map.h"
#include "world/world.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// This class represent the Game Master. It's the AI recounting the state of
// the world, describing situations and locations. It has a memory such that
// descriptions don't have to be recreated from scratch every time.
// Unless differently specified, descriptions will be memorised.
typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
  map_t *descriptions;
} master_t;

// Allocate the master and related resources
master_t *masterCreate(world_t *world);

// Describe the given location and writes the output to provided string.
// The input string will be truncated.
void masterDescribeLocation(master_t *, const location_t *, string_t *);

// Describe the given object and writes the output to the provided string.
// The input string will be truncated.
void masterDescribeObject(master_t *, const object_t *, string_t *);

// Puts the content of a readable item into string_t
void masterReadItem(master_t *, const item_t *, string_t *);

// Describes the action in the input string using the world and the target
// object as context
void masterDescribeAction(master_t *, const world_t *, const string_t *,
                          const object_t *, string_t *);

// Use the last input, the game state and the current world to describe the end
// of the adventure
void masterDescribeEndGame(master_t *, const string_t *, const world_t *,
                           game_state_t, string_t *);

// Forget the description of a given object that was previously described.
void masterForget(master_t *, const object_t *);

// Destroys master and all associated resources
void masterDestroy(master_t **);

// This is only exposed to speed up unit tests.
// Else to test this functionality we would need to depend on ai instantiation
typedef Buffer(const char *) words_t;
words_t *wordsCreate(size_t len);
void wordsDestroy(words_t **self);
int masterIsValidResponse(string_t *, words_t *);
