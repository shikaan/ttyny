#pragma once

#include "../buffers.h"
#include "action.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../buffers.h"
#include <stdint.h>

typedef struct requirements_t requirements_t;
static inline void requirementsDestroy(requirements_t **self);

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

static const object_state_t OBJECT_STATE_ANY = 255;

// List of descriptions.
// They will be used by the language model to describe objects or situations.
typedef Buffer(char *) descriptions_t;

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

static inline void transitionDestroy(transition_t **self) {
  if (!self || !*self)
    return;

  requirementsDestroy(&(*self)->requirements);
  deallocate(self);
}

static inline void objectDestroyInner(object_t **self) {
  if (!self || !*self)
    return;

  object_t *obj = (*self);

  if (obj->descriptions) {
    for (size_t i = 0; i < obj->descriptions->used; i++) {
      char *description = bufAt(obj->descriptions, i);
      deallocate(&description);
    }
    deallocate(&obj->descriptions);
  }

  if (obj->transitions) {
    for (size_t i = 0; i < obj->transitions->used; i++) {
      transition_t transition = bufAt(obj->transitions, i);
      requirementsDestroy(&transition.requirements);
    }
    deallocate(&obj->transitions);
  }
}

typedef struct {
  object_name_t name;
  object_state_t state;
} requirement_tuple_t;

typedef Buffer(requirement_tuple_t) requirement_tuples_t;

static inline requirement_tuples_t *requirementTuplesCreate(size_t length) {
  requirement_tuples_t *tuples = NULL;
  bufCreate(requirement_tuples_t, requirement_tuples_t, tuples, length);
  return tuples;
}

typedef struct requirements_t {
  requirement_tuples_t *inventory;
  requirement_tuples_t *items;
  requirement_tuples_t *locations;
  uint16_t turns;
} requirements_t;

typedef enum {
  REQUIREMENTS_RESULT_OK,
  REQUIREMENTS_RESULT_NO_REQUIREMENTS,
  REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM,
  REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM,
  REQUIREMENTS_RESULT_MISSING_WORLD_ITEM,
  REQUIREMENTS_RESULT_INVALID_WORLD_ITEM,
  REQUIREMENTS_RESULT_INVALID_LOCATION,
  REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS,
} requirements_result_t;

static inline void requirementsDestroy(requirements_t **self) {
  if (!self || !*self)
    return;

  requirements_t *req = *self;
  deallocate(&req->items);
  deallocate(&req->inventory);
  deallocate(&req->locations);
  deallocate(self);
}
