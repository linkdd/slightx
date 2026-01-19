#include <klibc/mem/str.h>
#include <klibc/mem/bytes.h>
#include <klibc/algo/format.h>
#include <klibc/assert.h>


str str_format(allocator a, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  str s = str_vformat(a, fmt, ap);
  va_end(ap);

  return s;
}


static void len_formatter(void *udata, str chunk) {
  usize *sz = (usize *)udata;
  assert(sz != NULL);

  *sz += chunk.length;
}


static void copy_formatter(void *udata, str chunk) {
  str *dest = (str *)udata;
  assert(dest != NULL);

  assert_release((dest->capacity - dest->length) >= chunk.length);

  memcpy(dest->data + dest->length, chunk.data, chunk.length);
  dest->length += chunk.length;
}


str str_vformat(allocator a, const char *fmt, va_list args) {
  usize     len  = 0;
  formatter lenf = {
    .consume = len_formatter,
    .udata   = &len,
  };
  va_list len_args;
  va_copy(len_args, args);
  formatter_apply_v(&lenf, fmt, len_args);
  va_end(len_args);

  str s = {
    .data     = allocate_v(a, len, sizeof(char)),
    .length   = len,
    .capacity = len * sizeof(char),
    .owned    = true,
  };
  formatter copyf = {
    .consume = copy_formatter,
    .udata   = &s,
  };
  formatter_apply_v(&copyf, fmt, args);

  return s;
}
