#pragma once

#include "alloc.h"
#include "buffers.h"
#include "panic.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef const char *key_t;
typedef void *value_t;
typedef uint64_t map_size_t;

typedef enum {
  MAP_RESULT_OK = 0,
  MAP_ERROR_FULL,
  MAP_ERROR_NOT_FOUND
} map_result_t;

typedef struct {
  map_size_t size;
  key_t *keys;
  value_t *values;
} map_t;

static inline map_size_t mapMakeKey(const map_t *self, key_t key) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < strlen(key); i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash % self->size;
}

static inline map_result_t mapGetIndex(const map_t *self, key_t key,
                                       map_size_t *result) {
  panicif(!self, "map cannot not be null");
  const map_size_t index = mapMakeKey(self, key);

  for (map_size_t i = 0; i < self->size; i++) {
    map_size_t probed_idx = (index + i) % self->size;
    const key_t probed_key = self->keys[probed_idx];

    if (!probed_key) {
      continue;
    }

    if (strcmp(probed_key, key) == 0) {
      *result = probed_idx;
      return MAP_RESULT_OK;
    }
  }

  return MAP_ERROR_NOT_FOUND;
}

static inline map_t *mapCreate(map_size_t size) {
  panicif(size == 0, "size cannot be zero");

  map_t *self = allocate(sizeof(map_t));
  if (!self)
    return NULL;

  self->keys = allocate(sizeof(const char *) * size);
  if (!self->keys) {
    deallocate(&self);
    return NULL;
  }

  self->values = allocate(sizeof(void *) * size);
  if (!self->values) {
    deallocate(&self);
    deallocate(&self->keys);
    return NULL;
  }

  self->size = size;

  return self;
}

[[nodiscard]] static inline map_result_t mapSet(map_t *self, const key_t key,
                                                value_t value) {
  panicif(!self, "map cannot not be null");
  map_size_t index = mapMakeKey(self, key);
  key_t old_key = self->keys[index];
  int collides_with_old_key = !!old_key && strcmp(key, old_key) != 0;

  // When there is a collision with another key, look for the next free index
  if (collides_with_old_key) {
    uint8_t i;

    for (i = 1; i < self->size; i++) {
      map_size_t probed_idx = (index + i) % self->size;
      const key_t probed_key = self->keys[probed_idx];

      // If the value does not exist or exists and is the same key
      if (!probed_key || (probed_key && strcmp(probed_key, key) == 0)) {
        index = probed_idx;
        goto set;
      }
    }

    if (i == self->size) {
      return MAP_ERROR_FULL;
    }
  }

set:
  self->keys[index] = key;
  self->values[index] = value;
  return MAP_RESULT_OK;
}

static inline value_t mapGet(const map_t *self, const key_t key) {
  panicif(!self, "map cannot not be null");
  map_size_t index;
  if (mapGetIndex(self, key, &index) == MAP_RESULT_OK) {
    return self->values[index];
  }
  return NULL;
}

static inline value_t mapDelete(map_t *self, const key_t key) {
  panicif(!self, "map cannot not be null");
  map_size_t index;
  if (mapGetIndex(self, key, &index) == MAP_RESULT_OK) {
    value_t previous = self->values[index];
    self->values[index] = NULL;
    self->keys[index] = NULL;
    return previous;
  }
  return NULL;
}

static inline void mapDestroy(map_t **self) {
  if (!self || !*self)
    return;

  deallocate(&(*self)->keys);
  deallocate(&(*self)->values);
  deallocate(self);
}
