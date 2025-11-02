#pragma once

#define dim(ConstString) "\x1b[2m" ConstString "\x1b[0m"
#define bold(ConstString) "\x1b[1m" ConstString "\x1b[0m"
#define italic(ConstString) "\x1b[3m" ConstString "\x1b[0m"
#define underline(ConstString) "\x1b[4m" ConstString "\x1b[0m"

#define fg_black(ConstString) "\x1b[30m" ConstString "\x1b[0m"
#define fg_red(ConstString) "\x1b[31m" ConstString "\x1b[0m"
#define fg_green(ConstString) "\x1b[32m" ConstString "\x1b[0m"
#define fg_yellow(ConstString) "\x1b[33m" ConstString "\x1b[0m"
#define fg_blue(ConstString) "\x1b[34m" ConstString "\x1b[0m"
#define fg_magenta(ConstString) "\x1b[35m" ConstString "\x1b[0m"
#define fg_cyan(ConstString) "\x1b[36m" ConstString "\x1b[0m"
#define fg_white(ConstString) "\x1b[37m" ConstString "\x1b[0m"

#define bg_black(ConstString) "\x1b[40m" ConstString "\x1b[0m"
#define bg_red(ConstString) "\x1b[41m" ConstString "\x1b[0m"
#define bg_green(ConstString) "\x1b[42m" ConstString "\x1b[0m"
#define bg_yellow(ConstString) "\x1b[43m" ConstString "\x1b[0m"
#define bg_blue(ConstString) "\x1b[44m" ConstString "\x1b[0m"
#define bg_magenta(ConstString) "\x1b[45m" ConstString "\x1b[0m"
#define bg_cyan(ConstString) "\x1b[46m" ConstString "\x1b[0m"
#define bg_white(ConstString) "\x1b[47m" ConstString "\x1b[0m"
