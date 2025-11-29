#pragma once

#include "../buffers.h"
#include "item.h"
#include "object.h"
#include <stddef.h>

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

static inline location_t *locationCreate(void) {
  location_t *location = allocate(sizeof(location_t));
  if (!location)
    return NULL;
  location->object.type = OBJECT_TYPE_LOCATION;
  return location;
}

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

static inline int locationsFindByName(locations_t *self, const_object_name_t name) {
  panicif(!self, "locations cannot be null");

  size_t i;
  bufEach(self, i) {
    if (objectNameEq(bufAt(self, i)->object.name, name)) {
      return (int)i;
    }
  }
  return -1;
}

static inline void locationsDestroy(locations_t **self) { deallocate(self); }
