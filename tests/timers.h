#pragma once

#include <mach/mach_time.h>
#include <stdint.h>

// TODO: make this support at least linux for CI
uint64_t readTimer(void) { return mach_continuous_time(); }
