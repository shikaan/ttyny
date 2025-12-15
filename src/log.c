#include "log.h"

#include "lib/panic.h"
#include <syslog.h>
#include <stdio.h>

static inline int logLevelToSyslogPriority(log_level_t level) {
  switch (level) {
  case LOG_LEVEL_ERROR:
    return LOG_ERR;
  case LOG_LEVEL_INFO:
    return LOG_INFO;
  case LOG_LEVEL_DEBUG:
    return LOG_DEBUG;
  case LOG_LEVEL_UNKNOWN:
  default:
    panic("unreachable");
  }
  return -1;
}

void logAt(log_level_t level, const char *file, int line, const char *fmt,
           ...) {
  va_list ap;
  va_start(ap, fmt);
  int priority = logLevelToSyslogPriority(level);
#if !defined(NDEBUG) || defined(DEBUG)
  char prefix[256];
  int n = snprintf(prefix, sizeof(prefix), "%s:%d: ", file, line);
  if (n < 0) {
    vsyslog(priority, fmt, ap);
    va_end(ap);
    return;
  }

  char msg[2048];
  int m = vsnprintf(msg, sizeof(msg), fmt, ap);

  if (m < 0) {
    vsyslog(priority, fmt, ap);
    va_end(ap);
    return;
  }

  syslog(priority, "%s%s", prefix, msg);
  va_end(ap);
#else
  (void)file;
  (void)line;
  vsyslog(priority, fmt, ap);
  va_end(ap);
#endif
}

void logInit(log_level_t level) {
  panicif(level == LOG_LEVEL_UNKNOWN, "unexpected log level");
  openlog("ttyny", LOG_CONS, LOG_USER);
  setlogmask(LOG_UPTO(logLevelToSyslogPriority(level)));
  atexit(closelog);
}
