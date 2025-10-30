#pragma once

#include <stdio.h>
#include <stdlib.h>

static inline void __panic(const char msg[], const char file[], int line) {
  fprintf(stderr, "panic: %s\n  at %s:%d\n", msg, file, line);
  abort();
}

static inline void __panicif(int condition, const char msg[], const char file[],
                             int line) {
  if (condition) {
    __panic(msg, file, line);
  }
}

#define panic(Msg) __panic(Msg, __FILE__, __LINE__)
#define panicif(Condition, Msg) __panicif(Condition, Msg, __FILE__, __LINE__)

#if defined(DEBUG) || !defined(NDEBUG)
#define panicdbg(Condition, Msg) panicif(Condition, Msg)
#else
#define panicdbg(Condition, Msg)
#endif
