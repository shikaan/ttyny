#pragma once

#include "ending.h"
#include "item.h"
#include "location.h"
#include "object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct world_t;
typedef struct world_t world_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,

  GAME_STATES
} game_state_t;

struct world_t {
  // All item definitions available in the story.
  // This is an owning list: the rest uses these items, but don't own them
  items_t *items;
  // All location definitions available in the story.
  // This is an owning list: the rest uses these locations, but don't own them
  locations_t *locations;
  // Possible termination states of the story.
  endings_t *endings;

  // How many turns since the game has started.
  uint16_t turns;
  // Non-owning list of objects carried by the player.
  items_t *inventory;
  // Where is the story currently taking place.
  location_t *location;
  // Description of the end of the game. Not null only if game is over.
  // It's owned by the endings list
  char *end_game;
};

world_t *worldFromJSONString(string_t *);
world_t *worldFromJSONFile(const char*);

void worldDestroy(world_t **);

// Attempts an object transition due to `action`.
// The result of the attempt is store in result.
void worldTransitionObject(world_t *, object_t *, action_type_t,
                           transition_result_t *);

void worldDigest(world_t *, game_state_t *);

void worldAreRequirementsMet(world_t *, requirements_t *,
                             requirements_result_t *);
