#pragma once

#include "buffers.h"

#define MODEL_PATH "./models/TinyMistral-248M-v2.5-Instruct.Q8_0.gguf"

typedef enum {
  AI_RESULT_OK = 0,
  AI_RESULT_ERROR_LOAD_MODEL_FAILED,
  AI_RESULT_ERROR_CREATE_CONTEXT_FAILED,
  AI_RESULT_ERROR_TOKENIZATION_FAILED,
  AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED,
  AI_RESULT_ERROR_BATCH_DECODING_FAILED,
  AI_RESULT_ERROR_TOKEN_PARSING_FAILED,
  AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED,
  AI_RESULT_ERROR,
} ai_result_t;

ai_result_t aiInit(const char *model_path);
ai_result_t aiGenerate(const string_t *prompt, string_t *response);
void aiFormatResult(ai_result_t result, string_t *response);