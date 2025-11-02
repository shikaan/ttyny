#pragma once
#include "buffers.h"
#include "world.h"
#include "tty.h"
#include <pthread.h>
#include <stdatomic.h>

#define prompt fg_yellow
#define command fg_blue
#define location fg_magenta
#define item fg_cyan

typedef struct {
  pthread_t tid;
  atomic_int stop;
} ui_handle_t;

ui_handle_t* loadingStart(void);
void loadingStop(ui_handle_t **);

void printError(string_t*);
void printStateUpdate(string_t*);
void printCommandOutput(string_t*);
void printDescription(string_t*);
void printPrompt(void);

void printWelcomeScreen(const char *, string_t *);
void printLocationChange(string_t *, location_t *);

void screenClear(void);
