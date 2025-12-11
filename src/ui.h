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

// TODO: these area both printing and formatting. They don't conform to the
// print_callback_t type
void uiPrintEndGame(string_t *, game_state_t, const world_t *);
void uiPrintOpeningCredits(const world_t *);
