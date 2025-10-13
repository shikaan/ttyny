#include "../src/world.h"
#include "ggml.h"

// --- Objects ---
static object_t sword = {
    .name = "Old Sword",
    .descriptions =
        {
            "A rusty sword lies on the ground.",
            "The sword gleams after being cleaned.",
            "The sword is chipped from battle.",
            "The sword is broken and useless.",
        },
    .traits = 0b00000011, // collectible + change damage
    .value = 5,
    .transitions = 0b0001101000110010,
};

static object_t apple = {
    .name = "Red Apple",
    .descriptions =
        {
            "A shiny red apple hangs from a tree.",
            "The apple is in your hand.",
            "The apple has a bite taken out of it.",
            "The apple core is discarded.",
        },
    .traits = 0b00000101, // collectible + change health
    .value = 10,
    .transitions = 0b0001101000110010,
};

static object_t troll = {
    .name = "Bridge Troll",
    .descriptions =
        {
            "A grumpy troll blocks the bridge.",
            "The troll eyes you warily.",
            "The troll is wounded.",
            "The troll has fled!",
        },
    .traits = 0b00001000, // ephemeral
    .value = -10,
    .transitions = 0b0001101000110010,
};

// --- Object buffers ---
static objects_t clearing_objects = {.length = 1, .used = 1, .data = {&sword}};

static objects_t forest_objects = {.length = 1, .used = 1, .data = {&apple}};

static objects_t bridge_objects = {.length = 1, .used = 1, .data = {&troll}};

// --- Forward declarations for locations ---
static locations_t clearing_exits, forest_exits, bridge_exits;
static location_t clearing, forest, bridge;

// --- Location buffers for exits ---
static locations_t clearing_exits = {.length = 1, .used = 1, .data = {&forest}};

static locations_t forest_exits = {
    .length = 2, .used = 2, .data = {&clearing, &bridge}};

static locations_t bridge_exits = {.length = 1, .used = 1, .data = {&forest}};

// --- Locations ---
static location_t clearing = {
    .name = "Clearing",
    .descriptions = {"A sunny clearing with soft grass.",
                     "The clearing feels peaceful.",
                     "You notice signs of a struggle.",
                     "The clearing is eerily quiet."},
    .objects = &clearing_objects,
    .exits = (struct locations_t *)&clearing_exits,
    .traits = 0,
    .transitions = 0b0001101000110010};

static location_t forest = {
    .name = "Forest",
    .descriptions = {"Tall trees surround you.", "The forest is dark and cool.",
                     "You hear distant growling.", "The forest is silent."},
    .objects = &forest_objects,
    .exits = (struct locations_t *)&forest_exits,
    .traits = 0,
    .transitions = 0b0001101000110010};

static location_t bridge = {
    .name = "Bridge",
    .descriptions = {"A rickety bridge crosses a deep chasm.",
                     "The bridge sways in the wind.",
                     "The bridge is stained with blood.",
                     "The bridge is safe and quiet."},
    .objects = &bridge_objects,
    .exits = (struct locations_t *)&bridge_exits,
    .traits = 0,
    .transitions = 0b0001101000110010};

// --- All locations and objects buffers ---
static locations_t all_locations = {
    .length = 3, .used = 3, .data = {&clearing, &forest, &bridge}};

static objects_t all_objects = {
    .length = 3, .used = 3, .data = {&sword, &apple, &troll}};

// --- Digest function ---
static game_state_t bridge_troll_digest(world_state_t *state) {
  if (!state->inventory)
    return GAME_STATE_CONTINUE;
  for (size_t i = 0; i < state->inventory->used; ++i) {
    object_t *object = bufAt(state->inventory, i);
    if (object->name == sword.name) {
      return GAME_STATE_VICTORY;
    }
  }
  if (state->health == 0)
    return GAME_STATE_DEAD;
  return GAME_STATE_CONTINUE;
}

// --- Setting and world ---
static const setting_t bridge_troll_setting = {
    .premise = "Defeat the troll and cross the bridge to win.",
    .initial_location = &forest,
    .digest = bridge_troll_digest};

static objects_t inventory = {
    .length = 5,
    .used = 0,
    .data = {NULL, NULL, NULL, NULL, NULL},
};

world_t world = {
    .setting = bridge_troll_setting,
    .state = {.turns = 0, .health = 20, .damage = 2, .inventory = &inventory},
    .locations = &all_locations,
    .objects = &all_objects};
