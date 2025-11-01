#pragma once

#include "alloc.h"
#include "buffers.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum {
  ACTION_TYPE_UNKNOWN = -1,
  ACTION_TYPE_MOVE = 0,
  ACTION_TYPE_TAKE,
  ACTION_TYPE_DROP,
  ACTION_TYPE_USE,
  ACTION_TYPE_EXAMINE,

  ACTION_TYPE_HELP,
  ACTION_TYPE_STATUS,
  ACTION_TYPE_QUIT,

  ACTION_TYPES,
} action_type_t;

static string_t ACTION_MOVE = strConst("move");
static string_t ACTION_USE = strConst("use");
static string_t ACTION_TAKE = strConst("take");
static string_t ACTION_DROP = strConst("drop");
static string_t ACTION_EXAMINE = strConst("examine");
static string_t ACTION_HELP = strConst("/help");
static string_t ACTION_STATUS = strConst("/status");
static string_t ACTION_QUIT = strConst("/quit");

static action_type_t actions_types[ACTION_TYPES] = {
    ACTION_TYPE_MOVE,   ACTION_TYPE_TAKE,    ACTION_TYPE_DROP,
    ACTION_TYPE_USE,    ACTION_TYPE_EXAMINE, ACTION_TYPE_HELP,
    ACTION_TYPE_STATUS, ACTION_TYPE_QUIT};

static string_t *action_names[ACTION_TYPES] = {
    &ACTION_MOVE,    &ACTION_TAKE, &ACTION_DROP,   &ACTION_USE,
    &ACTION_EXAMINE, &ACTION_HELP, &ACTION_STATUS, &ACTION_QUIT};

typedef enum {
  OBJECT_TYPE_UNKNOWN = -1,
  OBJECT_TYPE_ITEM = 0,
  OBJECT_TYPE_LOCATION,

  OBJECT_TYPES,
} object_type_t;

typedef const char *object_name_t;

static inline int objectNameEq(object_name_t self, object_name_t other) {
  return strcmp(self, other) == 0;
}

typedef enum {
  FAILURE_TYPE_INVALID_TARGET,
  FAILURE_TYPE_INVALID_LOCATION,
  FAILURE_TYPE_INVALID_ITEM,
  FAILURE_TYPE_CANNOT_COLLECT_ITEM,

  FAILURES,
} failure_type_t;

static string_t FAILURE_INVALID_TARGET = strConst("missing or invalid target");
static string_t FAILURE_INVALID_LOCATION =
    strConst("missing or invalid location");
static string_t FAILURE_INVALID_ITEM = strConst("missing or invalid item");
static string_t FAILURE_CANNOT_COLLECT_ITEM = strConst("cannot collect item");

static string_t *failure_names[FAILURES] = {
    &FAILURE_INVALID_TARGET,
    &FAILURE_INVALID_LOCATION,
    &FAILURE_INVALID_ITEM,
    &FAILURE_CANNOT_COLLECT_ITEM,
};

// Boolean map of traits for objects and location.
// The traits are fixed. Interactions in the game change the state (last 2 bits)
// according to the transactions
//
// ITEM  (76543210)
// 0     = collectible
// 1     = ...
// 2     = ...
// 3     = ...
// 4     = ...
// 5     = ...
// 6     = ...
// 7     = ...
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

typedef uint8_t object_state_t;

typedef Buffer(const char *) state_descriptions_t;

typedef struct {
  action_type_t trigger;
  object_state_t from;
  object_state_t to;
} transition_t;

typedef Buffer(transition_t) transitions_t;

typedef struct {
  // Name of the object
  object_name_t name;
  // Type of the object
  object_type_t type;
  // Current state of the object
  object_state_t current_state;
  // Description of the object
  const char *description;
  // Human-readable state descriptions
  state_descriptions_t *state_descriptions;
  // Object traits as per above
  traits_t traits;
  // Transitions from one state to the next
  transitions_t *transitions;
} object_t;

static inline void objectTransition(object_t *self, action_type_t action) {
  if (!self->transitions)
    return;

  for (size_t i = 0; i < self->transitions->used; i++) {
    transition_t transition = bufAt(self->transitions, i);
    if (transition.trigger == action &&
        self->current_state == transition.from) {
      self->current_state = transition.to;
      return;
    }
  }
}

static inline int objectIsCollectible(object_t *self) {
  return (self->traits & 0b00000001) == 1;
}

typedef struct {
  object_t object;
} item_t;

typedef Buffer(item_t *) items_t;

static const items_t NO_ITEMS = {0, 0, {}};

static inline items_t *itemsCreate(size_t length) {
  items_t *items = NULL;
  makeBufCreate(items_t, item_t *, items, length);
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
  makeBufCreate(locations_t, location_t *, locations, length);
  return locations;
}

static inline void locationsDestroy(locations_t **self) { deallocate(self); }

typedef struct {
  // How many turns since the game has started
  uint16_t turns;
  // Objects carried by the character
  items_t *inventory;
} state_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,
} game_state_t;

// Returns game state based on current world state
typedef game_state_t (*digest_t)(state_t *);

typedef struct {
  // State of the world
  state_t state;
  // Locations of the world
  locations_t *locations;
  // Objects in the world
  items_t *items;
  // Pointer to the location where the adventure starts
  location_t *current_location;
  // Callback to call on each round to see if the game is over
  digest_t digest;
} world_t;
