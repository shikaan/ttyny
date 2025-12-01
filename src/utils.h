#pragma once

#include "lib/tty.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

#define cleanup(Callback) __attribute__((cleanup(Callback)))

#define LOG_DEBUG 2
#define LOG_INFO 1
#define LOG_ERROR 0
#define LOG_NONE -1

#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define debug(Fmt, ...)                                                        \
  {                                                                            \
    printf(dim("[DEBUG] %s:%d - "), __FILE__, __LINE__);                       \
    printf(dim(Fmt) __VA_OPT__(, __VA_ARGS__));                                \
    puts("");                                                                  \
  }
#else
#define debug(Fmt, ...)
#endif

#if LOG_LEVEL >= LOG_INFO
#define info(Fmt, ...)                                                         \
  {                                                                            \
    printf("[INFO] %s:%d - ", __FILE__, __LINE__);                             \
    printf(Fmt __VA_OPT__(, __VA_ARGS__));                                     \
    puts("");                                                                  \
  }
#else
#define info(Fmt, ...)
#endif

#if LOG_LEVEL >= LOG_ERROR
#define error(Fmt, ...)                                                        \
  {                                                                            \
    printf(fg_red("[ERROR] %s:%d - "), __FILE__, __LINE__);                    \
    printf(fg_red(Fmt) __VA_OPT__(, __VA_ARGS__));                             \
    puts("");                                                                  \
  }
#else
#define error(Fmt, ...)
#endif
