#pragma once
#include "buffers.h"
#include <pthread.h>
#include <stdatomic.h>

typedef struct {
  pthread_t tid;
  atomic_int stop;
} ui_handle_t;

ui_handle_t* loadingStart(void);
void loadingStop(ui_handle_t **);
void printResponse(string_t *);
