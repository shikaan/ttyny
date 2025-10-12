#pragma once

#include "buffers.h"
#include <llama.h>

typedef enum {
  AI_RESULT_OK = 0,
  AI_RESULT_ERROR_ALLOCATION_FAILED,
  AI_RESULT_ERROR_LOAD_MODEL_FAILED,
  AI_RESULT_ERROR_CREATE_CONTEXT_FAILED,
  AI_RESULT_ERROR_TOKENIZATION_FAILED,
  AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED,
  AI_RESULT_ERROR_BATCH_DECODING_FAILED,
  AI_RESULT_ERROR_TOKEN_PARSING_FAILED,
  AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED,
  AI_RESULT_ERROR,
} ai_result_t;

ai_result_t aiResultGetLast(void);
void aiResultFormat(ai_result_t, string_t *);

typedef enum {
  PROMPT_TYPE_USR = 0,
  PROMPT_TYPE_SYS = 1,

  PROMPT_TYPES,
} prompt_type_t;

typedef struct {
  const string_t *prompts[PROMPT_TYPES];
} config_t;

typedef struct {
  struct llama_model *model;
  const struct llama_vocab *vocabulary;
  struct llama_context *context;
  struct llama_sampler *sampler;
  config_t *configuration;
} ai_t;

[[nodiscard]] ai_t *aiCreate(const char *, config_t *);
void aiPrompt(ai_t *, prompt_type_t, const string_t *, string_t *);
void aiDestory(ai_t **);

/// MODEL CONFIGURATIONS

static string_t LFM2_USR_PROMPT = {
    31,
    {"<|im_start|>user\n%s<|im_end|>\n"},
};

static string_t LFM2_SYS_PROMPT = {
    48,
    {"<|startoftext|><|im_start|>system\n%s<|im_end|>\n"},
};

static config_t LFM2 = {.prompts = {&LFM2_USR_PROMPT, &LFM2_SYS_PROMPT}};
