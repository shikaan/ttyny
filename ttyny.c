#include "src/cli.h"
#include "src/fmt.h"
#include "src/lib/alloc.h"
#include "src/lib/buffers.h"
#include "src/lib/panic.h"
#include "src/master.h"
#include "src/parser.h"
#include "src/ui.h"
#include "src/utils.h"
#include "src/world/action.h"
#include "src/world/command.h"
#include "src/world/item.h"
#include "src/world/object.h"
#include "src/world/world.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef strings_t states_t;
states_t *statesCreate(size_t cap) {
  states_t *states;
  bufCreate(states_t, string_t *, states, cap);

  for (size_t i = 0; i < cap; i++) {
    bufPush(states, strCreate(256));
  }

  states->len = 0;
  return states;
}

void statesReset(states_t *self) { self->len = 0; }

string_t *statesNext(states_t *self) {
  self->len++;
  return bufAt(self, self->len - 1);
}

void statesDestroy(states_t **self) {
  if (!self || !(*self))
    return;

  for (size_t i = 0; i < (*self)->cap; i++) {
    string_t *str = bufAt(*self, i);
    strDestroy(&str);
  }

  deallocate(self);
}

int quit(string_t *response, ui_handle_t *loading, const world_t *world) {
  uiLoadingStop(&loading);
  uiFormatAndPrintEndGame(response, GAME_STATE_DEAD, world);
  return 0;
}

int main(int argc, char **argv) {
  cli_args_t cli_args;
  cliParseArgs(argc, argv, &cli_args);

  world_result_t world_result;
  world_t *world cleanup(worldDestroy) =
      worldFromJSONFile(cli_args.story_path, &world_result);
  if (!world) {
    const char *msg =
        world_result == WORLD_RESULT_INVALID_JSON
            ? "invalid story. Check the logs or run ./tools/validate "
              "<path-to-story.json> for more information."
            : "cannot open provided file. Ensure the file exists and it's "
              "readable.";
    cliPrintError(msg);
    cliPrintUsageAndExit();
  }

  string_t *input cleanup(strDestroy) = strCreate(512);
  string_t *response cleanup(strDestroy) = strCreate(4096);

  states_t *states cleanup(statesDestroy) = statesCreate(3);

  string_t *target cleanup(strDestroy) = strCreate(128);

  master_t *master cleanup(masterDestroy) = masterCreate(world);
  panicif(!master, "cannot create master");

  parser_t *parser cleanup(parserDestroy) = parserCreate();
  panicif(!parser, "cannot create parser");

  locations_t *locations cleanup(locationsDestroy) =
      locationsCreate(world->locations->cap);
  items_t *items cleanup(itemsDestroy) = itemsCreate(world->items->cap);

  uiClearScreen();
#ifdef NDEBUG
  fmtWelcomeScreen(response);
  uiPrintCommandOutput(response);
  fgetc(stdin);
  uiClearScreen();

  uiFormatAndPrintOpeningCredits(world);
  fgetc(stdin);
  uiClearScreen();
#endif
  ui_handle_t *loading = uiLoadingStart();

  masterDescribeLocation(master, world->location, response);
  uiLoadingStop(&loading);
  fmtCapitalizeWorldObjects(response, world);
  uiPrintDescription(response);

  string_t *state = statesNext(states);
  fmtLocationChange(state, world->location);
  uiPrintStateUpdates(states);

  cliPromptInit();
  game_state_t game_state;
  cli_readline_result_t readline_result;

  while (1) {
    readline_result = cliReadline(input);

    switch (readline_result) {
    case CLI_READLINE_RESULT_EMPTY:
      continue;
    case CLI_READLINE_RESULT_QUIT:
      return quit(response, loading, world);
    case CLI_READLINE_RESULT_OK:
    default:
      break;
    }

    strClear(response);
    loading = uiLoadingStart();

    operation_t operation;
    parserGetOperation(parser, &operation, input);

    if (operation.type == OPERATION_TYPE_COMMAND) {
      switch (operation.as.command) {
      case COMMAND_TYPE_HELP: {
        fmtHelp(response, world);
        uiLoadingStop(&loading);
        uiPrintCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_STATUS: {
        fmtStatus(response, world);
        uiLoadingStop(&loading);
        uiPrintCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_TLDR: {
        fmtTldr(response, world);
        uiLoadingStop(&loading);
        uiPrintCommandOutput(response);
        break;
      }
      case COMMAND_TYPE_QUIT: {
        return quit(response, loading, world);
      }
      case COMMAND_TYPE_UNKNOWN:
      case COMMAND_TYPES:
      default: {
        strFmt(response, "That's not a command I recognize...");
        uiLoadingStop(&loading);
        uiPrintError(response);
        break;
      }
      }
      continue;
    }

    // Perform some cheap validation before invoking further AI
    if (!strchr(input->data, ' ')) {
      strFmt(response, "I need more details...");
      uiLoadingStop(&loading);
      uiPrintError(response);
      continue;
    }

    action_type_t action = operation.as.action;

    // Advance turn count only for actions, not for commands
    world->turns++;

    item_t *item = NULL;
    location_t *location = NULL;

    bufClear(items, NULL);
    bufClear(locations, NULL);
    statesReset(states);

    transition_result_t trans_result;
    print_string_callback_t printCallback = uiPrintDescription;

    switch (action) {
    case ACTION_TYPE_MOVE: {
      parserExtractTarget(parser, input, world->location->exits, items,
                          &location, &item);

      if (!location) {
        strFmt(response, "You cannot go there!");
        printCallback = uiPrintError;
        break;
      }

      trans_result =
          worldExecuteTransition(world, &location->object, action, NULL, NULL);
      if (trans_result == TRANSITION_RESULT_MISSING_ITEM) {
        strFmt(response, "You need an item or a key to go there...");
        printCallback = uiPrintError;
        break;
      } else if (trans_result == TRANSITION_RESULT_INVALID_TARGET) {
        strFmt(response, "This way is locked by some contraption.");
        printCallback = uiPrintError;
        break;
      }

      world->location = location;
      masterDescribeLocation(master, location, response);
      printCallback = uiPrintDescription;

      state = statesNext(states);
      fmtLocationChange(state, world->location);
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
          printCallback = uiPrintReadable;
        } else {
          masterDescribeObject(master, &item->object, response);
          printCallback = uiPrintDescription;
        }
        break;
      }

      if (location) {
        masterDescribeObject(master, &location->object, response);
        printCallback = uiPrintDescription;
        break;
      }

      strFmt(response, "I don't understand... Can you rephrase that?");
      printCallback = uiPrintError;
      break;
    }
    case ACTION_TYPE_TAKE: {
      parserExtractTarget(parser, input, locations, world->location->items,
                          &location, &item);

      if (!item) {
        strFmt(response, "Take what? You need to be more specific than that.");
        printCallback = uiPrintError;
        break;
      }

      if (!item->collectible) {
        strFmt(response, "You cannot pick that up.");
        printCallback = uiPrintError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state = OBJECT_STATE_ANY;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);
      if (trans_result != TRANSITION_RESULT_OK &&
          trans_result != TRANSITION_RESULT_NO_TRANSITION) {
        strFmt(response, "You cannot take that.");
        printCallback = uiPrintError;
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

      printCallback = uiPrintDescription;
      state = statesNext(states);
      fmtTake(state, item);

      if (affected) {
        state = statesNext(states);
        fmtTransition(state, affected);
      }
      break;
    }
    case ACTION_TYPE_DROP: {
      parserExtractTarget(parser, input, locations, world->inventory, &location,
                          &item);

      if (!item) {
        strFmt(response, "You cannot drop something that you don't own.");
        printCallback = uiPrintError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state = OBJECT_STATE_ANY;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);
      if (trans_result != TRANSITION_RESULT_OK &&
          trans_result != TRANSITION_RESULT_NO_TRANSITION) {
        strFmt(response, "You cannot drop that.");
        printCallback = uiPrintError;
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

      printCallback = uiPrintDescription;
      state = statesNext(states);
      fmtDrop(state, item);

      if (affected) {
        state = statesNext(states);
        fmtTransition(state, affected);
      }

      break;
    }
    case ACTION_TYPE_USE: {
      bufCat(items, world->inventory);
      bufCat(items, world->location->items);
      parserExtractTarget(parser, input, locations, items, &location, &item);

      if (!item) {
        strFmt(response, "Not sure what you mean.");
        printCallback = uiPrintError;
        break;
      }

      object_t *affected = NULL;
      object_state_t affected_initial_state = OBJECT_STATE_ANY;
      trans_result = worldExecuteTransition(world, &item->object, action,
                                            &affected, &affected_initial_state);

      switch (trans_result) {
      case TRANSITION_RESULT_OK:
        masterDescribeAction(master, world, input, &item->object, affected,
                             affected_initial_state, response);
        masterForget(master, &item->object, OBJECT_NAMESPACE);
        masterForget(master, &item->object, ITEM_NAMESPACE);

        printCallback = uiPrintDescription;
        state = statesNext(states);
        fmtUse(state, item);

        if (affected) {
          state = statesNext(states);
          fmtTransition(state, affected);
        }

        break;
      case TRANSITION_RESULT_MISSING_ITEM:
        strFmt(response, "You need a utensil for that.");
        printCallback = uiPrintError;
        break;
      case TRANSITION_RESULT_INVALID_TARGET:
        strFmt(response, "Something isn't quite right for that.");
        printCallback = uiPrintError;
        break;
      case TRANSITION_RESULT_NO_TRANSITION:
      default:
        strFmt(response, "Did you mean %s? Unfortunately, it cannot be used...",
               item->object.name);
        printCallback = uiPrintError;
        break;
      }
      break;
    }
    case ACTION_TYPES:
    case ACTION_TYPE_UNKNOWN:
    default:
      strFmt(response, "Not sure how to do that...");
      printCallback = uiPrintError;
    }

    worldDigest(world, &game_state);
    if (game_state != GAME_STATE_CONTINUE) {
      masterDescribeEndGame(master, input, world, game_state, response);
      uiLoadingStop(&loading);
      uiPrintDescription(response);
      uiFormatAndPrintEndGame(response, game_state, world);
      return 0;
    }

    uiLoadingStop(&loading);
    fmtCapitalizeWorldObjects(response, world);
    printCallback(response);
    uiPrintStateUpdates(states);
  }

  return 0;
}
