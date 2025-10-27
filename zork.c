#include "assets/story.h"
#include "src/buffers.h"
#include "src/narrator.h"
#include "src/panic.h"
#include "src/parser.h"
#include "src/ui.h"
#include "src/utils.h"
#include "src/world.h"
#include <ggml.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *target cleanup(strDestroy) = strCreate(128);
  world_t *world = &troll_bridge_world;

  narrator_t *narrator cleanup(narratorDestroy) = narratorCreate();
  panicif(!narrator, "cannot create narrator");

  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot create narrator");

  ui_handle_t loading = loadingStart();
  narratorDescribeWorld(narrator, world, response);
  loadingWait(loading);
  puts(response->data);

  while (1) {
    world->state.turns++;

    printf("> ");
    strReadFrom(input, stdin);

    strClear(response);
    loading = loadingStart();

    action_t action = parserExtractAction(parser, input);

    item_t *item = NULL;
    location_t *location = NULL;

    const locations_t *locations = NULL;
    const items_t *items = NULL;

    switch (action) {
    case ACTION_MOVE: {
      items = &NO_ITEMS;
      locations = world->current_location->exits;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!location) {
        narratorCommentFailure(narrator, FAILURE_INVALID_TARGET, input,
                               response);
        goto print;
      }

      world->current_location = location;
      narratorDescribeWorld(narrator, &troll_bridge_world, response);
      strFmtAppend(response, "\n~> Location: %s", location->object.name);
      goto print;
    }
    case ACTION_EXAMINE: {
      // TODO: how to examine the inventory?
      items = world->current_location->items;
      locations = world->current_location->exits;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (item) {
        narratorDescribeObject(narrator, &item->object, response);
        goto print;
      }

      if (location) {
        narratorDescribeObject(narrator, &location->object, response);
        goto print;
      }

      narratorCommentFailure(narrator, FAILURE_INVALID_TARGET, input, response);
      goto print;
    }
    case ACTION_TAKE: {
      items = world->current_location->items;
      locations = &NO_LOCATIONS;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_INVALID_ITEM, input, response);
        goto print;
      }

      itemsAdd(world->state.inventory, item);
      itemsRemove(world->current_location->items, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "\n~> %s taken", item->object.name);
      goto print;
    }
    case ACTION_DROP: {
      items = world->state.inventory;
      locations = &NO_LOCATIONS;
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        narratorCommentFailure(narrator, FAILURE_INVALID_ITEM, input, response);
        goto print;
      }

      itemsAdd(world->current_location->items, item);
      itemsRemove(world->state.inventory, item);
      narratorCommentSuccess(narrator, world, input, response);
      strFmtAppend(response, "\n~> %s dropped", item->object.name);
      goto print;
    }
    case ACTION_USE:
    case ACTIONS:
    case ACTION_UNKNOWN:
    default:
      strFmt(response, "Not sure how to do that...");
      goto print;
    }

  print:
    loadingWait(loading);
    puts(response->data);

    switch (world->digest(&world->state)) {
    case GAME_STATE_VICTORY:
      puts("You won!");
      return EXIT_SUCCESS;
    case GAME_STATE_DEAD:
      puts("You're dead!");
      return EXIT_FAILURE;
    case GAME_STATE_CONTINUE:
    default:
      break;
    };
  }

  return 0;
}
