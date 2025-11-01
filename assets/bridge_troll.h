#include "../src/world.h"

// --- State descriptions for objects ---
static state_descriptions_t bridge_states = bufConst(2, "safe", "swaying");

static state_descriptions_t sword_states = bufConst(2, "on the ground", "held");

static state_descriptions_t apple_states =
    bufConst(4, "on a tree", "held", "core only", "on the ground");

static state_descriptions_t troll_states =
    bufConst(2, "suspicious", "friendly");

static state_descriptions_t clearing_states = bufConst(1, "sunny");

static state_descriptions_t forest_states = bufConst(1, "dark");

// --- Transitions for objects ---
static transitions_t apple_transitions =
    bufConst(3, {.trigger = ACTION_TYPE_TAKE, .from = 0, .to = 1},
      {.trigger = ACTION_TYPE_USE, .from = 1, .to = 2},
      {.trigger = ACTION_TYPE_DROP, .from = 1, .to = 3});

static transitions_t sword_transitions =
    bufConst(2, {.trigger = ACTION_TYPE_TAKE, .from = 0, .to = 1},
      {.trigger = ACTION_TYPE_DROP, .from = 1, .to = 0});

static transitions_t troll_transitions =
    bufConst(2, {.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1});

static transitions_t bridge_transitions =
    bufConst(1, {.trigger = ACTION_TYPE_MOVE, .from = 0, .to = 1});

// --- Objects ---
static item_t sword = {.object = {.name = "Old Sword",
                                  .type = OBJECT_TYPE_ITEM,
                                  .current_state = 0, // on the ground
                                  .description = "rusty,heavy,old",
                                  .state_descriptions = &sword_states,
                                  .traits = 0b00000001, // collectible
                                  .transitions = &sword_transitions}};

static item_t apple = {.object = {.name = "Apple",
                                  .type = OBJECT_TYPE_ITEM,
                                  .current_state = 0, // on a tree
                                  .description = "shiny,red",
                                  .state_descriptions = &apple_states,
                                  .traits = 0b00000001, // collectible
                                  .transitions = &apple_transitions}};

static item_t troll = {.object = {.name = "Bridge Troll",
                                  .type = OBJECT_TYPE_ITEM,
                                  .current_state = 0, // suspicious
                                  .description = "grumpy,smelly,huge",
                                  .state_descriptions = &troll_states,
                                  .traits = 0b00000000,
                                  .transitions = &troll_transitions}};

// --- Object buffers ---
// The item list for every room needs to be as long as all objects
static items_t clearing_objects = {3, 1, {&sword, NULL, NULL}};
static items_t forest_objects = {3, 1, {&apple, NULL, NULL}};
static items_t bridge_objects = {3, 1, {&troll, NULL, NULL}};

// --- Forward declarations for locations ---
static locations_t clearing_exits, forest_exits, bridge_exits;
static location_t clearing, forest, bridge;

// --- Location buffers for exits ---
static locations_t clearing_exits = {1, 1, {(struct location_t *)&forest}};

static locations_t forest_exits = {
    2, 2, {(struct location_t *)&clearing, (struct location_t *)&bridge}};

static locations_t bridge_exits = {1, 1, {(struct location_t *)&forest}};

// --- Locations ---
static location_t clearing = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Clearing",
               .current_state = 0, // sunny
               .description = "sunny,grassy,peaceful",
               .state_descriptions = &clearing_states,
               .traits = 0,
               .transitions = NULL},
    .items = &clearing_objects,
    .exits = &clearing_exits,
};

static location_t forest = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Forest",
               .current_state = 0, // dark
               .description = "dark,cool,ominous",
               .state_descriptions = &forest_states,
               .traits = 0,
               .transitions = NULL},
    .exits = &forest_exits,
    .items = &forest_objects,
};

static location_t bridge = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Bridge",
               .current_state = 0, // safe
               .description = "rickety,over chasm,windy",
               .state_descriptions = &bridge_states,
               .traits = 0,
               .transitions = &bridge_transitions},
    .items = &bridge_objects,
    .exits = &bridge_exits,
};

// --- All locations and objects buffers ---
static locations_t all_locations = {
    3,
    3,
    {
        (struct location_t *)&clearing,
        (struct location_t *)&forest,
        (struct location_t *)&bridge,
    },
};

static items_t all_objects = {3, 3, {&sword, &apple, &troll}};

// --- Digest function ---
static game_state_t bridge_troll_digest(state_t *state) {
  for (size_t i = 0; i < state->inventory->used; ++i) {
    item_t *object = bufAt(state->inventory, i);
    if (object->object.name == sword.object.name) {
      return GAME_STATE_VICTORY;
    }
  }
  return GAME_STATE_CONTINUE;
}

// --- Setting and world ---
static items_t inventory = {
    .length = 3,
    .used = 0,
    .data = {NULL, NULL, NULL},
};

world_t troll_bridge_world = {.state = {.turns = 0, .inventory = &inventory},
                              .locations = &all_locations,
                              .digest = bridge_troll_digest,
                              .current_location = &forest,
                              .items = &all_objects};
