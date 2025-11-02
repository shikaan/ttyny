#include "../src/world.h"

// --- State descriptions for items ---
static state_descriptions_t badge_states = bufConst(2, "unused", "scanned");
static state_descriptions_t usb_states = bufConst(2, "unarchived", "archived");
static state_descriptions_t cup_states = bufConst(2, "full", "empty");
static state_descriptions_t camera_states =
    bufConst(2, "monitoring", "player detected");
static state_descriptions_t panel_states = bufConst(2, "idle", "called");
static state_descriptions_t door_states = bufConst(2, "closed", "open");
static state_descriptions_t map_states = bufConst(2, "unread", "studied");

// --- State descriptions for locations ---
static state_descriptions_t lobby_states = bufConst(1, "quiet");
static state_descriptions_t office_states = bufConst(1, "after-hours");
static state_descriptions_t server_room_states = bufConst(1, "humming");
static state_descriptions_t rooftop_states = bufConst(1, "breezy");
static state_descriptions_t corridor_states = bufConst(1, "echoing");
static state_descriptions_t parking_states = bufConst(1, "vacant");

// --- Transitions for items (only meaningful changes) ---
static transitions_t badge_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

static transitions_t usb_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

static transitions_t cup_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

static transitions_t camera_transitions = bufConst(
    1, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1});

static transitions_t panel_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

static transitions_t door_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

static transitions_t map_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});

// --- Items ---
static item_t access_badge = {
    .object = {.name = "Access Badge",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "plastic,white,qr-coded",
               .state_descriptions = &badge_states,
               .traits = 0b00000001,
               .transitions = &badge_transitions}};

static item_t usb_drive = {.object = {.name = "USB Drive",
                                      .type = OBJECT_TYPE_ITEM,
                                      .current_state = 0,
                                      .description = "metallic,small,blue-led",
                                      .state_descriptions = &usb_states,
                                      .traits = 0b00000001,
                                      .transitions = &usb_transitions}};

static item_t coffee_cup = {.object = {.name = "Coffee Cup",
                                       .type = OBJECT_TYPE_ITEM,
                                       .current_state = 0,
                                       .description = "paper,logo-stained,warm",
                                       .state_descriptions = &cup_states,
                                       .traits = 0b00000001,
                                       .transitions = &cup_transitions}};

static item_t security_camera = {
    .object = {.name = "Security Camera",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "ceiling-mounted,blinking red led",
               .state_descriptions = &camera_states,
               .traits = 0b00000000,
               .transitions = &camera_transitions}};

static item_t elevator_panel = {
    .object = {.name = "Elevator Panel",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "finger-smudged,backlit",
               .state_descriptions = &panel_states,
               .traits = 0b00000000,
               .transitions = &panel_transitions}};

static item_t exit_door = {
    .object = {.name = "Exit Door",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "metal,panic-bar,green-sign",
               .state_descriptions = &door_states,
               .traits = 0b00000000,
               .transitions = &door_transitions}};

static item_t city_map = {.object = {.name = "City Map",
                                     .type = OBJECT_TYPE_ITEM,
                                     .current_state = 0,
                                     .description = "creased,ink-detailed",
                                     .state_descriptions = &map_states,
                                     .traits = 0b00000001,
                                     .transitions = &map_transitions}};

// --- Object buffers per location (length = total items = 7) ---
static items_t lobby_objects = {
    7, 2, {&access_badge, &coffee_cup, NULL, NULL, NULL, NULL, NULL}};
static items_t office_objects = {
    7, 2, {&usb_drive, &security_camera, NULL, NULL, NULL, NULL, NULL}};
static items_t server_room_objects = {
    7, 1, {&elevator_panel, NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t rooftop_objects = {
    7, 1, {&city_map, NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t corridor_objects = {
    7, 1, {&exit_door, NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t parking_objects = {
    7, 0, {NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

// --- Forward declarations for locations ---
static locations_t lobby_exits, office_exits, server_room_exits, rooftop_exits,
    corridor_exits, parking_exits;
static location_t lobby, open_office, server_room, rooftop_garden,
    exit_corridor, parking_level;

// --- Exit buffers ---
static locations_t lobby_exits = {
    2,
    2,
    {(struct location_t *)&open_office, (struct location_t *)&exit_corridor}};
static locations_t office_exits = {3,
                                   3,
                                   {(struct location_t *)&lobby,
                                    (struct location_t *)&server_room,
                                    (struct location_t *)&rooftop_garden}};
static locations_t server_room_exits = {
    2,
    2,
    {(struct location_t *)&open_office, (struct location_t *)&parking_level}};
static locations_t rooftop_exits = {1, 1, {(struct location_t *)&open_office}};
static locations_t corridor_exits = {
    2, 2, {(struct location_t *)&lobby, (struct location_t *)&parking_level}};
static locations_t parking_exits = {3,
                                    3,
                                    {(struct location_t *)&server_room,
                                     (struct location_t *)&exit_corridor,
                                     (struct location_t *)&lobby}};

// --- Locations ---
static location_t lobby = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Lobby",
               .current_state = 0,
               .description = "quiet,reception-dark,glass-front",
               .state_descriptions = &lobby_states,
               .traits = 0,
               .transitions = NULL},
    .items = &lobby_objects,
    .exits = &lobby_exits};

static location_t open_office = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Open Office",
               .current_state = 0,
               .description = "cubicles,low hum,monitor glow",
               .state_descriptions = &office_states,
               .traits = 0,
               .transitions = NULL},
    .items = &office_objects,
    .exits = &office_exits};

static location_t server_room = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Server Room",
               .current_state = 0,
               .description = "cold,fan-noise,blinking lights",
               .state_descriptions = &server_room_states,
               .traits = 0,
               .transitions = NULL},
    .items = &server_room_objects,
    .exits = &server_room_exits};

static location_t rooftop_garden = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Rooftop Garden",
               .current_state = 0,
               .description = "breezy,string-lights,planters",
               .state_descriptions = &rooftop_states,
               .traits = 0,
               .transitions = NULL},
    .items = &rooftop_objects,
    .exits = &rooftop_exits};

static location_t exit_corridor = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Exit Corridor",
               .current_state = 0,
               .description = "echoing,white walls,fire signage",
               .state_descriptions = &corridor_states,
               .traits = 0,
               .transitions = NULL},
    .items = &corridor_objects,
    .exits = &corridor_exits};

static location_t parking_level = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Parking Level",
               .current_state = 0,
               .description = "vacant,concrete,fluorescent buzz",
               .state_descriptions = &parking_states,
               .traits = 0,
               .transitions = NULL},
    .items = &parking_objects,
    .exits = &parking_exits};

// --- Aggregate buffers ---
static locations_t all_locations = {
    6,
    6,
    {(struct location_t *)&lobby, (struct location_t *)&open_office,
     (struct location_t *)&server_room, (struct location_t *)&rooftop_garden,
     (struct location_t *)&exit_corridor, (struct location_t *)&parking_level}};
static items_t all_objects = {7,
                              7,
                              {&access_badge, &usb_drive, &coffee_cup,
                               &security_camera, &elevator_panel, &exit_door,
                               &city_map}};

// --- Digest function ---
// Adds failure: 8 turns after the Security Camera is logged (examined) if
// victory not achieved.
static game_state_t urban_escape_digest(world_state_t *state) {
  static int cameraLoggedTurn = -1; // -1 means not yet logged
  int badgeScanned = 0;
  int usbArchived = 0;
  int doorOpen = 0;

  // Inventory-based objectives
  for (size_t i = 0; i < state->inventory->used; ++i) {
    item_t *obj = bufAt(state->inventory, i);
    if (obj->object.name == access_badge.object.name &&
        obj->object.current_state == 1)
      badgeScanned = 1;
    else if (obj->object.name == usb_drive.object.name &&
             obj->object.current_state == 1)
      usbArchived = 1;
  }

  // World objects (non-collectible + camera monitoring)
  for (size_t i = 0; i < all_objects.used; ++i) {
    item_t *obj = all_objects.data[i];
    if (!obj)
      continue;
    if (obj->object.name == exit_door.object.name &&
        obj->object.current_state == 1)
      doorOpen = 1;
    if (obj->object.name == security_camera.object.name &&
        obj->object.current_state == 1 && cameraLoggedTurn == -1)
      cameraLoggedTurn = state->turns; // first time logged
  }

  // Victory takes precedence over failure
  if (badgeScanned && usbArchived && doorOpen)
    return GAME_STATE_VICTORY;

  // Failure if camera logged and too many turns elapsed
  if (cameraLoggedTurn != -1 && (int)state->turns - cameraLoggedTurn >= 8)
    return GAME_STATE_DEAD;

  return GAME_STATE_CONTINUE;
}

// --- Player inventory ---
static items_t urban_inventory = {
    .length = 7, .used = 0, .data = {NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

// --- World definition ---
world_t urban_escape_world = {
    .state = {.turns = 0, .inventory = &urban_inventory},
    .locations = &all_locations,
    .digest = urban_escape_digest,
    .current_location = &lobby,
    .items = &all_objects,
    .end_game = {
        "Emergency lights flicker; procedures incomplete. You remain inside.",
        "Badge scanned, data archived, exit door open. You slip out unnoticed.",
        "Security sweeps in; the exit locks behind the converging footsteps."}};
