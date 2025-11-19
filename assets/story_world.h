#pragma once
// Auto-generated adventure world header (static asset - single init call)
// Depends only on world.h
#include "../src/world.h"

// Victory conditions:
// 1. Restore Lighthouse Beacon (LIGHTHOUSE_BASE state -> beacon restored).
// 2. Assemble Museum Exhibition (CURATOR_LEDGER -> completed entries AND
// ARCHIVE_GALLERY -> exhibition assembled). Death: Laboratory breach
// (SEALED_LABORATORY -> breached OR VOLATILE_PHIAL -> ruptured).

// All data is statically allocated. Call story_world_init() once to get world.

// Static data arrays - no cleanup needed
static const char *compass_state_descs[] = {"inactive",
                                            "aligned to true north"};
static const char *map_state_descs[] = {"legible"};
static const char *key_state_descs[] = {"intact"};
static const char *handle_state_descs[] = {"unmounted"};
static const char *lens_state_descs[] = {"dim", "glowing with red light"};
static const char *emitter_state_descs[] = {"unassembled", "mounted",
                                            "activated"};
static const char *ledger_state_descs[] = {"blank", "entries recorded",
                                           "completed entries"};
static const char *phial_state_descs[] = {"sealed", "ruptured"};
static const char *switch_state_descs[] = {"off", "activated"};
static const char *cliff_state_descs[] = {"normal"};
static const char *base_state_descs[] = {"dark", "beacon restored"};
static const char *gallery_state_descs[] = {"vacant", "exhibition assembled"};
static const char *workshop_state_descs[] = {"intact"};
static const char *fog_state_descs[] = {"hazy"};
static const char *lab_state_descs[] = {"stable", "breached"};

// Macro to make static buffer from array
#define STATIC_BUF(name, type, array)                                          \
  static struct {                                                              \
    size_t length;                                                             \
    size_t used;                                                               \
    type data[arrLen(array)];                                                  \
  } name = {arrLen(array), arrLen(array), {[0 ... arrLen(array) - 1] = 0}}

// State description buffers
STATIC_BUF(compass_states, const char *, compass_state_descs);
STATIC_BUF(map_states, const char *, map_state_descs);
STATIC_BUF(key_states, const char *, key_state_descs);
STATIC_BUF(handle_states, const char *, handle_state_descs);
STATIC_BUF(lens_states, const char *, lens_state_descs);
STATIC_BUF(emitter_states, const char *, emitter_state_descs);
STATIC_BUF(ledger_states, const char *, ledger_state_descs);
STATIC_BUF(phial_states, const char *, phial_state_descs);
STATIC_BUF(switch_states, const char *, switch_state_descs);
STATIC_BUF(cliff_states, const char *, cliff_state_descs);
STATIC_BUF(base_states, const char *, base_state_descs);
STATIC_BUF(gallery_states, const char *, gallery_state_descs);
STATIC_BUF(workshop_states, const char *, workshop_state_descs);
STATIC_BUF(fog_states, const char *, fog_state_descs);
STATIC_BUF(lab_states, const char *, lab_state_descs);

// Forward declarations for items/locations
static item_t ANTIQUE_COMPASS;
static item_t WEATHERED_MAP_FRAGMENT;
static item_t SILVER_FILIGREE_KEY;
static item_t ENGRAVED_WINDING_HANDLE;
static item_t PRISMATIC_LENS_CORE;
static item_t BRONZE_SIGNAL_EMITTER;
static item_t CURATOR_LEDGER;
static item_t VOLATILE_PHIAL;
static item_t RUSTED_SWITCH;

static location_t CLIFF_APPROACH;
static location_t LIGHTHOUSE_BASE;
static location_t ARCHIVE_GALLERY;
static location_t MECHANICAL_WORKSHOP;
static location_t FOG_CHAMBER;
static location_t SEALED_LABORATORY;

// Transition buffers
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} compass_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} lens_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[2];
} emitter_trans = {2, 2, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[2];
} ledger_trans = {2, 2, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} phial_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} switch_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} base_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} gallery_trans = {1, 1, {{0}}};
static struct {
  size_t length;
  size_t used;
  transition_t data[1];
} lab_trans = {1, 1, {{0}}};

// Required items buffers
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} emitter_req_mount = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[3];
} emitter_req_activate = {3, 3, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} ledger_req_entries = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[5];
} ledger_req_complete = {5, 5, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} switch_req = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} base_req = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} gallery_req = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} lab_req = {1, 1, {NULL}};

// Location items
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} cliff_items = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} base_items = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[2];
} gallery_items = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} workshop_items = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} fog_items = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct item_t *data[1];
} lab_items = {1, 1, {NULL}};

// Location exits
static struct {
  size_t length;
  size_t used;
  struct location_t *data[2];
} cliff_exits = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[2];
} base_exits = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[2];
} gallery_exits = {2, 2, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[1];
} workshop_exits = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[1];
} fog_exits = {1, 1, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[1];
} lab_exits = {1, 1, {NULL}};

// World collections
static struct {
  size_t length;
  size_t used;
  struct item_t *data[8];
} WORLD_ITEMS = {8, 8, {NULL}};
static struct {
  size_t length;
  size_t used;
  struct location_t *data[6];
} WORLD_LOCATIONS = {6, 6, {NULL}};

static game_state_t world_digest(world_t *world) {
  int beacon_restored = LIGHTHOUSE_BASE.object.current_state == 1;
  int exhibition_complete = ARCHIVE_GALLERY.object.current_state == 1 &&
                            CURATOR_LEDGER.object.current_state == 2;
  int laboratory_breached = SEALED_LABORATORY.object.current_state == 1;
  int phial_ruptured = VOLATILE_PHIAL.object.current_state == 1;

  if (laboratory_breached) {
    world->current_end_game = "sealed laboratory breached";
    return GAME_STATE_DEAD;
  }
  if (phial_ruptured) {
    world->current_end_game = "volatile phial ruptured";
    return GAME_STATE_DEAD;
  }
  if (beacon_restored) {
    world->current_end_game = "lighthouse beacon restored";
    return GAME_STATE_VICTORY;
  }
  if (exhibition_complete) {
    world->current_end_game = "museum exhibition assembled";
    return GAME_STATE_VICTORY;
  }
  return GAME_STATE_CONTINUE;
}

// Single initialization function - call once, no cleanup needed
static inline void story_world_init(world_t *world) {
  // Copy state descriptions
  for (size_t i = 0; i < arrLen(compass_state_descs); i++)
    compass_states.data[i] = compass_state_descs[i];
  for (size_t i = 0; i < arrLen(map_state_descs); i++)
    map_states.data[i] = map_state_descs[i];
  for (size_t i = 0; i < arrLen(key_state_descs); i++)
    key_states.data[i] = key_state_descs[i];
  for (size_t i = 0; i < arrLen(handle_state_descs); i++)
    handle_states.data[i] = handle_state_descs[i];
  for (size_t i = 0; i < arrLen(lens_state_descs); i++)
    lens_states.data[i] = lens_state_descs[i];
  for (size_t i = 0; i < arrLen(emitter_state_descs); i++)
    emitter_states.data[i] = emitter_state_descs[i];
  for (size_t i = 0; i < arrLen(ledger_state_descs); i++)
    ledger_states.data[i] = ledger_state_descs[i];
  for (size_t i = 0; i < arrLen(phial_state_descs); i++)
    phial_states.data[i] = phial_state_descs[i];
  for (size_t i = 0; i < arrLen(switch_state_descs); i++)
    switch_states.data[i] = switch_state_descs[i];
  for (size_t i = 0; i < arrLen(cliff_state_descs); i++)
    cliff_states.data[i] = cliff_state_descs[i];
  for (size_t i = 0; i < arrLen(base_state_descs); i++)
    base_states.data[i] = base_state_descs[i];
  for (size_t i = 0; i < arrLen(gallery_state_descs); i++)
    gallery_states.data[i] = gallery_state_descs[i];
  for (size_t i = 0; i < arrLen(workshop_state_descs); i++)
    workshop_states.data[i] = workshop_state_descs[i];
  for (size_t i = 0; i < arrLen(fog_state_descs); i++)
    fog_states.data[i] = fog_state_descs[i];
  for (size_t i = 0; i < arrLen(lab_state_descs); i++)
    lab_states.data[i] = lab_state_descs[i];

  // Initialize items
  ANTIQUE_COMPASS = (item_t){
      .object = {.name = "antique compass",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Brass compass with a cracked glass face; needle "
                     "twitches near the coast.",
                 .state_descriptions = (descriptions_t *)&compass_states,
                 .transitions = (transitions_t *)&compass_trans},
      .collectible = true};
  compass_trans.data[0] = (transition_t){.trigger = ACTION_TYPE_EXAMINE,
                                         .from = 0,
                                         .to = 1,
                                         .required_items = NULL};

  WEATHERED_MAP_FRAGMENT = (item_t){
      .object = {.name = "weathered map fragment",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Torn parchment showing rocky inlet and a sketched "
                     "lantern symbol.",
                 .state_descriptions = (descriptions_t *)&map_states,
                 .transitions = NULL},
      .collectible = true};

  SILVER_FILIGREE_KEY = (item_t){
      .object = {.name = "silver filigree key",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Intricate key whose bow forms a lighthouse outline.",
                 .state_descriptions = (descriptions_t *)&key_states,
                 .transitions = NULL},
      .collectible = true};

  ENGRAVED_WINDING_HANDLE = (item_t){
      .object = {.name = "engraved winding handle",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Cast iron handle etched with tidal markings; fits "
                     "a heavy mechanism.",
                 .state_descriptions = (descriptions_t *)&handle_states,
                 .transitions = NULL},
      .collectible = true};

  PRISMATIC_LENS_CORE = (item_t){
      .object = {.name = "prismatic lens core",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Faceted crystal cylinder scattering thin rainbows.",
                 .state_descriptions = (descriptions_t *)&lens_states,
                 .transitions = (transitions_t *)&lens_trans},
      .collectible = true};
  lens_trans.data[0] = (transition_t){
      .trigger = ACTION_TYPE_USE, .from = 0, .to = 1, .required_items = NULL};

  emitter_req_mount.data[0] = &ENGRAVED_WINDING_HANDLE;
  emitter_req_mount.data[1] = &PRISMATIC_LENS_CORE;
  emitter_req_activate.data[0] = &ENGRAVED_WINDING_HANDLE;
  emitter_req_activate.data[1] = &PRISMATIC_LENS_CORE;
  emitter_req_activate.data[2] = &SILVER_FILIGREE_KEY;

  BRONZE_SIGNAL_EMITTER = (item_t){
      .object = {.name = "bronze signal emitter",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Circular bronze frame with mounting brackets and "
                     "empty socket.",
                 .state_descriptions = (descriptions_t *)&emitter_states,
                 .transitions = (transitions_t *)&emitter_trans},
      .collectible = true};
  emitter_trans.data[0] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 0,
                     .to = 1,
                     .required_items = (items_t *)&emitter_req_mount};
  emitter_trans.data[1] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 1,
                     .to = 2,
                     .required_items = (items_t *)&emitter_req_activate};

  ledger_req_entries.data[0] = &ANTIQUE_COMPASS;
  ledger_req_entries.data[1] = &WEATHERED_MAP_FRAGMENT;
  ledger_req_complete.data[0] = &ANTIQUE_COMPASS;
  ledger_req_complete.data[1] = &WEATHERED_MAP_FRAGMENT;
  ledger_req_complete.data[2] = &PRISMATIC_LENS_CORE;
  ledger_req_complete.data[3] = &SILVER_FILIGREE_KEY;
  ledger_req_complete.data[4] = &ENGRAVED_WINDING_HANDLE;

  CURATOR_LEDGER = (item_t){
      .object = {.name = "curator ledger",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Leather bound ledger listing exhibit slots and "
                     "point tallies.",
                 .state_descriptions = (descriptions_t *)&ledger_states,
                 .transitions = (transitions_t *)&ledger_trans},
      .collectible = true};
  ledger_trans.data[0] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 0,
                     .to = 1,
                     .required_items = (items_t *)&ledger_req_entries};
  ledger_trans.data[1] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 1,
                     .to = 2,
                     .required_items = (items_t *)&ledger_req_complete};

  VOLATILE_PHIAL =
      (item_t){.object = {.name = "volatile phial",
                          .type = OBJECT_TYPE_ITEM,
                          .current_state = 0,
                          .description =
                              "Thin glass phial housing swirling black vapor; "
                              "label warns about breach.",
                          .state_descriptions = (descriptions_t *)&phial_states,
                          .transitions = (transitions_t *)&phial_trans},
               .collectible = true};
  phial_trans.data[0] = (transition_t){
      .trigger = ACTION_TYPE_USE, .from = 0, .to = 1, .required_items = NULL};

  switch_req.data[0] = &BRONZE_SIGNAL_EMITTER;

  RUSTED_SWITCH = (item_t){
      .object = {.name = "rusted switch",
                 .type = OBJECT_TYPE_ITEM,
                 .current_state = 0,
                 .description =
                     "Wall mounted corroded lever near beacon machinery.",
                 .state_descriptions = (descriptions_t *)&switch_states,
                 .transitions = (transitions_t *)&switch_trans},
      .collectible = false};
  switch_trans.data[0] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 0,
                     .to = 1,
                     .required_items = (items_t *)&switch_req};

  // Initialize location items
  cliff_items.data[0] = &ANTIQUE_COMPASS;
  cliff_items.data[1] = &WEATHERED_MAP_FRAGMENT;
  base_items.data[0] = &BRONZE_SIGNAL_EMITTER;
  base_items.data[1] = &RUSTED_SWITCH;
  gallery_items.data[0] = &CURATOR_LEDGER;
  gallery_items.data[1] = &SILVER_FILIGREE_KEY;
  workshop_items.data[0] = &ENGRAVED_WINDING_HANDLE;
  fog_items.data[0] = &PRISMATIC_LENS_CORE;
  lab_items.data[0] = &VOLATILE_PHIAL;

  // Initialize locations
  CLIFF_APPROACH = (location_t){
      .object = {.name = "cliff approach",
                 .type = OBJECT_TYPE_LOCATION,
                 .current_state = 0,
                 .description = "Grassy path descending toward fog veiled "
                                "lighthouse base.",
                 .state_descriptions = (descriptions_t *)&cliff_states,
                 .transitions = NULL},
      .items = (items_t *)&cliff_items,
      .exits = (locations_t *)&cliff_exits};

  base_req.data[0] = &BRONZE_SIGNAL_EMITTER;
  base_req.data[1] = &RUSTED_SWITCH;

  LIGHTHOUSE_BASE = (location_t){
      .object = {.name = "lighthouse base",
                 .type = OBJECT_TYPE_LOCATION,
                 .current_state = 0,
                 .description =
                     "Stone foundation chamber echoing with distant wave "
                     "impacts.",
                 .state_descriptions = (descriptions_t *)&base_states,
                 .transitions = (transitions_t *)&base_trans},
      .items = (items_t *)&base_items,
      .exits = (locations_t *)&base_exits};
  base_trans.data[0] = (transition_t){.trigger = ACTION_TYPE_USE,
                                      .from = 0,
                                      .to = 1,
                                      .required_items = (items_t *)&base_req};

  gallery_req.data[0] = &CURATOR_LEDGER;

  ARCHIVE_GALLERY = (location_t){
      .object = {.name = "archive gallery",
                 .type = OBJECT_TYPE_LOCATION,
                 .current_state = 0,
                 .description =
                     "Dust layered exhibit hall with vacant display pedestals.",
                 .state_descriptions = (descriptions_t *)&gallery_states,
                 .transitions = (transitions_t *)&gallery_trans},
      .items = (items_t *)&gallery_items,
      .exits = (locations_t *)&gallery_exits};
  gallery_trans.data[0] =
      (transition_t){.trigger = ACTION_TYPE_USE,
                     .from = 0,
                     .to = 1,
                     .required_items = (items_t *)&gallery_req};

  MECHANICAL_WORKSHOP = (location_t){
      .object = {.name = "mechanical workshop",
                 .type = OBJECT_TYPE_LOCATION,
                 .current_state = 0,
                 .description = "Workbenches with spare cogs and a dismantled "
                                "winding drum.",
                 .state_descriptions = (descriptions_t *)&workshop_states,
                 .transitions = NULL},
      .items = (items_t *)&workshop_items,
      .exits = (locations_t *)&workshop_exits};

  FOG_CHAMBER = (location_t){
      .object =
          {.name = "fog chamber",
           .type = OBJECT_TYPE_LOCATION,
           .current_state = 0,
           .description =
               "Circular room where dense vapor curls around a lens mount.",
           .state_descriptions = (descriptions_t *)&fog_states,
           .transitions = NULL},
      .items = (items_t *)&fog_items,
      .exits = (locations_t *)&fog_exits};

  lab_req.data[0] = &SILVER_FILIGREE_KEY;

  SEALED_LABORATORY = (location_t){
      .object = {.name = "sealed laboratory",
                 .type = OBJECT_TYPE_LOCATION,
                 .current_state = 0,
                 .description =
                     "Steel door enclave containing experimental volatile "
                     "compounds.",
                 .state_descriptions = (descriptions_t *)&lab_states,
                 .transitions = (transitions_t *)&lab_trans},
      .items = (items_t *)&lab_items,
      .exits = (locations_t *)&lab_exits};
  lab_trans.data[0] = (transition_t){.trigger = ACTION_TYPE_MOVE,
                                     .from = 0,
                                     .to = 1,
                                     .required_items = (items_t *)&lab_req};

  // Connect exits
  cliff_exits.data[0] = (struct location_t *)&LIGHTHOUSE_BASE;
  cliff_exits.data[1] = (struct location_t *)&ARCHIVE_GALLERY;
  base_exits.data[0] = (struct location_t *)&FOG_CHAMBER;
  base_exits.data[1] = (struct location_t *)&MECHANICAL_WORKSHOP;
  gallery_exits.data[0] = (struct location_t *)&CLIFF_APPROACH;
  gallery_exits.data[1] = (struct location_t *)&SEALED_LABORATORY;
  workshop_exits.data[0] = (struct location_t *)&LIGHTHOUSE_BASE;
  fog_exits.data[0] = (struct location_t *)&LIGHTHOUSE_BASE;
  lab_exits.data[0] = (struct location_t *)&ARCHIVE_GALLERY;

  // Build world collections
  WORLD_ITEMS.data[0] = &ANTIQUE_COMPASS;
  WORLD_ITEMS.data[1] = &WEATHERED_MAP_FRAGMENT;
  WORLD_ITEMS.data[2] = &SILVER_FILIGREE_KEY;
  WORLD_ITEMS.data[3] = &ENGRAVED_WINDING_HANDLE;
  WORLD_ITEMS.data[4] = &PRISMATIC_LENS_CORE;
  WORLD_ITEMS.data[5] = &BRONZE_SIGNAL_EMITTER;
  WORLD_ITEMS.data[6] = &CURATOR_LEDGER;
  WORLD_ITEMS.data[7] = &VOLATILE_PHIAL;

  WORLD_LOCATIONS.data[0] = (struct location_t *)&CLIFF_APPROACH;
  WORLD_LOCATIONS.data[1] = (struct location_t *)&LIGHTHOUSE_BASE;
  WORLD_LOCATIONS.data[2] = (struct location_t *)&ARCHIVE_GALLERY;
  WORLD_LOCATIONS.data[3] = (struct location_t *)&MECHANICAL_WORKSHOP;
  WORLD_LOCATIONS.data[4] = (struct location_t *)&FOG_CHAMBER;
  WORLD_LOCATIONS.data[5] = (struct location_t *)&SEALED_LABORATORY;

  // Initialize world
  world->state.turns = 0;
  world->state.inventory = itemsCreate(16);
  world->locations = (locations_t *)&WORLD_LOCATIONS;
  world->items = (items_t *)&WORLD_ITEMS;
  world->current_location = &CLIFF_APPROACH;
  world->current_end_game = NULL;
  world->digest = world_digest;
}

// Static world instance - call story_world_init() to initialize
static world_t story_world;
