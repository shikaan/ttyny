#pragma once

#include "item.h"
#include "location.h"
#include "object.h"
#include <stdint.h>

typedef struct requirements_t {
  items_t* inventory;
  items_t* items;
  locations_t* locations;
  uint16_t turns;
} requirements_t;

typedef enum {
 REQUIREMENTS_RESULT_OK,
 REQUIREMENTS_RESULT_NO_REQUIREMENTS,
 REQUIREMENTS_RESULT_MISSING_INVENTORY_ITEM,
 REQUIREMENTS_RESULT_INVALID_INVENTORY_ITEM,
 REQUIREMENTS_RESULT_MISSING_WORLD_ITEM,
 REQUIREMENTS_RESULT_INVALID_WORLD_ITEM,
 REQUIREMENTS_RESULT_INVALID_LOCATION,
 REQUIREMENTS_RESULT_NOT_ENOUGH_TURNS,
} requirements_result_t;
