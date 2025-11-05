#pragma once

#include "alloc.h"
#include "buffers.h"
#include <stdint.h>

typedef uint8_t ring_size_t;
typedef void *ring_value_t;

typedef struct {
  ring_size_t length;
  ring_size_t head;
  ring_value_t data[];
} ring_t;

static inline ring_t *ringCreate(ring_size_t length) {
  ring_t *ring = allocate(sizeof(ring_t) + (sizeof(ring_value_t) * length));
  if (!ring)
    return NULL;

  ring->head = 0;
  ring->length = length;
  return ring;
}

static inline ring_value_t ringPush(ring_t *self, void* value) {
  ring_value_t evicted = self->data[self->head];
  self->data[self->head] = value;
  self->head = (self->head + 1) % self->length;
  return evicted;
}

static inline ring_value_t ringPushUniq(ring_t *self, void* value) {
  for (ring_size_t i = 0; i < self->length; i++) {
    if (value == self->data[i])
      return NULL;
  }
  return ringPush(self, value);
}

static inline ring_value_t ringAt(ring_t *self, ring_size_t index) {
  ring_size_t physical_idx = (self->head + index) % self->length;
  return self->data[physical_idx];
}

static inline void ringDestroy(ring_t **self) {
  if (!self || !*self)
    return;
  deallocate(self);
}
