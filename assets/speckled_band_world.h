#pragma once

#include "../src/world.h"

/*
 * THE ADVENTURE OF THE SPECKLED BAND
 *
 * Theme: Victorian mystery investigation based on A.C. Doyle's Sherlock Holmes
 * story
 *
 * Victim: Julia Stoner (deceased), found dead in her locked bedroom at Stoke
 * Moran Player Role: Detective investigating the mysterious death and
 * protecting Helen Stoner
 *
 * Crime: Dr. Grimesby Roylott murdered his stepdaughter Julia using a trained
 * swamp adder to prevent her marriage and preserve his control over her
 * inheritance.
 *
 * VICTORY CONDITIONS (satisfy ANY one set):
 *
 * Set A - Method + Motive + Access:
 *   - Ventilator Grate (state 1): passage between rooms
 *   - Bell Rope (state 1): dummy rope positioned over ventilator
 *   - Will Document (state 1): financial motive revealed
 *
 * Set B - Physical Evidence + Opportunity:
 *   - Metal Safe (state 1): snake scales and tools
 *   - Milk Saucer (state 1): bait evidence
 *   - Bed Frame (state 1): bolted under ventilator
 *
 * OPTIONAL CLUES (enrich narrative, not required for victory):
 *   - Witness Statement: Helen's testimony
 *   - Medical Report: confirms locked room mystery
 *   - Brass Whistle: corroborates midnight whistle testimony
 *   - Leash Hook: reptile scale residue
 *
 * TIMED FAILURE:
 *   Trigger: First entry into Roylott's Study (ACTION_TYPE_MOVE to
 * roylott_study) Reason: Roylott notices intrusion in his private space,
 * confirms investigation, plans escape Duration: 12 turns after trigger
 *   Consequence: Roylott flees to India if evidence not assembled and acted
 * upon in time
 */

// Total items in world
#define TOTAL_ITEMS 10

/* ========================================================================
 * STATE DESCRIPTIONS
 * ======================================================================== */

// Witness Statement (collectible document)
static state_descriptions_t witness_statement_states_buf =
    bufConst(2, "sealed envelope",
             "Helen testimony: midnight whistling, metallic clang, Julia's "
             "final words: speckled band");

// Ventilator Grate (non-collectible, fixed between rooms)
static state_descriptions_t ventilator_grate_states_buf =
    bufConst(2, "ventilator grate between rooms",
             "ventilator: direct passage from Roylott study to Julia bedroom, "
             "unusually large");

// Bell Rope (non-collectible, fixed to wall)
static state_descriptions_t bell_rope_states_buf =
    bufConst(2, "bell rope hanging beside bed",
             "dummy rope: disconnected from bell, positioned directly over "
             "ventilator opening");

// Metal Safe (non-collectible, large fixed furniture)
static state_descriptions_t metal_safe_states_buf =
    bufConst(2, "locked iron safe in corner",
             "safe contents: snake scales, veterinary tools, exotic animal "
             "leash inside");

// Milk Saucer (collectible, small portable object)
static state_descriptions_t milk_saucer_states_buf = bufConst(
    2, "saucer of milk on floor",
    "milk saucer: fresh milk as snake bait, placed nightly by Roylott");

// Bed Frame (non-collectible, immovable furniture)
static state_descriptions_t bed_frame_states_buf =
    bufConst(2, "iron bed frame against wall",
             "bed bolted to floor: immovable, positioned directly beneath "
             "ventilator opening");

// Will Document (collectible document)
static state_descriptions_t will_document_states_buf =
    bufConst(2, "sealed legal envelope",
             "will terms: Roylott loses yearly income when daughters marry, "
             "motive established");

// Medical Report (collectible document)
static state_descriptions_t medical_report_states_buf =
    bufConst(2, "coroner sealed folder",
             "autopsy report: no poison detected, no wounds, death in locked "
             "room unexplained");

// Brass Whistle (collectible item)
static state_descriptions_t brass_whistle_states_buf =
    bufConst(2, "small brass dog whistle",
             "whistle: matches Helen testimony of midnight sound, used to "
             "recall trained snake");

// Leash Hook (non-collectible, mounted on wall)
static state_descriptions_t leash_hook_states_buf =
    bufConst(2, "iron hook mounted on wall",
             "hook: reptile scale residue, recently used for snake handling");

/* ========================================================================
 * TRANSITIONS
 * ======================================================================== */

// Witness Statement: dual trigger (EXAMINE/USE) for document
static transitions_t witness_statement_transitions_buf =
    bufConst(2, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}),
             ((transition_t){ACTION_TYPE_USE, 0, 1}));

// Ventilator Grate: examine to inspect
static transitions_t ventilator_grate_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

// Bell Rope: examine to discover it's fake
static transitions_t bell_rope_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

// Metal Safe: use to open and reveal contents
static transitions_t metal_safe_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_USE, 0, 1}));

// Milk Saucer: examine to understand purpose
static transitions_t milk_saucer_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

// Bed Frame: examine to discover it's bolted
static transitions_t bed_frame_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

// Will Document: dual trigger (EXAMINE/USE) for document
static transitions_t will_document_transitions_buf =
    bufConst(2, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}),
             ((transition_t){ACTION_TYPE_USE, 0, 1}));

// Medical Report: dual trigger (EXAMINE/USE) for document
static transitions_t medical_report_transitions_buf =
    bufConst(2, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}),
             ((transition_t){ACTION_TYPE_USE, 0, 1}));

// Brass Whistle: examine to match testimony
static transitions_t brass_whistle_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

// Leash Hook: examine to find residue
static transitions_t leash_hook_transitions_buf =
    bufConst(1, ((transition_t){ACTION_TYPE_EXAMINE, 0, 1}));

/* ========================================================================
 * ITEM DECLARATIONS
 * ======================================================================== */

static item_t witness_statement = {
    .object = {.name = "Witness Statement",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description =
                   "sealed envelope from Helen Stoner containing her testimony",
               .state_descriptions = &witness_statement_states_buf,
               .traits = 0b00000001, // collectible
               .transitions = &witness_statement_transitions_buf}};

static item_t ventilator_grate = {
    .object = {.name = "Ventilator Grate",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description =
                   "metal grate connecting this room to adjacent chamber",
               .state_descriptions = &ventilator_grate_states_buf,
               .traits = 0, // non-collectible (fixed infrastructure)
               .transitions = &ventilator_grate_transitions_buf}};

static item_t bell_rope = {
    .object = {.name = "Bell Rope",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "velvet bell rope hanging beside the bed",
               .state_descriptions = &bell_rope_states_buf,
               .traits = 0, // non-collectible (fixed to wall)
               .transitions = &bell_rope_transitions_buf}};

static item_t metal_safe = {
    .object = {.name = "Metal Safe",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "heavy iron safe with brass fittings",
               .state_descriptions = &metal_safe_states_buf,
               .traits = 0, // non-collectible (large fixed furniture)
               .transitions = &metal_safe_transitions_buf}};

static item_t milk_saucer = {
    .object = {.name = "Milk Saucer",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "small porcelain saucer with fresh milk",
               .state_descriptions = &milk_saucer_states_buf,
               .traits = 0b00000001, // collectible (small portable object)
               .transitions = &milk_saucer_transitions_buf}};

static item_t bed_frame = {
    .object = {.name = "Bed Frame",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "sturdy iron bed frame with worn mattress",
               .state_descriptions = &bed_frame_states_buf,
               .traits = 0, // non-collectible (immovable furniture)
               .transitions = &bed_frame_transitions_buf}};

static item_t will_document = {
    .object = {.name = "Will Document",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "legal envelope containing inheritance terms",
               .state_descriptions = &will_document_states_buf,
               .traits = 0b00000001, // collectible
               .transitions = &will_document_transitions_buf}};

static item_t medical_report = {
    .object = {.name = "Medical Report",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "official coroner folder with wax seal",
               .state_descriptions = &medical_report_states_buf,
               .traits = 0b00000001, // collectible
               .transitions = &medical_report_transitions_buf}};

static item_t brass_whistle = {
    .object = {.name = "Brass Whistle",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "small brass dog whistle on chain",
               .state_descriptions = &brass_whistle_states_buf,
               .traits = 0b00000001, // collectible
               .transitions = &brass_whistle_transitions_buf}};

static item_t leash_hook = {
    .object = {.name = "Leash Hook",
               .type = OBJECT_TYPE_ITEM,
               .current_state = 0,
               .description = "iron hook mounted on study wall",
               .state_descriptions = &leash_hook_states_buf,
               .traits = 0, // non-collectible (mounted fixture)
               .transitions = &leash_hook_transitions_buf}};

/* ========================================================================
 * LOCATION ITEM BUFFERS (length = TOTAL_ITEMS, padded with NULL)
 * ======================================================================== */

// Entry Hall: where investigation begins, initial briefing materials
static items_t grand_foyer_items_buf =
    bufInit(TOTAL_ITEMS, 2, &witness_statement, &medical_report, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL);

// Julia's Bedroom: crime scene with key structural evidence
static items_t julia_bedroom_items_buf =
    bufInit(TOTAL_ITEMS, 3, &ventilator_grate, &bell_rope, &bed_frame, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL);

// Roylott's Study: perpetrator's room with incriminating items
static items_t roylott_study_items_buf =
    bufInit(TOTAL_ITEMS, 3, &metal_safe, &brass_whistle, &will_document, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL);

// Connecting Passage: transitional space with additional evidence
static items_t connecting_passage_items_buf =
    bufInit(TOTAL_ITEMS, 2, &milk_saucer, &leash_hook, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL);

/* ========================================================================
 * LOCATION STATE DESCRIPTIONS
 * ======================================================================== */

static state_descriptions_t grand_foyer_states_buf =
    bufConst(1, "grand but decaying entrance, cold draft, faded portraits");

static state_descriptions_t julia_bedroom_states_buf =
    bufConst(1, "locked bedroom, musty and intact since tragedy");

static state_descriptions_t roylott_study_states_buf = bufConst(
    1, "cluttered study, tobacco smoke, exotic animal smell, Indian artifacts");

static state_descriptions_t connecting_passage_states_buf =
    bufConst(1, "narrow passage between bedrooms, damp stone, dim gaslight");

/* ========================================================================
 * LOCATION DECLARATIONS
 * ======================================================================== */

// Forward declarations for circular references
static location_t grand_foyer;
static location_t julia_bedroom;
static location_t roylott_study;
static location_t connecting_passage;

// Entry Hall exits
static locations_t grand_foyer_exits_buf =
    bufConst(2, (struct location_t *)&julia_bedroom,
             (struct location_t *)&roylott_study);

// Julia's Bedroom exits
static locations_t julia_bedroom_exits_buf =
    bufConst(2, (struct location_t *)&grand_foyer,
             (struct location_t *)&connecting_passage);

// Roylott's Study exits
static locations_t roylott_study_exits_buf =
    bufConst(2, (struct location_t *)&grand_foyer,
             (struct location_t *)&connecting_passage);

// Connecting Passage exits
static locations_t connecting_passage_exits_buf =
    bufConst(2, (struct location_t *)&julia_bedroom,
             (struct location_t *)&roylott_study);

// Location objects
static location_t grand_foyer = {
    .object = {.name = "Grand Foyer",
               .type = OBJECT_TYPE_LOCATION,
               .current_state = 0,
               .description = "main entrance of Stoke Moran manor",
               .state_descriptions = &grand_foyer_states_buf,
               .traits = 0b00000001, // lit
               .transitions = NULL},
    .items = &grand_foyer_items_buf,
    .exits = &grand_foyer_exits_buf};

static location_t julia_bedroom = {
    .object = {.name = "Julia's Bedroom",
               .type = OBJECT_TYPE_LOCATION,
               .current_state = 0,
               .description =
                   "Julia Stoner's bedroom where she died mysteriously",
               .state_descriptions = &julia_bedroom_states_buf,
               .traits = 0b00000001, // lit
               .transitions = NULL},
    .items = &julia_bedroom_items_buf,
    .exits = &julia_bedroom_exits_buf};

static location_t roylott_study = {
    .object = {.name = "Roylott Study",
               .type = OBJECT_TYPE_LOCATION,
               .current_state = 0,
               .description = "Dr. Grimesby Roylott's private study",
               .state_descriptions = &roylott_study_states_buf,
               .traits = 0b00000001, // lit
               .transitions = NULL},
    .items = &roylott_study_items_buf,
    .exits = &roylott_study_exits_buf};

static location_t connecting_passage = {
    .object = {.name = "Connecting Passage",
               .type = OBJECT_TYPE_LOCATION,
               .current_state = 0,
               .description = "passage between Julia's and Roylott's rooms",
               .state_descriptions = &connecting_passage_states_buf,
               .traits = 0b00000001, // lit
               .transitions = NULL},
    .items = &connecting_passage_items_buf,
    .exits = &connecting_passage_exits_buf};

/* ========================================================================
 * AGGREGATE COLLECTIONS
 * ======================================================================== */

static locations_t all_locations = bufConst(
    4, (struct location_t *)&grand_foyer, (struct location_t *)&julia_bedroom,
    (struct location_t *)&roylott_study,
    (struct location_t *)&connecting_passage);

static items_t all_objects =
    bufConst(TOTAL_ITEMS, &witness_statement, &ventilator_grate, &bell_rope,
             &metal_safe, &milk_saucer, &bed_frame, &will_document,
             &medical_report, &brass_whistle, &leash_hook);

/* ========================================================================
 * INVENTORY
 * ======================================================================== */

static items_t inventory = bufInit(TOTAL_ITEMS, 0, NULL, NULL, NULL, NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL);

/* ========================================================================
 * DIGEST FUNCTION
 * ======================================================================== */

/*
 * Game state evaluation with timed failure mechanism.
 *
 * TIMED FAILURE TRIGGER:
 *   - Starts when player first enters Roylott's Study (location transition)
 *   - Reason: Roylott notices intrusion, confirms Helen brought investigator,
 * plans escape
 *   - Duration: 12 turns after entering study
 *   - Consequence: If evidence not assembled within 12 turns, Roylott flees to
 * India
 *
 * VICTORY CONDITIONS (any ONE set satisfies):
 *
 * Set A - Method + Motive + Access (3 items):
 *   1. Ventilator Grate state 1: proves passage between rooms
 *   2. Bell Rope state 1: dummy rope positioned over ventilator
 *   3. Will Document state 1: financial motive established
 *
 * Set B - Physical Evidence + Opportunity (3 items):
 *   1. Metal Safe state 1: snake scales and veterinary tools
 *   2. Milk Saucer state 1: bait evidence
 *   3. Bed Frame state 1: bolted under ventilator
 *
 * EVALUATION ORDER:
 *   1. Check victory conditions first (success prioritized)
 *   2. Check timed failure expiry
 *   3. Default continue
 */
static game_state_t digest(world_state_t *state) {
  // Static variable to track when countdown begins (-1 = not started)
  static int trigger_turn = -1;

  // Start countdown when player first interacts with ANY item in Roylott's
  // Study This indicates they've entered and begun investigating his private
  // space
  if (trigger_turn == -1) {
    if (metal_safe.object.current_state > 0 ||
        brass_whistle.object.current_state > 0 ||
        will_document.object.current_state > 0) {
      trigger_turn = state->turns;
    }
  }

  // VICTORY SET A: Method + Motive + Access
  // Player proves: ventilator passage, dummy rope placement, financial motive
  if (ventilator_grate.object.current_state == 1 &&
      bell_rope.object.current_state == 1 &&
      will_document.object.current_state == 1) {
    return GAME_STATE_VICTORY;
  }

  // VICTORY SET B: Physical Evidence + Opportunity
  // Player proves: snake equipment, bait system, victim positioning
  if (metal_safe.object.current_state == 1 &&
      milk_saucer.object.current_state == 1 &&
      bed_frame.object.current_state == 1) {
    return GAME_STATE_VICTORY;
  }

  // TIMED FAILURE: 12 turns after entering Roylott's Study (interacting with
  // study items) If timer started and 12 turns elapsed without victory, Roylott
  // flees
  if (trigger_turn >= 0 && state->turns >= trigger_turn + 12) {
    return GAME_STATE_DEAD;
  }

  // Game continues
  return GAME_STATE_CONTINUE;
}

/* ========================================================================
 * WORLD INSTANCE
 * ======================================================================== */

static world_t speckled_band_world = {
    .state = {.turns = 0, .inventory = &inventory},
    .locations = &all_locations,
    .items = &all_objects,
    .current_location = &grand_foyer,
    .digest = digest,
    .end_game = {
        // GAME_STATE_CONTINUE (index 0)
        "",

        // GAME_STATE_VICTORY (index 1)
        "Evidence assembled. Dr. Grimesby Roylott murdered Julia Stoner using "
        "a trained swamp adder (the speckled band). "
        "The snake entered through the ventilator via the dummy bell-rope, "
        "targeting Julia in her bolted bed positioned beneath the opening. "
        "Motive: Roylott would lose control of Julia's inheritance upon her "
        "marriage. "
        "Method confirmed by snake scales in safe, milk bait system, and "
        "whistle to recall the serpent. "
        "Helen Stoner is saved, Roylott arrested, justice served.",

        // GAME_STATE_DEAD (index 2)
        "Time expired. Your intrusion into Dr. Roylott's study alerted him to "
        "the investigation. "
        "Sensing exposure was imminent, he fled England to his connections in "
        "India beyond extradition."}};

static world_t *world = &speckled_band_world;
