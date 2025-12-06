// set (v0.0.1)
// ---
//
// A simple hashset with owned keysd. It handles conflicts
// through linear probing and has static size.
//
// ```c
// set_t* set = setCreate(10);
//
// setAdd("key"); // returns result
//
// setHas("key"); // retuns true
// setHas("another key"); // retuns false
//
// setDelete("key");
//
// setDestroy(&set);
// ```
// ___HEADER_END___

#pragma once

#include "alloc.h"
#include "buffers.h"
#include "panic.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef char *set_key_t;
typedef const char *const_set_key_t;
typedef uint64_t set_size_t;

static char SET_TOMBSTONE[] = "___TOMBSTONE!!@@##";

typedef enum {
  SET_RESULT_OK = 0,
  SET_ERROR_FULL,
  SET_ERROR_NOT_FOUND
} set_result_t;

typedef struct {
  set_size_t size;
  set_key_t *keys;
} set_t;

static inline set_size_t setMakeKey(const set_t *self, const_set_key_t key) {
  uint64_t hash = 14695981039346656037U;
  const uint64_t prime = 1099511628211U;

  for (size_t i = 0; i < strlen(key); i++) {
    hash ^= (uint64_t)(unsigned char)key[i];
    hash *= prime;
  }

  return hash % self->size;
}

static inline set_result_t setGetIndex(const set_t *self, const_set_key_t key,
                                       set_size_t *result) {
  panicif(!self, "set cannot not be null");
  const set_size_t index = setMakeKey(self, key);

  for (set_size_t i = 0; i < self->size; i++) {
    set_size_t probed_idx = (index + i) % self->size;
    const set_key_t probed_key = self->keys[probed_idx];

    if (!probed_key) {
      return SET_ERROR_NOT_FOUND;
    }

    // Not a mistake! SET_Tombstones are pointers to the constant string
    if (probed_key == SET_TOMBSTONE) {
      continue;
    }

    if (strcmp(probed_key, key) == 0) {
      *result = probed_idx;
      return SET_RESULT_OK;
    }
  }

  return SET_ERROR_NOT_FOUND;
}

static inline set_t *setCreate(set_size_t size) {
  panicif(size <= 0, "size cannot be null");
  set_t *self = allocate(sizeof(set_t));
  if (!self)
    return NULL;

  self->keys = allocate(sizeof(const char *) * size);
  if (!self->keys) {
    deallocate(&self);
    return NULL;
  }

  self->size = size;

  return self;
}

__attribute__((warn_unused_result)) static inline set_result_t
setAdd(set_t *self, const_set_key_t key) {
  panicif(!self, "set cannot not be null");
  set_size_t index = setMakeKey(self, key);
  set_key_t old_key = self->keys[index];
  int collides_with_old_key =
      !!old_key && strcmp(key, old_key) != 0 && old_key != SET_TOMBSTONE;

  // When there is a collision with another key, look for the next free index
  if (collides_with_old_key) {
    uint8_t i;

    for (i = 1; i < self->size; i++) {
      set_size_t probed_idx = (index + i) % self->size;
      const set_key_t probed_key = self->keys[probed_idx];

      // We write on the index when:
      //  - probed_key is null -> the key was not touched
      //  - probed_key is SET_TOMBSTONE -> the key was written and then deleted
      const int should_write_on_index =
          !probed_key || probed_key == SET_TOMBSTONE;
      if (should_write_on_index) {
        index = probed_idx;
        goto set;
      }

      // The key is already there just return
      if (!strcmp(probed_key, key)) {
        return SET_RESULT_OK;
      }
    }

    if (i == self->size) {
      return SET_ERROR_FULL;
    }
  }

set:
  self->keys[index] = strdup(key);
  return SET_RESULT_OK;
}

static inline int setHas(const set_t *self, const_set_key_t key) {
  panicif(!self, "set cannot not be null");
  set_size_t index;
  return setGetIndex(self, key, &index) == SET_RESULT_OK;
}

static inline void setDelete(set_t *self, const_set_key_t key) {
  panicif(!self, "set cannot not be null");
  set_size_t index;
  if (setGetIndex(self, key, &index) == SET_RESULT_OK) {
    set_key_t old_key = self->keys[index];
    deallocate(&old_key);
    self->keys[index] = SET_TOMBSTONE;
  }
  return;
}

static inline size_t setUsed(const set_t *self) {
  size_t used = 0;
  for (size_t i = 0; i < self->size; i++) {
    set_key_t key = self->keys[i];
    if (key && key != SET_TOMBSTONE) {
      used++;
    }
  }
  return used;
}

static inline void setDestroy(set_t **self) {
  if (!self || !*self)
    return;

  for (size_t i = 0; i < (*self)->size; i++) {
    if ((*self)->keys[i] != SET_TOMBSTONE) {
      deallocate(&(*self)->keys[i]);
    }
  }

  deallocate(&(*self)->keys);
  deallocate(self);
}
