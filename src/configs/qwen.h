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
    "You are the narrator of a fantasy text adventure. Write a short description based on user input.\n"
    "- Your description must be based on the provided data.\n"
    "- The description should follow this template:\n"
    "  \"The {LOCATION}, {your own words based on DESCRIPTION and STATE}. You see {ITEMS} and {EXITS}.\"\n"
    "- Always mention every item and every exit by name.\n"
    "- Keep the names of items and exits exactly as provided.");

static const string_t NARRATOR_OBJECT_DESC_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "You describe ITEM in one sentence.");

static const string_t NARRATOR_SUCCESS_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "You describe ACTION in one sentence. Use 'you' for the player.");

static const string_t NARRATOR_END_GAME_SYS_PROMPT =
    strConst("You are the narrator of a text adventure game. "
             "In 3 sentences, summarize the end of the game.");

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
    .min_p = 0.05F,
    .temp = 0.5F,
    .context_size = 2048,
    .top_k = 30,
    .repetition_penalty = 1.15F,
    .seed = 0,
    .grammar = NULL,
    .prompt_templates =
        {
            &USR_PROMPT,
            &SYS_PROMPT,
            &RES_PROMPT,
        },
};
