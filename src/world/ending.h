#pragma once

#include "../buffers.h"
#include "object.h"
#include <stdbool.h>

typedef struct {
  bool success;
  const char *reason;
  requirements_t *requirements;
} ending_t;

typedef Buffer(ending_t*) endings_t;
