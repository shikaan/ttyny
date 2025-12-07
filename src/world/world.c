#include "world.h"
#include "../utils.h"
#include "action.h"
#include "ending.h"
#include "item.h"
#include "location.h"
#include "object.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <yyjson.h>

void worldDestroy(world_t **self) {
  if (!self || !*self)
    return;

  world_t *world = (*self);
  itemsDestroy(&world->inventory);

  size_t i;
  if (world->items) {
    bufEach(world->items, i) {
      item_t *item = bufAt(world->items, i);
      itemDestroy(&item);
    }
    itemsDestroy(&world->items);
  }

  if (world->locations) {
    bufEach(world->locations, i) {
      location_t *location = bufAt(world->locations, i);
      locationDestroy(&location);
    }
    locationsDestroy(&world->locations);
  }

  if (world->endings) {
    bufEach(world->endings, i) {
      ending_t *ending = bufAt(world->endings, i);
      endingDestory(&ending);
    }
    endingsDestroy(&world->endings);
  }

  setDestroy(&world->discovered_items);
  setDestroy(&world->discovered_locations);
  setDestroy(&world->solved_puzzles);
  deallocate(&world->meta.title);
  deallocate(&world->meta.author);

  deallocate(self);
}

requirements_result_t worldAreRequirementsMet(const world_t *self,
                                              requirements_t *requirements) {
  if (requirements->turns != 0) {
    if (self->turns < requirements->turns) {
      return REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS;
    }
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
          return REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM;
        }
      } else {
        return REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM;
      }
    }
  }

  if (requirements->items) {
    bufEach(requirements->items, i) {
      requirement_tuple_t tuple = bufAt(requirements->items, i);
      int idx = itemsFindByName(self->items, tuple.name);
      if (idx >= 0) {
        item_t *world_item = bufAt(self->items, (size_t)idx);
        if (tuple.state != OBJECT_STATE_ANY &&
            tuple.state != world_item->object.state) {
          return REQUIREMENTS_RESULT_INVALID_WORLD_ITEM;
        }
      } else {
        return REQUIREMENTS_RESULT_MISSING_WORLD_ITEM;
      }
    }
  }

  if (requirements->locations) {
    bufEach(requirements->locations, i) {
      requirement_tuple_t tuple = bufAt(requirements->locations, i);
      int idx = locationsFindByName(self->locations, tuple.name);
      panicif(idx < 0, "location requirement references non-existing location");
      location_t *world_location = bufAt(self->locations, (size_t)idx);
      if (tuple.state != OBJECT_STATE_ANY &&
          tuple.state != world_location->object.state) {
        return REQUIREMENTS_RESULT_INVALID_LOCATION;
      }
    }
  }

  if (requirements->current_location) {
    panicif(!self->location, "current location cannot be null");
    requirement_tuple_t *tuple = requirements->current_location;
    if (!objectNameEq(tuple->name, self->location->object.name)) {
      return REQUIREMENTS_RESULT_CURRENT_LOCATION_MISMATCH;
    }
    if (tuple->state != OBJECT_STATE_ANY &&
        tuple->state != self->location->object.state) {
      return REQUIREMENTS_RESULT_INVALID_CURRENT_LOCATION;
    }
  }

  if (requirements->turns != 0 || requirements->inventory ||
      requirements->items || requirements->locations ||
      requirements->current_location) {
    return REQUIREMENTS_RESULT_OK;
  }

  return REQUIREMENTS_RESULT_NO_REQUIREMENTS;
}

static inline object_t *findObjectByName(const world_t *self,
                                         const char *name) {
  int idx = itemsFindByName(self->items, name);
  if (idx >= 0) {
    return (object_t *)bufAt(self->items, (size_t)idx);
  }

  idx = locationsFindByName(self->locations, name);
  if (idx >= 0) {
    return (object_t *)bufAt(self->locations, (size_t)idx);
  }

  return NULL;
}

transition_result_t
worldExecuteTransition(const world_t *self, const object_t *object,
                       action_type_t action, object_t **affected,
                       object_state_t *affected_initial_state) {
  if (!object->transitions) {
    return TRANSITION_RESULT_OK;
  }

  requirements_result_t requirements_result;
  size_t i = 0;
  bufEach(object->transitions, i) {
    transition_t transition = bufAt(object->transitions, i);
    if (!transition.target)
      continue;

    object_t *target_object = findObjectByName(self, transition.target->name);
    object_state_t target_state = transition.target->state;

    if (transition.action == action &&
        target_object->state == transition.from) {
      requirements_result =
          worldAreRequirementsMet(self, transition.requirements);
      switch (requirements_result) {
      case REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM:
      case REQUIREMENTS_RESULT_MISSING_WORLD_ITEM:
        return TRANSITION_RESULT_MISSING_ITEM;
      case REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM:
      case REQUIREMENTS_RESULT_INVALID_WORLD_ITEM:
      case REQUIREMENTS_RESULT_INVALID_LOCATION:
        return TRANSITION_RESULT_INVALID_TARGET;
      case REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS:
      case REQUIREMENTS_RESULT_INVALID_CURRENT_LOCATION:
      case REQUIREMENTS_RESULT_CURRENT_LOCATION_MISMATCH:
        return TRANSITION_RESULT_NO_TRANSITION;
      case REQUIREMENTS_RESULT_OK:
      case REQUIREMENTS_RESULT_NO_REQUIREMENTS:
      default: {
        if (!target_object || (target_state != OBJECT_STATE_ANY &&
                               target_state != target_object->state)) {
          return TRANSITION_RESULT_NO_TRANSITION;
        }

        if (affected) {
          *affected = target_object;
        }
        if (affected_initial_state) {
          *affected_initial_state = target_object->state;
        }

        target_object->state = transition.to;
        return TRANSITION_RESULT_OK;
      }
      }
    }
  }

  return TRANSITION_RESULT_NO_TRANSITION;
}

void worldDigest(world_t *self, game_state_t *result) {
  // Update score
  (void)setAdd(self->discovered_locations, self->location->object.name);

  size_t i = 0;
  bufEach(self->location->items, i) {
    item_t *item = bufAt(self->location->items, i);
    (void)setAdd(self->discovered_items, item->object.name);
  }

  bufEach(self->items, i) {
    item_t *item = bufAt(self->items, i);
    if (item->object.state != 0) {
      (void)setAdd(self->solved_puzzles, item->object.name);
    }
  }

  bufEach(self->locations, i) {
    location_t *location = bufAt(self->locations, i);
    if (location->object.state != 0) {
      (void)setAdd(self->solved_puzzles, location->object.name);
    }
  }

  // check if game is over
  requirements_result_t requirements_result;
  bufEach(self->endings, i) {
    ending_t *ending = bufAt(self->endings, i);
    requirements_result = worldAreRequirementsMet(self, ending->requirements);

    if (requirements_result == REQUIREMENTS_RESULT_OK) {
      *result = ending->success ? GAME_STATE_VICTORY : GAME_STATE_DEAD;
      self->end_game = ending->reason;
      return;
    }
  }

  *result = GAME_STATE_CONTINUE;
}

static void parseRequirementTupleFromJSONVal(yyjson_val *raw,
                                             requirement_tuple_t *tuple) {
  if (!yyjson_is_str(raw)) {
    if (!yyjson_is_null(raw))
      error("requirement tuple is not valid");
    return;
  }

  const char *raw_tuple = yyjson_get_str(raw);

  char *c = strchr(raw_tuple, '.');
  if (c) {
    tuple->name = strndup(raw_tuple, (unsigned)(c - raw_tuple));
    c++;
    tuple->state =
        (unsigned char)strtol(c, NULL, 10); // TODO: unverified casting!
  } else {
    tuple->name = strdup(raw_tuple);
    tuple->state = OBJECT_STATE_ANY;
  }
}

static requirement_tuples_t *requirementTuplesFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_arr(raw)) {
    if (!yyjson_is_null(raw))
      error("requirements is not valid");
    return NULL;
  }

  yyjson_val *requirement_val;
  size_t i, requirements_len = yyjson_arr_size(raw);

  requirement_tuples_t *tuples = requirementTuplesCreate(requirements_len);
  if (!tuples) {
    error("cannot allocate tuples");
    return NULL;
  }

  yyjson_arr_foreach(raw, i, requirements_len, requirement_val) {
    requirement_tuple_t tuple;
    parseRequirementTupleFromJSONVal(requirement_val, &tuple);
    bufPush(tuples, tuple);
  }

  return tuples;
}

static requirements_t *requirementsFromJSONVal(yyjson_val *raw) {
  requirements_t *result = requirementsCreate();

  yyjson_val *inventory = yyjson_obj_get(raw, "inventory");
  result->inventory = requirementTuplesFromJSONVal(inventory);

  yyjson_val *items = yyjson_obj_get(raw, "items");
  result->items = requirementTuplesFromJSONVal(items);

  yyjson_val *locations = yyjson_obj_get(raw, "locations");
  result->locations = requirementTuplesFromJSONVal(locations);

  yyjson_val *turns = yyjson_obj_get(raw, "turns");
  result->turns = (uint16_t)yyjson_get_uint(turns); // TODO: unverified casting!

  yyjson_val *current_location = yyjson_obj_get(raw, "current_location");
  if (yyjson_is_null(current_location)) {
    result->current_location = NULL;
  } else {
    requirement_tuple_t *tuple = allocate(sizeof(requirement_tuple_t));
    parseRequirementTupleFromJSONVal(current_location, tuple);
    result->current_location = tuple;
  }

  return result;
}

static ending_t *endingFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_obj(raw)) {
    error("ending is not an object");
    return NULL;
  }

  ending_t *ending = allocate(sizeof(ending_t));
  if (!ending) {
    error("cannot create ending");
    return NULL;
  }

  yyjson_val *value = yyjson_obj_get(raw, "state");
  const char *state = yyjson_get_str(value);
  ending->success = strcmp(state, "win") == 0;

  value = yyjson_obj_get(raw, "reason");
  ending->reason = strdup(yyjson_get_str(value));

  value = yyjson_obj_get(raw, "requirements");
  ending->requirements = requirementsFromJSONVal(value);

  return ending;
}

static endings_t *endingsFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_arr(raw)) {
    error("endings is not an array");
    return NULL;
  }

  yyjson_val *ending_val;
  size_t i, endings_len = yyjson_arr_size(raw);

  endings_t *endings;
  bufCreate(endings_t, ending_t, endings, endings_len);

  if (!endings) {
    error("cannot create endings");
    return NULL;
  }

  yyjson_arr_foreach(raw, i, endings_len, ending_val) {
    bufPush(endings, endingFromJSONVal(ending_val));
  }

  return endings;
}

static action_type_t actionFromJSONVal(yyjson_val *val) {
  if (!yyjson_is_str(val)) {
    error("action is not a string");
    return ACTION_TYPE_UNKNOWN;
  }

  const char *action_str = yyjson_get_str(val);

  if (!strcmp(action_str, "use")) {
    return ACTION_TYPE_USE;
  } else if (!strcmp(action_str, "move")) {
    return ACTION_TYPE_MOVE;
  } else if (!strcmp(action_str, "examine")) {
    return ACTION_TYPE_EXAMINE;
  } else if (!strcmp(action_str, "take")) {
    return ACTION_TYPE_TAKE;
  } else if (!strcmp(action_str, "drop")) {
    return ACTION_TYPE_DROP;
  } else {
    return ACTION_TYPE_UNKNOWN;
  }
}

static void parseTransitionFromJSONVal(yyjson_val *raw,
                                       transition_t *transition,
                                       action_type_t action) {
  if (!yyjson_is_obj(raw)) {
    error("transition is not an object");
    return;
  }

  yyjson_val *from = yyjson_obj_get(raw, "from");
  transition->from = (object_state_t)yyjson_get_uint(from);

  yyjson_val *to = yyjson_obj_get(raw, "to");
  transition->to = (object_state_t)yyjson_get_uint(to);

  yyjson_val *target = yyjson_obj_get(raw, "target");
  requirement_tuple_t *tuple = allocate(sizeof(requirement_tuple_t));
  parseRequirementTupleFromJSONVal(target, tuple);
  // TODO: what to do when requirements cannot be parsed?
  transition->target = tuple;

  yyjson_val *requirements = yyjson_obj_get(raw, "requirements");
  transition->requirements = requirementsFromJSONVal(requirements);

  transition->action = action;
}

static transitions_t *transitionsFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_arr(raw)) {
    error("transitions is not an array");
    return NULL;
  }

  yyjson_val *val;
  size_t i, transitions_len = yyjson_arr_size(raw);

  // Estimate max size (each transition object can have multiple actions)
  transitions_t *transitions = NULL;
  bufCreate(transitions_t, transition_t, transitions, transitions_len * 5);

  if (!transitions) {
    error("cannot allocate transitions");
    return NULL;
  }

  yyjson_arr_foreach(raw, i, transitions_len, val) {
    if (!yyjson_is_obj(val)) {
      error("transition is not an object");
      continue;
    }

    yyjson_val *actions = yyjson_obj_get(val, "actions");

    if (!yyjson_is_arr(actions)) {
      error("actions is not an array");
      continue;
    }

    size_t j, actions_len;
    yyjson_val *action_val;
    yyjson_arr_foreach(actions, j, actions_len, action_val) {
      action_type_t action = actionFromJSONVal(action_val);

      transition_t transition;
      parseTransitionFromJSONVal(val, &transition, action);
      bufPush(transitions, transition);
    }
  }

  return transitions;
}

static item_t *itemFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_obj(raw)) {
    error("item is not an object");
    return NULL;
  }

  item_t *item = itemCreate();

  yyjson_val *name = yyjson_obj_get(raw, "name");
  item->object.name = strdup(yyjson_get_str(name));

  // type is set by the constructor

  yyjson_val *descriptions = yyjson_obj_get(raw, "descriptions");

  if (yyjson_is_arr(descriptions)) {
    size_t length = yyjson_arr_size(descriptions);
    bufCreate(descriptions_t, char *, item->object.descriptions, length);
    if (item->object.descriptions) {
      size_t idx;
      yyjson_val *val;
      yyjson_arr_foreach(descriptions, idx, length, val) {
        bufPush(item->object.descriptions, strdup(yyjson_get_str(val)));
      }
    }
  }

  yyjson_val *transitions = yyjson_obj_get(raw, "transitions");
  item->object.transitions = transitionsFromJSONVal(transitions);

  yyjson_val *collectible = yyjson_obj_get(raw, "collectible");
  item->collectible = yyjson_get_bool(collectible);

  yyjson_val *readable = yyjson_obj_get(raw, "readable");
  item->readable = yyjson_get_bool(readable);

  return item;
}

static items_t *itemsFromJSONVal(yyjson_val *raw) {
  if (!yyjson_is_arr(raw)) {
    error("items is not an array");
    return NULL;
  }

  yyjson_val *val;
  size_t idx, max = yyjson_arr_size(raw);

  items_t *items = itemsCreate(max);
  if (!items) {
    error("cannot allocate items");
    return NULL;
  }

  yyjson_arr_foreach(raw, idx, max, val) {
    bufPush(items, itemFromJSONVal(val));
  }

  return items;
}

static location_t *locationFromJSONVal(yyjson_val *raw, items_t *world_items) {
  if (!yyjson_is_obj(raw)) {
    error("location is not an object");
    return NULL;
  }

  location_t *location = locationCreate();

  yyjson_val *name = yyjson_obj_get(raw, "name");
  location->object.name = strdup(yyjson_get_str(name));

  // type is set by the constructor

  yyjson_val *descriptions = yyjson_obj_get(raw, "descriptions");

  if (yyjson_is_arr(descriptions)) {
    size_t length = yyjson_arr_size(descriptions);
    bufCreate(descriptions_t, char *, location->object.descriptions, length);
    if (location->object.descriptions) {
      size_t idx;
      yyjson_val *val;
      yyjson_arr_foreach(descriptions, idx, length, val) {
        bufPush(location->object.descriptions, strdup(yyjson_get_str(val)));
      }
    }
  }

  yyjson_val *transitions = yyjson_obj_get(raw, "transitions");
  location->object.transitions = transitionsFromJSONVal(transitions);

  yyjson_val *items = yyjson_obj_get(raw, "items");
  if (yyjson_is_arr(items)) {
    size_t i, length = yyjson_arr_size(items);
    yyjson_val *raw_item;
    location->items = itemsCreate(world_items->cap);

    yyjson_arr_foreach(items, i, length, raw_item) {
      if (!yyjson_is_str(raw_item)) {
        error("item name is not a string");
        continue;
      }

      const char *item_name = yyjson_get_str(raw_item);
      int item_idx = itemsFindByName(world_items, item_name);
      if (item_idx < 0) {
        error("cannot find item: %s", item_name);
        continue;
      }

      bufPush(location->items, bufAt(world_items, (size_t)item_idx));
    }
  }

  // exits can only be populated once the list of all location is complete
  // we need a second pass

  return location;
}

static locations_t *locationsFromJSONVal(yyjson_val *raw,
                                         items_t *world_items) {
  if (!yyjson_is_arr(raw)) {
    error("locations is not an array");
    return NULL;
  }

  yyjson_val *raw_location;
  size_t i, length = yyjson_arr_size(raw);

  locations_t *locations = locationsCreate(length);
  if (!locations) {
    error("cannot allocate locations");
    return NULL;
  }

  yyjson_arr_foreach(raw, i, length, raw_location) {
    bufPush(locations, locationFromJSONVal(raw_location, world_items));
  }

  return locations;
}

static void populateLocationsExits(yyjson_val *raw,
                                   locations_t *world_locations) {
  if (!yyjson_is_arr(raw)) {
    error("locations is not an array");
    return;
  }

  yyjson_val *raw_location;
  size_t i, length = yyjson_arr_size(raw);

  yyjson_arr_foreach(raw, i, length, raw_location) {
    if (!yyjson_is_obj(raw_location)) {
      error("location is not an object");
      continue;
    }

    yyjson_val *name = yyjson_obj_get(raw_location, "name");
    const char *location_name = yyjson_get_str(name);
    if (!location_name) {
      error("location name is not a valid string");
      continue;
    }

    int location_idx = locationsFindByName(world_locations, location_name);
    if (location_idx < 0) {
      error("cannot find referenced location");
      continue;
    }

    location_t *location = bufAt(world_locations, (size_t)location_idx);

    yyjson_val *exits = yyjson_obj_get(raw_location, "exits");
    if (!yyjson_is_arr(exits)) {
      error("locations is not an array");
      continue;
    }

    size_t raw_exit_idx, exits_length = yyjson_arr_size(exits);
    yyjson_val *raw_exit;
    location->exits = locationsCreate(exits_length);
    if (!location->exits) {
      error("cannot allocate locations");
      continue;
    }

    yyjson_arr_foreach(exits, raw_exit_idx, exits_length, raw_exit) {
      if (!yyjson_is_str(raw_exit)) {
        error("exit name is not a string");
        continue;
      }

      const char *exit_name = yyjson_get_str(raw_exit);

      // Do not allow locations pointing to themselves
      if (strcmp(location_name, exit_name) == 0)
        continue;

      int exit_idx = locationsFindByName(world_locations, exit_name);
      if (exit_idx < 0)
        continue;

      bufPush(location->exits, bufAt(world_locations, (size_t)exit_idx));
    }
  }
}

static void parseMetaFromJSONVal(yyjson_val *raw, meta_t *meta) {
  if (!yyjson_is_obj(raw)) {
    error("meta is not an object");
    return;
  }

  yyjson_val *title = yyjson_obj_get(raw, "title");
  const char *title_str = yyjson_get_str(title);
  if (!title_str) {
    error("title is not a valid string");
    return;
  }

  yyjson_val *author = yyjson_obj_get(raw, "author");
  const char *author_str = yyjson_get_str(author);
  if (!author_str) {
    error("author is not a valid string");
    return;
  }

  meta->author = strdup(author_str);
  meta->title = strdup(title_str);
}

static world_t *worldFromJSONDoc(yyjson_doc *doc) {
  yyjson_val *root = yyjson_doc_get_root(doc);
  world_t *world = allocate(sizeof(world_t));
  if (!world) {
    return NULL;
  }

  yyjson_val *endings = yyjson_obj_get(root, "endings");
  world->endings = endingsFromJSONVal(endings);
  if (!world->endings) {
    error("cannot parse endings");
    worldDestroy(&world);
    return NULL;
  }

  yyjson_val *items = yyjson_obj_get(root, "items");
  world->items = itemsFromJSONVal(items);
  if (!world->items || bufIsEmpty(world->items)) {
    error("cannot parse items");
    worldDestroy(&world);
    return NULL;
  }

  yyjson_val *locations = yyjson_obj_get(root, "locations");
  world->locations = locationsFromJSONVal(locations, world->items);
  if (!world->locations || bufIsEmpty(world->locations)) {
    error("cannot parse locations");
    worldDestroy(&world);
    return NULL;
  }

  populateLocationsExits(locations, world->locations);

  if (!world->locations || bufIsEmpty(world->locations)) {
    error("world must have at least one location");
    worldDestroy(&world);
    return NULL;
  }

  world->inventory = itemsCreate(world->items->cap);
  world->location = bufAt(world->locations, 0);
  world->end_game = NULL;

  yyjson_val *meta = yyjson_obj_get(root, "meta");
  parseMetaFromJSONVal(meta, &world->meta);

  world->turns = 0;
  world->discovered_items = setCreate(world->items->cap);
  world->discovered_locations = setCreate(world->locations->cap);

  // Assuming at most one puzzle per object
  world->solved_puzzles =
      setCreate(world->items->cap + world->locations->cap);

  if (!world->inventory || !world->discovered_items ||
      !world->discovered_locations || !world->solved_puzzles) {
    worldDestroy(&world);
    return NULL;
  }

  return world;
}

world_t *worldFromJSONString(string_t *json) {
  yyjson_doc *doc = yyjson_read(json->data, json->len, 0);
  if (!doc)
    return NULL;

  world_t *world = worldFromJSONDoc(doc);
  yyjson_doc_free(doc);
  return world;
}

world_t *worldFromJSONFile(const char *path) {
  yyjson_doc *doc = yyjson_read_file(path, 0, NULL, NULL);
  if (!doc)
    return NULL;

  world_t *world = worldFromJSONDoc(doc);
  yyjson_doc_free(doc);
  return world;
}
