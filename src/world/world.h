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

// forward declared from loader.h
world_t *worldCreateFromFile(const char *path);

static inline void worldDestroy(world_t **self) {
  if (!self || !*self)
    return;

  world_t *world = (*self);
  itemsDestroy(&world->inventory);

  size_t i;
  bufEach(world->items, i) {
    item_t *item = bufAt(world->items, i);
    itemDestroy(&item);
  }
  itemsDestroy(&world->items);

  bufEach(world->locations, i) {
    location_t *location = bufAt(world->locations, i);
    locationDestroy(&location);
  }
  locationsDestroy(&world->locations);

  bufEach(world->endings, i) {
    ending_t *ending = bufAt(world->endings, i);
    endingDestory(&ending);
  }
  endingsDestroy(&world->endings);

  deallocate(self);
}

static void worldAreRequirementsMet(world_t *self, requirements_t *requirements,
                                    requirements_result_t *result) {
#define done(Result)                                                           \
  *result = Result;                                                            \
  return

  if (requirements->turns != 0) {
    if (self->turns >= requirements->turns) {
      done(REQUIREMENTS_RESULT_OK);
    }
    done(REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS);
  }

  size_t i;
  if (requirements->inventory) {
    bufEach(requirements->inventory, i) {
      requirement_tuple_t tuple = bufAt(requirements->inventory, i);
      int idx = itemsFindByName(self->inventory, tuple.name);
      if (idx >= 0) {
        item_t *inventory_item = bufAt(self->inventory, (size_t)idx);
        if (tuple.state != OBJECT_STATE_ANY &&
            tuple.state != inventory_item->object.state) {
          done(REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM);
        }
      } else {
        done(REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM);
      }
    }
    done(REQUIREMENTS_RESULT_OK);
  }

  if (requirements->items) {
    bufEach(requirements->items, i) {
      requirement_tuple_t tuple = bufAt(requirements->items, i);
      int idx = itemsFindByName(self->items, tuple.name);
      if (idx >= 0) {
        item_t *world_item = bufAt(self->items, (size_t)idx);
        if (tuple.state != OBJECT_STATE_ANY &&
            tuple.state != world_item->object.state) {
          done(REQUIREMENTS_RESULT_INVALID_WORLD_ITEM);
        }
      } else {
        done(REQUIREMENTS_RESULT_MISSING_WORLD_ITEM);
      }
    }
    done(REQUIREMENTS_RESULT_OK);
  }

  if (requirements->locations) {
    bufEach(requirements->locations, i) {
      requirement_tuple_t tuple = bufAt(requirements->locations, i);
      int idx = locationsFindByName(self->locations, tuple.name);
      panicif(idx < 0, "Requirements reference non-existing location");
      location_t *world_location = bufAt(self->locations, (size_t)idx);
      if (tuple.state != OBJECT_STATE_ANY &&
          tuple.state != world_location->object.state) {
        done(REQUIREMENTS_RESULT_INVALID_LOCATION);
      }
    }
    done(REQUIREMENTS_RESULT_OK);
  }

  done(REQUIREMENTS_RESULT_NO_REQUIREMENTS);
#undef done
}

// Attempts an object transition due to `action`.
// The result of the attempt is store in result.
static inline void worldTransitionObject(world_t *self, object_t *object,
                                         action_type_t action,
                                         transition_result_t *result) {
  if (!object->transitions) {
    *result = TRANSITION_RESULT_OK;
    return;
  }

  requirements_result_t requirements_result;
  for (size_t i = 0; i < object->transitions->used; i++) {
    transition_t transition = bufAt(object->transitions, i);
    if (transition.action == action && object->state == transition.from) {
      worldAreRequirementsMet(self, transition.requirements,
                              &requirements_result);
      switch (requirements_result) {
      case REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM:
      case REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM:
      case REQUIREMENTS_RESULT_MISSING_WORLD_ITEM:
      case REQUIREMENTS_RESULT_INVALID_WORLD_ITEM:
      case REQUIREMENTS_RESULT_INVALID_LOCATION:
      case REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS:
        *result = TRANSITION_RESULT_NO_TRANSITION;
        return;
      case REQUIREMENTS_RESULT_OK:
      case REQUIREMENTS_RESULT_NO_REQUIREMENTS:
      default:
        object->state = transition.to;
        *result = TRANSITION_RESULT_OK;
        return;
      }
    }
  }

  *result = TRANSITION_RESULT_NO_TRANSITION;
}

static inline void worldDigest(world_t *self, game_state_t *result) {
  requirements_result_t requirements_result;

  for (size_t i = 0; i < self->endings->used; i++) {
    ending_t *ending = bufAt(self->endings, i);
    worldAreRequirementsMet(self, ending->requirements, &requirements_result);

    if (requirements_result == REQUIREMENTS_RESULT_OK) {
      *result = ending->success ? GAME_STATE_VICTORY : GAME_STATE_DEAD;
      self->end_game = ending->reason;
      return;
    }
  }

  *result = GAME_STATE_CONTINUE;
}
