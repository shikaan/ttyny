#pragma once

#include "../ai.h"
#include "../lib/buffers.h"

static const string_t PARSER_ACTION_SYS_PROMPT =
    strConst("Classify the action in one verb.");

static const string_t PARSER_TARGET_SYS_PROMPT = strConst(
    "You are a parser for a text adventure game. Your job is to determine "
    "the target of the user's command.\n"
    "Users can use synonyms or describe things differently.\n"
    "Choose the single best option that matches the user's "
    "intent. When there is no match, respond with 'unknown'.");

static const string_t MASTER_WORLD_DESC_SYS_PROMPT =
    strConst("You are the narrator of a text adventure. Write a "
             "short description based on provided data.\n"
             "- Be concise\n"
             "- You must use 'you' for the player\n"
             "- You must follow this template:\n"
             "  \"{LOCATION in your own words based on DESCRIPTION}."
             " {ITEMS you see}. {EXITS you see}.\"\n"
             "- You must preserve capitalization of items and exits.");

static const string_t MASTER_OBJECT_DESC_SYS_PROMPT =
    strConst("You are the narrator of a fantasy game. "
             "You describe ITEM in one sentence.");

static const string_t MASTER_ACTION_SYS_PROMPT =
    strConst("You are the narrator of a text adventure. Describe ACTION in one "
             "sentence.\n"
             " - Always use 'you' for the player.\n"
             " - ONLY ACTION. NO feelings. NO additional details.\n"
             " - Use the context ensure the output is coherent.\n"
             "If TRANSITION_TARGET is present, describe the transition in a "
             "second sentence.");

static const string_t MASTER_END_GAME_SYS_PROMPT = strConst(
    "You are the narrator of a text adventure game writing the final scene. "
    "In 2 sentences, describe how ACTION led to ENDING due to REASON. "
    "Use 'you' for the player. DO NOT INVENT ADDITIONAL DETAILS. Use previous "
    "output for context.");

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
    .temp = 0.55F,
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
