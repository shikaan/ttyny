#pragma once

#include "lib/buffers.h"
#include <llama.h>
#include <stdint.h>

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
  AI_RESULT_ERROR_INVALID_OUTPUT_DETECTED,
  AI_RESULT_ERROR,
} ai_result_t;

ai_result_t aiResultGetLast(void);
void aiResultFormat(ai_result_t, string_t *);

typedef enum {
  PROMPT_TYPE_USR = 0,
  PROMPT_TYPE_SYS,
  PROMPT_TYPE_RES,

  PROMPT_TYPES,
} prompt_type_t;

typedef struct {
  const char *path;
  const string_t *prompt_templates[PROMPT_TYPES];
  const string_t *grammar;
  float min_p;
  float temp;
  float repetition_penalty;
  uint32_t context_size;
  int32_t top_k;
  uint32_t seed;
} config_t;

typedef struct {
  struct llama_model *model;
  const struct llama_vocab *vocabulary;
  struct llama_context *context;
  struct llama_sampler *sampler;
  config_t *configuration;
} ai_t;

__attribute__((warn_unused_result)) ai_t *aiCreate(config_t *, ai_result_t *);
void aiDestroy(ai_t **);

void aiGenerate(ai_t *, ai_result_t *, const string_t *, string_t *);
void aiSetGrammar(ai_t *, ai_result_t *, string_t *);
void aiReset(ai_t *, ai_result_t *);
