#pragma once

#include "../buffers.h"
#include "item.h"
#include "object.h"

struct location_t;
typedef struct location_t location_t;

// A location is a game object representing a place the user can visit.
typedef Buffer(location_t *) locations_t;

static inline void locationsDestroy(locations_t **self);

struct location_t {
  object_t object;
  // Objects to be found in this location
  items_t *items;
  // Exits from this location into other locations
  locations_t *exits;
};

static inline void locationDestroy(location_t **self) {
  if (!self || !*self)
    return;

  object_t *obj = &(*self)->object;
  objectDestroyInner(&obj);

  itemsDestroy(&(*self)->items);
  locationsDestroy(&(*self)->exits);

  deallocate(self);
}

static inline locations_t *locationsCreate(size_t length) {
  locations_t *locations = NULL;
  bufCreate(locations_t, location_t *, locations, length);
  return locations;
}

static inline int locationsFind(locations_t *self, location_t *location) {
  for (size_t i = 0; i < self->used; i++) {
    if (bufAt(self, i) == location) {
      return (int)i;
    }
  }
  return -1;
}

static inline int locationsFindByName(locations_t *self, object_name_t name) {
  for (size_t i = 0; i < self->used; i++) {
    if (objectNameEq(bufAt(self, i)->object.name, name)) {
      return (int)i;
    }
  }
  return -1;
}

static inline void locationsCat(locations_t *self, locations_t *other) {
  for (size_t i = 0; i < other->used; i++) {
    bufPush(self, bufAt(other, i));
  }
}

static inline void locationsDestroy(locations_t **self) { deallocate(self); }
