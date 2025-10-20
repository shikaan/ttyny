#include "ggml.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

typedef pthread_t ui_handle_t;

static void sleep_ms(unsigned ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void *loading(void *args) {
  (void)args;
  const char *text = "Thinking";
  const char *dot_variants[] = {".  ", ".. ", "..."};
  for (size_t i = 0; i < 6; i++) {
    const char *dots = dot_variants[i % 3];
    printf("\033[2K\r%s%s", text, dots);
    fflush(stdout);
    sleep_ms(250);
  }
  printf("\033[2K\r");
  fflush(stdout);
  return NULL;
}

static inline ui_handle_t loadingStart(void) {
  pthread_t tid;
  pthread_create(&tid, NULL, loading, NULL);
  return tid;
}

static inline void loadingWait(ui_handle_t handle) {
  pthread_join(handle, NULL);
}
