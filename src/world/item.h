#pragma once

#include "object.h"
#include <stdbool.h>
#include <stddef.h>

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

static inline item_t *itemCreate(void) {
  item_t *item = allocate(sizeof(item_t));
  if (!item)
    return NULL;
  item->object.type = OBJECT_TYPE_ITEM;
  return item;
}

static inline void itemDestroy(item_t **self) {
  if (!self || !*self)
    return;

  object_t *obj = &(*self)->object;
  objectDestroyInner(&obj);

  deallocate(self);
}

static inline items_t *itemsCreate(size_t length) {
  items_t *items = NULL;
  bufCreate(items_t, struct item_t *, items, length);
  return items;
}

static inline int itemsFind(items_t *self, item_t *item) {
  panicif(!self, "items cannot be null");
  bufFind(self, item);
}

static inline int itemsFindByName(items_t *self, const_object_name_t name) {
  panicif(!self, "items cannot be null");
  size_t i;
  bufEach(self, i) {
    if (objectNameEq(bufAt(self, i)->object.name, name)) {
      return (int)i;
    }
  }
  return -1;
}

static inline void itemsDestroy(items_t **self) { deallocate(self); }
