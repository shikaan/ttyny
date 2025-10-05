#include "ggml.h"
#include <llama.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODEL_PATH "./models/TinyMistral-248M-v2.5-Instruct.Q8_0.gguf"
#define PROMPT_MAX 1024
#define RESPONSE_MAX 4096

void error(const char *string) {
  fprintf(stderr, "error: %s\n", string);
  exit(EXIT_FAILURE);
}

void filterLogs(enum ggml_log_level level, const char *text, void *data) {
  (void)data;
  if (level >= GGML_LOG_LEVEL_ERROR) {
    fprintf(stderr, "%s", text);
  }
}

void generate(const char *prompt, char *response, size_t response_length,
              const struct llama_vocab *vocab, struct llama_context *ctx,
              struct llama_sampler *smpl) {
  // (void)response_length;
  // (void)response;
  int response_offset = 0;
  const int size = (int)strlen(prompt);

  const bool is_first =
      llama_memory_seq_pos_max(llama_get_memory(ctx), 0) == -1;

  const int tok_count =
      -llama_tokenize(vocab, prompt, size, NULL, 0, is_first, true);

  llama_token *tokens = malloc(sizeof(llama_token) * (size_t)tok_count);

  if (llama_tokenize(vocab, prompt, size, tokens, tok_count, is_first, true) <
      0) {
    return error("tokenization failed");
  }

  llama_batch batch = llama_batch_get_one(tokens, tok_count);
  llama_token token_id;

  while (true) {
    uint32_t n_ctx = llama_n_ctx(ctx);
    llama_pos n_ctx_used =
        llama_memory_seq_pos_max(llama_get_memory(ctx), 0) + 1;
    if (n_ctx_used + batch.n_tokens > (int)n_ctx) {
      return error("context size exceeded");
    }

    if (llama_decode(ctx, batch) != 0) {
      return error("failed to decode");
    };

    // sample the next token
    token_id = llama_sampler_sample(smpl, ctx, -1);

    // is it an end of generation?
    if (llama_vocab_is_eog(vocab, token_id)) {
      break;
    }

    // convert the token to a string, print it and add it to the response
    char buf[256] = {};
    int n = llama_token_to_piece(vocab, token_id, buf, sizeof(buf), 0, false);
    if (n < 0) {
      return error("failed to convert token to piece");
    }

    response_offset +=
        snprintf(response + response_offset, response_length, "%s", buf);

    // prepare the next batch with the sampled token
    batch = llama_batch_get_one(&token_id, 1);
  }
}

int main(void) {
  llama_log_set(filterLogs, NULL);

  ggml_backend_load_all();
  struct llama_model_params params = llama_model_default_params();
  params.n_gpu_layers = 99;

  struct llama_model *model = llama_model_load_from_file(MODEL_PATH, params);
  if (!model) {
    error("failed to load model");
    return 1;
  }

  const struct llama_vocab *vocab = llama_model_get_vocab(model);

  struct llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = 2048;
  ctx_params.n_batch = 2048;

  struct llama_context *ctx = llama_init_from_model(model, ctx_params);
  if (!ctx) {
    fprintf(stderr, "%s: error: failed to create the llama_context\n",
            __func__);
    return 1;
  }

  struct llama_sampler *smpl =
      llama_sampler_chain_init(llama_sampler_chain_default_params());
  llama_sampler_chain_add(smpl, llama_sampler_init_min_p(0.05f, 1));
  llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.2f));
  llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

  char prompt[] = "What's the capital of Italy?";
  char response[RESPONSE_MAX] = {};
  generate(prompt, response, RESPONSE_MAX, vocab, ctx, smpl);

  puts(response);

  return 0;
}
