#pragma once

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static inline double_t avgllu(size_t size, uint64_t *samples) {
  uint64_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += (samples)[i];
  }

  return (double)sum / (double)size;
}

static inline int ltllu(const void *arg1, const void *arg2) {
  const uint64_t *a = (const uint64_t *)arg1;
  const uint64_t *b = (const uint64_t *)arg2;
  return *a < *b ? -1 : (*a > *b) ? 1 : 0;
}

static inline uint64_t percllu(uint8_t p, size_t size, uint64_t *samples) {
  panicif(p > 100 || !p, "percentage should be 1-100");
  qsort(samples, size, sizeof(samples[0]), ltllu);
  size_t idx = (size_t)floor((double)size * (double)p / 100) - 1;
  return samples[idx];
}
