#pragma once
// Auto-generated adventure world header (runtime buffer setup)
// Depends only on world.h
#include "../src/world.h"

// Victory conditions:
// 1. Restore Lighthouse Beacon (LIGHTHOUSE_BASE state -> beacon restored).
// 2. Assemble Museum Exhibition (CURATOR_LEDGER -> completed entries AND
// ARCHIVE_GALLERY -> exhibition assembled). Death: Laboratory breach
// (SEALED_LABORATORY -> breached OR VOLATILE_PHIAL -> ruptured).

// We avoid static initialization of flexible array members; create buffers at
// init.

static item_t ANTIQUE_COMPASS; // states: inactive -> aligned to true north
static item_t WEATHERED_MAP_FRAGMENT;  // legible
static item_t SILVER_FILIGREE_KEY;     // intact
static item_t ENGRAVED_WINDING_HANDLE; // unmounted
static item_t PRISMATIC_LENS_CORE;     // dim -> glowing with red light
static item_t BRONZE_SIGNAL_EMITTER;   // unassembled -> mounted -> activated
static item_t CURATOR_LEDGER; // blank -> entries recorded -> completed entries
static item_t VOLATILE_PHIAL; // sealed -> ruptured
static item_t RUSTED_SWITCH;  // off -> activated (non-collectible)

static location_t CLIFF_APPROACH;      // normal
static location_t LIGHTHOUSE_BASE;     // dark -> beacon restored
static location_t ARCHIVE_GALLERY;     // vacant -> exhibition assembled
static location_t MECHANICAL_WORKSHOP; // intact
static location_t FOG_CHAMBER;         // hazy
static location_t SEALED_LABORATORY;   // stable -> breached

static items_t *WORLD_ITEMS = NULL; // collectible items only
static locations_t *WORLD_LOCATIONS = NULL;

// Helper to build state descriptions buffer
static state_descriptions_t *make_states(size_t n, const char **descs) {
  state_descriptions_t *buf = NULL;
  makeBufCreate(state_descriptions_t, const char *, buf, n);
  for (size_t i = 0; i < n; i++) {
    bufPush(buf, descs[i]);
  }
  return buf;
}

static transitions_t *make_transitions(size_t n) {
  transitions_t *buf = NULL;
  makeBufCreate(transitions_t, transition_t, buf, n);
  return buf;
}

static items_t *make_items_list(size_t n) {
  items_t *buf = NULL;
  makeBufCreate(items_t, struct item_t *, buf, n);
  return buf;
}

static locations_t *make_locations_list(size_t n) {
  locations_t *buf = NULL;
  makeBufCreate(locations_t, struct location_t *, buf, n);
  return buf;
}

static void build_items(void) {
  // ANTIQUE_COMPASS
  ANTIQUE_COMPASS.object.name = "antique compass";
  ANTIQUE_COMPASS.object.type = OBJECT_TYPE_ITEM;
  ANTIQUE_COMPASS.object.current_state = 0;
  ANTIQUE_COMPASS.object.description = "Brass compass with a cracked glass "
                                       "face; needle twitches near the coast.";
  const char *compass_states[] = {"inactive", "aligned to true north"};
  ANTIQUE_COMPASS.object.state_descriptions = make_states(2, compass_states);
  ANTIQUE_COMPASS.object.transitions = make_transitions(1);
  transition_t compass_examine = {.trigger = ACTION_TYPE_EXAMINE,
                                  .from = 0,
                                  .to = 1,
                                  .required_items = NULL};
  bufPush(ANTIQUE_COMPASS.object.transitions, compass_examine);
  ANTIQUE_COMPASS.collectible = true;

  WEATHERED_MAP_FRAGMENT.object.name = "weathered map fragment";
  WEATHERED_MAP_FRAGMENT.object.type = OBJECT_TYPE_ITEM;
  WEATHERED_MAP_FRAGMENT.object.current_state = 0;
  WEATHERED_MAP_FRAGMENT.object.description =
      "Torn parchment showing rocky inlet and a sketched lantern symbol.";
  const char *map_states[] = {"legible"};
  WEATHERED_MAP_FRAGMENT.object.state_descriptions = make_states(1, map_states);
  WEATHERED_MAP_FRAGMENT.object.transitions = NULL;
  WEATHERED_MAP_FRAGMENT.collectible = true;

  SILVER_FILIGREE_KEY.object.name = "silver filigree key";
  SILVER_FILIGREE_KEY.object.type = OBJECT_TYPE_ITEM;
  SILVER_FILIGREE_KEY.object.current_state = 0;
  SILVER_FILIGREE_KEY.object.description =
      "Intricate key whose bow forms a lighthouse outline.";
  const char *key_states[] = {"intact"};
  SILVER_FILIGREE_KEY.object.state_descriptions = make_states(1, key_states);
  SILVER_FILIGREE_KEY.object.transitions = NULL;
  SILVER_FILIGREE_KEY.collectible = true;

  ENGRAVED_WINDING_HANDLE.object.name = "engraved winding handle";
  ENGRAVED_WINDING_HANDLE.object.type = OBJECT_TYPE_ITEM;
  ENGRAVED_WINDING_HANDLE.object.current_state = 0;
  ENGRAVED_WINDING_HANDLE.object.description =
      "Cast iron handle etched with tidal markings; fits a heavy mechanism.";
  const char *handle_states[] = {"unmounted"};
  ENGRAVED_WINDING_HANDLE.object.state_descriptions =
      make_states(1, handle_states);
  ENGRAVED_WINDING_HANDLE.object.transitions = NULL;
  ENGRAVED_WINDING_HANDLE.collectible = true;

  PRISMATIC_LENS_CORE.object.name = "prismatic lens core";
  PRISMATIC_LENS_CORE.object.type = OBJECT_TYPE_ITEM;
  PRISMATIC_LENS_CORE.object.current_state = 0;
  PRISMATIC_LENS_CORE.object.description =
      "Faceted crystal cylinder scattering thin rainbows.";
  const char *lens_states[] = {"dim", "glowing with red light"};
  PRISMATIC_LENS_CORE.object.state_descriptions = make_states(2, lens_states);
  PRISMATIC_LENS_CORE.object.transitions = make_transitions(1);
  transition_t lens_use = {
      .trigger = ACTION_TYPE_USE, .from = 0, .to = 1, .required_items = NULL};
  bufPush(PRISMATIC_LENS_CORE.object.transitions, lens_use);
  PRISMATIC_LENS_CORE.collectible = true;

  BRONZE_SIGNAL_EMITTER.object.name = "bronze signal emitter";
  BRONZE_SIGNAL_EMITTER.object.type = OBJECT_TYPE_ITEM;
  BRONZE_SIGNAL_EMITTER.object.current_state = 0;
  BRONZE_SIGNAL_EMITTER.object.description =
      "Circular bronze frame with mounting brackets and empty socket.";
  const char *emitter_states[] = {"unassembled", "mounted", "activated"};
  BRONZE_SIGNAL_EMITTER.object.state_descriptions =
      make_states(3, emitter_states);
  BRONZE_SIGNAL_EMITTER.object.transitions = make_transitions(2);
  items_t *emitter_req_mount = make_items_list(2);
  bufPush(emitter_req_mount, &ENGRAVED_WINDING_HANDLE);
  bufPush(emitter_req_mount, &PRISMATIC_LENS_CORE);
  transition_t emitter_mount = {.trigger = ACTION_TYPE_USE,
                                .from = 0,
                                .to = 1,
                                .required_items = emitter_req_mount};
  bufPush(BRONZE_SIGNAL_EMITTER.object.transitions, emitter_mount);
  items_t *emitter_req_activate = make_items_list(3);
  bufPush(emitter_req_activate, &ENGRAVED_WINDING_HANDLE);
  bufPush(emitter_req_activate, &PRISMATIC_LENS_CORE);
  bufPush(emitter_req_activate, &SILVER_FILIGREE_KEY);
  transition_t emitter_activate = {.trigger = ACTION_TYPE_USE,
                                   .from = 1,
                                   .to = 2,
                                   .required_items = emitter_req_activate};
  bufPush(BRONZE_SIGNAL_EMITTER.object.transitions, emitter_activate);
  BRONZE_SIGNAL_EMITTER.collectible = true;

  CURATOR_LEDGER.object.name = "curator ledger";
  CURATOR_LEDGER.object.type = OBJECT_TYPE_ITEM;
  CURATOR_LEDGER.object.current_state = 0;
  CURATOR_LEDGER.object.description =
      "Leather bound ledger listing exhibit slots and point tallies.";
  const char *ledger_states[] = {"blank", "entries recorded",
                                 "completed entries"};
  CURATOR_LEDGER.object.state_descriptions = make_states(3, ledger_states);
  CURATOR_LEDGER.object.transitions = make_transitions(2);
  items_t *ledger_req_entries = make_items_list(2);
  bufPush(ledger_req_entries, &ANTIQUE_COMPASS);
  bufPush(ledger_req_entries, &WEATHERED_MAP_FRAGMENT);
  transition_t ledger_entries = {.trigger = ACTION_TYPE_USE,
                                 .from = 0,
                                 .to = 1,
                                 .required_items = ledger_req_entries};
  bufPush(CURATOR_LEDGER.object.transitions, ledger_entries);
  items_t *ledger_req_complete = make_items_list(5);
  bufPush(ledger_req_complete, &ANTIQUE_COMPASS);
  bufPush(ledger_req_complete, &WEATHERED_MAP_FRAGMENT);
  bufPush(ledger_req_complete, &PRISMATIC_LENS_CORE);
  bufPush(ledger_req_complete, &SILVER_FILIGREE_KEY);
  bufPush(ledger_req_complete, &ENGRAVED_WINDING_HANDLE);
  transition_t ledger_complete = {.trigger = ACTION_TYPE_USE,
                                  .from = 1,
                                  .to = 2,
                                  .required_items = ledger_req_complete};
  bufPush(CURATOR_LEDGER.object.transitions, ledger_complete);
  CURATOR_LEDGER.collectible = true;

  VOLATILE_PHIAL.object.name = "volatile phial";
  VOLATILE_PHIAL.object.type = OBJECT_TYPE_ITEM;
  VOLATILE_PHIAL.object.current_state = 0;
  VOLATILE_PHIAL.object.description = "Thin glass phial housing swirling black "
                                      "vapor; label warns about breach.";
  const char *phial_states[] = {"sealed", "ruptured"};
  VOLATILE_PHIAL.object.state_descriptions = make_states(2, phial_states);
  VOLATILE_PHIAL.object.transitions = make_transitions(1);
  transition_t phial_use = {
      .trigger = ACTION_TYPE_USE, .from = 0, .to = 1, .required_items = NULL};
  bufPush(VOLATILE_PHIAL.object.transitions, phial_use);
  VOLATILE_PHIAL.collectible = true;

  RUSTED_SWITCH.object.name = "rusted switch";
  RUSTED_SWITCH.object.type = OBJECT_TYPE_ITEM;
  RUSTED_SWITCH.object.current_state = 0;
  RUSTED_SWITCH.object.description =
      "Wall mounted corroded lever near beacon machinery.";
  const char *switch_states[] = {"off", "activated"};
  RUSTED_SWITCH.object.state_descriptions = make_states(2, switch_states);
  RUSTED_SWITCH.object.transitions = make_transitions(1);
  items_t *switch_req = make_items_list(1);
  bufPush(switch_req, &BRONZE_SIGNAL_EMITTER);
  transition_t switch_use = {.trigger = ACTION_TYPE_USE,
                             .from = 0,
                             .to = 1,
                             .required_items = switch_req};
  bufPush(RUSTED_SWITCH.object.transitions, switch_use);
  RUSTED_SWITCH.collectible = false;
}

static void build_locations(void) {
  CLIFF_APPROACH.object.name = "cliff approach";
  CLIFF_APPROACH.object.type = OBJECT_TYPE_LOCATION;
  CLIFF_APPROACH.object.current_state = 0;
  CLIFF_APPROACH.object.description =
      "Grassy path descending toward fog veiled lighthouse base.";
  const char *cliff_states[] = {"normal"};
  CLIFF_APPROACH.object.state_descriptions = make_states(1, cliff_states);
  CLIFF_APPROACH.object.transitions = NULL;
  CLIFF_APPROACH.items = make_items_list(2);
  bufPush(CLIFF_APPROACH.items, &ANTIQUE_COMPASS);
  bufPush(CLIFF_APPROACH.items, &WEATHERED_MAP_FRAGMENT);
  CLIFF_APPROACH.exits = make_locations_list(2);

  LIGHTHOUSE_BASE.object.name = "lighthouse base";
  LIGHTHOUSE_BASE.object.type = OBJECT_TYPE_LOCATION;
  LIGHTHOUSE_BASE.object.current_state = 0;
  LIGHTHOUSE_BASE.object.description =
      "Stone foundation chamber echoing with distant wave impacts.";
  const char *base_states[] = {"dark", "beacon restored"};
  LIGHTHOUSE_BASE.object.state_descriptions = make_states(2, base_states);
  LIGHTHOUSE_BASE.object.transitions = make_transitions(1);
  items_t *base_req = make_items_list(2);
  bufPush(base_req, &BRONZE_SIGNAL_EMITTER);
  bufPush(base_req, &RUSTED_SWITCH);
  transition_t base_activate = {.trigger = ACTION_TYPE_USE,
                                .from = 0,
                                .to = 1,
                                .required_items = base_req};
  bufPush(LIGHTHOUSE_BASE.object.transitions, base_activate);
  LIGHTHOUSE_BASE.items = make_items_list(2);
  bufPush(LIGHTHOUSE_BASE.items, &BRONZE_SIGNAL_EMITTER);
  bufPush(LIGHTHOUSE_BASE.items, &RUSTED_SWITCH);
  LIGHTHOUSE_BASE.exits = make_locations_list(2);

  ARCHIVE_GALLERY.object.name = "archive gallery";
  ARCHIVE_GALLERY.object.type = OBJECT_TYPE_LOCATION;
  ARCHIVE_GALLERY.object.current_state = 0;
  ARCHIVE_GALLERY.object.description =
      "Dust layered exhibit hall with vacant display pedestals.";
  const char *gallery_states[] = {"vacant", "exhibition assembled"};
  ARCHIVE_GALLERY.object.state_descriptions = make_states(2, gallery_states);
  ARCHIVE_GALLERY.object.transitions = make_transitions(1);
  items_t *gallery_req = make_items_list(1);
  bufPush(gallery_req, &CURATOR_LEDGER);
  transition_t gallery_assemble = {.trigger = ACTION_TYPE_USE,
                                   .from = 0,
                                   .to = 1,
                                   .required_items = gallery_req};
  bufPush(ARCHIVE_GALLERY.object.transitions, gallery_assemble);
  ARCHIVE_GALLERY.items = make_items_list(2);
  bufPush(ARCHIVE_GALLERY.items, &CURATOR_LEDGER);
  bufPush(ARCHIVE_GALLERY.items, &SILVER_FILIGREE_KEY);
  ARCHIVE_GALLERY.exits = make_locations_list(2);

  MECHANICAL_WORKSHOP.object.name = "mechanical workshop";
  MECHANICAL_WORKSHOP.object.type = OBJECT_TYPE_LOCATION;
  MECHANICAL_WORKSHOP.object.current_state = 0;
  MECHANICAL_WORKSHOP.object.description =
      "Workbenches with spare cogs and a dismantled winding drum.";
  const char *workshop_states[] = {"intact"};
  MECHANICAL_WORKSHOP.object.state_descriptions =
      make_states(1, workshop_states);
  MECHANICAL_WORKSHOP.object.transitions = NULL;
  MECHANICAL_WORKSHOP.items = make_items_list(1);
  bufPush(MECHANICAL_WORKSHOP.items, &ENGRAVED_WINDING_HANDLE);
  MECHANICAL_WORKSHOP.exits = make_locations_list(1);

  FOG_CHAMBER.object.name = "fog chamber";
  FOG_CHAMBER.object.type = OBJECT_TYPE_LOCATION;
  FOG_CHAMBER.object.current_state = 0;
  FOG_CHAMBER.object.description =
      "Circular room where dense vapor curls around a lens mount.";
  const char *fog_states[] = {"hazy"};
  FOG_CHAMBER.object.state_descriptions = make_states(1, fog_states);
  FOG_CHAMBER.object.transitions = NULL;
  FOG_CHAMBER.items = make_items_list(1);
  bufPush(FOG_CHAMBER.items, &PRISMATIC_LENS_CORE);
  FOG_CHAMBER.exits = make_locations_list(1);

  SEALED_LABORATORY.object.name = "sealed laboratory";
  SEALED_LABORATORY.object.type = OBJECT_TYPE_LOCATION;
  SEALED_LABORATORY.object.current_state = 0;
  SEALED_LABORATORY.object.description =
      "Steel door enclave containing experimental volatile compounds.";
  const char *lab_states[] = {"stable", "breached"};
  SEALED_LABORATORY.object.state_descriptions = make_states(2, lab_states);
  SEALED_LABORATORY.object.transitions = make_transitions(1);
  items_t *lab_req = make_items_list(1);
  bufPush(lab_req, &VOLATILE_PHIAL);
  transition_t lab_breach = {.trigger = ACTION_TYPE_USE,
                             .from = 0,
                             .to = 1,
                             .required_items = lab_req};
  bufPush(SEALED_LABORATORY.object.transitions, lab_breach);
  SEALED_LABORATORY.items = make_items_list(1);
  bufPush(SEALED_LABORATORY.items, &VOLATILE_PHIAL);
  SEALED_LABORATORY.exits = make_locations_list(1);
}

static void connect_exits(void) {
  bufPush(CLIFF_APPROACH.exits, (struct location_t *)&LIGHTHOUSE_BASE);
  bufPush(CLIFF_APPROACH.exits, (struct location_t *)&ARCHIVE_GALLERY);
  bufPush(LIGHTHOUSE_BASE.exits, (struct location_t *)&FOG_CHAMBER);
  bufPush(LIGHTHOUSE_BASE.exits, (struct location_t *)&MECHANICAL_WORKSHOP);
  bufPush(ARCHIVE_GALLERY.exits, (struct location_t *)&CLIFF_APPROACH);
  bufPush(ARCHIVE_GALLERY.exits, (struct location_t *)&SEALED_LABORATORY);
  bufPush(MECHANICAL_WORKSHOP.exits, (struct location_t *)&LIGHTHOUSE_BASE);
  bufPush(FOG_CHAMBER.exits, (struct location_t *)&LIGHTHOUSE_BASE);
  bufPush(SEALED_LABORATORY.exits, (struct location_t *)&ARCHIVE_GALLERY);
}

static void build_world_collections(void) {
  WORLD_ITEMS = make_items_list(8);
  bufPush(WORLD_ITEMS, &ANTIQUE_COMPASS);
  bufPush(WORLD_ITEMS, &WEATHERED_MAP_FRAGMENT);
  bufPush(WORLD_ITEMS, &SILVER_FILIGREE_KEY);
  bufPush(WORLD_ITEMS, &ENGRAVED_WINDING_HANDLE);
  bufPush(WORLD_ITEMS, &PRISMATIC_LENS_CORE);
  bufPush(WORLD_ITEMS, &BRONZE_SIGNAL_EMITTER);
  bufPush(WORLD_ITEMS, &CURATOR_LEDGER);
  bufPush(WORLD_ITEMS, &VOLATILE_PHIAL);

  WORLD_LOCATIONS = make_locations_list(6);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&CLIFF_APPROACH);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&LIGHTHOUSE_BASE);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&ARCHIVE_GALLERY);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&MECHANICAL_WORKSHOP);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&FOG_CHAMBER);
  bufPush(WORLD_LOCATIONS, (struct location_t *)&SEALED_LABORATORY);
}

static game_state_t world_digest(world_state_t *state) {
  (void)state; // state currently unused for victory logic, reserved for
               // turn/point counting
  int beacon_restored = LIGHTHOUSE_BASE.object.current_state == 1;
  int exhibition_complete = ARCHIVE_GALLERY.object.current_state == 1 &&
                            CURATOR_LEDGER.object.current_state == 2;
  int laboratory_breached = SEALED_LABORATORY.object.current_state == 1 ||
                            VOLATILE_PHIAL.object.current_state == 1;
  if (laboratory_breached)
    return GAME_STATE_DEAD;
  if (beacon_restored || exhibition_complete)
    return GAME_STATE_VICTORY;
  return GAME_STATE_CONTINUE;
}

static const char *END_GAME_MESSAGES[GAME_STATES] = {
    "continue exploring and assembling artifacts",
    "victory: coastal routes guided or museum exhibition assembled",
    "failure: laboratory breach consumed breathable air"};

static inline void init(world_t *world) {
  build_items();
  build_locations();
  connect_exits();
  build_world_collections();
  world->state.turns = 0;
  world->state.inventory = itemsCreate(16);
  world->locations = WORLD_LOCATIONS;
  world->items = WORLD_ITEMS;
  world->current_location = &CLIFF_APPROACH;
  world->digest = world_digest;
  world->end_game[GAME_STATE_CONTINUE] = END_GAME_MESSAGES[GAME_STATE_CONTINUE];
  world->end_game[GAME_STATE_VICTORY] = END_GAME_MESSAGES[GAME_STATE_VICTORY];
  world->end_game[GAME_STATE_DEAD] = END_GAME_MESSAGES[GAME_STATE_DEAD];
}

static world_t story_world;
