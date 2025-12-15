#pragma once

#include "lib/tty.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

#define cleanup(Callback) __attribute__((cleanup(Callback)))

#define NAMESTR "ttyny"
static const char *NAME_NO_TTY = NAMESTR;
static const char *NAME = fg_green(bold(NAMESTR));
#undef NAMESTR

#ifndef VERSION
#define VERSION "v0.0.0"
#endif

#ifndef SHA
#define SHA "dev"
#endif
