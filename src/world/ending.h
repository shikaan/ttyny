#pragma once

#include "../buffers.h"
#include "object.h"
#include <stdbool.h>

typedef struct {
  bool success;
  char *reason;
  requirements_t *requirements;
} ending_t;

typedef Buffer(ending_t *) endings_t;

static inline void endingDestory(ending_t **self) {
  if (!self || !*self)
    return;

  deallocate(&(*self)->reason);
  requirementsDestroy(&(*self)->requirements);
  deallocate(self);
}

static inline void endingsDestroy(endings_t **self) { deallocate(self); }
