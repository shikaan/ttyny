#include "../src/world.h"

// --- State descriptions for objects ---
static state_descriptions_t torch_states =
    bufConst(3, "on the ground", "held", "lit");

static state_descriptions_t shard_states =
    bufConst(3, "embedded", "held", "glowing");

static state_descriptions_t coin_states =
    bufConst(3, "buried", "held", "tossed");

static state_descriptions_t herb_states =
    bufConst(3, "growing", "held", "used");

static state_descriptions_t key_states =
    bufConst(3, "hidden", "held", "used");

static state_descriptions_t rune_states =
    bufConst(3, "dormant", "held", "activated");

static state_descriptions_t golem_states =
    bufConst(3, "dormant", "awakened", "friendly");

// --- State descriptions for locations ---
static state_descriptions_t grove_states = bufConst(1, "lush");
static state_descriptions_t riverbank_states = bufConst(1, "calm");
static state_descriptions_t cave_states = bufConst(1, "sparkling");
static state_descriptions_t tower_states = bufConst(1, "crumbling");
static state_descriptions_t portal_chamber_states =
    bufConst(2, "inactive", "active");

// --- Transitions for objects ---
static transitions_t torch_transitions =
    bufConst(4,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2},
             {.trigger = ACTION_DROP, .from = 1, .to = 0},
             {.trigger = ACTION_DROP, .from = 2, .to = 0});

static transitions_t shard_transitions =
    bufConst(2,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2});

static transitions_t coin_transitions =
    bufConst(2,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_DROP, .from = 1, .to = 2});

static transitions_t herb_transitions =
    bufConst(3,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2},
             {.trigger = ACTION_DROP, .from = 1, .to = 0});

static transitions_t key_transitions =
    bufConst(2,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2});

static transitions_t rune_transitions =
    bufConst(2,
             {.trigger = ACTION_TAKE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2});

static transitions_t golem_transitions =
    bufConst(2,
             {.trigger = ACTION_EXAMINE, .from = 0, .to = 1},
             {.trigger = ACTION_USE, .from = 1, .to = 2});

// --- Transitions for locations ---
static transitions_t portal_location_transitions =
    bufConst(1, {.trigger = ACTION_USE, .from = 0, .to = 1});

// --- Objects ---
static item_t torch = {
    .object = {.name = "Torch",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "wooden,sooty",
               .state_descriptions = &torch_states,
               .traits = 0b00000001, // collectible
               .transitions = &torch_transitions}};

static item_t crystal_shard = {
    .object = {.name = "Crystal Shard",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "prismatic,cold",
               .state_descriptions = &shard_states,
               .traits = 0b00000001, // collectible
               .transitions = &shard_transitions}};

static item_t ancient_coin = {
    .object = {.name = "Ancient Coin",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "golden,worn",
               .state_descriptions = &coin_states,
               .traits = 0b00000001, // collectible
               .transitions = &coin_transitions}};

static item_t healing_herb = {
    .object = {.name = "Healing Herb",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "fragrant,green",
               .state_descriptions = &herb_states,
               .traits = 0b00000001, // collectible
               .transitions = &herb_transitions}};

static item_t mystic_key = {
    .object = {.name = "Mystic Key",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "ornate,silver",
               .state_descriptions = &key_states,
               .traits = 0b00000001, // collectible
               .transitions = &key_transitions}};

static item_t portal_rune = {
    .object = {.name = "Portal Rune",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "etched,stone",
               .state_descriptions = &rune_states,
               .traits = 0b00000001, // collectible
               .transitions = &rune_transitions}};

static item_t stone_golem = {
    .object = {.name = "Stone Golem",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "massive,cracked,ancient",
               .state_descriptions = &golem_states,
               .traits = 0b00000000,
               .transitions = &golem_transitions}};

// --- Object buffers ---
// The item list for every room needs to be as long as all objects
static items_t grove_objects = {7, 2,
                                {&torch, &healing_herb, NULL, NULL, NULL, NULL,
                                 NULL}};
static items_t riverbank_objects = {7, 1,
                                    {&ancient_coin, NULL, NULL, NULL, NULL,
                                     NULL, NULL}};
static items_t cave_objects = {7, 2,
                               {&crystal_shard, &stone_golem, NULL, NULL, NULL,
                                NULL, NULL}};
static items_t tower_objects = {7, 1,
                                {&mystic_key, NULL, NULL, NULL, NULL, NULL,
                                 NULL}};
static items_t portal_objects = {7, 1,
                                 {&portal_rune, NULL, NULL, NULL, NULL, NULL,
                                  NULL}};

// --- Forward declarations for locations ---
static locations_t grove_exits, riverbank_exits, cave_exits, tower_exits,
    portal_exits;
static location_t ancient_grove, riverbank, crystal_cave, ruined_tower,
    portal_chamber;

// --- Location buffers for exits ---
static locations_t grove_exits = {3, 3,
                                  {(struct location_t *)&riverbank,
                                   (struct location_t *)&crystal_cave,
                                   (struct location_t *)&ruined_tower}};

static locations_t riverbank_exits = {
    2, 2, {(struct location_t *)&ancient_grove,
           (struct location_t *)&crystal_cave}};

static locations_t cave_exits = {
    3, 3, {(struct location_t *)&ancient_grove,
           (struct location_t *)&riverbank,
           (struct location_t *)&portal_chamber}};

static locations_t tower_exits = {
    1, 1, {(struct location_t *)&ancient_grove},
};

static locations_t portal_exits = {
    1, 1, {(struct location_t *)&crystal_cave},
};

// --- Locations ---
static location_t ancient_grove = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Ancient Grove",
               .current_state = 0,
               .description = "towering oaks,faint whispers",
               .state_descriptions = &grove_states,
               .traits = 0,
               .transitions = NULL},
    .items = &grove_objects,
    .exits = &grove_exits,
};

static location_t riverbank = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Riverbank",
               .current_state = 0,
               .description = "gentle current,pebbled shore",
               .state_descriptions = &riverbank_states,
               .traits = 0,
               .transitions = NULL},
    .items = &riverbank_objects,
    .exits = &riverbank_exits,
};

static location_t crystal_cave = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Crystal Cave",
               .current_state = 0,
               .description = "echoing,glittering walls",
               .state_descriptions = &cave_states,
               .traits = 0,
               .transitions = NULL},
    .items = &cave_objects,
    .exits = &cave_exits,
};

static location_t ruined_tower = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Ruined Tower",
               .current_state = 0,
               .description = "ivy-clad,broken stair",
               .state_descriptions = &tower_states,
               .traits = 0,
               .transitions = NULL},
    .items = &tower_objects,
    .exits = &tower_exits,
};

static location_t portal_chamber = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Portal Chamber",
               .current_state = 0, // inactive
               .description = "ringed runes,thin air",
               .state_descriptions = &portal_chamber_states,
               .traits = 0,
               .transitions = &portal_location_transitions},
    .items = &portal_objects,
    .exits = &portal_exits,
};

// --- All locations and objects buffers ---
static locations_t all_locations = {
    5,
    5,
    {
        (struct location_t *)&ancient_grove,
        (struct location_t *)&riverbank,
        (struct location_t *)&crystal_cave,
        (struct location_t *)&ruined_tower,
        (struct location_t *)&portal_chamber,
    },
};

static items_t all_objects = {
    7, 7, {&torch, &crystal_shard, &ancient_coin, &healing_herb, &mystic_key,
           &portal_rune, &stone_golem}};

// --- Digest function ---
static game_state_t ancient_portal_digest(state_t *state) {
  int hasActivatedRune = 0;
  int hasGlowingShard = 0;
  int hasUsedKey = 0;

  for (size_t i = 0; i < state->inventory->used; ++i) {
    item_t *object = bufAt(state->inventory, i);
    if (object->object.name == portal_rune.object.name &&
        object->object.current_state == 2) {
      hasActivatedRune = 1;
    } else if (object->object.name == crystal_shard.object.name &&
               object->object.current_state == 2) {
      hasGlowingShard = 1;
    } else if (object->object.name == mystic_key.object.name &&
               object->object.current_state == 2) {
      hasUsedKey = 1;
    }
  }

  if (hasActivatedRune && hasGlowingShard && hasUsedKey)
    return GAME_STATE_VICTORY;

  return GAME_STATE_CONTINUE;
}

// --- Setting and world ---
static items_t ancient_portal_inventory = {
    .length = 7,
    .used = 0,
    .data = {NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

world_t ancient_portal_world = {
    .state = {.turns = 0, .inventory = &ancient_portal_inventory},
    .locations = &all_locations,
    .digest = ancient_portal_digest,
    .current_location = &ancient_grove,
    .items = &all_objects};
