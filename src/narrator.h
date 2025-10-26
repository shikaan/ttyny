#pragma once

#include "ai.h"
#include "buffers.h"
#include "world.h"

#include "configs/qwen.h"

typedef struct {
  ai_t *ai;
  string_t *prompt;
  string_t *summary;
} narrator_t;

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

  strFmt(self->prompt, sys_prompt_tpl->data, NARRATOR_SYS_PROMPT.data);

  worldMakeSummary(world, self->summary);
  strFmtAppend(self->prompt, "%s", self->summary->data);
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
         "You are the narrator of a dark fantasy game. You describe ITEM in "
         "one sentence. Be witty.");

  state_t item_state = objectGetState(object->traits);
  strFmtAppend(self->prompt, "ITEM:\n name: %s\n description: %s\n state: %s\n",
               object->name, object->description, object->states[item_state]);

  strFmtAppend(self->prompt, res_prompt_tpl->data, "");

  strClear(description);

  // TODO: maybe this can keep _some_ memory?
  aiReset(self->ai);
  aiGenerate(self->ai, self->prompt, description);
  // TODO: validate output
}
