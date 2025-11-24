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

#define SAMPLE_SIZE 100
const int MICROSECONDS = 10000000;

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

int main(void) {
  suite(describeLocation);
  return report();
}
