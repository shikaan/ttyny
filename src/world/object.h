#pragma once

#include <stdint.h>
#include <string.h>
#include "../buffers.h"
#include "action.h"

struct requirements_t;
typedef struct requirements_t requirements_t;

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

// A number representing the state of a given object
// States are described in descriptions_t such that the n-th description
// corresponds to the state n.
typedef uint8_t object_state_t;

// List of descriptions.
// They will be used by the language model to describe objects or situations.
typedef Buffer(const char *) descriptions_t;

// When an object is affected by `trigger` its state changes from `from` to `to`
typedef struct {
  action_type_t action;
  object_state_t from;
  object_state_t to;
  requirements_t *requirements;
} transition_t;

// List of transitions for a given object
typedef Buffer(transition_t) transitions_t;

// Objects describe generic game objects, such as items, locations, or NPCs
// (not implemented yet).
typedef struct {
  // Unique Identifier of the object
  object_name_t name;
  // Type of the object
  object_type_t type;
  // Current state of the object
  object_state_t state;
  // Human-readable state descriptions
  descriptions_t *descriptions;
  // Transitions from one state to the next
  transitions_t *transitions;
} object_t;

typedef enum {
  TRANSITION_RESULT_OK,
  TRANSITION_RESULT_NO_TRANSITION,
  TRANSITION_RESULT_MISSING_ITEM,
} transition_result_t;
