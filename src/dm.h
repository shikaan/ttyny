#pragma once

#include "ai.h"
#include "buffers.h"
#include "map.h"
#include "ring.h"
#include "world.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
  map_t* descriptions;
  ring_t* history;
} dm_t;

dm_t *dmCreate(world_t* world);

void dmDescribeLocation(dm_t *, const location_t *, string_t *);
void dmDescribeObject(dm_t *, const object_t *, string_t *);
void dmDescribeSuccess(dm_t *, const world_t *, const string_t *, string_t *);
void dmDescribeEndGame(dm_t *, const world_t *, game_state_t, string_t *);

void dmDestroy(dm_t **);
