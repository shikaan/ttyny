#pragma once

#include "../lib/set.h"
#include "ending.h"
#include "item.h"
#include "location.h"
#include "object.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct world_t;
struct meta_t;
typedef struct world_t world_t;
typedef struct meta_t meta_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,

  GAME_STATES
} game_state_t;

struct meta_t {
  // Title of the story, shown in the credits
  char *title;
  // Author of the story, shown in the credits
  char *author;
};

struct world_t {
  // All item definitions available in the story.
  // This is an owning list: the rest uses these items, but don't own them
  items_t *items;
  // All location definitions available in the story.
  // This is an owning list: the rest uses these locations, but don't own them
  locations_t *locations;
  // Possible termination states of the story.
  endings_t *endings;

  // Non-owning list of objects carried by the player.
  items_t *inventory;
  // Where is the story currently taking place.
  location_t *location;
  // Description of the end of the game. Not null only if game is over.
  // It's owned by the endings list
  char *end_game;

  // Metadata used for presentational purposes
  meta_t meta;

  // Number of items the player discovered.
  // Discovering an item means entering the room where its located.
  set_t *discovered_items;
  // Number of locations the player discovered.
  // Discovering a location means entering walking into it.
  set_t *discovered_locations;
  // Number of solved puzzles.
  // Solving a puzzle equates to triggering a transition.
  set_t *solved_puzzles;
  // How many turns since the game has started.
  // Running commands does not contribute to the number of turns.
  size_t turns;
};

// Creates a world from a JSON string allocating all the required resources
world_t *worldFromJSONString(string_t *);

// Creates a world from a JSON file allocating all the required resources
world_t *worldFromJSONFile(const char *);

// Destroy a world and frees all allocated resources
void worldDestroy(world_t **);

// Attempts an object transition due to `action`.
// The result of the attempt is store in result.
void worldTransitionObject(world_t *, object_t *, action_type_t,
                           transition_result_t *);

// Check whether the game is over and returns the game state
void worldDigest(world_t *, game_state_t *);

void worldAreRequirementsMet(world_t *, requirements_t *,
                             requirements_result_t *);
