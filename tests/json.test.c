#include "test.h"

#include "../src/utils.h"
#include "../src/world/world.h"
#include <string.h>

// Helper to build a transitions buffer with two entries without invoking
// bufCreate inside a void test function
static transitions_t *makeTransitions2(transition_t a, transition_t b) {
  transitions_t *t = NULL;
  bufCreate(transitions_t, transition_t, t, 2);
  if (!t) {
    return NULL;
  }
  bufPush(t, a);
  bufPush(t, b);
  return t;
}

// Equality helpers
static bool requirementTuplesEquals(requirement_tuples_t *a,
                                    requirement_tuples_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (a->used != b->used)
    return false;

  for (size_t i = 0; i < a->used; i++) {
    requirement_tuple_t ta = bufAt(a, i);
    requirement_tuple_t tb = bufAt(b, i);
    if (strcmp(ta.name, tb.name) != 0)
      return false;
    if (ta.state != tb.state)
      return false;
  }
  return true;
}

static bool requirementsEquals(requirements_t *a, requirements_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (a->turns != b->turns)
    return false;
  if (!requirementTuplesEquals(a->inventory, b->inventory))
    return false;
  if (!requirementTuplesEquals(a->items, b->items))
    return false;
  if (!requirementTuplesEquals(a->locations, b->locations))
    return false;
  // Compare single current_location tuple (may be NULL)
  if (a->current_location == NULL && b->current_location == NULL) {
    // both absent OK
  } else if (a->current_location == NULL || b->current_location == NULL) {
    return false;
  } else {
    if (strcmp(a->current_location->name, b->current_location->name) != 0)
      return false;
    if (a->current_location->state != b->current_location->state)
      return false;
  }
  return true;
}

static bool endingEquals(ending_t *a, ending_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (a->success != b->success)
    return false;
  if (strcmp(a->reason, b->reason) != 0)
    return false;
  if (!requirementsEquals(a->requirements, b->requirements))
    return false;
  return true;
}

static bool endingsEquals(endings_t *a, endings_t *b) {
  // Treat NULL and empty arrays as equivalent
  bool a_empty = (a == NULL || a->used == 0);
  bool b_empty = (b == NULL || b->used == 0);

  if (a_empty && b_empty)
    return true;
  if (a_empty || b_empty)
    return false;

  if (a->used != b->used)
    return false;

  for (size_t i = 0; i < a->used; i++) {
    ending_t *ea = bufAt(a, i);
    ending_t *eb = bufAt(b, i);
    if (!endingEquals(ea, eb))
      return false;
  }
  return true;
}

static bool descriptionsEquals(descriptions_t *a, descriptions_t *b) {
  // Treat NULL and empty arrays as equivalent
  bool a_empty = (a == NULL || a->used == 0);
  bool b_empty = (b == NULL || b->used == 0);

  if (a_empty && b_empty)
    return true;
  if (a_empty || b_empty)
    return false;

  if (a->used != b->used)
    return false;

  for (size_t i = 0; i < a->used; i++) {
    char *da = bufAt(a, i);
    char *db = bufAt(b, i);
    if (da == NULL && db == NULL)
      continue;
    if (da == NULL || db == NULL)
      return false;
    if (strcmp(da, db) != 0)
      return false;
  }
  return true;
}

static bool transitionsEquals(transitions_t *a, transitions_t *b) {
  // Treat NULL and empty arrays as equivalent
  bool a_empty = (a == NULL || a->used == 0);
  bool b_empty = (b == NULL || b->used == 0);

  if (a_empty && b_empty)
    return true;
  if (a_empty || b_empty)
    return false;

  if (a->used != b->used)
    return false;

  for (size_t i = 0; i < a->used; i++) {
    transition_t ta = bufAt(a, i);
    transition_t tb = bufAt(b, i);
    if (ta.action != tb.action)
      return false;
    if (ta.from != tb.from)
      return false;
    if (ta.to != tb.to)
      return false;
    if ((ta.target == NULL) != (tb.target == NULL))
      return false;
    if (ta.target && tb.target) {
      if (strcmp(ta.target->name, tb.target->name) != 0)
        return false;
      if (ta.target->state != tb.target->state)
        return false;
    }
    if (!requirementsEquals(ta.requirements, tb.requirements))
      return false;
  }
  return true;
}

static bool objectEquals(object_t *a, object_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (strcmp(a->name, b->name) != 0)
    return false;
  if (a->type != b->type)
    return false;
  if (a->state != b->state)
    return false;
  if (!descriptionsEquals(a->descriptions, b->descriptions))
    return false;
  if (!transitionsEquals(a->transitions, b->transitions))
    return false;
  return true;
}

static bool itemEquals(item_t *a, item_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (!objectEquals(&a->object, &b->object))
    return false;
  if (a->collectible != b->collectible)
    return false;
  if (a->readable != b->readable)
    return false;
  return true;
}

static bool itemsEquals(items_t *a, items_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;
  if (a->used != b->used)
    return false;

  for (size_t i = 0; i < a->used; i++) {
    item_t *ia = bufAt(a, i);
    item_t *ib = bufAt(b, i);
    if (!itemEquals(ia, ib))
      return false;
  }
  return true;
}

static bool worldEquals(world_t *a, world_t *b) {
  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;

  if (!itemsEquals(a->items, b->items))
    return false;

  if (!itemsEquals(a->inventory, b->inventory))
    return false;

  if (!endingsEquals(a->endings, b->endings))
    return false;

  if (a->turns != b->turns)
    return false;

  if (a->end_game == NULL && b->end_game == NULL) {
    // Both NULL, ok
  } else if (a->end_game == NULL || b->end_game == NULL) {
    return false;
  } else if (strcmp(a->end_game, b->end_game) != 0) {
    return false;
  }

  // Locations deep-ish comparison (object, items, exits (by name), current location)
  if (a->locations == NULL && b->locations == NULL) {
    // ok
  } else if (a->locations == NULL || b->locations == NULL) {
    return false;
  } else {
    if (a->locations->used != b->locations->used)
      return false;
    for (size_t i = 0; i < a->locations->used; i++) {
      location_t *la = bufAt(a->locations, i);
      location_t *lb = bufAt(b->locations, i);
      if (!objectEquals(&la->object, &lb->object))
        return false;
      if (!itemsEquals(la->items, lb->items))
        return false;
      // Exits shallow comparison (order + names)
      if ((la->exits == NULL) != (lb->exits == NULL))
        return false;
      if (la->exits && lb->exits) {
        if (la->exits->used != lb->exits->used)
          return false;
        for (size_t j = 0; j < la->exits->used; j++) {
          location_t *ea = bufAt(la->exits, j);
          location_t *eb = bufAt(lb->exits, j);
          if (strcmp(ea->object.name, eb->object.name) != 0)
            return false;
        }
      }
    }
  }
  // Current location pointer/name
  if ((a->location == NULL) != (b->location == NULL))
    return false;
  if (a->location && b->location) {
    if (strcmp(a->location->object.name, b->location->object.name) != 0)
      return false;
  }

  return true;
}

// Test case structure
typedef struct {
  const char *name;
  const char *json;
  world_t *expected;
} parser_test_t;

void endings(void) {
  // Empty buffers to normalize expected worlds (payload always has items, locations, endings)
  static items_t empty_items = bufInit(0, 0);
  static locations_t empty_locations = bufInit(0, 0); // keep as empty for exits
  // Added mandatory start location (loader now requires at least one location)
  static char start_name[] = "start";
  static char start_desc[] = "Start";
  static descriptions_t start_descs = bufConst(1, start_desc);
  static location_t start_location = {
      .object =
          {
              .name = start_name,
              .type = OBJECT_TYPE_LOCATION,
              .state = 0,
              .descriptions = &start_descs,
              .transitions = NULL,
          },
      .items = &empty_items,
      .exits = &empty_locations,
  };
  static locations_t start_locations = bufConst(1, &start_location);
  static items_t empty_inventory = bufInit(0, 0);
  // Build expected world 1: Persephone's box (lose)
  static char box_name[] = "Sealed Box";
  static const requirement_tuple_t box_tuple = {.name = box_name, .state = 1};
  static requirement_tuples_t box_items = bufConst(1, box_tuple);
  static requirements_t box_reqs = {
      .inventory = NULL,
      .items = &box_items,
      .locations = NULL,
      .current_location = NULL,
      .turns = 2,
  };
  static char persephone_reason[] = "You opened Persephone's box.";
  static ending_t persephone_ending = {
      .success = false,
      .reason = persephone_reason,
      .requirements = &box_reqs,
  };
  static endings_t persephone_endings = bufConst(1, &persephone_ending);
  static world_t persephone_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &persephone_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 2: Victory
  static char key_name[] = "key";
  static char sword_name[] = "sword";
  static const requirement_tuple_t key_tuple = {.name = key_name,
                                                .state = OBJECT_STATE_ANY};
  static const requirement_tuple_t sword_tuple = {.name = sword_name,
                                                  .state = OBJECT_STATE_ANY};
  static requirement_tuples_t inv_items = bufConst(2, key_tuple, sword_tuple);
  static requirements_t victory_reqs = {
      .inventory = &inv_items,
      .items = NULL,
      .locations = NULL,
      .current_location = NULL,
      .turns = 5,
  };
  static char victory_reason[] = "Victory!";
  static ending_t victory_ending = {
      .success = true,
      .reason = victory_reason,
      .requirements = &victory_reqs,
  };
  static endings_t victory_endings = bufConst(1, &victory_ending);
  static world_t victory_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &victory_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 3: Location requirement
  static char dungeon_name[] = "dungeon";
  static const requirement_tuple_t dungeon_tuple = {.name = dungeon_name,
                                                    .state = 0};
  static requirement_tuples_t dungeon_locs = bufConst(1, dungeon_tuple);
  static requirements_t dungeon_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = &dungeon_locs,
      .current_location = NULL,
      .turns = 0,
  };
  static char trapped_reason[] = "You are trapped!";
  static ending_t trapped_ending = {
      .success = false,
      .reason = trapped_reason,
      .requirements = &dungeon_reqs,
  };
  static endings_t trapped_endings = bufConst(1, &trapped_ending);
  static world_t trapped_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &trapped_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 4: Turn-based ending
  static requirements_t timeout_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .current_location = NULL,
      .turns = 50,
  };
  static char timeout_reason[] = "Time ran out!";
  static ending_t timeout_ending = {
      .success = false,
      .reason = timeout_reason,
      .requirements = &timeout_reqs,
  };
  static endings_t timeout_endings = bufConst(1, &timeout_ending);
  static world_t timeout_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &timeout_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 5: Multiple endings
  static char escape_reason[] = "You escaped!";
  static requirements_t escape_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .current_location = NULL,
      .turns = 0,
  };
  static ending_t escape_ending = {
      .success = true,
      .reason = escape_reason,
      .requirements = &escape_reqs,
  };
  static char death_reason[] = "You died!";
  static requirements_t death_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .current_location = NULL,
      .turns = 100,
  };
  static ending_t death_ending = {
      .success = false,
      .reason = death_reason,
      .requirements = &death_reqs,
  };
  static endings_t multiple_endings = {
      .data = {&escape_ending, &death_ending},
      .used = 2,
      .length = 2,
  };
  static world_t multiple_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &multiple_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 6: Complex - inventory + items + turns
  static char torch_name[] = "torch";
  static char altar_name[] = "altar";
  static const requirement_tuple_t torch_tuple = {.name = torch_name,
                                                  .state = 2};
  static const requirement_tuple_t altar_tuple = {.name = altar_name,
                                                  .state = 1};
  static requirement_tuples_t complex_inv = bufConst(1, torch_tuple);
  static requirement_tuples_t complex_items = bufConst(1, altar_tuple);
  static requirements_t complex_reqs = {
      .inventory = &complex_inv,
      .items = &complex_items,
      .locations = NULL,
      .current_location = NULL,
      .turns = 10,
  };
  static char ritual_reason[] = "The ritual is complete!";
  static ending_t ritual_ending = {
      .success = true,
      .reason = ritual_reason,
      .requirements = &complex_reqs,
  };
  static endings_t ritual_endings = bufConst(1, &ritual_ending);
  static world_t ritual_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &ritual_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected world 7: current location requirement
  static char throne_name[] = "throne";
  static requirement_tuple_t current_loc_tuple = {
      .name = throne_name,
      .state = OBJECT_STATE_ANY,
  };
  static requirements_t current_loc_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
      .current_location = &current_loc_tuple,
  };
  static char throne_reason[] = "You sit upon the throne.";
  static ending_t throne_ending = {
      .success = true,
      .reason = throne_reason,
      .requirements = &current_loc_reqs,
  };
  static endings_t throne_endings = bufConst(1, &throne_ending);
  static world_t throne_world = {
      .items = &empty_items,
      .locations = &start_locations,
      .endings = &throne_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  parser_test_t tests[] = {
      {
          .name = "lose with item requirement",
          .json = "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [], \"endings\": "
                  "[{\"state\": \"lose\","
                  "\"reason\": \"You opened Persephone's box.\","
                  "\"requirements\": {\"inventory\": null, \"items\": "
                  "[\"Sealed Box.1\"],"
                  "\"locations\": null, \"turns\": 2, \"current_location\": null}}]}",
          .expected = &persephone_world,
      },
      {
          .name = "win with inventory",
          .json = "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [],\"endings\": [{\"state\": "
                  "\"win\","
                  "\"reason\": \"Victory!\","
                  "\"requirements\": {\"inventory\": [\"key\", \"sword\"],"
                  "\"items\": null, \"locations\": null, \"turns\": 5, \"current_location\": null}}]}",
          .expected = &victory_world,
      },
      {
          .name = "lose with location requirement",
          .json = "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [],\"endings\": [{\"state\": "
                  "\"lose\","
                  "\"reason\": \"You are trapped!\","
                  "\"requirements\": {\"inventory\": null, \"items\": null,"
                  "\"locations\": [\"dungeon.0\"], \"turns\": 0, \"current_location\": null}}]}",
          .expected = &trapped_world,
      },
      {
          .name = "lose with turn requirement",
          .json =
              "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [], \"endings\": [{\"state\": "
              "\"lose\","
              "\"reason\": \"Time ran out!\","
              "\"requirements\": {\"inventory\": null, \"items\": null,"
              "\"locations\": null, \"turns\": 50, \"current_location\": null}}]}",
          .expected = &timeout_world,
      },
      {
          .name = "multiple endings",
          .json = "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [], \"endings\": ["
                  "{\"state\": \"win\","
                  "\"reason\": \"You escaped!\","
                  "\"requirements\": {\"inventory\": null, \"items\": null,"
                  "\"locations\": null, \"turns\": 0, \"current_location\": null}},"
                  "{\"state\": \"lose\","
                  "\"reason\": \"You died!\","
                  "\"requirements\": {\"inventory\": null, \"items\": null,"
                  "\"locations\": null, \"turns\": 100, \"current_location\": null}}"
                  "]}",
          .expected = &multiple_world,
      },
      {
          .name = "complex requirements (inventory + items + turns)",
          .json =
              "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [], \"endings\": [{\"state\": "
              "\"win\","
              "\"reason\": \"The ritual is complete!\","
              "\"requirements\": {\"inventory\": [\"torch.2\"], \"items\": "
              "[\"altar.1\"],"
              "\"locations\": null, \"turns\": 10, \"current_location\": null}}]}",
          .expected = &ritual_world,
      },
      {
          .name = "win with current_location requirement",
          .json = "{\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"items\": [], \"endings\": [{\"state\": \"win\",\"reason\": \"You sit upon the throne.\",\"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": \"throne\"}}]}",
          .expected = &throne_world,
      },
  };

  for (size_t i = 0; i < arrLen(tests); i++) {
    parser_test_t tc = tests[i];
    string_t *buf cleanup(strDestroy) = strCreate(2048);
    strFmt(buf, "%s", tc.json);
    world_t *world cleanup(worldDestroy) = worldFromJSONString(buf);
    expectTrue(worldEquals(world, tc.expected), tc.name);
  }
}

void items(void) {
  // Empty buffers to match payload shape (always arrays present)
  static locations_t empty_locations = bufInit(0, 0);
  static endings_t empty_endings = bufInit(0, 0);
  static items_t empty_inventory = bufInit(0, 0);
  // Mandatory start location for all item-only worlds
  static char start_name[] = "start";
  static char start_desc[] = "Start";
  static descriptions_t start_descs = bufConst(1, start_desc);
  static location_t start_location = {
      .object =
          {
              .name = start_name,
              .type = OBJECT_TYPE_LOCATION,
              .state = 0,
              .descriptions = &start_descs,
              .transitions = NULL,
          },
      .items = &empty_inventory, // no items initially
      .exits = &empty_locations,
  };
  static locations_t start_locations = bufConst(1, &start_location);
  // Build expected item 1: Simple collectible
  static char key_name[] = "key";
  static char key_desc[] = "A rusty key";
  static descriptions_t key_descs = bufConst(1, key_desc);
  static item_t key_item = {
      .object =
          {
              .name = key_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &key_descs,
              .transitions = NULL,
          },
      .collectible = true,
      .readable = false,
  };
  static items_t key_items = {
      .data = {&key_item},
      .used = 1,
      .length = 1,
  };
  static world_t key_world = {
      .items = &key_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected item 2: Item with transition
  static char box_name[] = "box";
  static char box_desc1[] = "A sealed box";
  static char box_desc2[] = "An open box";
  static descriptions_t box_descs = {
      .data = {box_desc1, box_desc2},
      .used = 2,
      .length = 2,
  };
  static requirements_t box_reqs = {
      .inventory = NULL,
      .items = NULL,
      .current_location = NULL,
      .locations = NULL,
      .turns = 0,
  };
  static requirement_tuple_t box_target_tuple = { box_name, 0 };
  static const transition_t box_transition = {
      .action = ACTION_TYPE_USE,
      .from = 0,
      .to = 1,
      .target = &box_target_tuple,
      .requirements = &box_reqs,
  };
  static transitions_t box_transitions = bufConst(1, box_transition);
  static item_t box_item = {
      .object =
          {
              .name = box_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &box_descs,
              .transitions = &box_transitions,
          },
      .collectible = true,
      .readable = false,
  };
  static items_t box_items = {
      .data = {&box_item},
      .used = 1,
      .length = 1,
  };
  static world_t box_world = {
      .items = &box_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Build expected item 3: Multiple items
  static char sword_name[] = "sword";
  static char sword_desc[] = "A sharp sword";
  static descriptions_t sword_descs = bufConst(1, sword_desc);
  static item_t sword_item = {
      .object =
          {
              .name = sword_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &sword_descs,
              .transitions = NULL,
          },
      .collectible = true,
      .readable = false,
  };
  static char shield_name[] = "shield";
  static char shield_desc[] = "A sturdy shield";
  static descriptions_t shield_descs = bufConst(1, shield_desc);
  static item_t shield_item = {
      .object =
          {
              .name = shield_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &shield_descs,
              .transitions = NULL,
          },
      .collectible = true,
      .readable = false,
  };
  static items_t multi_items = {
      .data = {&sword_item, &shield_item},
      .used = 2,
      .length = 2,
  };
  static world_t multi_world = {
      .items = &multi_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // Additional expected worlds for items suite
  // 1) Non-collectible item
  static char key_nc_name[] = "key";
  static char key_nc_desc[] = "A rusty key";
  static descriptions_t key_nc_descs = bufConst(1, key_nc_desc);
  static item_t key_nc_item = {
      .object =
          {
              .name = key_nc_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &key_nc_descs,
              .transitions = NULL,
          },
      .collectible = false,
      .readable = false,
  };
  static items_t key_nc_items = bufConst(1, &key_nc_item);
  static world_t key_nc_world = {
      .items = &key_nc_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // 2) Readable item
  static char scroll_name[] = "scroll";
  static char scroll_desc[] = "An ancient scroll";
  static descriptions_t scroll_descs = bufConst(1, scroll_desc);
  static item_t scroll_item = {
      .object =
          {
              .name = scroll_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &scroll_descs,
              .transitions = NULL,
          },
      .collectible = true,
      .readable = true,
  };
  static items_t scroll_items = bufConst(1, &scroll_item);
  static world_t scroll_world = {
      .items = &scroll_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // 3) Item with multiple transitions/actions (non-targeted)
  static char device_name[] = "device";
  static char device_desc1[] = "A dormant device";
  static char device_desc2[] = "An active device";
  static descriptions_t device_descs = bufConst(2, device_desc1, device_desc2);
  static requirements_t device_reqs_use = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
  };
  static requirements_t device_reqs_examine = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
  };
  static requirement_tuple_t device_target_use0 = { device_name, 0 };
  static requirement_tuple_t device_target_examine1 = { device_name, 1 };
  static transition_t device_transition_use = {
      .action = ACTION_TYPE_USE,
      .from = 0,
      .to = 1,
      .target = &device_target_use0,
      .requirements = &device_reqs_use,
  };
  static transition_t device_transition_examine = {
      .action = ACTION_TYPE_EXAMINE,
      .from = 1,
      .to = 1,
      .target = &device_target_examine1,
      .requirements = &device_reqs_examine,
  };
  transitions_t *device_transitions =
      makeTransitions2(device_transition_use, device_transition_examine);

  static item_t device_item = {
      .object =
          {
              .name = device_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &device_descs,
              .transitions = NULL,
          },
      .collectible = false,
      .readable = false,
  };
  device_item.object.transitions = device_transitions;

  static items_t device_items = bufConst(1, &device_item);
  static world_t device_world = {
      .items = &device_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // 4) Item missing descriptions (empty array)
  static char blank_name[] = "blank";
  static descriptions_t blank_descs = bufInit(0, 0);
  static item_t blank_item = {
      .object =
          {
              .name = blank_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &blank_descs,
              .transitions = NULL,
          },
      .collectible = true,
      .readable = false,
  };
  static items_t blank_items = bufConst(1, &blank_item);
  static world_t blank_world = {
      .items = &blank_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  // 5) Targeted multi-action transitions (self-targeting)
  static requirement_tuple_t device_target0 = { device_name, 0 };
  static requirement_tuple_t device_target1 = { device_name, 1 };
  static requirements_t device_target_reqs = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
      .current_location = NULL,
  };
  static const transition_t device_targeted_use = {
      .action = ACTION_TYPE_USE,
      .from = 0,
      .to = 1,
      .target = &device_target0,
      .requirements = &device_target_reqs,
  };
  static const transition_t device_targeted_examine = {
      .action = ACTION_TYPE_EXAMINE,
      .from = 1,
      .to = 1,
      .target = &device_target1,
      .requirements = &device_target_reqs,
  };
  static transitions_t device_targeted_transitions =
      bufConst(2, device_targeted_use, device_targeted_examine);
  static item_t device_targeted_item = {
      .object =
          {
              .name = device_name,
              .type = OBJECT_TYPE_ITEM,
              .state = 0,
              .descriptions = &device_descs,
              .transitions = &device_targeted_transitions,
          },
      .collectible = false,
      .readable = false,
  };
  static items_t device_targeted_items = bufConst(1, &device_targeted_item);
  static world_t device_targeted_world = {
      .items = &device_targeted_items,
      .locations = &start_locations,
      .endings = &empty_endings,
      .inventory = &empty_inventory,
      .turns = 0,
      .location = &start_location,
      .end_game = NULL,
  };

  parser_test_t tests[] = {
      {
          .name = "simple collectible item",
          .json = "{\"items\": [{\"name\": \"key\", \"type\": \"item\","
                  "\"descriptions\": [\"A rusty key\"], \"transitions\": [],"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &key_world,
      },
      {
          .name = "item with transition",
          .json = "{\"items\": [{\"name\": \"box\", \"type\": \"item\","
                  "\"descriptions\": [\"A sealed box\", \"An open box\"],"
                  "\"transitions\": [{\"actions\": [\"use\"], \"from\": 0, "
                  "\"to\": 1, \"target\": \"box.0\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}}],"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &box_world,
      },
      {
          .name = "multiple items",
          .json =
              "{\"items\": ["
              "{\"name\": \"sword\", \"type\": \"item\","
              "\"descriptions\": [\"A sharp sword\"], \"transitions\": [],"
              "\"collectible\": true, \"readable\": false},"
              "{\"name\": \"shield\", \"type\": \"item\","
              "\"descriptions\": [\"A sturdy shield\"], \"transitions\": [],"
              "\"collectible\": true, \"readable\": false}"
              "], \"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &multi_world,
      },
      {
          .name = "simple item with null transitions",
          .json = "{\"items\": [{\"name\": \"key\", \"type\": \"item\","
                  "\"descriptions\": [\"A rusty key\"], \"transitions\": null,"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &key_world,
      },
      {
          .name = "simple item without transitions field",
          .json = "{\"items\": [{\"name\": \"key\", \"type\": \"item\","
                  "\"descriptions\": [\"A rusty key\"],"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &key_world,
      },
      {
          .name = "non-collectible item",
          .json = "{\"items\": [{\"name\": \"key\", \"type\": \"item\","
                  "\"descriptions\": [\"A rusty key\"], \"transitions\": [],"
                  "\"collectible\": false, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &key_nc_world,
      },
      {
          .name = "readable item",
          .json =
              "{\"items\": [{\"name\": \"scroll\", \"type\": \"item\","
              "\"descriptions\": [\"An ancient scroll\"], \"transitions\": [],"
              "\"collectible\": true, \"readable\": true}],"
              "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &scroll_world,
      },
      {
          .name = "item with multiple transitions and actions",
          .json =
              "{\"items\": [{\"name\": \"device\", \"type\": \"item\","
              "\"descriptions\": [\"A dormant device\", \"An active device\"],"
              "\"transitions\": ["
              "{\"actions\": [\"use\"], \"from\": 0, \"to\": 1, \"target\": \"device.0\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}},"
              "{\"actions\": [\"examine\"], \"from\": 1, \"to\": 1, \"target\": \"device.1\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}}"
              "],"
              "\"collectible\": false, \"readable\": false}],"
              "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &device_world,
      },
      {
          .name = "item missing descriptions (empty array)",
          .json = "{\"items\": [{\"name\": \"blank\", \"type\": \"item\","
                  "\"descriptions\": [], \"transitions\": [],"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &blank_world,
      },
      {
          .name = "targeted multi-action transitions",
          .json =
              "{\"items\": [{\"name\": \"device\", \"type\": \"item\","
              "\"descriptions\": [\"A dormant device\", \"An active device\"],"
              "\"transitions\": ["
              "{\"actions\": [\"use\"], \"from\": 0, \"to\": 1, \"target\": \"device.0\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}},"
              "{\"actions\": [\"examine\"], \"from\": 1, \"to\": 1, \"target\": \"device.1\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}}"
              "],"
              "\"collectible\": false, \"readable\": false}],"
              "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &device_targeted_world,
      },
      {
          .name = "item with targeted single transition",
          .json = "{\"items\": [{\"name\": \"box\", \"type\": \"item\","
                  "\"descriptions\": [\"A sealed box\", \"An open box\"],"
                  "\"transitions\": [{\"actions\": [\"use\"], \"from\": 0, \"to\": 1, \"target\": \"box.0\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}}],"
                  "\"collectible\": true, \"readable\": false}],"
                  "\"locations\": [{\"name\": \"start\", \"type\": \"location\", \"descriptions\": [\"Start\"], \"transitions\": [], \"items\": [], \"exits\": []}], \"endings\": []}",
          .expected = &box_world,
      },
  };

  for (size_t i = 0; i < arrLen(tests); i++) {
    parser_test_t tc = tests[i];
    string_t *buf cleanup(strDestroy) = strCreate(4096);
    strFmt(buf, "%s", tc.json);
    world_t *world cleanup(worldDestroy) = worldFromJSONString(buf);
    expectTrue(worldEquals(world, tc.expected), tc.name);
  }

}
void locations(void) {
    // Shared empty buffers
    static items_t empty_items = bufInit(0, 0);
    static items_t empty_inventory = bufInit(0, 0);
    static endings_t empty_endings = bufInit(0, 0);
    static locations_t empty_locations_buf = bufInit(0, 0);

    // 1) Single minimal location
    static char hall_name[] = "hall";
    static char hall_desc[] = "A long hall";
    static descriptions_t hall_descs = bufConst(1, hall_desc);
    static location_t hall_location = {
        .object =
            {
                .name = hall_name,
                .type = OBJECT_TYPE_LOCATION,
                .state = 0,
                .descriptions = &hall_descs,
                .transitions = NULL,
            },
        .items = &empty_items,
        .exits = &empty_locations_buf,
    };
    static locations_t single_locations = bufConst(1, &hall_location);
    static world_t single_world = {
        .items = &empty_items,
        .locations = &single_locations,
        .endings = &empty_endings,
        .inventory = &empty_inventory,
        .turns = 0,
        .location = &hall_location,
        .end_game = NULL,
    };

    // 2) Two locations with exits linking each other
    static char kitchen_name[] = "kitchen";
    static char hall2_desc[] = "Hall";
    static descriptions_t hall2_descs = bufConst(1, hall2_desc);
    static char kitchen_desc[] = "Kitchen";
    static descriptions_t kitchen_descs = bufConst(1, kitchen_desc);

    static location_t hall2_location = {
        .object =
            {
                .name = hall_name, // reuse "hall"
                .type = OBJECT_TYPE_LOCATION,
                .state = 0,
                .descriptions = &hall2_descs,
                .transitions = NULL,
            },
        .items = &empty_items,
        .exits = NULL, // will set after kitchen defined
    };
    static location_t kitchen_location = {
        .object =
            {
                .name = kitchen_name,
                .type = OBJECT_TYPE_LOCATION,
                .state = 0,
                .descriptions = &kitchen_descs,
                .transitions = NULL,
            },
        .items = &empty_items,
        .exits = NULL,
    };
    static locations_t hall_exits = {
        .data = {&kitchen_location},
        .used = 1,
        .length = 1,
    };
    static locations_t kitchen_exits = {
        .data = {&hall2_location},
        .used = 1,
        .length = 1,
    };
    static locations_t multi_locations = {
        .data = {&hall2_location, &kitchen_location},
        .used = 2,
        .length = 2,
    };
    // Assign exits now that buffers are built
    static bool init_links_done = false;
    if (!init_links_done) {
      hall2_location.exits = &hall_exits;
      kitchen_location.exits = &kitchen_exits;
      init_links_done = true;
    }
    static world_t exits_world = {
        .items = &empty_items,
        .locations = &multi_locations,
        .endings = &empty_endings,
        .inventory = &empty_inventory,
        .turns = 0,
        .location = &hall2_location,
        .end_game = NULL,
    };

    // 3) Location with item reference
    static char key_name[] = "key";
    static char key_desc[] = "A rusty key";
    static descriptions_t key_descs = bufConst(1, key_desc);
    static item_t key_item = {
        .object =
            {
                .name = key_name,
                .type = OBJECT_TYPE_ITEM,
                .state = 0,
                .descriptions = &key_descs,
                .transitions = NULL,
            },
        .collectible = true,
        .readable = false,
    };
    static items_t key_items = bufConst(1, &key_item);

    static char store_name[] = "store";
    static char store_desc[] = "A small store";
    static descriptions_t store_descs = bufConst(1, store_desc);
    static items_t store_items = bufConst(1, &key_item);
    static location_t store_location = {
        .object =
            {
                .name = store_name,
                .type = OBJECT_TYPE_LOCATION,
                .state = 0,
                .descriptions = &store_descs,
                .transitions = NULL,
            },
        .items = &store_items,
        .exits = &empty_locations_buf,
    };
    static locations_t store_locations = bufConst(1, &store_location);
    static world_t store_world = {
        .items = &key_items,
        .locations = &store_locations,
        .endings = &empty_endings,
        .inventory = &empty_inventory,
        .turns = 0,
        .location = &store_location,
        .end_game = NULL,
    };

    // 4) Location with multi-action transition
    static char lab_name[] = "lab";
    static char lab_desc1[] = "A dark lab";
    static char lab_desc2[] = "A lit lab";
    static descriptions_t lab_descs = {
        .data = {lab_desc1, lab_desc2},
        .used = 2,
        .length = 2,
    };
    static requirements_t lab_reqs = {
        .inventory = NULL,
        .items = NULL,
        .locations = NULL,
        .turns = 0,
    };
    static requirement_tuple_t lab_target0 = { lab_name, 0 };
    static transition_t lab_trans_use = {
        .action = ACTION_TYPE_USE,
        .from = 0,
        .to = 1,
        .target = &lab_target0,
        .requirements = &lab_reqs,
    };
    static transition_t lab_trans_examine = {
        .action = ACTION_TYPE_EXAMINE,
        .from = 0,
        .to = 1,
        .target = &lab_target0,
        .requirements = &lab_reqs,
    };
    transitions_t *lab_transitions_ptr = NULL;
    lab_transitions_ptr = makeTransitions2(lab_trans_use, lab_trans_examine);
    static location_t lab_location = {
        .object =
            {
                .name = lab_name,
                .type = OBJECT_TYPE_LOCATION,
                .state = 0,
                .descriptions = &lab_descs,
                .transitions = NULL,
            },
        .items = &empty_items,
        .exits = &empty_locations_buf,
    };
    lab_location.object.transitions = lab_transitions_ptr;
    static locations_t lab_locations = bufConst(1, &lab_location);
    static world_t lab_world = {
        .items = &empty_items,
        .locations = &lab_locations,
        .endings = &empty_endings,
        .inventory = &empty_inventory,
        .turns = 0,
        .location = &lab_location,
        .end_game = NULL,
    };

    parser_test_t tests[] = {
        {
            .name = "single location minimal",
            .json = "{\"items\": [], \"locations\": [{\"name\": \"hall\", "
                    "\"type\": \"location\", \"descriptions\": [\"A long hall\"],"
                    "\"transitions\": [], \"items\": [], \"exits\": []}], "
                    "\"endings\": []}",
            .expected = &single_world,
        },
        {
            .name = "locations with exits",
            .json =
                "{\"items\": [], \"locations\": ["
                "{\"name\": \"hall\", \"type\": \"location\", "
                "\"descriptions\": [\"Hall\"], \"transitions\": [], "
                "\"items\": [], \"exits\": [\"kitchen\"]},"
                "{\"name\": \"kitchen\", \"type\": \"location\", "
                "\"descriptions\": [\"Kitchen\"], \"transitions\": [], "
                "\"items\": [], \"exits\": [\"hall\"]}"
                "], \"endings\": []}",
            .expected = &exits_world,
        },
        {
            .name = "location with item reference",
            .json =
                "{\"items\": [{\"name\": \"key\", \"type\": \"item\", "
                "\"descriptions\": [\"A rusty key\"], \"transitions\": [], "
                "\"collectible\": true, \"readable\": false}], "
                "\"locations\": [{\"name\": \"store\", \"type\": \"location\", "
                "\"descriptions\": [\"A small store\"], \"transitions\": [], "
                "\"items\": [\"key\"], \"exits\": []}], \"endings\": []}",
            .expected = &store_world,
        },
        {
            .name = "location with multi-action transition",
            .json =
                "{\"items\": [], \"locations\": [{\"name\": \"lab\", "
                "\"type\": \"location\", \"descriptions\": [\"A dark lab\", "
                "\"A lit lab\"], \"transitions\": [{\"actions\": [\"use\", "
                "\"examine\"], \"from\": 0, \"to\": 1, \"target\": \"lab.0\", \"requirements\": {\"inventory\": null, \"items\": null, \"locations\": null, \"turns\": 0, \"current_location\": null}}], \"items\": [], "
                "\"exits\": []}], \"endings\": []}",
            .expected = &lab_world,
        },
    };

    for (size_t i = 0; i < arrLen(tests); i++) {
      parser_test_t tc = tests[i];
      string_t *buf cleanup(strDestroy) = strCreate(4096);
      strFmt(buf, "%s", tc.json);
      world_t *world cleanup(worldDestroy) = worldFromJSONString(buf);
      expectTrue(worldEquals(world, tc.expected), tc.name);
    }
}

int main(void) {
  suite(endings);
  suite(items);
  suite(locations);
  return report();
}
