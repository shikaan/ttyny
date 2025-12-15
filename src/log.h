#pragma once

#include <stdarg.h>
#include <stdlib.h>

typedef enum {
  LOG_LEVEL_UNKNOWN = -1,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG,
} log_level_t;

#define loge(Fmt, ...)                                                         \
  logAt(LOG_LEVEL_ERROR, __FILE__, __LINE__, (Fmt), ##__VA_ARGS__);
#define logi(Fmt, ...)                                                         \
  logAt(LOG_LEVEL_INFO, __FILE__, __LINE__, (Fmt), ##__VA_ARGS__);
#define logd(Fmt, ...)                                                         \
  logAt(LOG_LEVEL_DEBUG, __FILE__, __LINE__, (Fmt), ##__VA_ARGS__);

void logAt(log_level_t, const char *, int, const char *, ...);
void logInit(log_level_t level);
