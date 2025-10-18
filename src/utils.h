#include <stdarg.h>
#include <stdio.h>

#if defined(DEBUG) || !defined(NDEBUG)
static inline void debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  printf("\x1b[2m");
  vprintf(fmt, args);
  printf("\x1b[0m\n");
  va_end(args);
}
#else
static inline void debug(const char *fmt, ...) {
  (void)fmt;
}
#endif
