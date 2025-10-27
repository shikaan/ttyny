#pragma once

#include "ai.h"
#include "buffers.h"
#include "world.h"

#include "configs/qwen.h"
#include <stdio.h>

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
} narrator_t;

typedef struct {
  const char *action;
  const failures_t failure;
  const char *response;
} failure_shot_t;

static failure_shot_t failure_shots[] = {
    {"look at the key", FAILURE_INVALID_TARGET,
     "You can't see any such thing."},
    {"inspect banana", FAILURE_INVALID_TARGET,
     "A banana? The area is distinctly banana-free."},
    {"examine tree", FAILURE_INVALID_TARGET,
     "It is difficult to examine a tree that isn't here."},
    {"examine spaceship", FAILURE_INVALID_TARGET,
     "The notion of a spaceship here is, frankly, ludicrous."},
    {"go north", FAILURE_INVALID_LOCATION, "You can't go that way."},
    {"enter the closet", FAILURE_INVALID_LOCATION,
     "The closet, pleasant as it may be, is not an exit."},
    {"enter the bottle", FAILURE_INVALID_LOCATION,
     "You'd have to be considerably smaller to fit in there."},
    {"go to the kitchen", FAILURE_INVALID_LOCATION,
     "You can't get there from here."},
};

static inline narrator_t *narratorCreate(void) {
  narrator_t *narrator = allocate(sizeof(narrator_t));
  panicif(!narrator, "cannot allocate narrator");

  narrator->ai = aiCreate(&NARRATOR_CONFIG);
  panicif(!narrator->ai, "cannot allocate AI for narrator");

  narrator->prompt = strCreate(4096);
  panicif(!narrator->prompt, "cannot allocate prompt buffer");

  narrator->summary = strCreate(4096);
  panicif(!narrator->summary, "cannot allocate summary buffer");

  return narrator;
}

static inline void narratorDestroy(narrator_t **self) {
  if (!self || !*self)
    return;

  aiDestory(&(*self)->ai);
  strDestroy(&(*self)->prompt);
  strDestroy(&(*self)->summary);
  deallocate(self);
}

static inline void narratorDescribeWorld(narrator_t *self, world_t *world,
                                         string_t *description) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         NARRATOR_WORLD_DESC_SYS_PROMPT.data);

  worldMakeSummary(world, self->summary);
  strFmtAppend(self->prompt, "\n%s", self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  strClear(description);

  // TODO: maybe this can keep _some_ memory?
  aiReset(self->ai);
  aiGenerate(self->ai, self->prompt, description);
  // TODO: validate output
}

static inline void narratorDescribeObject(narrator_t *self, object_t *object,
                                          string_t *description) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data,
         NARRATOR_OBJECT_DESC_SYS_PROMPT.data);

  state_t item_state = objectGetState(object->traits);
  strFmtAppend(self->prompt,
               "\nITEM:\n name: %s\n description: %s\n state: %s\n",
               object->name, object->description, object->states[item_state]);

  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  strClear(description);

  // TODO: maybe this can keep _some_ memory?
  aiReset(self->ai);
  aiGenerate(self->ai, self->prompt, description);
  // TODO: validate output
}

static inline void narratorCommentFailure(narrator_t *self, failures_t failure,
                                          string_t *input, string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_FAILURE_SYS_PROMPT.data);

  const char *tpl = "ACTION: %s\n"
                    "FAILURE: %s";

  char shot_buffer[256] = {};
  for (size_t i = 0; i < arrLen(failure_shots); i++) {
    const failure_shot_t shot = failure_shots[i];
    snprintf(shot_buffer, sizeof(shot_buffer), tpl, shot.action,
             failure_names[shot.failure]->data);
    strFmtAppend(self->prompt, usr_prompt_tpl->data, shot_buffer);
    strFmtAppend(self->prompt, res_prompt_tpl->data, shot.response);
  }

  strFmt(self->summary, tpl, input->data, failure_names[failure]->data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, self->summary->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  strClear(comment);

  aiReset(self->ai);
  aiGenerate(self->ai, self->prompt, comment);
}

static inline void narratorCommentSuccess(narrator_t *self, world_t *world,
                                          const string_t *input,
                                          string_t *comment) {
  const config_t *config = self->ai->configuration;
  const string_t *sys_prompt_tpl = config->prompt_templates[PROMPT_TYPE_SYS];
  const string_t *usr_prompt_tpl = config->prompt_templates[PROMPT_TYPE_USR];
  const string_t *res_prompt_tpl = config->prompt_templates[PROMPT_TYPE_RES];

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_SUCCESS_SYS_PROMPT.data);
  worldMakeSummary(world, self->summary);
  strFmtAppend(self->prompt, "\n%s", self->summary->data);
  strFmtAppend(self->prompt, usr_prompt_tpl->data, input->data);
  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  strClear(comment);

  aiReset(self->ai);
  aiGenerate(self->ai, self->prompt, comment);
}
