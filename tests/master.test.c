#include "../src/master.h"
#include "../src/buffers.h"
#include "../src/utils.h"
#include "fixtures/world.h"
#include "test.h"
#include "timers.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>

void expectLtf(const double a, const double b, const char *name) {
  char msg[256];
  snprintf(msg, 256, "Expected %f < %f", a, b);
  expect(a < b, name, msg);
}

// This test file is meant to test how effective a given prompt is by running
// instructions repeatedly and see how long it takes for it to generate a valid
// input

enum { SAMPLE_SIZE = 100, MICROSECONDS = 10000000 };

double_t avg(size_t size, uint64_t samples[SAMPLE_SIZE]) {
  uint64_t sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += (samples)[i];
  }

  return (double)sum / (double)size;
}

void describeLocation(void) {
  string_t *buffer cleanup(strDestroy) = strCreate(1024);
  master_t *master cleanup(masterDestroy) = masterCreate(world);

  uint64_t samples[SAMPLE_SIZE] = {};

  info("doing %d measurements\n", SAMPLE_SIZE);
  for (size_t i = 0; i < SAMPLE_SIZE; i++) {
    location_t *room =
        (location_t *)world->locations->data[i % world->locations->length];
    strClear(buffer);
    uint64_t elapsed = readTimer();
    masterDescribeLocation(master, room, buffer);
    elapsed = readTimer() - elapsed;
    debug("Attempt #%lu duration: %f\n", i + 1,
          (double)elapsed / (double)MICROSECONDS);
    samples[i] = elapsed;
    masterForget(master, &room->object);

    if (i != 0 && (i % 10) == 0) {
      info("current average: %fs\n", avg(i, samples) / (double)MICROSECONDS);
    }
  }

  expectLtf(avg(SAMPLE_SIZE, samples) / (double)MICROSECONDS, 5.0,
            "average generation time is < 5s");
}

void describeEndgame(void) {
  string_t *buffer cleanup(strDestroy) = strCreate(1024);
  string_t *input cleanup(strDestroy) = strCreate(128);
  master_t *master cleanup(masterDestroy) = masterCreate(world);

  uint64_t samples[SAMPLE_SIZE] = {};

  struct {
    const char *ending;
    const char *input;
    game_state_t state;
  } end_game[] = {
      {"poisoned to death", "use the apple", GAME_STATE_DEAD},
      {"lighthouse beacon restored", "use the beacon", GAME_STATE_VICTORY},
      {"museum exhibition assembled", "use the artifact", GAME_STATE_VICTORY},
      {"sealed laboratory breached", "use the door", GAME_STATE_DEAD},
      {"volatile phial ruptured", "drop the phial", GAME_STATE_DEAD},
      {"drowned in the ocean", "move to the water", GAME_STATE_DEAD},
      {"fell from the cliff", "move to the edge", GAME_STATE_DEAD},
      {"crushed by falling rocks", "move to the cave", GAME_STATE_DEAD},
      {"burned in the fire", "examine the flame", GAME_STATE_DEAD},
      {"frozen to death", "move outside", GAME_STATE_DEAD},
      {"treasure discovered", "use the chest", GAME_STATE_VICTORY},
      {"ancient curse lifted", "examine the scroll", GAME_STATE_VICTORY},
      {"island rescued", "use the signal", GAME_STATE_VICTORY},
      {"devoured by sea creature", "move to the depths", GAME_STATE_DEAD},
      {"trapped in collapsed tunnel", "use the boulder", GAME_STATE_DEAD},
      {"infected by strange spores", "examine the mushroom", GAME_STATE_DEAD},
      {"electrocuted by machinery", "use the lever", GAME_STATE_DEAD},
      {"lost in the fog forever", "move into the mist", GAME_STATE_DEAD},
      {"starved to death", "drop the food", GAME_STATE_DEAD},
      {"ship arrives for rescue", "use the flare", GAME_STATE_VICTORY},
      {"poisoned by toxic fumes", "examine the vapor", GAME_STATE_DEAD},
      {"bitten by venomous snake", "take the serpent", GAME_STATE_DEAD},
      {"ancient artifact secured", "take the relic", GAME_STATE_VICTORY},
      {"gateway sealed successfully", "use the portal", GAME_STATE_VICTORY},
      {"consumed by darkness", "drop the light", GAME_STATE_DEAD},
      {"mind controlled by entity", "examine the mirror", GAME_STATE_DEAD},
      {"blown off the tower", "move to the parapet", GAME_STATE_DEAD},
      {"mystery of the island solved", "examine the map", GAME_STATE_VICTORY},
      {"mauled by wild beast", "move to the creature", GAME_STATE_DEAD},
      {"impaled on ancient trap", "move to the chamber", GAME_STATE_DEAD},
      {"survivors united", "use the horn", GAME_STATE_VICTORY},
      {"dissolved in acid pool", "use the liquid", GAME_STATE_DEAD},
      {"strangled by vines", "examine the plants", GAME_STATE_DEAD},
      {"knowledge preserved", "examine the inscriptions", GAME_STATE_VICTORY},
      {"spirit finally laid to rest", "use the bones", GAME_STATE_VICTORY},
      {"suffocated in sealed room", "use the hatch", GAME_STATE_DEAD},
      {"struck by lightning", "take the metal rod", GAME_STATE_DEAD},
      {"community saved from plague", "use the cure", GAME_STATE_VICTORY},
      {"torn apart by machinery", "use the gears", GAME_STATE_DEAD},
      {"petrified by gaze", "examine the statue", GAME_STATE_DEAD},
      {"ancient library restored", "use the books", GAME_STATE_VICTORY},
      {"bled out from wounds", "move to the guardian", GAME_STATE_DEAD},
      {"possessed by malevolent force", "use the amulet", GAME_STATE_DEAD},
      {"navigation system repaired", "use the compass", GAME_STATE_VICTORY},
      {"buried alive in sand", "examine the hole", GAME_STATE_DEAD},
      {"radiation exposure", "move to the reactor", GAME_STATE_DEAD},
      {"harmony restored to island", "use the flute", GAME_STATE_VICTORY},
      {"attacked by swarm", "examine the nest", GAME_STATE_DEAD},
      {"drowned in quicksand", "move to the pit", GAME_STATE_DEAD},
      {"legacy documented for posterity", "use the journal",
       GAME_STATE_VICTORY},
  };

  // end game always comes with a pre-described room
  masterDescribeLocation(master, world->location, buffer);
  strClear(buffer); // we don't really care about this description though

  size_t num_scenarios = arrLen(end_game);
  info("doing %d measurements across %zu scenarios\n", SAMPLE_SIZE,
       num_scenarios);

  for (size_t i = 0; i < SAMPLE_SIZE; i++) {
    size_t scenario_idx = i % num_scenarios;
    world->end_game = strdup(end_game[scenario_idx].ending);
    game_state_t state = end_game[scenario_idx].state;
    strFmt(input, "%s", end_game[scenario_idx].input);

    strClear(buffer);
    uint64_t elapsed = readTimer();
    masterDescribeEndGame(master, input, world, state, buffer);
    elapsed = readTimer() - elapsed;

    debug("Attempt #%lu duration: %f\n", i + 1,
          (double)elapsed / (double)MICROSECONDS);
    samples[i] = elapsed;
    debug("RESPONSE (end: %s, input: %s, state: %s)\n > %s\n---\n",
          world->end_game, end_game[scenario_idx].input,
          state == GAME_STATE_VICTORY ? "victory" : "dead", buffer->data);

    if (i != 0 && (i % 10) == 0) {
      info("current average: %fs\n", avg(i, samples) / (double)MICROSECONDS);
    }
    deallocate(&world->end_game);
  }

  expectLtf(avg(SAMPLE_SIZE, samples) / (double)MICROSECONDS, 5.0,
            "average generation time is < 5s");
}

int main(void) {
  suite(describeLocation);
  // FIXME: this test is a bit weird.
  //   Timing is not really an issue here, but I have no good way to measure
  //   quality of the output. Having a list of outputs to look at is still
  //   valuable
  suite(describeEndgame);
  return report();
}
