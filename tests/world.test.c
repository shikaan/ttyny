#include "test.h"

#include "../src/utils.h"
#include "../src/world/item.h"
#include "../src/world/location.h"
#include "../src/world/world.h"
#include <string.h>

void item(void) {
  char *first_name = strdup("first");
  char *last_name = strdup("last");
  case("itemsCreate");
  items_t *items cleanup(itemsDestroy) = itemsCreate(2);
  panicif(!items, "cannot create items");
  expectEqllu(items->used, 0, "correct used");
  expectEqllu(items->length, 2, "correct length");

  case("itemCreate");
  item_t *item_1 cleanup(itemDestroy) = itemCreate();
  panicif(!item_1, "cannot create item");
  item_t *item_2 cleanup(itemDestroy) = itemCreate();
  panicif(!item_2, "cannot create item");

  item_1->object.name = first_name;
  item_2->object.name = last_name;

  expectEqli(item_1->object.type, OBJECT_TYPE_ITEM, "type is correct");
  panicif(item_2->object.type != OBJECT_TYPE_ITEM, "unexpected unmatching type");

  case("itemsFindByName");
  bufPush(items, item_1);
  expectEqli(itemsFindByName(items, first_name), 0, "finds item");
  expectEqli(itemsFindByName(items, last_name), -1, "does not find missing item");
}

void location(void) {
  char *first_name = strdup("first");
  char *last_name = strdup("last");
  case("locationsCreate");
  locations_t *locations cleanup(locationsDestroy) = locationsCreate(2);
  panicif(!locations, "cannot create locations");
  expectEqllu(locations->used, 0, "correct used");
  expectEqllu(locations->length, 2, "correct length");

  case("locationCreate");
  location_t *location_1 cleanup(locationDestroy) = locationCreate();
  panicif(!location_1, "cannot create location");
  location_t *location_2 cleanup(locationDestroy) = locationCreate();
  panicif(!location_2, "cannot create location");

  location_1->object.name = first_name;
  location_2->object.name = last_name;

  expectEqli(location_1->object.type, OBJECT_TYPE_LOCATION, "type is correct");
  panicif(location_2->object.type != OBJECT_TYPE_LOCATION, "unexpected unmatching type");

  case("locationsFindByName");
  bufPush(locations, location_1);
  expectEqli(locationsFindByName(locations, first_name), 0, "finds location");
  expectEqli(locationsFindByName(locations, last_name), -1, "does not find missing location");
}

void digest(void) {
  static char win[] = "win";
  static char lose[] = "lose";
  static char location_name[] = "location";
  static location_t some_location = { .object.name = win};
  static location_t lose_location = {.object.name = location_name};
  static requirement_tuple_t tuple = {.name = location_name, .state = OBJECT_STATE_ANY};
  static requirements_t REQ_WIN_TURNS = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 5,
  };
  static ending_t ENDING_WIN = {
      .success = true,
      .reason = win,
      .requirements = &REQ_WIN_TURNS,
  };
  static requirements_t REQ_LOSE_TURNS = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 12,
  };
  static requirements_t REQ_CURRENT_LOCATION = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
      .current_location = &tuple,
  };
  static ending_t ENDING_LOSE = {
      .success = false,
      .reason = lose,
      .requirements = &REQ_LOSE_TURNS,
  };
  static ending_t ENDING_LOSE_CURRENT_LOCATION = {
      .success = false,
      .reason = lose,
      .requirements = &REQ_CURRENT_LOCATION,
  };
  static endings_t ENDINGS = bufConst(3, &ENDING_WIN, &ENDING_LOSE, &ENDING_LOSE_CURRENT_LOCATION);
  world_t w = {
      .items = NULL,
      .locations = NULL,
      .endings = &ENDINGS,
      .turns = 0,
      .inventory = NULL,
      .location = &some_location,
      .end_game = NULL,
  };

  game_state_t state;

  worldDigest(&w, &state);
  expectEqlu(state, GAME_STATE_CONTINUE, "digest continues at turn 0");
  expectNull(w.end_game, "end_game still NULL");

  w.turns = 5;
  w.end_game = NULL;
  worldDigest(&w, &state);
  expectEqlu(state, GAME_STATE_VICTORY, "victory at turn 5");
  expectEqls(w.end_game, win, sizeof(win), "victory reason matches");

  w.turns = 12;
  w.end_game = NULL;
  worldDigest(&w, &state);
  expectEqlu(state, GAME_STATE_VICTORY, "victory takes precedence over loss");

  REQ_WIN_TURNS.turns = 100; // now win cannot trigger
  w.turns = 12;
  w.end_game = NULL;
  worldDigest(&w, &state);
  expectEqlu(state, GAME_STATE_DEAD, "digest yields death at turn 12");
  expectEqls(w.end_game, lose, sizeof(lose), "death reason matches");

  w.turns = 1;
  w.location = &lose_location;
  w.end_game = NULL;
  worldDigest(&w, &state);
  expectEqlu(state, GAME_STATE_DEAD, "triggers death when current location is met");
  expectEqls(w.end_game, lose, sizeof(lose), "death reason matches");
}

void transition(void) {
  static char tool_name[] = "tool";
  static char other_name[] = "other";
  static requirements_t no_reqs = {0};
  static const transition_t tr_1 = { ACTION_TYPE_USE, 0, 1, &no_reqs };
  static const transition_t tr_2 = { ACTION_TYPE_USE, 1, 2, &no_reqs };
  static char state[] = "state";
  static transitions_t transitions = bufConst(2, tr_1, tr_2);
  static descriptions_t descriptions = bufConst(3, state, state, state);
  static item_t item_1 = {{tool_name, OBJECT_TYPE_ITEM,0,&descriptions,&transitions}, false, false};
  static item_t item_2 = {{other_name, OBJECT_TYPE_ITEM,0,&descriptions,NULL}, false, false};
  static requirement_tuples_t items_reqs = bufConst(1, {tool_name, 0});
  static requirements_t reqs = {&items_reqs, NULL, NULL, NULL, 0};
  static const transition_t tr_3 = { ACTION_TYPE_USE, 0, 1, &reqs };
  static transitions_t transitions_with_reqs = bufConst(1, tr_3);
  static item_t item_3 = {{other_name, OBJECT_TYPE_ITEM,0,&descriptions,&transitions_with_reqs}, false, false};
  static items_t items = bufConst(3, &item_1, &item_2, &item_3);
  static items_t inventory = bufConst(0);
  world_t w = {
      .items = &items,
      .locations = NULL,
      .endings = NULL,
      .turns = 0,
      .inventory = &inventory,
      .location = NULL,
      .end_game = NULL,
  };

  transition_result_t tr;
  worldTransitionObject(&w, &item_1.object, ACTION_TYPE_USE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_OK, "transitions");
  expectEqli(item_1.object.state, 1, "correct state");

  worldTransitionObject(&w, &item_1.object, ACTION_TYPE_TAKE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_NO_TRANSITION, "no transition on wrong action");
  expectEqli(item_1.object.state, 1, "state unchanged");

  worldTransitionObject(&w, &item_1.object, ACTION_TYPE_USE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_OK, "transitions");
  expectEqli(item_1.object.state, 2, "correct state");

  worldTransitionObject(&w, &item_1.object, ACTION_TYPE_USE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_NO_TRANSITION, "no further transition");
  expectEqli(item_1.object.state, 2, "state unchanged");

  worldTransitionObject(&w, &item_2.object, ACTION_TYPE_USE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_OK, "noop is ok");
  expectEqli(item_2.object.state, 0, "state unchanged without transitions");

  worldTransitionObject(&w, &item_3.object, ACTION_TYPE_USE, &tr);
  expectEqlu(tr, TRANSITION_RESULT_MISSING_ITEM, "transition blocked (missing item)");
  expectEqli(item_3.object.state, 0, "state unchanged with unmet requirement");
}

void requirements(void) {
  static char tool_name[] = "tool";
  requirements_result_t rr;
  static char description[] = "d";
  static descriptions_t descriptions = bufConst(1, description);
  static item_t item_1 = {{tool_name, OBJECT_TYPE_ITEM,0,&descriptions,NULL}, false, false};
  static items_t items = bufConst(1, &item_1);
  static items_t no_items = bufConst(0);
  world_t w = {
      .items = &items,
      .locations = NULL,
      .endings = NULL,
      .turns = 0,
      .inventory = &items,
      .location = NULL,
      .end_game = NULL,
  };

  case("inventory");
  static requirement_tuples_t req_inv = bufConst(1, (requirement_tuple_t){ tool_name, 0 });
  static requirements_t reqs_inv = {
      .inventory = &req_inv,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
  };

  worldAreRequirementsMet(&w, &reqs_inv, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_OK, "inventory: ok");

  req_inv.data[0].state = 1;
  worldAreRequirementsMet(&w, &reqs_inv, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM, "inventory: invalid");

  req_inv.data[0].state = 0;
  w.inventory = &no_items;
  worldAreRequirementsMet(&w, &reqs_inv, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM, "inventory: missing");
  w.inventory = &items; // restore

  case("items");
  static requirement_tuples_t req_items = bufConst(1, (requirement_tuple_t){ tool_name, 0 });
  static requirements_t reqs_items = {
      .inventory = NULL,
      .items = &req_items,
      .locations = NULL,
      .turns = 0,
  };

  worldAreRequirementsMet(&w, &reqs_items, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_OK, "items: ok");

  // Change world item state so requirement tuple mismatches
  item_1.object.state = 1;
  worldAreRequirementsMet(&w, &reqs_items, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_INVALID_WORLD_ITEM, "items: invalid");
  item_1.object.state = 0; // restore

  // Missing world item name
  char missing_name[] ="missing";
  req_items.data[0].name = missing_name;
  worldAreRequirementsMet(&w, &reqs_items, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_MISSING_WORLD_ITEM, "items: missing");
  req_items.data[0].name = tool_name; // restore

  static char loc_desc_str[] = "loc";
  static descriptions_t loc_desc = bufConst(1, loc_desc_str);
  static char loc_1_name[] = "place";
  static location_t loc_1 = {{loc_1_name, OBJECT_TYPE_LOCATION, 0, &loc_desc, NULL}, NULL, NULL};
  static locations_t locations = bufConst(1, &loc_1);
  w.locations = &locations;

  case("locations");
  static requirement_tuples_t req_locs = bufConst(1, (requirement_tuple_t){ loc_1_name, 0 });
  static requirements_t reqs_locs = {
      .inventory = NULL,
      .items = NULL,
      .locations = &req_locs,
      .turns = 0,
  };

  worldAreRequirementsMet(&w, &reqs_locs, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_OK, "locations: ok");

  // Invalid location state
  loc_1.object.state = 1;
  worldAreRequirementsMet(&w, &reqs_locs, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_INVALID_LOCATION, "location: invalid");
  loc_1.object.state = 0; // restore

  case("turns");
  static requirements_t reqs_turns = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 3,
  };
  w.turns = 2;
  worldAreRequirementsMet(&w, &reqs_turns, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS, "turns not enough");
  w.turns = 3;
  worldAreRequirementsMet(&w, &reqs_turns, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_OK, "turns enough");

  case("current_location");
  w.location = &loc_1;
  w.location->object.state = 0;
  static requirement_tuple_t tuple = { loc_1_name, 0 };
  static requirements_t reqs_current_loc = {
      .inventory = NULL,
      .items = NULL,
      .current_location = &tuple,
      .turns = 0,
  };
  worldAreRequirementsMet(&w, &reqs_current_loc, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_OK, "current_location: ok");

  w.location->object.state = 1;
  worldAreRequirementsMet(&w, &reqs_current_loc, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_INVALID_CURRENT_LOCATION, "current_location: invalid");
  w.location->object.state = 0;

  static char other_place_desc_str[] = "other_place";
  static location_t loc_2 = {{other_place_desc_str, OBJECT_TYPE_LOCATION, 0, &loc_desc, NULL}, NULL, NULL};
  w.location = &loc_2;
  worldAreRequirementsMet(&w, &reqs_current_loc, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_CURRENT_LOCATION_MISMATCH, "current_location: mismatch");
  w.location = &loc_1; // restore

  case("no requirements");
  static requirements_t reqs_none = {
      .inventory = NULL,
      .items = NULL,
      .locations = NULL,
      .turns = 0,
  };
  worldAreRequirementsMet(&w, &reqs_none, &rr);
  expectEqlu(rr, REQUIREMENTS_RESULT_NO_REQUIREMENTS, "no requirements result");
}

int main(void) {
  suite(item);
  suite(location);
  suite(digest);
  suite(transition);
  suite(requirements);
  return report();
}
