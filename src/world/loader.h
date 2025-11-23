#pragma once

// !!!
// THIS FILE IS AI GENERATED
// It's here to test whether using JSON as a format for stories yields better
// stories in the LLMs
// !!!

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../vendor/jsmn.h"
#include "action.h"
#include "world.h"

// ----------------------------- Configuration --------------------------------

#ifndef LOADER_DEFAULT_INITIAL_INVENTORY_CAPACITY
#define LOADER_DEFAULT_INITIAL_INVENTORY_CAPACITY 16
#endif

// ----------------------------- Utilities ------------------------------------

static int loaderStrEqTok(const char *json, jsmntok_t *tok, const char *s) {
  size_t len = (size_t)(tok->end - tok->start);
  return strlen(s) == len && strncmp(json + tok->start, s, len) == 0;
}

static long loaderParseLong(const char *json, jsmntok_t *tok) {
  char buf[64];
  size_t len = (size_t)(tok->end - tok->start);
  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;
  memcpy(buf, json + tok->start, len);
  buf[len] = '\0';
  return strtol(buf, NULL, 10);
}

// Recursively skip a token subtree; returns index of next sibling token.
static int loaderSkipToken(jsmntok_t *tokens, int index) {
  jsmntok_t *t = &tokens[index];
  int children = 0;
  if (t->type == JSMN_OBJECT) {
    children = t->size * 2; // key + value per pair
  } else if (t->type == JSMN_ARRAY) {
    children = t->size;
  }
  int i = index + 1;
  for (int c = 0; c < children; c++) {
    i = loaderSkipToken(tokens, i);
  }
  return i;
}

// Find value token index for key inside object token.
static int loaderFindObjectKey(const char *json, jsmntok_t *tokens,
                               int objIndex, const char *key) {
  jsmntok_t *obj = &tokens[objIndex];
  if (obj->type != JSMN_OBJECT)
    return -1;
  int i = objIndex + 1;
  for (int pair = 0; pair < obj->size; pair++) {
    jsmntok_t *k = &tokens[i];
    int valIndex = i + 1;
    if (k->type == JSMN_STRING && loaderStrEqTok(json, k, key)) {
      return valIndex;
    }
    // Skip value subtree
    i = loaderSkipToken(tokens, valIndex);
  }
  return -1;
}

static char *loaderDupUnescaped(const char *src, size_t len) {
  char *dst = allocate(len + 1);
  if (!dst) return NULL;
  size_t di = 0;
  for (size_t si = 0; si < len; si++) {
    if (src[si] == '\\' && si + 1 < len) {
      char c = src[si + 1];
      switch (c) {
        case 'n': dst[di++] = '\n'; si++; continue;
        case 't': dst[di++] = '\t'; si++; continue;
        case 'r': dst[di++] = '\r'; si++; continue;
        case '\\': dst[di++] = '\\'; si++; continue;
        case '"': dst[di++] = '"'; si++; continue;
        default:
          // Unknown escape: keep the backslash literal
          dst[di++] = src[si];
          continue;
      }
    }
    dst[di++] = src[si];
  }
  dst[di] = '\0';
  return dst;
}

// ----------------------------- Action Mapping -------------------------------

static action_type_t loaderActionFromString(const char *start, size_t len) {
#define streq(Action)                                                          \
  len == Action.used &&strncmp(start, Action.data, Action.used) == 0

  if (streq(ACTION_USE))
    return ACTION_TYPE_USE;
  if (streq(ACTION_MOVE))
    return ACTION_TYPE_MOVE;
  if (streq(ACTION_TAKE))
    return ACTION_TYPE_TAKE;
  if (streq(ACTION_DROP))
    return ACTION_TYPE_DROP;
  if (streq(ACTION_EXAMINE))
    return ACTION_TYPE_EXAMINE;
  return ACTION_TYPE_UNKNOWN;
#undef streq
}

// ----------------------------- Lookup Helpers -------------------------------

static item_t *loaderFindItemByName(items_t *items, const char *name) {
  for (size_t i = 0; i < items->used; i++) {
    item_t *it = bufAt(items, i);
    if (strcmp(it->object.name, name) == 0)
      return it;
  }
  return NULL;
}

static location_t *loaderFindLocationByName(locations_t *locations,
                                            const char *name) {
  for (size_t i = 0; i < locations->used; i++) {
    location_t *loc = bufAt(locations, i);
    if (strcmp(loc->object.name, name) == 0)
      return loc;
  }
  return NULL;
}

// ----------------------------- Requirements ---------------------------------

static requirements_t *
loaderParseEndingRequirements(const char *json, jsmntok_t *tokens,
                              int reqObjIndex, items_t *worldItems,
                              locations_t *worldLocations) {
  if (reqObjIndex < 0) {
    // Create dummy (no requirements).
    return requirementsCreate();
  }
  jsmntok_t *reqObj = &tokens[reqObjIndex];
  if (reqObj->type != JSMN_OBJECT) {
    return requirementsCreate();
  }

  requirements_t *req = requirementsCreate();
  if (!req)
    return NULL;

  int invIdx = loaderFindObjectKey(json, tokens, reqObjIndex, "inventory");
  int itemsIdx = loaderFindObjectKey(json, tokens, reqObjIndex, "items");
  int locsIdx = loaderFindObjectKey(json, tokens, reqObjIndex, "locations");
  int turnsIdx = loaderFindObjectKey(json, tokens, reqObjIndex, "turns");

  // Inventory (schema: array of "name" or "name.state" strings ->
  // requirement_tuples_t)
  if (invIdx >= 0 && tokens[invIdx].type == JSMN_ARRAY) {
    jsmntok_t *arr = &tokens[invIdx];
    if (arr->size > 0) {
      requirement_tuples_t *buf = requirementTuplesCreate((size_t)arr->size);
      if (!buf) {
        deallocate(&req);
        return NULL;
      }
      int idx = invIdx + 1;
      size_t used = 0;
      for (int i = 0; i < arr->size; i++) {
        jsmntok_t *elem = &tokens[idx];
        if (elem->type == JSMN_STRING) {
          size_t len = (size_t)(elem->end - elem->start);
          char *raw = strndup(json + elem->start, len);
          if (raw) {
            // Parse optional ".state"
            object_state_t desired_state = 0;
            char *dot = strrchr(raw, '.');
            if (dot) {
              char *endptr = NULL;
              long state_val = strtol(dot + 1, &endptr, 10);
              if (endptr && *endptr == '\0' && state_val >= 0 &&
                  state_val <= 255) {
                *dot = '\0';
                desired_state = (object_state_t)state_val;
              }
            }
            item_t *worldItem = loaderFindItemByName(worldItems, raw);
            if (!worldItem) {
              fprintf(stderr,
                      "loader: requirement references unknown item '%s'\n",
                      raw);
              deallocate(&raw);
            } else {
              // Reuse world item name pointer (stable, not freed here)
              requirement_tuple_t tuple = {.name = worldItem->object.name,
                                           .state = desired_state};
              bufSet(buf, used, tuple);
              used++;
              deallocate(&raw);
            }
          }
        }
        idx = loaderSkipToken(tokens, idx);
      }
      buf->used = used;
      req->inventory = buf;
    }
  }

  // Items (world state requirements)
  if (itemsIdx >= 0 && tokens[itemsIdx].type == JSMN_ARRAY) {
    jsmntok_t *arr = &tokens[itemsIdx];
    if (arr->size > 0) {
      requirement_tuples_t *buf = requirementTuplesCreate((size_t)arr->size);
      if (!buf) {
        deallocate(&req);
        return NULL;
      }
      int idx = itemsIdx + 1;
      size_t used = 0;
      for (int i = 0; i < arr->size; i++) {
        jsmntok_t *elem = &tokens[idx];
        if (elem->type == JSMN_STRING) {
          size_t len = (size_t)(elem->end - elem->start);
          char *raw = strndup(json + elem->start, len);
          if (raw) {
            object_state_t desired_state = 0;
            char *dot = strrchr(raw, '.');
            if (dot) {
              char *endptr = NULL;
              long state_val = strtol(dot + 1, &endptr, 10);
              if (endptr && *endptr == '\0' && state_val >= 0 &&
                  state_val <= 255) {
                *dot = '\0';
                desired_state = (object_state_t)state_val;
              }
            }
            item_t *worldItem = loaderFindItemByName(worldItems, raw);
            if (!worldItem) {
              fprintf(
                  stderr,
                  "loader: requirement references unknown world item '%s'\n",
                  raw);
              deallocate(&raw);
            } else {
              requirement_tuple_t tuple = {.name = worldItem->object.name,
                                           .state = desired_state};
              bufSet(buf, used, tuple);
              used++;
              deallocate(&raw);
            }
          }
        }
        idx = loaderSkipToken(tokens, idx);
      }
      buf->used = used;
      req->items = buf;
    }
  }

  // Locations
  if (locsIdx >= 0 && tokens[locsIdx].type == JSMN_ARRAY) {
    jsmntok_t *arr = &tokens[locsIdx];
    if (arr->size > 0) {
      requirement_tuples_t *buf = requirementTuplesCreate((size_t)arr->size);
      if (!buf) {
        deallocate(&req);
        return NULL;
      }
      int idx = locsIdx + 1;
      size_t used = 0;
      for (int i = 0; i < arr->size; i++) {
        jsmntok_t *elem = &tokens[idx];
        if (elem->type == JSMN_STRING) {
          size_t len = (size_t)(elem->end - elem->start);
          char *raw = strndup(json + elem->start, len);
          if (raw) {
            object_state_t desired_state = 0;
            char *dot = strrchr(raw, '.');
            if (dot) {
              char *endptr = NULL;
              long state_val = strtol(dot + 1, &endptr, 10);
              if (endptr && *endptr == '\0' && state_val >= 0 &&
                  state_val <= 255) {
                *dot = '\0';
                desired_state = (object_state_t)state_val;
              }
            }
            location_t *worldLoc =
                loaderFindLocationByName(worldLocations, raw);
            if (!worldLoc) {
              fprintf(stderr,
                      "loader: requirement references unknown location '%s'\n",
                      raw);
              deallocate(&raw);
            } else {
              requirement_tuple_t tuple = {.name = worldLoc->object.name,
                                           .state = desired_state};
              bufSet(buf, used, tuple);
              used++;
              deallocate(&raw);
            }
          }
        }
        idx = loaderSkipToken(tokens, idx);
      }
      buf->used = used;
      req->locations = buf;
    }
  }

  // Turns
  if (turnsIdx >= 0 && (tokens[turnsIdx].type == JSMN_PRIMITIVE ||
                        tokens[turnsIdx].type == JSMN_STRING)) {
    req->turns = (uint16_t)loaderParseLong(json, &tokens[turnsIdx]);
  }

  return req;
}

// ----------------------------- Transitions ----------------------------------

static transitions_t *loaderExpandTransitions(const char *json,
                                              jsmntok_t *tokens, int arrIndex) {
  if (arrIndex < 0) {
    // Provide empty transitions buffer
  }
  transitions_t *result = NULL;
  int count = 0;

  if (arrIndex >= 0 && tokens[arrIndex].type == JSMN_ARRAY) {
    jsmntok_t *arr = &tokens[arrIndex];
    // Worst-case expansion: each entry may have up to 5 actions.
    count = arr->size * 5;
  } else {
    count = 0;
  }

  bufCreate(transitions_t, transition_t, result, count > 0 ? (size_t)count : 1);
  if (!result)
    return NULL;

  if (arrIndex < 0 || tokens[arrIndex].type != JSMN_ARRAY) {
    // No transitions
    return result;
  }

  int idx = arrIndex + 1;
  size_t used = 0;
  jsmntok_t *arr = &tokens[arrIndex];
  for (int i = 0; i < arr->size; i++) {
    jsmntok_t *entry = &tokens[idx];
    if (entry->type == JSMN_OBJECT) {
      int actionsIdx = loaderFindObjectKey(json, tokens, idx, "actions");
      int fromIdx = loaderFindObjectKey(json, tokens, idx, "from");
      int toIdx = loaderFindObjectKey(json, tokens, idx, "to");
      // requirements present but ignored -> create dummy
      requirements_t *req = requirementsCreate();
      if (!req) {
        deallocate(&result);
        return NULL;
      }

      long fromVal =
          (fromIdx >= 0) ? loaderParseLong(json, &tokens[fromIdx]) : 0;
      long toVal = (toIdx >= 0) ? loaderParseLong(json, &tokens[toIdx]) : 0;

      if (actionsIdx >= 0 && tokens[actionsIdx].type == JSMN_ARRAY) {
        int aIdx = actionsIdx + 1;
        jsmntok_t *aArr = &tokens[actionsIdx];
        for (int ai = 0; ai < aArr->size; ai++) {
          jsmntok_t *actionTok = &tokens[aIdx];
          if (actionTok->type == JSMN_STRING) {
            action_type_t at = loaderActionFromString(
                json + actionTok->start,
                (size_t)(actionTok->end - actionTok->start));
            if (at != ACTION_TYPE_UNKNOWN) {
              transition_t t = {
                  .action = at,
                  .from = (object_state_t)fromVal,
                  .to = (object_state_t)toVal,
                  .requirements = req,
              };
              if (used < result->length) {
                bufSet(result, used, t);
                used++;
              }
            }
          }
          aIdx = loaderSkipToken(tokens, aIdx);
        }
      } else {
        // No actions -> still create a single transition with UNKNOWN (ignored
        // by world)
        transition_t t = {
            .action = ACTION_TYPE_UNKNOWN,
            .from = (object_state_t)fromVal,
            .to = (object_state_t)toVal,
            .requirements = req,
        };
        bufSet(result, used, t);
        used++;
      }
    }
    idx = loaderSkipToken(tokens, idx);
  }
  result->used = used;
  return result;
}

// ----------------------------- Descriptions ---------------------------------

static descriptions_t *
loaderParseDescriptions(const char *json, jsmntok_t *tokens, int arrIndex) {
  if (arrIndex < 0 || tokens[arrIndex].type != JSMN_ARRAY) {
    // Create a single empty description to avoid NULL pointers.
    descriptions_t *d = NULL;
    bufCreate(descriptions_t, const char *, d, 1);
    char *empty = {0};
    if (d) {
      bufSet(d, 0, empty);
      d->used = 1;
    }
    return d;
  }

  jsmntok_t *arr = &tokens[arrIndex];
  descriptions_t *d = NULL;
  bufCreate(descriptions_t, const char *, d,
            (size_t)arr->size > 0 ? (size_t)arr->size : 1);
  if (!d)
    return NULL;

  int idx = arrIndex + 1;
  size_t used = 0;
  for (int i = 0; i < arr->size; i++) {
    jsmntok_t *elem = &tokens[idx];
    if (elem->type == JSMN_STRING) {
      char *text =
          loaderDupUnescaped(json + elem->start, (size_t)(elem->end - elem->start));
      if (!text) {
        deallocate(&d);
        return NULL;
      }
      bufSet(d, used, text);
      used++;
    }
    idx = loaderSkipToken(tokens, idx);
  }
  d->used = used;
  return d;
}

// ----------------------------- Items ----------------------------------------

static items_t *loaderParseItems(const char *json, jsmntok_t *tokens,
                                 int arrIndex) {
  if (arrIndex < 0 || tokens[arrIndex].type != JSMN_ARRAY) {
    items_t *items = itemsCreate(1);
    return items;
  }

  jsmntok_t *arr = &tokens[arrIndex];
  items_t *items = itemsCreate((size_t)arr->size > 0 ? (size_t)arr->size : 1);
  if (!items)
    return NULL;

  int idx = arrIndex + 1;
  for (int i = 0; i < arr->size; i++) {
    jsmntok_t *objTok = &tokens[idx];
    if (objTok->type == JSMN_OBJECT) {
      int nameIdx = loaderFindObjectKey(json, tokens, idx, "name");
      int typeIdx = loaderFindObjectKey(json, tokens, idx, "type");
      int descIdx = loaderFindObjectKey(json, tokens, idx, "descriptions");
      int transIdx = loaderFindObjectKey(json, tokens, idx, "transitions");
      int collectibleIdx =
          loaderFindObjectKey(json, tokens, idx, "collectible");
      int readableIdx = loaderFindObjectKey(json, tokens, idx, "readable");

      if (nameIdx < 0 || typeIdx < 0) {
        fprintf(stderr, "loader: item missing mandatory fields\n");
        idx = loaderSkipToken(tokens, idx);
        continue;
      }

      // Validate type == "item"
      if (!loaderStrEqTok(json, &tokens[typeIdx], "item")) {
        fprintf(stderr, "loader: item type must be 'item'\n");
      }

      char *name =
          strndup(json + tokens[nameIdx].start,
                  (size_t)(tokens[nameIdx].end - tokens[nameIdx].start));
      if (!name) {
        idx = loaderSkipToken(tokens, idx);
        break;
      }

      descriptions_t *descriptions =
          loaderParseDescriptions(json, tokens, descIdx);
      transitions_t *transitions =
          loaderExpandTransitions(json, tokens, transIdx);

      bool collectible = false;
      bool readable = false;

      if (collectibleIdx >= 0) {
        collectible = (loaderStrEqTok(json, &tokens[collectibleIdx], "true") ||
                       loaderStrEqTok(json, &tokens[collectibleIdx], "True") ||
                       loaderStrEqTok(json, &tokens[collectibleIdx], "1"));
      }
      if (readableIdx >= 0) {
        readable = (loaderStrEqTok(json, &tokens[readableIdx], "true") ||
                    loaderStrEqTok(json, &tokens[readableIdx], "True") ||
                    loaderStrEqTok(json, &tokens[readableIdx], "1"));
      }

      item_t *item = allocate(sizeof(item_t));
      if (!item) {
        deallocate(&name);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }
      item->object.name = name;
      item->object.type = OBJECT_TYPE_ITEM;
      item->object.state = 0;
      item->object.descriptions = descriptions;
      item->object.transitions = transitions;
      item->collectible = collectible;
      item->readable = readable;

      bufPush(items, item);
    }
    idx = loaderSkipToken(tokens, idx);
  }
  return items;
}

// ----------------------------- Locations ------------------------------------

typedef struct {
  locations_t *locations;
} loader_locations_intermediate_t;

static locations_t *loaderParseLocations(const char *json, jsmntok_t *tokens,
                                         int arrIndex, items_t *worldItems) {
  if (arrIndex < 0 || tokens[arrIndex].type != JSMN_ARRAY) {
    locations_t *locations = locationsCreate(1);
    return locations;
  }

  jsmntok_t *arr = &tokens[arrIndex];
  locations_t *locations =
      locationsCreate((size_t)arr->size > 0 ? (size_t)arr->size : 1);
  if (!locations)
    return NULL;

  int idx = arrIndex + 1;
  for (int i = 0; i < arr->size; i++) {
    jsmntok_t *objTok = &tokens[idx];
    if (objTok->type == JSMN_OBJECT) {
      int nameIdx = loaderFindObjectKey(json, tokens, idx, "name");
      int typeIdx = loaderFindObjectKey(json, tokens, idx, "type");
      int descIdx = loaderFindObjectKey(json, tokens, idx, "descriptions");
      int transIdx = loaderFindObjectKey(json, tokens, idx, "transitions");
      int itemsIdx = loaderFindObjectKey(json, tokens, idx, "items");
      int exitsIdx = loaderFindObjectKey(json, tokens, idx, "exits");

      if (nameIdx < 0 || typeIdx < 0) {
        fprintf(stderr, "loader: location missing mandatory fields\n");
        idx = loaderSkipToken(tokens, idx);
        continue;
      }

      if (!loaderStrEqTok(json, &tokens[typeIdx], "location")) {
        fprintf(stderr, "loader: location type must be 'location'\n");
      }

      char *name =
          strndup(json + tokens[nameIdx].start,
                  (size_t)(tokens[nameIdx].end - tokens[nameIdx].start));
      if (!name) {
        idx = loaderSkipToken(tokens, idx);
        break;
      }

      descriptions_t *descriptions =
          loaderParseDescriptions(json, tokens, descIdx);
      transitions_t *transitions =
          loaderExpandTransitions(json, tokens, transIdx);

      // Items
      items_t *locItems = itemsCreate(8);
      if (!locItems) {
        deallocate(&name);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }
      if (itemsIdx >= 0 && tokens[itemsIdx].type == JSMN_ARRAY) {
        int itIdx = itemsIdx + 1;
        jsmntok_t *itArr = &tokens[itemsIdx];
        for (int j = 0; j < itArr->size; j++) {
          jsmntok_t *itmTok = &tokens[itIdx];
          if (itmTok->type == JSMN_STRING) {
            char *iname = strndup(json + itmTok->start,
                                  (size_t)(itmTok->end - itmTok->start));
            if (iname) {
              item_t *worldItem = loaderFindItemByName(worldItems, iname);
              if (!worldItem) {
                fprintf(stderr,
                        "loader: location '%s' references unknown item '%s'\n",
                        name, iname);
              } else {
                bufPush(locItems, worldItem);
              }
              deallocate(&iname);
            }
          }
          itIdx = loaderSkipToken(tokens, itIdx);
        }
      }

      // Exits (names resolved later after all locations parsed)
      locations_t *exits = locationsCreate(8);
      if (!exits) {
        deallocate(&name);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }
      if (exitsIdx >= 0 && tokens[exitsIdx].type == JSMN_ARRAY) {
        int exIdx = exitsIdx + 1;
        jsmntok_t *exArr = &tokens[exitsIdx];
        for (int j = 0; j < exArr->size; j++) {
          jsmntok_t *locNameTok = &tokens[exIdx];
          if (locNameTok->type == JSMN_STRING) {
            char *lname =
                strndup(json + locNameTok->start,
                        (size_t)(locNameTok->end - locNameTok->start));
            if (lname) {
              // Temporarily store NULL; we'll resolve after all locations are
              // created.
              location_t *dummy = allocate(sizeof(location_t));
              if (!dummy) {
                deallocate(&lname);
              } else {
                dummy->object.name =
                    lname; // Use name only for later resolution.
                dummy->object.type = OBJECT_TYPE_LOCATION;
                dummy->object.state = 0;
                dummy->object.descriptions = NULL;
                dummy->object.transitions = NULL;
                dummy->items = NULL;
                dummy->exits = NULL;
                bufPush(exits, dummy);
              }
            }
          }
          exIdx = loaderSkipToken(tokens, exIdx);
        }
      }

      location_t *location = allocate(sizeof(location_t));
      if (!location) {
        deallocate(&name);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }
      location->object.name = name;
      location->object.type = OBJECT_TYPE_LOCATION;
      location->object.state = 0;
      location->object.descriptions = descriptions;
      location->object.transitions = transitions;
      location->items = locItems;
      location->exits = exits;

      bufPush(locations, location);
    }
    idx = loaderSkipToken(tokens, idx);
  }

  // Resolve exits: each dummy location pointer replaced by pointer to real
  // location.
  for (size_t i = 0; i < locations->used; i++) {
    location_t *loc = bufAt(locations, i);
    locations_t *exits = loc->exits;
    for (size_t j = 0; j < exits->used; j++) {
      location_t *dummy = bufAt(exits, j);
      const char *name = dummy->object.name;
      location_t *real = loaderFindLocationByName(locations, name);
      if (!real) {
        fprintf(stderr, "loader: unresolved exit '%s' in location '%s'\n", name,
                loc->object.name);
      }
      // Replace pointer
      bufSet(exits, j, real);
      // Free dummy name only (struct & name)
      // deallocate(&dummy->object.name);
      deallocate(&dummy);
    }
  }

  return locations;
}

// ----------------------------- Endings --------------------------------------

static endings_t *loaderParseEndings(const char *json, jsmntok_t *tokens,
                                     int arrIndex, items_t *worldItems,
                                     locations_t *worldLocations) {
  endings_t *endings = NULL;

  if (arrIndex < 0 || tokens[arrIndex].type != JSMN_ARRAY) {
    bufCreate(endings_t, ending_t *, endings, 1);
    if (endings)
      endings->used = 0;
    return endings;
  }

  jsmntok_t *arr = &tokens[arrIndex];
  bufCreate(endings_t, ending_t *, endings,
            (size_t)arr->size > 0 ? (size_t)arr->size : 1);
  if (!endings)
    return NULL;

  int idx = arrIndex + 1;
  size_t used = 0;
  for (int i = 0; i < arr->size; i++) {
    jsmntok_t *objTok = &tokens[idx];
    if (objTok->type == JSMN_OBJECT) {
      int stateIdx = loaderFindObjectKey(json, tokens, idx, "state");
      int reasonIdx = loaderFindObjectKey(json, tokens, idx, "reason");
      int reqIdx = loaderFindObjectKey(json, tokens, idx, "requirements");

      if (stateIdx < 0 || reasonIdx < 0 || reqIdx < 0) {
        fprintf(stderr, "loader: ending missing mandatory fields\n");
        idx = loaderSkipToken(tokens, idx);
        continue;
      }

      bool success = loaderStrEqTok(json, &tokens[stateIdx], "win");
      char *reason =
          strndup(json + tokens[reasonIdx].start,
                  (size_t)(tokens[reasonIdx].end - tokens[reasonIdx].start));
      if (!reason) {
        idx = loaderSkipToken(tokens, idx);
        continue;
      }

      requirements_t *requirements = loaderParseEndingRequirements(
          json, tokens, reqIdx, worldItems, worldLocations);
      if (!requirements) {
        deallocate(&reason);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }

      ending_t *ending = allocate(sizeof(ending_t));
      if (!ending) {
        deallocate(&reason);
        deallocate(&requirements);
        idx = loaderSkipToken(tokens, idx);
        continue;
      }
      ending->success = success;
      ending->reason = reason;
      ending->requirements = requirements;

      bufSet(endings, used, ending);
      used++;
    }
    idx = loaderSkipToken(tokens, idx);
  }
  endings->used = used;
  return endings;
}

// ----------------------------- World Construction ---------------------------

static world_t *loaderBuildWorld(items_t *items, locations_t *locations,
                                 endings_t *endings) {
  world_t *world = allocate(sizeof(world_t));
  if (!world)
    return NULL;

  items_t *inventory = itemsCreate(LOADER_DEFAULT_INITIAL_INVENTORY_CAPACITY);
  if (!inventory) {
    deallocate(&world);
    return NULL;
  }

  world->items = items;
  world->locations = locations;
  world->endings = endings;
  world->turns = 0;
  world->inventory = inventory;
  world->location =
      (locations && locations->used > 0) ? bufAt(locations, 0) : NULL;
  world->end_game = NULL;

  return world;
}

// ----------------------------- Cleanup (failure only) -----------------------

// Minimal cleanup to avoid leaking on fatal error.
// We don't export a destructor per instructions; used internally on failure.
static void loaderDestroyWorld(world_t **worldPtr) {
  if (!worldPtr || !*worldPtr)
    return;
  world_t *w = *worldPtr;

  deallocate(&w);
  *worldPtr = NULL;
}

// ----------------------------- File Reading ---------------------------------

static char *loaderReadFile(const char *path, size_t *outLen) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "loader: cannot open file '%s'\n", path);
    return NULL;
  }
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    fprintf(stderr, "loader: fseek failed\n");
    return NULL;
  }
  long size = ftell(fp);
  if (size < 0) {
    fclose(fp);
    fprintf(stderr, "loader: ftell failed\n");
    return NULL;
  }
  rewind(fp);
  char *data = allocate((size_t)size + 1);
  if (!data) {
    fclose(fp);
    return NULL;
  }
  size_t read = fread(data, 1, (size_t)size, fp);
  fclose(fp);
  if (read != (size_t)size) {
    fprintf(stderr, "loader: fread truncated\n");
    deallocate(&data);
    return NULL;
  }
  data[size] = '\0';
  if (outLen)
    *outLen = (size_t)size;
  return data;
}

// ----------------------------- Parsing Root ---------------------------------

static world_t *loaderParseJson(const char *json, size_t length) {
  // Tokenization (grow tokens until success)
  jsmn_parser parser;
  jsmn_init(&parser);

  unsigned int tokensCapacity = 4096;
  jsmntok_t *tokens = allocate(sizeof(jsmntok_t) * (size_t)tokensCapacity);
  if (!tokens)
    return NULL;

  int r;
  while ((r = jsmn_parse(&parser, json, length, tokens, tokensCapacity)) ==
         JSMN_ERROR_NOMEM) {
    tokensCapacity *= 2;
    jsmntok_t *newTokens = allocate(sizeof(jsmntok_t) * (size_t)tokensCapacity);
    if (!newTokens) {
      deallocate(&tokens);
      return NULL;
    }
    memcpy(newTokens, tokens, sizeof(jsmntok_t) * (size_t)(tokensCapacity / 2));
    deallocate(&tokens);
    tokens = newTokens;
    jsmn_init(&parser);
  }

  if (r < 0) {
    fprintf(stderr, "loader: JSON parse error %d\n", r);
    deallocate(&tokens);
    return NULL;
  }

  int tokenCount = r;
  if (tokenCount < 1 || tokens[0].type != JSMN_OBJECT) {
    fprintf(stderr, "loader: root JSON must be an object\n");
    deallocate(&tokens);
    return NULL;
  }

  int root = 0;
  int itemsIdx = loaderFindObjectKey(json, tokens, root, "items");
  int locationsIdx = loaderFindObjectKey(json, tokens, root, "locations");
  int endingsIdx = loaderFindObjectKey(json, tokens, root, "endings");

  if (itemsIdx < 0 || locationsIdx < 0 || endingsIdx < 0) {
    fprintf(stderr,
            "loader: missing top-level fields (items/locations/endings)\n");
    deallocate(&tokens);
    return NULL;
  }

  items_t *items = loaderParseItems(json, tokens, itemsIdx);
  if (!items) {
    deallocate(&tokens);
    return NULL;
  }

  locations_t *locations =
      loaderParseLocations(json, tokens, locationsIdx, items);
  if (!locations) {
    // free items
    world_t *tmp = loaderBuildWorld(items, NULL, NULL);
    loaderDestroyWorld(&tmp);
    deallocate(&tokens);
    return NULL;
  }

  endings_t *endings =
      loaderParseEndings(json, tokens, endingsIdx, items, locations);
  if (!endings) {
    world_t *tmp = loaderBuildWorld(items, locations, NULL);
    loaderDestroyWorld(&tmp);
    deallocate(&tokens);
    return NULL;
  }

  world_t *world = loaderBuildWorld(items, locations, endings);
  if (!world) {
    world_t *tmp = loaderBuildWorld(items, locations, endings);
    loaderDestroyWorld(&tmp);
    deallocate(&tokens);
    return NULL;
  }

  deallocate(&tokens);
  return world;
}

// ----------------------------- Public API -----------------------------------

world_t *worldCreateFromFile(const char *path) {
  size_t len = 0;
  char *json = loaderReadFile(path, &len);
  if (!json)
    return NULL;

  world_t *world = loaderParseJson(json, len);

  deallocate(&json);
  return world;
}
