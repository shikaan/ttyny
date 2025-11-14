#pragma once

#include "tty.h"

#define cleanup(Callback) __attribute__((cleanup(Callback)))

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ERROR 2

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

#if LOG_LEVEL <= LOG_DEBUG
#define debug(Fmt, ...)                                                        \
  {                                                                            \
    printf(dim("%s:%d\t[DEBUG] - "), __FILE__, __LINE__);                      \
    printf(dim(Fmt) __VA_OPT__(, __VA_ARGS__));                                \
    puts("");                                                                  \
  }
#else
#define debug(Fmt, ...)
#endif

#if LOG_LEVEL <= LOG_INFO
#define info(Fmt, ...)                                                         \
  {                                                                            \
    printf("%s:%d\t[INFO] - ", __FILE__, __LINE__);                            \
    printf(Fmt __VA_OPT__(, __VA_ARGS__));                                     \
    puts("");                                                                  \
  }
#else
#define info(Fmt, ...)
#endif

#if LOG_LEVEL <= LOG_ERROR
#define error(Fmt, ...)                                                        \
  {                                                                            \
    printf(fg_red("%s:%d\t[ERROR] - "), __FILE__, __LINE__);                   \
    printf(fg_red(Fmt) __VA_OPT__(, __VA_ARGS__));                             \
    puts("");                                                                  \
  }
#else
#define error(Fmt, ...)
#endif
