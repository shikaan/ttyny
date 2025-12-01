#pragma once
#include "lib/buffers.h"
#include "world/world.h"
#include <pthread.h>
#include <stdatomic.h>

typedef struct {
  pthread_t tid;
  atomic_int stop;
} ui_handle_t;

ui_handle_t *loadingStart(void);
void loadingStop(ui_handle_t **);

void printError(string_t *);
void printStateUpdate(string_t *);
void printCommandOutput(string_t *);
void printDescription(string_t *);
void printReadable(string_t *);
void printEndGame(string_t *, game_state_t);

void formatWelcomeScreen(string_t *);
void formatLocationChange(string_t *, location_t *);
void formatHelp(string_t *, const world_t *);
void formatStatus(string_t *, const world_t *);
void formatTldr(string_t *, const world_t *);
void formatTake(string_t *, const item_t *);
void formatDrop(string_t *, const item_t *);
void formatUse(string_t *, const item_t *);

void screenClear(void);
