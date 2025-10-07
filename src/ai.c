#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "ggml-backend.h"
#include <ggml.h>
#include <llama.h>
#include <stddef.h>
#include <stdio.h>

#define GPU_LAYERS 99
#define MIN_P 0.05f
#define TEMP 0.1f

static struct llama_model *model = NULL;
static const struct llama_vocab *vocabulary = NULL;
static struct llama_context *context = NULL;
static struct llama_sampler *sampler = NULL;

#define throw(Error)                                                           \
  result = AI_RESULT_ERROR_LOAD_MODEL_FAILED;                                  \
  goto error;

static void filterLogs(enum ggml_log_level level, const char *text,
                       void *data) {
  (void)data;
  if (level >= GGML_LOG_LEVEL_ERROR) {
    fprintf(stderr, "%s", text);
  }
}

ai_result_t aiInit(const char *model_path) {
  ai_result_t result;
  llama_log_set(filterLogs, NULL);

  ggml_backend_load_all();
  struct llama_model_params params = llama_model_default_params();
  params.n_gpu_layers = GPU_LAYERS;

  model = llama_model_load_from_file(model_path, params);
  if (!model) {
    throw(AI_RESULT_ERROR_LOAD_MODEL_FAILED);
  }

  vocabulary = llama_model_get_vocab(model);

  struct llama_context_params ctx_params = llama_context_default_params();
  context = llama_init_from_model(model, ctx_params);
  if (!context) {
    throw(AI_RESULT_ERROR_CREATE_CONTEXT_FAILED);
  }

  sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
  llama_sampler_chain_add(sampler, llama_sampler_init_min_p(MIN_P, 1));
  llama_sampler_chain_add(sampler, llama_sampler_init_temp(TEMP));
  llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

  return AI_RESULT_OK;

error:
  aiTeardown();
  return result;
}

ai_result_t aiGenerate(const string_t *prompt, string_t *response) {
  int response_offset = 0;
  ai_result_t result;

  const bool is_first =
      llama_memory_seq_pos_max(llama_get_memory(context), 0) == -1;

  const int tok_count = -llama_tokenize(
      vocabulary, prompt->data, prompt->length, NULL, 0, is_first, true);

  llama_token *tokens = malloc(sizeof(llama_token) * (size_t)tok_count);

  if (llama_tokenize(vocabulary, prompt->data, prompt->length, tokens,
                     tok_count, is_first, true) < 0) {
    throw(AI_RESULT_ERROR_TOKENIZATION_FAILED);
  }

  llama_batch batch = llama_batch_get_one(tokens, tok_count);
  llama_token token_id;

  while (true) {
    uint32_t context_size = llama_n_ctx(context);
    llama_memory_t memory = llama_get_memory(context);
    llama_pos current_context_length = llama_memory_seq_pos_max(memory, 0) + 1;

    if (current_context_length + batch.n_tokens > (int)context_size) {
      throw(AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED);
    }

    if (llama_decode(context, batch) != 0) {
      throw(AI_RESULT_ERROR_BATCH_DECODING_FAILED);
    };

    token_id = llama_sampler_sample(sampler, context, -1);

    if (llama_vocab_is_eog(vocabulary, token_id)) {
      break;
    }

    char parsed_token[256] = {};
    ssize_t offset =
        llama_token_to_piece(vocabulary, token_id, parsed_token, 256, 0, false);

    if (offset < 0) {
      throw(AI_RESULT_ERROR_TOKEN_PARSING_FAILED);
    }

    if (response_offset + offset > response->length) {
      throw(AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED);
    }

    response_offset += snprintf(response->data + response_offset,
                                (size_t)response->length, "%s", parsed_token);

    batch = llama_batch_get_one(&token_id, 1);
  }

  deallocate(&tokens);
  return AI_RESULT_OK;

error:
  deallocate(&tokens);
  return result;
}

void aiFormatResult(ai_result_t result, string_t *response) {
  switch (result) {
  case AI_RESULT_OK:
    snprintf(response->data, (size_t)response->length, "ok");
    break;
  case AI_RESULT_ERROR_LOAD_MODEL_FAILED:
    snprintf(response->data, (size_t)response->length, "cannot load model");
    break;
  case AI_RESULT_ERROR_CREATE_CONTEXT_FAILED:
    snprintf(response->data, (size_t)response->length, "cannot create context");
    break;
  case AI_RESULT_ERROR_TOKENIZATION_FAILED:
    snprintf(response->data, (size_t)response->length, "tokenization failed");
    break;
  case AI_RESULT_ERROR_CONTEXT_LENGTH_EXCEEDED:
    snprintf(response->data, (size_t)response->length,
             "context length exceeded");
    break;
  case AI_RESULT_ERROR_BATCH_DECODING_FAILED:
    snprintf(response->data, (size_t)response->length, "batch decoding failed");
    break;
  case AI_RESULT_ERROR_TOKEN_PARSING_FAILED:
    snprintf(response->data, (size_t)response->length, "token parsing failed");
    break;
  case AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED:
    snprintf(response->data, (size_t)response->length,
             "response length exceeded");
    break;
  default:
  case AI_RESULT_ERROR:
    snprintf(response->data, (size_t)response->length, "unexpected error");
    break;
  }
}

void aiTeardown(void) {
  // TODO: ggml stuff is leaking, but I cannot understand how to free it
  // It's currently ignored in asan.supp

  llama_sampler_free(sampler);
  sampler = NULL;

  llama_free(context);
  context = NULL;

  llama_model_free(model);
  model = NULL;

  llama_backend_free();
}