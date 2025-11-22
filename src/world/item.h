#pragma once

#include "object.h"
#include <stdbool.h>

typedef struct item_t item_t;
typedef Buffer(struct item_t *) items_t;

// An item is an object the player can interact with. It represents a physical
// thing like a lamp or an apple.
typedef struct item_t {
  object_t object;
  // True if the item can be picked up
  bool collectible;
  // True if EXAMINE/USE should present: "<name> reads: \"<state description>\""
  bool readable;
} item_t;

static inline bool itemEquals(item_t *self, item_t *other) {
  return self == other;
}

static inline bool itemHasSameId(item_t *self, item_t *other) {
  return objectNameEq(self->object.name, other->object.name);
}

static inline items_t *itemsCreate(size_t length) {
  items_t *items = NULL;
  bufCreate(items_t, struct item_t *, items, length);
  return items;
}

static inline void itemsCat(items_t *self, items_t *other) {
  for (size_t i = 0; i < other->used; i++) {
    bufPush(self, bufAt(other, i));
  }
}

static inline void itemsRemove(items_t *self, item_t *item) {
  for (size_t i = 0; i < self->used; i++) {
    if (bufAt(self, i) == item) {
      item_t *last = bufAt(self, self->used - 1);
      bufSet(self, i, last);
      bufSet(self, self->used - 1, NULL);
      self->used--;
      break;
    }
  }
}

typedef bool (*item_cmp_t)(item_t *self, item_t *other);

static inline int itemsFind(items_t *self, item_t *item, item_cmp_t compare) {
  for (size_t i = 0; i < self->used; i++) {
    if (compare(bufAt(self, i), item)) {
      return (int)i;
    }
  }
  return -1;
}

static inline void itemsDestroy(items_t **self) { deallocate(self); }
