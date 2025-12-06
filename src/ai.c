#include "ai.h"
#include "lib/alloc.h"
#include "lib/buffers.h"
#include <ggml-backend.h>
#include <ggml.h>
#include <llama.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define GPU_LAYERS 99
static const char STOP_CHARS[] = {'[', '*', '('};

#define throw(Error)                                                           \
  *result = Error;                                                             \
  goto error;

static int containsStopChar(char string[]) {
  for (size_t i = 0; i < arrLen(STOP_CHARS); i++) {
    if (strchr(string, STOP_CHARS[i]) != NULL) {
      return 1;
    }
  }
  return 0;
}

static void filterLogs(enum ggml_log_level level, const char *text,
                         void *data) {
  (void)level;
  (void)text;
  (void)data;
}

static void initSampler(ai_t *ai, ai_result_t *res, config_t *configuration) {
  if (ai->sampler) {
    llama_sampler_free(ai->sampler);
  }

  ai->sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
  if (!ai->sampler) {
    *res = AI_RESULT_ERROR;
    return;
  }

  if (configuration->grammar) {
    struct llama_sampler *grammar_sampler = llama_sampler_init_grammar(
        ai->vocabulary, configuration->grammar->data, "root");

    if (!grammar_sampler) {
      *res = AI_RESULT_ERROR;
      return;
    }
    llama_sampler_chain_add(ai->sampler, grammar_sampler);
  }

  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_penalties(
                              -1, configuration->repetition_penalty, 0, 0));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_temp(configuration->temp));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_top_k(configuration->top_k));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_min_p(configuration->min_p, 1));

  uint32_t seed =
      configuration->seed == 0 ? (uint32_t)time(NULL) : configuration->seed;
  llama_sampler_chain_add(ai->sampler, llama_sampler_init_dist(seed));
  *res = AI_RESULT_OK;
}

ai_t *aiCreate(config_t *configuration, ai_result_t *result) {
  llama_log_set(filterLogs, NULL);

  ai_t *ai = allocate(sizeof(ai_t));
  if (!ai) {
    throw(AI_RESULT_ERROR_ALLOCATION_FAILED);
  }

  ggml_backend_load_all();
  struct llama_model_params params = llama_model_default_params();
  params.n_gpu_layers = GPU_LAYERS;

  ai->model = llama_model_load_from_file(configuration->path, params);
  if (!ai->model) {
    throw(AI_RESULT_ERROR_LOAD_MODEL_FAILED);
  }

  ai->vocabulary = llama_model_get_vocab(ai->model);

  struct llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = configuration->context_size;
  ai->context = llama_init_from_model(ai->model, ctx_params);
  if (!ai->context) {
    throw(AI_RESULT_ERROR_CREATE_CONTEXT_FAILED);
  }

  initSampler(ai, result, configuration);

  ai->configuration = configuration;
  *result = AI_RESULT_OK;
  return ai;

error:
  aiDestroy(&ai);
  return NULL;
}

void aiGenerate(ai_t *ai, ai_result_t *result, const string_t *prompt,
                string_t *response) {
  const bool is_first =
      llama_memory_seq_pos_max(llama_get_memory(ai->context), 0) == -1;

  const int tok_count = -llama_tokenize(
      ai->vocabulary, prompt->data, (int)prompt->used, NULL, 0, is_first, true);

  llama_token *tokens = allocate(sizeof(llama_token) * (size_t)tok_count);
  if (!tokens) {
    throw(AI_RESULT_ERROR_ALLOCATION_FAILED);
  }

  if (llama_tokenize(ai->vocabulary, prompt->data, (int)prompt->used, tokens,
                     tok_count, is_first, true) < 0) {
    throw(AI_RESULT_ERROR_TOKENIZATION_FAILED);
  }

  llama_batch batch = llama_batch_get_one(tokens, tok_count);
  llama_token token_id;

  while (true) {
    uint32_t context_size = llama_n_ctx(ai->context);
    llama_memory_t memory = llama_get_memory(ai->context);
    llama_pos current_context_length = llama_memory_seq_pos_max(memory, 0) + 1;

    if (current_context_length + batch.n_tokens > (int)context_size) {
      throw(AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED);
    }

    if (llama_decode(ai->context, batch) != 0) {
      throw(AI_RESULT_ERROR_BATCH_DECODING_FAILED);
    };

    token_id = llama_sampler_sample(ai->sampler, ai->context, -1);

    if (llama_vocab_is_eog(ai->vocabulary, token_id)) {
      break;
    }

    char parsed_token[256] = {};
    int32_t offset = llama_token_to_piece(
        ai->vocabulary, token_id, parsed_token, sizeof(parsed_token), 0, false);

    if (offset < 0) {
      throw(AI_RESULT_ERROR_TOKEN_PARSING_FAILED);
    }

    if (containsStopChar(parsed_token)) {
      throw(AI_RESULT_ERROR_INVALID_OUTPUT_DETECTED);
    }

    if (response->used + (size_t)offset > response->length) {
      throw(AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED);
    }

    strFmtAppend(response, "%s", parsed_token);
    batch = llama_batch_get_one(&token_id, 1);
  }

  *result = AI_RESULT_OK;
error:
  deallocate(&tokens);
}

void aiSetGrammar(ai_t *self, ai_result_t *result, string_t *grammar) {
  self->configuration->grammar = grammar;
  llama_memory_clear(llama_get_memory(self->context), true);
  initSampler(self, result, self->configuration);
}

void aiReset(ai_t *self, ai_result_t *result) {
  llama_memory_clear(llama_get_memory(self->context), true);
  initSampler(self, result, self->configuration);
}

void aiDestroy(ai_t **self) {
  if (!self || !*self)
    return;
  // TODO: ggml stuff is leaking, but I cannot understand how to free it
  // It's currently ignored in asan.supp

  llama_sampler_free((*self)->sampler);
  (*self)->sampler = NULL;

  llama_free((*self)->context);
  (*self)->context = NULL;

  llama_model_free((*self)->model);
  (*self)->model = NULL;

  llama_backend_free();
  deallocate(self);
}

void aiResultFormat(ai_result_t res, string_t *response) {
  switch (res) {
  case AI_RESULT_OK:
    strFmt(response, "ok");
    return;
  case AI_RESULT_ERROR_LOAD_MODEL_FAILED:
    strFmt(response, "cannot load model");
    return;
  case AI_RESULT_ERROR_CREATE_CONTEXT_FAILED:
    strFmt(response, "cannot create context");
    return;
  case AI_RESULT_ERROR_TOKENIZATION_FAILED:
    strFmt(response, "tokenization failed");
    return;
  case AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED:
    strFmt(response, "context length exceeded");
    return;
  case AI_RESULT_ERROR_BATCH_DECODING_FAILED:
    strFmt(response, "batch decoding failed");
    return;
  case AI_RESULT_ERROR_TOKEN_PARSING_FAILED:
    strFmt(response, "token parsing failed");
    return;
  case AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED:
    strFmt(response, "response length exceeded");
    return;
  case AI_RESULT_ERROR_ALLOCATION_FAILED:
    strFmt(response, "allocation failed");
    return;
  case AI_RESULT_ERROR_INVALID_OUTPUT_DETECTED:
    strFmt(response, "invalid output");
    return;
  default:
  case AI_RESULT_ERROR:
    strFmt(response, "unexpected error");
    return;
  }
}

#undef throw
