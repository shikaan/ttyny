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

  ACTION_TYPES,
} action_type_t;

typedef enum {
  COMMAND_TYPE_UNKNOWN = -1,
  COMMAND_TYPE_HELP = 0,
  COMMAND_TYPE_STATUS,
  COMMAND_TYPE_QUIT,
  COMMAND_TYPE_TLDR,

  COMMAND_TYPES
} command_type_t;

static string_t ACTION_MOVE = strConst("move");
static string_t ACTION_USE = strConst("use");
static string_t ACTION_TAKE = strConst("take");
static string_t ACTION_DROP = strConst("drop");
static string_t ACTION_EXAMINE = strConst("examine");

static string_t COMMAND_HELP = strConst("/help");
static string_t COMMAND_STATUS = strConst("/status");
static string_t COMMAND_QUIT = strConst("/quit");
static string_t COMMAND_TLDR = strConst("/tldr");

static action_type_t actions_types[ACTION_TYPES] = {
    ACTION_TYPE_MOVE, ACTION_TYPE_TAKE, ACTION_TYPE_DROP, ACTION_TYPE_USE,
    ACTION_TYPE_EXAMINE};

static string_t *action_names[ACTION_TYPES] = {
    &ACTION_MOVE, &ACTION_TAKE, &ACTION_DROP, &ACTION_USE, &ACTION_EXAMINE};

typedef enum {
  OBJECT_TYPE_UNKNOWN = -1,
  OBJECT_TYPE_ITEM = 0,
  OBJECT_TYPE_LOCATION,

  OBJECT_TYPES,
} object_type_t;

// Unique identifier of the object in the world
// must be human readable, because it will be displayed to the user
typedef const char *object_name_t;

static inline int objectNameEq(object_name_t self, object_name_t other) {
  return strcmp(self, other) == 0;
}

// Boolean map of traits for objects and locations.
//
// ITEM  (76543210)     |  LOCATION
// 0     = collectible  |  0     = lit
// 1     = ...          |  1     = ...
// 2     = ...          |  2     = ...
// 3     = ...          |  3     = ...
// 4     = ...          |  4     = ...
// 5     = ...          |  5     = ...
// 6     = ...          |  6     = ...
// 7     = ...          |  7     = ...
typedef uint8_t traits_t;

// A number representing the state of a given object
// States are described in state_descriptions_t such that the n-th description
// corresponds to the state n.
typedef uint8_t object_state_t;

// List of description of the state of the object.
// These descriptions will be used by the language model to describe the object
// in a given state
typedef Buffer(const char *) state_descriptions_t;

// When an object is affected by `trigger` its state changes from `from` to `to`
typedef struct {
  action_type_t trigger;
  object_state_t from;
  object_state_t to;
} transition_t;

// List of transitions for a given object
typedef Buffer(transition_t) transitions_t;

// Objects describe generic game objects, such as items, locations, or NPCs
// (not implemented yet).
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

typedef enum {
  TRANSITION_RESULT_OK,
  TRANSITION_RESULT_NO_TRANSITIONS,
} transition_result_t;

// Attempts an object transition due to `action`.
// The result of the attempt is store in result.
static inline void objectTransition(object_t *self, transition_result_t *result,
                                    action_type_t action) {
  if (!self->transitions) {
    *result = TRANSITION_RESULT_OK;
    return;
  }

  for (size_t i = 0; i < self->transitions->used; i++) {
    transition_t transition = bufAt(self->transitions, i);
    if (transition.trigger == action &&
        self->current_state == transition.from) {
      self->current_state = transition.to;
      *result = TRANSITION_RESULT_OK;
      return;
    }
  }

  *result = TRANSITION_RESULT_NO_TRANSITIONS;
}

static inline int objectIsCollectible(object_t *self) {
  return (self->traits & 0b00000001) == 1;
}

// An item is an object the player can interact with. It represents a physical
// thing like a lamp or an apple.
typedef struct {
  object_t object;
} item_t;

typedef Buffer(item_t *) items_t;

static inline items_t *itemsCreate(size_t length) {
  items_t *items = NULL;
  makeBufCreate(items_t, item_t *, items, length);
  return items;
}

static inline void itemsAdd(items_t *self, item_t *item) {
  bufPush(self, item);
  // TODO: make items_t extensible
}

static inline void itemsCat(items_t *self, items_t *other) {
  for (size_t i = 0; i < other->used; i++) {
    itemsAdd(self, bufAt(other, i));
  }
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

static inline void itemsClear(items_t *self) { bufClear(self, NULL); }

static inline void itemsDestroy(items_t **self) { deallocate(self); }

// A location is a game object representing a place the user can visit.
struct location_t; // Forward declaration
typedef Buffer(struct location_t *) locations_t;

typedef struct {
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

static inline void locationsClear(locations_t *self) { bufClear(self, NULL); }

static inline void locationsCat(locations_t *self, locations_t *other) {
  for (size_t i = 0; i < other->used; i++) {
    bufPush(self, bufAt(other, i));
  }
}

static inline void locationsDestroy(locations_t **self) { deallocate(self); }

typedef struct {
  // How many turns since the game has started
  uint16_t turns;
  // Objects carried by the character
  items_t *inventory;
} world_state_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,

  GAME_STATES
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
  // Brief description of end game conditions to allow AI to write a story
  const char *end_game[GAME_STATES];
} world_t;
