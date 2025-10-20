#include "ai.h"
#include "alloc.h"
#include "buffers.h"
#include "utils.h"
#include <ggml-backend.h>
#include <ggml.h>
#include <llama.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define GPU_LAYERS 99

static ai_result_t result = AI_RESULT_ERROR;
#define throw(Error)                                                           \
  result = Error;                                                              \
  goto error;

ai_result_t aiResultGetLast(void) { return result; }

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
  default:
  case AI_RESULT_ERROR:
    strFmt(response, "unexpected error");
    return;
  }
}

static void filterLogs(enum ggml_log_level level, const char *text,
                       void *data) {
  (void)data;
  if (level >= GGML_LOG_LEVEL_ERROR) {
    fprintf(stderr, "%s", text);
  }
}

ai_t *aiCreate(const char *model_path, config_t *configuration) {
  llama_log_set(filterLogs, NULL);

  ai_t *ai = allocate(sizeof(ai_t));
  if (!ai) {
    throw(AI_RESULT_ERROR_ALLOCATION_FAILED);
  }

  ggml_backend_load_all();
  struct llama_model_params params = llama_model_default_params();
  params.n_gpu_layers = GPU_LAYERS;

  ai->model = llama_model_load_from_file(model_path, params);
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

  ai->sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_min_p(configuration->min_p, 1));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_temp(configuration->temp));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_top_k(configuration->top_k));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_penalties(
                              -1, configuration->repetition_penalty, 0, 0));
  llama_sampler_chain_add(ai->sampler,
                          llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

  ai->configuration = configuration;
  result = AI_RESULT_OK;
  return ai;

error:
  aiDestory(&ai);
  return NULL;
}

static void aiGenerate(ai_t *ai, const string_t *prompt, string_t *response) {
  debug(prompt->data);
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
    ssize_t offset = llama_token_to_piece(ai->vocabulary, token_id,
                                          parsed_token, 256, 0, false);

    if (offset < 0) {
      throw(AI_RESULT_ERROR_TOKEN_PARSING_FAILED);
    }

    if (response->used + (size_t)offset > response->length) {
      throw(AI_RESULT_ERROR_RESPONSE_LENGTH_EXCEEDED);
    }

    strFmt(response, "%s%s", response->data, parsed_token);
    batch = llama_batch_get_one(&token_id, 1);
  }

  result = AI_RESULT_OK;
error:
  deallocate(&tokens);
}

static void aiTemplatePrompt(ai_t *self, prompt_type_t type,
                             const string_t *input, string_t *templated) {
  const string_t *template = self->configuration->prompts[type];
  if (templated->length < template->used + input->used) {
    result = AI_RESULT_ERROR;
    return;
  }
  strFmt(templated, template->data, input->data);
}

void aiUserPrompt(ai_t *self, const string_t *input, string_t *response) {
  const string_t *template = self->configuration->prompts[PROMPT_TYPE_USR];
  string_t *templated = strCreate(template->used + input->used);
  aiTemplatePrompt(self, PROMPT_TYPE_USR, input, templated);

  aiGenerate(self, templated, response);
  strDestroy(&templated);
}

void aiSystemPrompt(ai_t *self, const string_t *system, const string_t *user,
                    string_t *response) {
  const string_t *sys_tpl = self->configuration->prompts[PROMPT_TYPE_SYS];
  const string_t *usr_tpl = self->configuration->prompts[PROMPT_TYPE_USR];

  string_t *user_templated = strCreate(usr_tpl->used + user->used);
  aiTemplatePrompt(self, PROMPT_TYPE_USR, user, user_templated);

  string_t *system_templated =
      strCreate(sys_tpl->used + system->used + usr_tpl->used + user->used);
  aiTemplatePrompt(self, PROMPT_TYPE_SYS, system, system_templated);

  strCat(system_templated, user_templated);

  aiGenerate(self, system_templated, response);

  strDestroy(&system_templated);
  strDestroy(&user_templated);
}

void aiDestory(ai_t **self) {
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
