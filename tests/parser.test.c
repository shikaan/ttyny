#include "../src/parser.h"
#include "test.h"

void actions(void) {
  parser_t *parser __attribute__((cleanup(parserDestroy))) = parserCreate();
  panicif(!parser, "cannot initialize parser");

  string_t *cmd __attribute__((cleanup(strDestroy))) = strCreate(128);
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
  parser_t *parser __attribute__((cleanup(parserDestroy))) = parserCreate();
  panicif(!parser, "cannot initialize parser");

  string_t *cmd __attribute__((cleanup(strDestroy))) = strCreate(128);
  panicif(!cmd, "cannot initialize command buffer");

  string_t *target __attribute__((cleanup(strDestroy))) = strCreate(128);
  panicif(!target, "cannot initialize command buffer");

  items_t *items __attribute__((cleanup(itemsDestroy))) = itemsCreate(3);
  panicif(!items, "cannot initialize allowed buffer");
  item_t key = {.name = "key"};
  bufPush(items, &key);
  item_t coin = {.name = "coin"};
  bufPush(items, &coin);
  item_t sword = {.name = "sword"};
  bufPush(items, &sword);

  locations_t *locations __attribute__((cleanup(locationsDestroy))) = locationsCreate(3);
  panicif(!locations, "cannot initialize allowed buffer");
  location_t hall = {.name = "hall"};
  bufPush(locations, (struct location_t *)&hall);
  location_t kitchen = {.name = "kitchen"};
  bufPush(locations, (struct location_t *)&kitchen);
  location_t forest = {.name = "forest"};
  bufPush(locations, (struct location_t *)&forest);

#define test(Command, Target)                                                  \
  strFmt(cmd, "%s", Command);                                                  \
  parserExtractTarget(parser, cmd, locations, items, target);                  \
  expectEqls(Target, target->data, sizeof(Target), Command);

  case("location");
  test("go to the hall", "hall");
  test("walk to kitchen", "kitchen");
  test("enter the forest", "forest");
  test("move to the village", "unknown");
  test("let's go out in the woods", "forest");
  test("travel to the kitchen", "kitchen");
  test("I want to go to the hall", "hall");
  test("go to the castle", "unknown");
  test("can we please walk together to the kitchen now", "kitchen");
  test("I think we should enter the forest before sunset", "forest");

  case("item");
  test("grab the key from the table", "key");
  test("fetch the money", "coin");
  test("pick up the key", "key");
  test("get the key please", "key");
  test("take the key", "key");
  test("examine key closely", "key");
  test("check out the coin", "coin");
  test("observe coin", "coin");
  test("employ the lantern", "unknown");
  test("utilize the sword", "sword");
  test("discard the chair", "unknown");
#undef test
}

int main(void) {
  suite(actions);
  suite(targets);
  return report();
}
