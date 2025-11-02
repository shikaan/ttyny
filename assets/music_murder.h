#include "../src/world.h"

// --- State descriptions for items ---
static state_descriptions_t audio_log_states =
    bufConst(2, "encrypted", "manager threatens singer about insurance");
static state_descriptions_t crime_report_states =
    bufConst(2, "blank", "manager only person with singer before fall");
static state_descriptions_t suspect_profile_states =
    bufConst(2, "draft", "manager motive insurance payout");
static state_descriptions_t stage_camera_states =
    bufConst(2, "monitoring", "manager pushes singer");
static state_descriptions_t blood_sample_states =
    bufConst(2, "sealed", "planted blood sample does not match singer");
static state_descriptions_t stage_knife_states =
    bufConst(2, "unswabbed", "fingerprints wiped clean");
static state_descriptions_t press_badge_states =
    bufConst(2, "idle", "timestamp places manager near singer earlier");
static state_descriptions_t console_states =
    bufConst(2, "idle", "threat audio segment removed");
static state_descriptions_t mic_states =
    bufConst(2, "unused", "tamper buzz masks struggle sounds");
static state_descriptions_t setlist_states = bufConst(
    2, "unmarked", "break during the show lets manager isolate singer");
static state_descriptions_t insurance_email_states =
    bufConst(2, "unread", "double indemnity clause boosts payout");
static state_descriptions_t contract_addendum_states =
    bufConst(2, "unsigned", "manager signature forged");
static state_descriptions_t schedule_board_states =
    bufConst(2, "unreviewed", "gap matches time of singer fall");
static state_descriptions_t access_log_states =
    bufConst(2, "locked", "manager entered restricted area before fall");

// --- State descriptions for locations (single-state contemporary spaces) ---
static state_descriptions_t main_stage_states = bufConst(1, "lit");
static state_descriptions_t backstage_states = bufConst(1, "gear-stacked");
static state_descriptions_t mixing_booth_states = bufConst(1, "humming");
static state_descriptions_t dressing_room_states = bufConst(1, "quiet");
static state_descriptions_t media_lounge_states = bufConst(1, "empty");
static state_descriptions_t security_office_states =
    bufConst(1, "monitor glow");

// --- Transitions for items (only USE / EXAMINE) ---
static transitions_t audio_log_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t crime_report_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t suspect_profile_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t stage_camera_transitions = bufConst(
    1, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1});
static transitions_t blood_sample_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t stage_knife_transitions = bufConst(
    1, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1});
static transitions_t press_badge_transitions = bufConst(
    1, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1});
static transitions_t console_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t mic_transitions =
    bufConst(1, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t setlist_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t insurance_email_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t contract_addendum_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t schedule_board_transitions = bufConst(
    2, (transition_t){.trigger = ACTION_TYPE_EXAMINE, .from = 0, .to = 1},
    (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1});
static transitions_t access_log_transitions =
    bufConst(2, (transition_t){.trigger = ACTION_TYPE_USE, .from = 0, .to = 1},
             (transition_t){.trigger = ACTION_TYPE_EXAMINE,
                            .from = 0,
                            .to = 1}); // triggers timed failure (on USE)

// --- Items ---
static item_t audio_log = {
    .object = {.name = "Audio Log",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "portable recorder,scuffed,blue led",
               .state_descriptions = &audio_log_states,
               .traits = 0b00000001,
               .transitions = &audio_log_transitions}};

static item_t crime_scene_report = {
    .object = {.name = "Crime Scene Report",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "clipboard,paper stack,smudged",
               .state_descriptions = &crime_report_states,
               .traits = 0b00000001,
               .transitions = &crime_report_transitions}};

static item_t suspect_profile = {
    .object = {.name = "Suspect Profile",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "printout,faces,list",
               .state_descriptions = &suspect_profile_states,
               .traits = 0b00000001,
               .transitions = &suspect_profile_transitions}};

static item_t stage_camera = {
    .object = {.name = "Stage Camera",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "tripod-mounted,wide lens,green tally",
               .state_descriptions = &stage_camera_states,
               .traits = 0b00000000,
               .transitions = &stage_camera_transitions}};

static item_t blood_sample = {
    .object = {.name = "Blood Sample",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "vial,labelled,cool",
               .state_descriptions = &blood_sample_states,
               .traits = 0b00000001,
               .transitions = &blood_sample_transitions}};

static item_t stage_knife = {
    .object = {.name = "Stage Knife",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "steel,smear,dropped",
               .state_descriptions = &stage_knife_states,
               .traits = 0b00000001,
               .transitions =
                   &stage_knife_transitions}}; // end stage_knife initializer

static item_t press_badge = {
    .object = {.name = "Press Badge",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "laminate,barcode,lanyard",
               .state_descriptions = &press_badge_states,
               .traits = 0b00000001,
               .transitions = &press_badge_transitions}};

static item_t mixing_console = {
    .object = {.name = "Mixing Console",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "faders,led meters,cables",
               .state_descriptions = &console_states,
               .traits = 0b00000000,
               .transitions = &console_transitions}};

static item_t microphone = {
    .object = {.name = "Microphone",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "handheld,mesh grille,weighted",
               .state_descriptions = &mic_states,
               .traits = 0b00000001,
               .transitions = &mic_transitions}};

static item_t setlist = {
    .object = {.name = "Setlist",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "paper,marker scribbles,creases",
               .state_descriptions = &setlist_states,
               .traits = 0b00000001,
               .transitions = &setlist_transitions}};

static item_t insurance_email = {
    .object = {.name = "Insurance Email",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "tablet,inbox highlight,policy",
               .state_descriptions = &insurance_email_states,
               .traits = 0b00000000, // email viewed in place; not collectible
               .transitions = &insurance_email_transitions}};

static item_t contract_addendum = {
    .object = {.name = "Contract Addendum",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "stapled,page corner,dated",
               .state_descriptions = &contract_addendum_states,
               .traits = 0b00000001,
               .transitions = &contract_addendum_transitions}};

static item_t schedule_board = {
    .object = {.name = "Schedule Board",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "whiteboard,eraser streaks,columns",
               .state_descriptions = &schedule_board_states,
               .traits = 0b00000000,
               .transitions = &schedule_board_transitions}};

static item_t access_log = {
    .object = {.name = "Access Log",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "terminal,auth prompt,green cursor",
               .state_descriptions = &access_log_states,
               .traits = 0b00000000,
               .transitions = &access_log_transitions}};

// --- Object buffers per location (length = total items = 14) ---
static items_t main_stage_objects = {14,
                                     3,
                                     {&microphone, &setlist, &stage_camera,
                                      NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                      NULL, NULL, NULL, NULL}};
static items_t backstage_objects = {14,
                                    3,
                                    {&stage_knife, &press_badge,
                                     &schedule_board, NULL, NULL, NULL, NULL,
                                     NULL, NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t mixing_booth_objects = {
    14,
    3,
    {&mixing_console, &audio_log, &access_log, NULL, NULL, NULL, NULL, NULL,
     NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t dressing_room_objects = {
    14,
    3,
    {&suspect_profile, &crime_scene_report, &contract_addendum, NULL, NULL,
     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};
static items_t media_lounge_objects = {14,
                                       1,
                                       {&blood_sample, NULL, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        NULL, NULL, NULL}};
static items_t security_office_objects = {14,
                                          1,
                                          {&insurance_email, NULL, NULL, NULL,
                                           NULL, NULL, NULL, NULL, NULL, NULL,
                                           NULL, NULL, NULL, NULL}};

// --- Forward declarations for locations ---
static locations_t main_stage_exits, backstage_exits, mixing_booth_exits,
    dressing_room_exits, media_lounge_exits, security_office_exits;
static location_t main_stage, backstage_area, mixing_booth, dressing_room,
    media_lounge, security_office;

// --- Exit buffers ---
static locations_t main_stage_exits = {3,
                                       3,
                                       {(struct location_t *)&backstage_area,
                                        (struct location_t *)&mixing_booth,
                                        (struct location_t *)&media_lounge}};
static locations_t backstage_exits = {3,
                                      3,
                                      {(struct location_t *)&main_stage,
                                       (struct location_t *)&dressing_room,
                                       (struct location_t *)&security_office}};
static locations_t mixing_booth_exits = {
    2,
    2,
    {(struct location_t *)&main_stage, (struct location_t *)&security_office}};
static locations_t dressing_room_exits = {
    2,
    2,
    {(struct location_t *)&backstage_area, (struct location_t *)&media_lounge}};
static locations_t media_lounge_exits = {
    2,
    2,
    {(struct location_t *)&main_stage, (struct location_t *)&dressing_room}};
static locations_t security_office_exits = {
    3,
    3,
    {(struct location_t *)&backstage_area, (struct location_t *)&mixing_booth,
     (struct location_t *)&media_lounge}};

// --- Locations ---
static location_t main_stage = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Main Stage",
               .current_state = 0,
               .description = "lit,rigging,monitors",
               .state_descriptions = &main_stage_states,
               .traits = 0,
               .transitions = NULL},
    .items = &main_stage_objects,
    .exits = &main_stage_exits};

static location_t backstage_area = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Backstage",
               .current_state = 0,
               .description = "gear-stacked,cases,cables",
               .state_descriptions = &backstage_states,
               .traits = 0,
               .transitions = NULL},
    .items = &backstage_objects,
    .exits = &backstage_exits};

static location_t mixing_booth = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Mixing Booth",
               .current_state = 0,
               .description = "humming,screens,sliders",
               .state_descriptions = &mixing_booth_states,
               .traits = 0,
               .transitions = NULL},
    .items = &mixing_booth_objects,
    .exits = &mixing_booth_exits};

static location_t dressing_room = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Dressing Room",
               .current_state = 0,
               .description = "quiet,mirror bulbs,folding chairs",
               .state_descriptions = &dressing_room_states,
               .traits = 0,
               .transitions = NULL},
    .items = &dressing_room_objects,
    .exits = &dressing_room_exits};

static location_t media_lounge = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Media Lounge",
               .current_state = 0,
               .description = "empty,sofas,coffee table",
               .state_descriptions = &media_lounge_states,
               .traits = 0,
               .transitions = NULL},
    .items = &media_lounge_objects,
    .exits = &media_lounge_exits};

static location_t security_office = {
    .object = {.type = OBJECT_TYPE_LOCATION,
               .name = "Security Office",
               .current_state = 0,
               .description = "monitor glow,key rack,desk",
               .state_descriptions = &security_office_states,
               .traits = 0,
               .transitions = NULL},
    .items = &security_office_objects,
    .exits = &security_office_exits};

// --- Aggregate buffers ---
static locations_t all_locations = {
    6,
    6,
    {(struct location_t *)&main_stage, (struct location_t *)&backstage_area,
     (struct location_t *)&mixing_booth, (struct location_t *)&dressing_room,
     (struct location_t *)&media_lounge,
     (struct location_t *)&security_office}};
static items_t all_objects = {
    14,
    14,
    {&audio_log, &crime_scene_report, &suspect_profile, &stage_camera,
     &blood_sample, &stage_knife, &press_badge, &mixing_console, &microphone,
     &setlist, &insurance_email, &contract_addendum, &schedule_board,
     &access_log}};

// --- Digest function (victim: lead singer; player: touring audio engineer
// friend) --- Victory Condition A targets (all must reach final state):
//   Audio Log -> decoded
//   Crime Scene Report -> compiled
//   Suspect Profile -> finalized
// Victory Condition B targets (all must reach final state):
//   Stage Camera -> footage reviewed
//   Blood Sample -> analyzed
//   Stage Knife -> identified
// Additional clue items (not required for victory):
//   Insurance Email, Contract Addendum, Schedule Board, Access Log
// Timed Failure trigger:
//   Access Log -> extracted mgr entry alert started (10 turns to finish after
//   this)
static game_state_t music_murder_digest(world_state_t *state) {
  static int alertTurn = -1;
  int audioDecoded = 0;
  int reportCompiled = 0;
  int profileFinal = 0;
  int cameraReviewed = 0;
  int sampleAnalyzed = 0;
  int weaponIdentified = 0;

  // Scan global objects for states (independent of inventory)
  for (size_t i = 0; i < all_objects.used; ++i) {
    item_t *obj = all_objects.data[i];
    if (!obj)
      continue;
    if (obj->object.name == audio_log.object.name &&
        obj->object.current_state == 1)
      audioDecoded = 1;
    else if (obj->object.name == crime_scene_report.object.name &&
             obj->object.current_state == 1)
      reportCompiled = 1;
    else if (obj->object.name == suspect_profile.object.name &&
             obj->object.current_state == 1)
      profileFinal = 1;
    else if (obj->object.name == stage_camera.object.name &&
             obj->object.current_state == 1)
      cameraReviewed = 1;
    else if (obj->object.name == blood_sample.object.name &&
             obj->object.current_state == 1)
      sampleAnalyzed = 1;
    else if (obj->object.name == stage_knife.object.name &&
             obj->object.current_state == 1)
      weaponIdentified = 1;
    else if (obj->object.name == access_log.object.name &&
             obj->object.current_state == 1 && alertTurn == -1) {
      alertTurn = state->turns; // countdown starts
    }
  }

  // Prioritize victory over failure if simultaneous
  if ((audioDecoded && reportCompiled && profileFinal) ||
      (cameraReviewed && sampleAnalyzed && weaponIdentified)) {
    return GAME_STATE_VICTORY;
  }

  if (alertTurn != -1 && (int)state->turns - alertTurn >= 10) {
    return GAME_STATE_DEAD;
  }

  return GAME_STATE_CONTINUE;
}

// --- Player inventory ---
static items_t music_murder_inventory = {.length = 14,
                                         .used = 0,
                                         .data = {NULL, NULL, NULL, NULL, NULL,
                                                  NULL, NULL, NULL, NULL, NULL,
                                                  NULL, NULL, NULL, NULL}};

// --- World definition ---
world_t music_murder_world = {
    .state = {.turns = 0, .inventory = &music_murder_inventory},
    .locations = &all_locations,
    .digest = music_murder_digest,
    .current_location = &main_stage,
    .items = &all_objects,
    .end_game = {
        "",
        "You assemble the full chain: decoded threat naming the manager, "
        "timeline placing them alone with the singer, insurance fraud motive, "
        "camera shove, planted mismatched blood, wiped stage knife—authorities "
        "arrest the manager before the alert snowballs.",
        "Access log alert expired before you linked both evidence sets; "
        "manager purges consoles, spins leak as tragic fall, escapes with "
        "payout while the singer’s murder is misreported."}};
