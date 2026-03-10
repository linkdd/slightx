#pragma once

#include <klibc/types.h>

#include <klibc/mem/str.h>


typedef struct args args;
struct args {
  str rdinit;
};

RESULT_DECL(args, UNIT);


RESULT(args, UNIT) args_parse(str cmdline);
