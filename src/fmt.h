#pragma once
#include "lib/buffers.h"
#include "world/world.h"

#define promptfmt fg_yellow
#define commandfmt fg_blue
#define locationfmt fg_magenta
#define itemfmt fg_cyan
#define successfmt fg_green
#define failfmt fg_red
#define descriptionfmt italic
#define numberfmt bold

void fmtWelcomeScreen(string_t *);
void fmtLocationChange(string_t *, location_t *);
void fmtHelp(string_t *, const world_t *);
void fmtStatus(string_t *, const world_t *);
void fmtTldr(string_t *, const world_t *);
void fmtTake(string_t *, const item_t *);
void fmtDrop(string_t *, const item_t *);
void fmtUse(string_t *, const item_t *);
void fmtTransition(string_t *, const object_t *);
