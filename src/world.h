#pragma once

#include "buffers.h"
#include <stdint.h>

// Boolean map of traits for objects and location.
// The traits are fixed. Interactions in the game change the state (last 2 bits)
// according to the transactions
//
// OBJECT (76543210)
// 0     = collectible
// 1     = change damage
// 2     = change health
// 3     = ephemeral
// 4     = ...
// 5     = ...
// (6-7) = 4 possible states
//
// LOCATION (76543210)
// 0     = ...
// 1     = ...
// 2     = ...
// 3     = ...
// 4     = ...
// 5     = ...
// (6-7) = 4 possible states
typedef uint8_t traits_t;

typedef uint8_t state_t;

// 4 Transitions from one state to the next.
// 0010 1011 1101 0100 means 0 -> 2, 2 -> 3, 3 -> 1, 1 -> 0
typedef uint16_t transitions_t;

// TODO: ADD NPC

typedef struct {
  // Name of the object
  const char *name;
  // Description of the objects
  const char *description;
  // Human-readable state descriptions
  const char *states[4];
  // Object traits as per above
  traits_t traits;
  // In case the object changes damage or health, see by how much
  int8_t value;
  // Transitions from one state to the next
  transitions_t transitions;
} object_t;

typedef Buffer(object_t *) objects_t;

struct location_t; // Forward declaration
typedef Buffer(struct location_t *) locations_t;

typedef struct {
  // Name of the locations
  const char *name;
  // Description of the locations
  const char *description;
  // Descriptions of the location. One description per state (see traits).
  const char *states[4];
  // Objects to be found in this location
  objects_t *objects;
  // Exits from this location into other locations
  locations_t *exits;
  // Location traits
  traits_t traits;
  // Transitions from one state to the next
  transitions_t transitions;
} location_t;

typedef struct {
  // How many turns since the game has started
  uint16_t turns;
  // Health of our character
  uint8_t health;
  // How much damage does the character deal
  uint8_t damage;
  // Objects carried by the character
  objects_t *inventory;
} world_state_t;

typedef enum {
  GAME_STATE_CONTINUE = 0,
  GAME_STATE_VICTORY,
  GAME_STATE_DEAD,
} game_state_t;

// Returns game state based on current world state
typedef game_state_t (*digest_t)(world_state_t *);

typedef struct {
  // Introduction to the settings
  const char *context;
  // State of the world
  world_state_t state;
  // Locations of the world
  locations_t *locations;
  // Objects in the world
  objects_t *objects;
  // Pointer to the location where the adventure starts
  location_t *current_location;
  // Callback to call on each round to see if the game is over
  digest_t digest;
} world_t;

static inline state_t getState(traits_t traits) {
  return (traits & 0b11000000) >> 6;
}

// Executes a state transition
static inline traits_t transition(traits_t traits, transitions_t transitions) {
  state_t current_state = getState(traits);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t entry = (transitions >> (i * 4)) & 0xF;
    uint8_t from = (entry >> 2) & 0x3;
    if (from == current_state) {
      uint8_t to = (uint8_t)((entry & 0x3) << 6);
      return (traits & 0b00111111) | to;
    }
  }
  return traits;
}
