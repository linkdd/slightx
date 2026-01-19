#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>


typedef struct str str;
struct str {
  char  *data;
  usize  length;
  usize  capacity;
  bool   owned;
};


#define str_char(c)     (str){ .data = &(c),   .length = 1,             .capacity = 1,             .owned = false }
#define str_literal(s)  (str){ .data = ("" s), .length = sizeof(s) - 1, .capacity = sizeof(s) - 1, .owned = false }
#define str_null()      (str){ .data = NULL,   .length = 0,             .capacity = 0,             .owned = false }


str str_format (allocator a, const char *fmt, ...);
str str_vformat(allocator a, const char *fmt, va_list args);
