#pragma once

// ANSI escape sequence constants
#define ESC_RESET "\x1b[0m"
#define ESC_DIM "\x1b[2m"
#define ESC_BOLD "\x1b[1m"
#define ESC_ITALIC "\x1b[3m"
#define ESC_UNDERLINE "\x1b[4m"

#define ESC_FG_BLACK "\x1b[30m"
#define ESC_FG_RED "\x1b[31m"
#define ESC_FG_GREEN "\x1b[32m"
#define ESC_FG_YELLOW "\x1b[33m"
#define ESC_FG_BLUE "\x1b[34m"
#define ESC_FG_MAGENTA "\x1b[35m"
#define ESC_FG_CYAN "\x1b[36m"
#define ESC_FG_WHITE "\x1b[37m"

#define ESC_BG_BLACK "\x1b[40m"
#define ESC_BG_RED "\x1b[41m"
#define ESC_BG_GREEN "\x1b[42m"
#define ESC_BG_YELLOW "\x1b[43m"
#define ESC_BG_BLUE "\x1b[44m"
#define ESC_BG_MAGENTA "\x1b[45m"
#define ESC_BG_CYAN "\x1b[46m"
#define ESC_BG_WHITE "\x1b[47m"

#define dim(ConstString) ESC_DIM ConstString ESC_RESET
#define bold(ConstString) ESC_BOLD ConstString ESC_RESET
#define italic(ConstString) ESC_ITALIC ConstString ESC_RESET
#define underline(ConstString) ESC_UNDERLINE ConstString ESC_RESET

#define fg_black(ConstString) ESC_FG_BLACK ConstString ESC_RESET
#define fg_red(ConstString) ESC_FG_RED ConstString ESC_RESET
#define fg_green(ConstString) ESC_FG_GREEN ConstString ESC_RESET
#define fg_yellow(ConstString) ESC_FG_YELLOW ConstString ESC_RESET
#define fg_blue(ConstString) ESC_FG_BLUE ConstString ESC_RESET
#define fg_magenta(ConstString) ESC_FG_MAGENTA ConstString ESC_RESET
#define fg_cyan(ConstString) ESC_FG_CYAN ConstString ESC_RESET
#define fg_white(ConstString) ESC_FG_WHITE ConstString ESC_RESET

#define bg_black(ConstString) ESC_BG_BLACK ConstString ESC_RESET
#define bg_red(ConstString) ESC_BG_RED ConstString ESC_RESET
#define bg_green(ConstString) ESC_BG_GREEN ConstString ESC_RESET
#define bg_yellow(ConstString) ESC_BG_YELLOW ConstString ESC_RESET
#define bg_blue(ConstString) ESC_BG_BLUE ConstString ESC_RESET
#define bg_magenta(ConstString) ESC_BG_MAGENTA ConstString ESC_RESET
#define bg_cyan(ConstString) ESC_BG_CYAN ConstString ESC_RESET
#define bg_white(ConstString) ESC_BG_WHITE ConstString ESC_RESET
