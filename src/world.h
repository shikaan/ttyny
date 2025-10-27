#pragma once

#include "alloc.h"
#include "buffers.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
  ACTION_UNKNOWN = -1,
  ACTION_MOVE = 0,
  ACTION_TAKE,
  ACTION_DROP,
  ACTION_USE,
  ACTION_EXAMINE,

  ACTIONS,
} action_t;

static string_t ACTION_MOVE_NAME = strConst("move");
static string_t ACTION_USE_NAME = strConst("use");
static string_t ACTION_TAKE_NAME = strConst("take");
static string_t ACTION_DROP_NAME = strConst("drop");
static string_t ACTION_EXAMINE_NAME = strConst("examine");

static action_t actions_types[ACTIONS] = {
    ACTION_MOVE, ACTION_TAKE, ACTION_DROP, ACTION_USE, ACTION_EXAMINE,
};

static string_t *action_names[ACTIONS] = {
    &ACTION_MOVE_NAME, &ACTION_TAKE_NAME,    &ACTION_DROP_NAME,
    &ACTION_USE_NAME,  &ACTION_EXAMINE_NAME,
};

typedef enum {
  OBJ_TYPE_UNKNOWN = -1,
  OBJ_TYPE_ITEM = 0,
  OBJ_TYPE_LOCATION,

  OBJ_TYPES,
} obj_type_t;

typedef const char *obj_id_t;

static inline int objectIdEq(obj_id_t self, obj_id_t other) {
  return strcmp(self, other) == 0;
}

typedef enum {
  FAILURE_INVALID_TARGET,
  FAILURE_INVALID_LOCATION,
  FAILURE_INVALID_ITEM,

  FAILURES,
} failures_t;

static string_t FAILURE_INVALID_TARGET_NAME =
    strConst("missing or invalid target");
static string_t FAILURE_INVALID_LOCATION_NAME =
    strConst("missing or invalid location");
static string_t FAILURE_INVALID_ITEM_NAME = strConst("missing or invalid item");

static string_t *failure_names[FAILURES] = {
    &FAILURE_INVALID_TARGET_NAME,
    &FAILURE_INVALID_LOCATION_NAME,
    &FAILURE_INVALID_ITEM_NAME,
};

// Boolean map of traits for objects and location.
// The traits are fixed. Interactions in the game change the state (last 2 bits)
// according to the transactions
//
// ITEM  (76543210)
// 0     = collectible
// 1     = change damage
// 2     = change health
// 3     = ephemeral
// 4     = ...
// 5     = ...
// (6-7) = 4 possible states
//
// LOCATION (76543210)
// 0     = ...
// 1     = ...
// 2     = ...
// 3     = ...
// 4     = ...
// 5     = ...
// (6-7) = 4 possible states
typedef uint8_t traits_t;

typedef uint8_t state_t;

// 4 Transitions from one state to the next.
// 0010 1011 1101 0100 means 0 -> 2, 2 -> 3, 3 -> 1, 1 -> 0
typedef uint16_t transitions_t;

typedef struct {
  // Name of the object
  obj_id_t name;
  // Type of the object
  obj_type_t type;
  // Description of the object
  const char *description;
  // Human-readable state descriptions
  const char *states[4];
  // Object traits as per above
  traits_t traits;
  // Transitions from one state to the next
  transitions_t transitions;
} object_t;

typedef struct {
  // The base object MUST be the first member
  object_t object;
  // In case the object changes damage or health, see by how much
  int8_t value;
} item_t;

typedef Buffer(item_t *) items_t;

static const items_t NO_ITEMS = {0, 0, {}};

static inline items_t *itemsCreate(size_t length) {
  items_t *items = NULL;
  makeBufCreate(items_t, item_t, items, length);
  return items;
}

static inline void itemsAdd(items_t *self, item_t *item) {
  bufPush(self, item);
  // TODO: make items_t extensible
}

static inline void itemsRemove(items_t *self, item_t *item) {
  for (size_t i = 0; i < self->used; i++) {
    if (bufAt(self, i) == item) {
      item_t *last = bufAt(self, self->used - 1);
      bufSet(self, i, last);
      bufSet(self, self->used - 1, NULL);
      self->used--;
      break;
    }
  }
}

static inline void itemsDestroy(items_t **self) { deallocate(self); }

struct location_t; // Forward declaration
typedef Buffer(struct location_t *) locations_t;
static const locations_t NO_LOCATIONS = {0, 0, {}};

typedef struct {
  // The base object MUST be the first member
  object_t object;
  // Objects to be found in this location
  items_t *items;
  // Exits from this location into other locations
  locations_t *exits;
} location_t;

static inline locations_t *locationsCreate(size_t length) {
  locations_t *locations = NULL;
  makeBufCreate(locations_t, location_t, locations, length);
  return locations;
}

static inline void locationsDestroy(locations_t **self) { deallocate(self); }

typedef struct {
  // How many turns since the game has started
  uint16_t turns;
  // Health of our character
  uint8_t health;
  // How much damage does the character deal
  uint8_t damage;
  // Objects carried by the character
  items_t *inventory;
} world_state_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,
} game_state_t;

// Returns game state based on current world state
typedef game_state_t (*digest_t)(world_state_t *);

typedef struct {
  // State of the world
  world_state_t state;
  // Locations of the world
  locations_t *locations;
  // Objects in the world
  items_t *items;
  // Pointer to the location where the adventure starts
  location_t *current_location;
  // Callback to call on each round to see if the game is over
  digest_t digest;
} world_t;

static inline state_t objectGetState(traits_t traits) {
  return (traits & 0b11000000) >> 6;
}

// Executes a state transition
static inline traits_t objectTransition(traits_t traits,
                                        transitions_t transitions) {
  state_t current_state = objectGetState(traits);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t entry = (transitions >> (i * 4)) & 0xF;
    uint8_t from = (entry >> 2) & 0x3;
    if (from == current_state) {
      uint8_t to = (uint8_t)((entry & 0x3) << 6);
      return (traits & 0b00111111) | to;
    }
  }
  return traits;
}

static inline void worldMakeSummary(world_t *world, string_t *summary) {
  location_t *current_location = world->current_location;
  state_t current_location_state =
      objectGetState(current_location->object.traits);

  strFmt(summary, "LOCATION: %s (%s) [%s]\n", current_location->object.name,
         current_location->object.description,
         current_location->object.states[current_location_state]);

  strFmtAppend(summary, "ITEMS:\n");
  for (size_t i = 0; i < current_location->items->used; i++) {
    item_t *object = bufAt(current_location->items, i);
    state_t object_state = objectGetState(object->object.traits);
    strFmtAppend(summary, "  - %s (%s) [%s]\n", object->object.name,
                 object->object.description,
                 object->object.states[object_state]);
  }

  strFmtOffset(summary, summary->used, "EXITS:\n");
  for (size_t i = 0; i < current_location->exits->used; i++) {
    location_t *exit = (location_t *)bufAt(current_location->exits, i);
    state_t object_state = objectGetState(exit->object.traits);
    strFmtAppend(summary, "  - %s (%s) [%s]\n", exit->object.name,
                 exit->object.description, exit->object.states[object_state]);
  }
}
