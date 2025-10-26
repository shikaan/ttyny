#include "assets/story.h"
#include "src/ai.h"
#include "src/buffers.h"
#include "src/narrator.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/ui.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  string_t *input = strCreate(512);
  string_t *response = strCreate(4096);
  string_t *target = strCreate(128);
  world_t *world = &troll_bridge_world;

  narrator_t *narrator = narratorCreate();
  panicif(!narrator, "cannot instantiate narrator");

  parser_t *parser = parserCreate();
  panicif(!parser, "cannot instantiate narrator");

  ui_handle_t loading = loadingStart();
  narratorDescribeWorld(narrator, world, response);
  loadingWait(loading);
  puts(response->data);

  while (1) {
    printf("> ");
    strReadFrom(input, stdin);

    strClear(response);
    loading = loadingStart();

    action_t action = parserExtractAction(parser, input);
    parserExtractTarget(parser, input, world->locations, world->items, target);

    printf("action: %d, target: %s\n", action, target->data);

    narratorDescribeWorld(narrator, &troll_bridge_world, response);
    loadingWait(loading);
    puts(response->data);
  }

  return 0;
}
