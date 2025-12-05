#include "../src/lib/map.h"
#include "../src/utils.h"
#include "test.h"

void getSet(void) {
  map_t *map cleanup(mapDestroy) = mapCreate(5);
  map_result_t result;

  int value = 189, another_value = 185;
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
  expectEqlu(result, MAP_ERROR_FULL, "does not insert with full map");
  result = mapSet(map, "key4", &value);
  expectEqlu(result, MAP_RESULT_OK, "allows updates with full map");

  resolved = mapGet(map, "key1");
  panicif(!resolved, "resolve set value");
  resolved = mapDelete(map, "key1");
  expectEqli(value, *resolved, "returns deleted value");

  resolved = mapGet(map, "key1");
  expectNull(resolved, "does not resolve deleted value");

  result = mapSet(map, "key1", &another_value);
  expectEqlu(result, MAP_RESULT_OK, "deleted value can be reset");
  resolved = mapGet(map, "key1");
  expectEqli(*resolved, another_value, "returns correct value set after delete");
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

  map_t *map3 cleanup(mapDestroy) = mapCreate(5);
  (void)mapSet(map3, key1, &value1);
  (void)mapSet(map3, key2, &value1);
  (void)mapSet(map3, key2, &value2);

  int *resolved3 = mapGet(map3, key2);
  expectEqli(value2, *resolved3,
             "correct value when updating linearly-probed key");

  map_t *map4 cleanup(mapDestroy) = mapCreate(2);
  (void)mapSet(map4, key1, &value1);
  (void)mapSet(map4, key2, &value2);

  mapDelete(map, key1);

  int *resolved = mapGet(map, key2);
  panicif(!resolved, "did not find the colliding key");
  expectEqli(*resolved, value2, "correct value on removing colliding key");

  {
    map_t *m cleanup(mapDestroy) = mapCreate(2);
    int v1 = 11, v2 = 22;
    (void)mapSet(m, key1, &v1);
    (void)mapSet(m, key2, &v2);
    (void)mapDelete(m, key1);
    int *p = mapGet(m, key2);
    panicif(!p, "tail of chain retrievable after deleting head");
    expectEqli(*p, v2, "tail value intact");
  }
}

int main(void) {
  suite(getSet);
  suite(collisions);

  return report();
}
