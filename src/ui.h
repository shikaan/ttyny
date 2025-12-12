#pragma once
#include "lib/buffers.h"
#include "world/world.h"
#include <pthread.h>
#include <stdatomic.h>

typedef struct {
  pthread_t tid;
  atomic_int stop;
} ui_handle_t;

ui_handle_t *uiLoadingStart(void);
void uiLoadingStop(ui_handle_t **);

void uiClearScreen(void);

typedef void (*print_string_callback_t)(string_t *);
void uiPrintError(string_t *);
void uiPrintCommandOutput(string_t *);
void uiPrintDescription(string_t *);
void uiPrintReadable(string_t *);

void uiPrintStateUpdates(strings_t *);


void uiFormatAndPrintEndGame(string_t *, game_state_t, const world_t *);
void uiFormatAndPrintOpeningCredits(const world_t *);
