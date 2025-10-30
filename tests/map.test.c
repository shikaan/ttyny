#include "../src/map.h"
#include "../src/utils.h"
#include "test.h"

void getSet(void) {
  map_t *map cleanup(mapDestroy) = mapCreate(5);
  map_result_t result;

  int value = 189;
  result = mapSet(map, "key", &value);
  panicif(result != MAP_RESULT_OK, "set failed");

  int *resolved = mapGet(map, "key");
  expectEqli(value, *resolved, "retrieves the value");

  value = 2034;
  result = mapSet(map, "key", &value);
  panicif(result != MAP_RESULT_OK, "set failed");
  expectEqli(value, *resolved, "overrides the value");

  resolved = mapGet(map, "another");
  expectNull(resolved, "returns NULL if value is missing");

  result = mapSet(map, "key1", &value);
  result = mapSet(map, "key2", &value);
  result = mapSet(map, "key3", &value);
  result = mapSet(map, "key4", &value);
  result = mapSet(map, "key5", &value);
  expectEqlu(result, MAP_ERROR_FULL, "errors on full map");
}

void collisions(void) {
  map_t *map cleanup(mapDestroy) = mapCreate(5);
  map_result_t result;

  int value1 = 145;
  int value2 = 545;

  // These two strings are known to clash in FNV-1
  const char *key1 = "liquid";
  const char *key2 = "costarring";

  result = mapSet(map, key1, &value1);
  panicif(result != MAP_RESULT_OK, "set failed");

  result = mapSet(map, key2, &value2);
  panicif(result != MAP_RESULT_OK, "set failed");

  int *resolved1 = mapGet(map, key1);
  int *resolved2 = mapGet(map, key2);
  expectNeqi(*resolved1, *resolved2, "values don't overlap");
  expectEqli(*resolved1, value1, "retrieves correct value");
  expectEqli(*resolved2, value2, "retrieves correct other value");

  map_t *map2 cleanup(mapDestroy) = mapCreate(1);

  // setting with key1 and trying to retrieve with colliding key2
  result = mapSet(map2, key1, &value1);
  panicif(result != MAP_RESULT_OK, "set failed");

  expectNull(mapGet(map2, key2), "doesn't return item from colliding key");
}

int main(void) {
  suite(getSet);
  suite(collisions);

  return report();
}
