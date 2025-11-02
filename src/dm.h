#pragma once

#include "ai.h"
#include "buffers.h"
#include "world.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
} dm_t;

dm_t *dmCreate(void);

void dmDescribeWorld(dm_t *, const world_t *, string_t *);
void dmDescribeFail(dm_t *, failure_type_t, const string_t *, string_t *);
void dmDescribeSuccess(dm_t *, const world_t *, const string_t *, string_t *);
void dmDescribeObject(dm_t *, const object_t *, string_t *);
void dmNarrateEndGame(dm_t *, const world_t *, game_state_t, string_t *);

void dmDestroy(dm_t **);
