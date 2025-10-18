#include "../src/world.h"

// --- Objects ---
static object_t sword = {
    .name = "Old Sword",
    .description = "rusty,heavy,old",
    .states =
        {
            "on the ground",
            "held",
            "chipped",
            "broken",
        },
    .traits = 0b00000011, // collectible + change damage
    .value = 5,
    .transitions = 0b0001101101101010,
};

static object_t apple = {
    .name = "Red Apple",
    .description = "shiny,red",
    .states = {"on a tree", "held", "bitten", "eaten"},
    .traits = 0b00000101, // collectible + change health
    .value = 10,
    .transitions = 0b0001101101101010,
};

static object_t troll = {
    .name = "Bridge Troll",
    .description = "grumpy,smelly,huge,violent",
    .states =
        {
            "standing",
            "aggressive",
            "wounded",
            "fled",
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
static locations_t clearing_exits = {
    .length = 1, .used = 1, .data = {(struct location_t *)&forest}};

static locations_t forest_exits = {
    .length = 2,
    .used = 2,
    .data = {(struct location_t *)&clearing, (struct location_t *)&bridge}};

static locations_t bridge_exits = {
    .length = 1, .used = 1, .data = {(struct location_t *)&forest}};

// --- Locations ---
static location_t clearing = {.name = "Clearing",
                              .description = "sunny,grassy,peaceful",
                              .states = {"sunny"},
                              .objects = &clearing_objects,
                              .exits = &clearing_exits,
                              .traits = 0,
                              .transitions = 0};

static location_t forest = {.name = "Forest",
                            .description = "dark,cool,ominous",
                            .states = {"dark"},
                            .objects = &forest_objects,
                            .exits = &forest_exits,
                            .traits = 0,
                            .transitions = 0};

static location_t bridge = {
    .name = "Bridge",
    .description = "rickety,over chasm,windy",
    .states = {"safe", "swaying"},
    .objects = &bridge_objects,
    .exits = &bridge_exits,
    .traits = 0,
    .transitions = 0b0001101000110010};

// --- All locations and objects buffers ---
static locations_t all_locations = {.length = 3,
                                    .used = 3,
                                    .data = {(struct location_t *)&clearing,
                                             (struct location_t *)&forest,
                                             (struct location_t *)&bridge}};

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
static objects_t inventory = {
    .length = 5,
    .used = 0,
    .data = {NULL, NULL, NULL, NULL, NULL},
};

world_t world = {
    .context = "autumn,dusk,misty,cold,muddy,silent,distant crows,"
               "woodsmoke,mossy,rickety,sluggish,peasant",
    .state = {.turns = 0, .health = 20, .damage = 2, .inventory = &inventory},
    .locations = &all_locations,
    .digest = bridge_troll_digest,
    .current_location = &forest,
    .objects = &all_objects};
