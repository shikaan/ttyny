#include "../src/lib/set.h"
#include "../src/utils.h"
#include "test.h"

void addHas(void) {
  set_t *set cleanup(setDestroy) = setCreate(5);
  set_size_t size = 0;
  panicif(!set, "cannot create set");

  set_result_t res;
  res = setAdd(set, "key");
  size = setUsed(set);
  expectEqlu(res, SET_RESULT_OK, "add returns OK");
  expectEqllu(size, 1, "updates used");
  expectTrue(setHas(set, "key"), "finds inserted key");
  expectFalse(setHas(set, "another"), "returns 0 for missing key");

  case("idempotency");
  res = setAdd(set, "key"); // duplicate
  expectEqlu(res, SET_RESULT_OK, "adds existing");
  expectEqllu(setUsed(set), size, "does not update used");
  expectTrue(setHas(set, "key"), "has finds key");

  case("full set and deletion");
  (void)setAdd(set, "key1");
  (void)setAdd(set, "key2");
  (void)setAdd(set, "key3");
  (void)setAdd(set, "key4");

  res = setAdd(set, "key5");
  expectEqlu(res, SET_ERROR_FULL, "does not insert with full set");

  size = setUsed(set);
  setDelete(set, "key1");
  expectFalse(setHas(set, "key1"), "does not find deleted key");
  expectEqllu(setUsed(set), size-1, "reduces used");

  res = setAdd(set, "key1");
  expectEqlu(res, SET_RESULT_OK, "adds after deletion");
  expectTrue(setHas(set, "key1"), "finds reset key");
}

void collisions(void) {
  set_t *set cleanup(setDestroy) = setCreate(5);
  panicif(!set, "cannot create set");
  set_result_t res;

  // These two strings are known to clash in FNV-1
  const char *key1 = "liquid";
  const char *key2 = "costarring";

  res = setAdd(set, key1);
  panicif(res != SET_RESULT_OK, "add failed");
  res = setAdd(set, key2);
  panicif(res != SET_RESULT_OK, "add failed");

  expectTrue(setHas(set, key1), "has first colliding key");
  expectTrue(setHas(set, key2), "has second colliding key");

  set_t *set2 cleanup(setDestroy) = setCreate(1);
  res = setAdd(set2, key1);
  panicif(res != SET_RESULT_OK, "add failed");

  case("full set");
  expectEqlu(setAdd(set2, key2), SET_ERROR_FULL, "does not add colliding");
  expectFalse(setHas(set2, key2), "does not contain colliding key");
  expectEqlu(setAdd(set2, key1), SET_RESULT_OK, "allows idempotent add with colliding key");
  expectTrue(setHas(set2, key1), "contains previous colliding key");

  case("chained collisions");
  set_t *set4 cleanup(setDestroy) = setCreate(2);
  (void)setAdd(set4, key1);
  (void)setAdd(set4, key2);
  setDelete(set4, key1);
  expectTrue(setHas(set4, key2), "tail remains retrievable after deleting head");
}

int main(void) {
  suite(addHas);
  suite(collisions);

  return report();
}
