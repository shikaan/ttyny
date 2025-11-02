#pragma once

#include "../ai.h"
#include "../buffers.h"
#include <time.h>

static const string_t PARSER_ACTION_SYS_PROMPT =
    strConst("Classify the action in one verb.");

static const string_t PARSER_TARGET_SYS_PROMPT = strConst(
    "You are a parser for a text adventure game. Your job is to determine "
    "the target of the user's command.\n"
    "Users can use synonyms or describe things differently.\n"
    "Choose the single best option that matches the users's "
    "intent. When there is no match, respond with 'unknown'.");

static const string_t NARRATOR_WORLD_DESC_SYS_PROMPT = strConst(
    "You are the narrator of an adventure game. In exactly 2 "
    "sentences, describe the LOCATION to the player. Say what ITEMS "
    "are here and where the EXITS lead. Ignore INVENTORY. Be concise.");

static const string_t NARRATOR_OBJECT_DESC_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "You describe ITEM in one sentence.");

static const string_t NARRATOR_FAILURE_SYS_PROMPT = strConst(
    "You are the witty, sarcastic narrator of a classic text adventure. "
    "In one sentence, You describe FAILURE of ACTION with a dry sense of "
    "humor.");

static const string_t NARRATOR_SUCCESS_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "You describe ACTION in one sentence. Use 'you' for the player.");

static const string_t NARRATOR_END_GAME_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "In 2 sentences, describe ENDGAME for the player. Be encouraging. "
             "Use 'you'.");

static string_t RES_PROMPT = strConst("<|im_end|>\n<|im_start|>assistant\n%s");
static string_t USR_PROMPT = strConst("<|im_end|>\n<|im_start|>user\n%s");
static string_t SYS_PROMPT = strConst("<|im_start|>system\n%s");

static config_t PARSER_CONFIG = {
    .path = "./models/qwen2.5-1.5b-instruct-q4_k_m.gguf",
    .min_p = 0,
    .temp = 0,
    .context_size = 2048,
    .top_k = 1,
    .repetition_penalty = 1.0F,
    .seed = 0xFFFFFFFF,
    .grammar = NULL,
    .prompt_templates =
        {
            &USR_PROMPT,
            &SYS_PROMPT,
            &RES_PROMPT,
        },
};

static config_t NARRATOR_CONFIG = {
    .path = "./models/qwen2.5-1.5b-instruct-q4_k_m.gguf",
    .min_p = 0.1F,
    .temp = 0.8F,
    .context_size = 2048,
    .top_k = 50,
    .repetition_penalty = 1.3F,
    .seed = 0,
    .grammar = NULL,
    .prompt_templates =
        {
            &USR_PROMPT,
            &SYS_PROMPT,
            &RES_PROMPT,
        },
};
