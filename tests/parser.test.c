#include "../src/parser.h"
#include "../src/utils.h"
#include "test.h"

void actions(void) {
  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot initialize parser");

  string_t *cmd cleanup(strDestroy) = strCreate(128);
  panicif(!cmd, "cannot initialize command buffer");

  action_t action;

#define test(Command, Action)                                                  \
  strFmt(cmd, "%s", Command);                                                  \
  action = parserExtractAction(parser, cmd);                                   \
  expectEqli(action, Action, Command);

  case("move");
  test("go to the hall", ACTION_MOVE);
  test("walk to hall", ACTION_MOVE);
  test("enter the hall", ACTION_MOVE);
  test("move to hall", ACTION_MOVE);
  test("travel to hall", ACTION_MOVE);
  test("I want to go to the hall", ACTION_MOVE);

  case("take");
  test("grab the key from the table", ACTION_TAKE);
  test("fetch the object", ACTION_TAKE);
  test("pick up the key", ACTION_TAKE);
  test("get the key please", ACTION_TAKE);
  test("take the key", ACTION_TAKE);
  test("I want to take the key", ACTION_TAKE);
  test("retrieve the key", ACTION_TAKE);

  case("examine");
  test("look at the key", ACTION_EXAMINE);
  test("inspect the coin", ACTION_EXAMINE);
  test("examine key closely", ACTION_EXAMINE);
  test("check out the coin", ACTION_EXAMINE);
  test("study the key", ACTION_EXAMINE);
  test("observe coin", ACTION_EXAMINE);

  case("use");
  test("use the key", ACTION_USE);
  test("activate key", ACTION_USE);
  test("apply the coin", ACTION_USE);
  test("employ the lantern", ACTION_USE);
  test("utilize the sword", ACTION_USE);

  case("drop");
  test("drop the key on the ground", ACTION_DROP);
  test("put down the coin", ACTION_DROP);
  test("discard the lantern", ACTION_DROP);
#undef test
}

void targets(void) {
  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot initialize parser");

  string_t *cmd cleanup(strDestroy) = strCreate(128);
  panicif(!cmd, "cannot initialize command buffer");

  items_t *items cleanup(itemsDestroy) = itemsCreate(3);
  panicif(!items, "cannot initialize allowed buffer");
  item_t key = {.object.name = "key"};
  bufPush(items, &key);
  item_t coin = {.object.name = "coin"};
  bufPush(items, &coin);
  item_t sword = {.object.name = "sword"};
  bufPush(items, &sword);

  locations_t *locations cleanup(locationsDestroy) = locationsCreate(3);
  panicif(!locations, "cannot initialize allowed buffer");
  location_t hall = {.object.name = "hall"};
  bufPush(locations, (struct location_t *)&hall);
  location_t kitchen = {.object.name = "kitchen"};
  bufPush(locations, (struct location_t *)&kitchen);
  location_t forest = {.object.name = "forest"};
  bufPush(locations, (struct location_t *)&forest);

  // Just for testing, this room can exit to itself
  forest.exits = locations;
  forest.items = items;

  item_t* item = NULL;
  location_t* location = NULL;

#define testl(Command, Location)                                             \
    item = NULL;                                                               \
    location = NULL;                                                           \
    strFmt(cmd, "%s", Command);                                                \
    parserExtractTarget(parser, cmd, locations, items, &location, &item);      \
    expectTrue(&Location == location, Command);

#define testi(Command, Item)                                                 \
    item = NULL;                                                               \
    location = NULL;                                                           \
    strFmt(cmd, "%s", Command);                                                \
    parserExtractTarget(parser, cmd, locations, items, &location, &item);      \
    expectTrue(&Item == item, Command);

#define testn(Command)                                                       \
    item = NULL;                                                               \
    location = NULL;                                                           \
    strFmt(cmd, "%s", Command);                                                \
    parserExtractTarget(parser, cmd, locations, items, &location, &item);      \
    expectTrue(!location && !item, Command);

  case("location");
  testl("go to the hall", hall);
  testl("walk to kitchen", kitchen);
  testl("enter the forest", forest);
  testl("let's go out in the woods", forest);
  testl("travel to the kitchen", kitchen);
  testl("I want to go to the hall", hall);
  testl("can we please walk together to the kitchen now", kitchen);
  testl("I think we should enter the forest before sunset", forest);
  testn("move to the village");
  testn("go to the castle");

  case("item");
  testi("grab the key from the table", key);
  testi("fetch the money", coin);
  testi("pick up the key", key);
  testi("get the key please", key);
  testi("take the key", key);
  testi("examine key closely", key);
  testi("check out the coin", coin);
  testi("observe coin", coin);
  testi("utilize the sword", sword);
  testn("employ the lantern");
  testn("discard the chair");
#undef testl
#undef testi
#undef testn
}

int main(void) {
  suite(actions);
  suite(targets);
  return report();
}
