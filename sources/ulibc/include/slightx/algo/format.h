#pragma once

#include <slightx/types.h>
#include <slightx/mem/str.h>


typedef struct formatter formatter;
struct formatter {
  void (*consume)(void *udata, str chunk);

  void *udata;
};


void formatter_apply  (formatter *self, const char *fmt, ...);
void formatter_apply_v(formatter *self, const char *fmt, va_list args);
