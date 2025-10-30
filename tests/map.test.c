#include "../src/map.h"
#include "../src/utils.h"
#include "test.h"

void getSet(void) {
  map_t *map cleanup(mapDestroy) = mapCreate(5);

  int value = 189;
  mapSet(map, "key", &value);

  int *resolved = mapGet(map, "key");
  expectEqli(value, *resolved, "retrieves the value");

  value = 2034;
  mapSet(map, "key", &value);
  expectEqli(value, *resolved, "overrides the value");

  resolved = mapGet(map, "another");
  expectNull(resolved, "returns NULL if value is missing");
}

void collisions(void) {
  map_t *map cleanup(mapDestroy) = mapCreate(5);

  int value1 = 145;
  int value2 = 545;

  // These two strings are known to clash in FNV-1
  const char *key1 = "liquid";
  const char *key2 = "costarring";

  mapSet(map, key1, &value1);
  mapSet(map, key2, &value2);

  int *resolved1 = mapGet(map, key1);
  int *resolved2 = mapGet(map, key2);
  expectNeqi(*resolved1, *resolved2, "values don't overlap");
  expectEqli(*resolved1, value1, "retrieves correct value");
  expectEqli(*resolved2, value2, "retrieves correct other value");

  map_t *map2 cleanup(mapDestroy) = mapCreate(1);

  // setting with key1 and trying to retrieve with colliding key2
  mapSet(map2, key1, &value1);
  expectNull(mapGet(map2, key2), "doesn't return item from colliding key");
}

int main(void) {
  suite(getSet);
  suite(collisions);

  return report();
}
