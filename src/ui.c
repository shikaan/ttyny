#include "ui.h"
#include "alloc.h"
#include "buffers.h"
#include <stddef.h>
#include <stdio.h>

static void sleep_ms(unsigned ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}

static void *loading(void *args) {
  ui_handle_t *state = (ui_handle_t *)args;
  const char *text_variants[] = {"Thinking", "Almost there", "Still working",
                                 "Taking longer than expected", "Nearly ready"};
  const char *dot_variants[] = {".  ", ".. ", "..."};
  for (size_t i = 0; !state->stop; i++) {
    sleep_ms(200);
    const char *dots = dot_variants[i % 3];
    const char *text = text_variants[(i / 18) % 5];
    printf("\033[2K\r%s%s", text, dots);
    fflush(stdout);
  }
  printf("\033[2K\r");
  fflush(stdout);
  return NULL;
}

ui_handle_t *loadingStart(void) {
  ui_handle_t *handle = allocate(sizeof(ui_handle_t));
  panicif(!handle, "cannot allocate loader");

  handle->stop = 0;
  pthread_t tid;
  pthread_create(&tid, NULL, loading, handle);
  handle->tid = tid;
  return handle;
}

void loadingStop(ui_handle_t **handle) {
  (*handle)->stop = 1;
  pthread_join((*handle)->tid, NULL);
  deallocate(handle);
}

void printResponse(string_t *response) { puts(response->data); }
