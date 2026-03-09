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


str strview_from_cstr  (const char *s);
str strview_from_buffer(const u8 *buf, size_t bufsz);

str str_format (allocator a, const char *fmt, ...);
str str_vformat(allocator a, const char *fmt, va_list args);

str  str_clone(allocator a, str s);
void str_free (allocator a, str *s);

str str_slice(str s, usize start, usize length);

bool str_equal     (str a, str b);
bool str_startswith(str s, str prefix);

OPTION(usize) str_rfind(str s, char c);
