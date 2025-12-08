#include "../src/master.h"
#include "../src/lib/buffers.h"
#include "../src/utils.h"
#include "fixtures/world.h"
#include "stat.h"
#include "test.h"
#include "timers.h"
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

void describeLocation(void) {
  string_t *buffer cleanup(strDestroy) = strCreate(1024);
  master_t *master cleanup(masterDestroy) = masterCreate(world);

  uint64_t samples[SAMPLE_SIZE] = {};

  info("doing %d measurements\n", SAMPLE_SIZE);
  for (size_t i = 0; i < SAMPLE_SIZE; i++) {
    location_t *room =
        (location_t *)world->locations->data[i % world->locations->len];
    strClear(buffer);
    uint64_t elapsed = readTimer();
    masterDescribeLocation(master, room, buffer);
    elapsed = readTimer() - elapsed;
    debug("Attempt #%lu duration: %f\n", i + 1,
          (double)elapsed / (double)MICROSECONDS);
    samples[i] = elapsed;
    masterForget(master, &room->object, LOCATION_NAMESPACE);

    if (((i + 1) % 10) == 0) {
      info("snapshot at %lu", i + 1);
      info("  average: %fs", avgllu(i, samples) / (double)MICROSECONDS);
      info("   median: %fs",
           (double)percllu(50, i, samples) / (double)MICROSECONDS);
      info("      p90: %fs",
           (double)percllu(90, i, samples) / (double)MICROSECONDS);
      info("      max: %fs",
           (double)percllu(100, i, samples) / (double)MICROSECONDS);
    }
  }

  double average = avgllu(SAMPLE_SIZE, samples) / (double)MICROSECONDS;
  double median =
      (double)percllu(50, SAMPLE_SIZE, samples) / (double)MICROSECONDS;
  double p90 = (double)percllu(90, SAMPLE_SIZE, samples) / (double)MICROSECONDS;

  expectLtf(average, 2.0, "average generation time is < 2s");
  expectLtf(median, 2.0, "median generation time is < 2s");
  expectLtf(p90, 3.0, "p90 generation time is < 3s");
}

int main(void) {
  suite(describeLocation);
  return report();
}
