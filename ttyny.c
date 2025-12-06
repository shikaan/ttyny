#include "src/lib/buffers.h"
#include "src/lib/panic.h"
#include "src/master.h"
#include "src/parser.h"
#include "src/screen.h"
#include "src/utils.h"
#include "src/world/action.h"
#include "src/world/command.h"
#include "src/world/item.h"
#include "src/world/object.h"
#include "src/world/world.h"
#include <ggml.h>
#include <linenoise.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*print_callback_t)(string_t *);

void completion(const char *buf, linenoiseCompletions *lc) {
  if (buf[0] == '/') {
    for (size_t i = 0; i < COMMAND_TYPES; i++) {
      if (strncmp(buf, command_names[i]->data, strlen(buf)) == 0) {
        linenoiseAddCompletion(lc, command_names[i]->data);
      }
    }
  }
}

int quit(string_t *response, ui_handle_t *loading, const world_t *world) {
  loadingStop(&loading);
  printEndGame(response, GAME_STATE_DEAD, world);
  return 0;
}

void usage(const char *name) {
  fprintf(stderr,
          "%s is a small-language-model-powered game engine to play text "
          "adventure games in your terminal.\n"
          "Usage:\n"
          "  %s <path-to-story.json>\n"
          "\n"
          "Flags:\n"
          "  -h, --help      show this help\n"
          "  -v, --version   show version\n"
          "\n"
          "For more information https://github.com/shikaan/%s\n",
          name, name, name);
  exit(1);
}

int main(int argc, char **argv) {
  const char *name = strrchr(argv[0], '/') + 1;
  if (argc != 2) {
    usage(name);
  }
  const char *story_path = argv[1];

  world_t *world cleanup(worldDestroy) = worldFromJSONFile(story_path);
  if (!world) {
    fprintf(stderr, "%s: ", name);
    perror(story_path);
    usage(name);
  }

  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);
  string_t *state cleanup(strDestroy) = strCreate(1024);
  string_t *target cleanup(strDestroy) = strCreate(128);

  master_t *master cleanup(masterDestroy) = masterCreate(world);
  panicif(!master, "cannot create master");

  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot create parser");

  locations_t *locations cleanup(locationsDestroy) =
      locationsCreate(world->locations->length);
  items_t *items cleanup(itemsDestroy) = itemsCreate(world->items->length);

  screenClear();
#ifndef DEBUG
  formatWelcomeScreen(response);
  printCommandOutput(response);
  fgetc(stdin);
  screenClear();

  printOpeningCredits(world);
  fgetc(stdin);
  screenClear();
#endif
  ui_handle_t *loading = loadingStart();

  masterDescribeLocation(master, world->location, response);
  loadingStop(&loading);
  printDescription(response);

  formatLocationChange(response, world->location);
  printStateUpdate(response);

  linenoiseHistorySetMaxLen(15);
  linenoiseSetCompletionCallback(completion);
  game_state_t game_state;

  while (1) {
    char *line = linenoise("> ");

    // This is invoked on Ctrl+C/D
    if (!line) {
      return quit(response, loading, world);
    }

    strFmt(input, "%s", line);
    linenoiseFree(line);

    if (bufIsEmpty(input))
      continue;

    linenoiseHistoryAdd(input->data);

    strClear(response);
    loading = loadingStart();

    operation_t operation;
    parserGetOperation(parser, &operation, input);

    if (operation.type == OPERATION_TYPE_COMMAND) {
      switch (operation.as.command) {
      case COMMAND_TYPE_HELP: {
        formatHelp(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_STATUS: {
        formatStatus(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_TLDR: {
        formatTldr(response, world);
        loadingStop(&loading);
        printCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_QUIT: {
        return quit(response, loading, world);
      }
      case COMMAND_TYPE_UNKNOWN:
      case COMMAND_TYPES:
      default: {
        strFmt(response, "That's not a command I recognize...");
        loadingStop(&loading);
        printError(response);
        break;
      }
      }
      continue;
    }

    // Perform some cheap validation before invoking further AI
    if (!strchr(input->data, ' ')) {
      strFmt(response, "I need more details...");
      loadingStop(&loading);
      printError(response);
      continue;
    }

    action_type_t action = operation.as.action;

    // Advance turn count only for actions, not for commands
    world->turns++;

    item_t *item = NULL;
    location_t *location = NULL;

    bufClear(items, NULL);
    bufClear(locations, NULL);
    strClear(state);

    transition_result_t trans_result;
    print_callback_t printCallback = printDescription;

    switch (action) {
    case ACTION_TYPE_MOVE: {
      parserExtractTarget(parser, input, world->location->exits, items,
                          &location, &item);

      if (!location) {
        strFmt(response, "You cannot go there!");
        printCallback = printError;
        break;
      }

      trans_result =
          worldExecuteTransition(world, &location->object, action, NULL, NULL);
      if (trans_result == TRANSITION_RESULT_MISSING_ITEM) {
        strFmt(response, "You need an item or a key to go there...");
        printCallback = printError;
        break;
      } else if (trans_result == TRANSITION_RESULT_INVALID_TARGET) {
        strFmt(response, "This way is locked by some contraption.");
        printCallback = printError;
        break;
      }

      world->location = location;
      masterDescribeLocation(master, location, response);
      printCallback = printDescription;
      formatLocationChange(state, world->location);
      break;
    }
    case ACTION_TYPE_EXAMINE: {
      bufCat(items, world->location->items);
      bufCat(items, world->inventory);
      parserExtractTarget(parser, input, world->location->exits, items,
                          &location, &item);

      if (item) {
        // This is non-functional transition. No need to check result
        trans_result =
            worldExecuteTransition(world, &item->object, action, NULL, NULL);

        if (item->readable) {
          masterReadItem(master, item, response);
          printCallback = printReadable;
        } else {
          masterDescribeObject(master, &item->object, response);
          printCallback = printDescription;
        }
        break;
      }

      if (location) {
        masterDescribeObject(master, &location->object, response);
        printCallback = printDescription;
        break;
      }

      strFmt(response, "I don't understand... Can you rephrase that?");
      printCallback = printError;
      break;
    }
    case ACTION_TYPE_TAKE: {
      parserExtractTarget(parser, input, locations, world->location->items,
                          &location, &item);

      if (!item) {
        strFmt(response, "Take what? You need to be more specific than that.");
        printCallback = printError;
        break;
      }

      if (!item->collectible) {
        strFmt(response, "You cannot pick that up.");
        printCallback = printError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);
      if (trans_result != TRANSITION_RESULT_OK &&
          trans_result != TRANSITION_RESULT_NO_TRANSITION) {
        strFmt(response, "You cannot take that.");
        printCallback = printError;
        break;
      }

      bufPush(world->inventory, item);
      bufRemove(world->location->items, item, NULL);

      masterDescribeAction(master, world, input, &item->object, affected,
                           affected_initial_state, response);
      // When taking an object, the room description must be regenerated
      // else it'll mention objects you have in the inventory
      masterForget(master, &world->location->object, LOCATION_NAMESPACE);
      masterForget(master, &world->location->object, OBJECT_NAMESPACE);

      printCallback = printDescription;
      formatTake(state, item);
      break;
    }
    case ACTION_TYPE_DROP: {
      parserExtractTarget(parser, input, locations, world->inventory, &location,
                          &item);

      if (!item) {
        strFmt(response, "You cannot drop something that you don't own.");
        printCallback = printError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);
      if (trans_result != TRANSITION_RESULT_OK &&
          trans_result != TRANSITION_RESULT_NO_TRANSITION) {
        strFmt(response, "You cannot drop that.");
        printCallback = printError;
        break;
      }

      bufPush(world->location->items, item);
      bufRemove(world->inventory, item, NULL);

      masterDescribeAction(master, world, input, &item->object, affected,
                           affected_initial_state, response);
      // When dropping an object, the room description must be regenerated
      // else it won't mention the object just dropped
      masterForget(master, &world->location->object, LOCATION_NAMESPACE);
      masterForget(master, &world->location->object, OBJECT_NAMESPACE);

      printCallback = printDescription;
      formatDrop(state, item);

      break;
    }
    case ACTION_TYPE_USE: {
      bufCat(items, world->inventory);
      bufCat(items, world->location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you mean.");
        printCallback = printError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);

      switch (trans_result) {
      case TRANSITION_RESULT_OK:
        masterDescribeAction(master, world, input, &item->object, affected,
                             affected_initial_state, response);
        masterForget(master, &item->object, OBJECT_NAMESPACE);
        masterForget(master, &item->object, ITEM_NAMESPACE);

        printCallback = printDescription;
        formatUse(state, item);
        break;
      case TRANSITION_RESULT_MISSING_ITEM:
        strFmt(response, "You need a utensil for that.");
        printCallback = printError;
        break;
      case TRANSITION_RESULT_INVALID_TARGET:
        strFmt(response, "Something isn't quite right for that.");
        printCallback = printError;
        break;
      case TRANSITION_RESULT_NO_TRANSITION:
      default:
        strFmt(response, "Did you mean %s? Unfortunately, it cannot be used...",
               item->object.name);
        printCallback = printError;
        break;
      }
      break;
    }
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      strFmt(response, "Not sure how to do that...");
      printCallback = printError;
    }

    worldDigest(world, &game_state);
    if (game_state != GAME_STATE_CONTINUE) {
      masterDescribeEndGame(master, input, world, game_state, response);
      loadingStop(&loading);
      printDescription(response);
      printEndGame(response, game_state, world);
      return 0;
    }

    loadingStop(&loading);
    printCallback(response);
    if (!bufIsEmpty(state)) {
      printStateUpdate(state);
    }
  }

  return 0;
}
