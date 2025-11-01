#pragma once

#include "alloc.h"
#include "panic.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef const char *key_t;
typedef void *value_t;
typedef uint8_t map_size_t;

typedef enum { MAP_RESULT_OK = 0, MAP_ERROR_FULL } map_result_t;

typedef struct {
  map_size_t size;
  key_t *keys;
  value_t *values;
} map_t;

static inline map_size_t memoryMakeKey(const map_t *self, key_t key) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < strlen(key); i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash % self->size;
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

static inline int hasCollision(const map_t *self, key_t key, uint8_t index) {
  key_t old_key = self->keys[index];
  return old_key != NULL && strcmp(key, old_key) != 0;
}

[[nodiscard]] static inline map_result_t mapSet(map_t *self, const char *key,
                                                void *value) {
  panicif(!self, "map cannot not be null");
  uint8_t index = memoryMakeKey(self, key);

  if (hasCollision(self, key, index)) {
    uint8_t i;

    for (i = 1; i < self->size; i++) {
      uint8_t probed_idx = (index + i) % self->size;
      const key_t probed_key = self->keys[probed_idx];
      if (!probed_key) {
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
  const uint8_t index = memoryMakeKey(self, key);

  for (uint8_t i = 0; i < self->size; i++) {
    uint8_t probed_idx = (index + i) % self->size;
    const key_t probed_key = self->keys[probed_idx];

    if (!probed_key)
      return NULL;

    if (strcmp(probed_key, key) == 0) {
      return self->values[probed_idx];
    }
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
