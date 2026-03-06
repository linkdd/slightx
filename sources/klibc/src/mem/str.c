#include <klibc/mem/str.h>
#include <klibc/mem/bytes.h>
#include <klibc/algo/format.h>
#include <klibc/assert.h>


str strview_from_cstr(const char *s) {
  usize len = (usize)strlen(s);

  return (str){
    .data     = (char*)s,
    .length   = len,
    .capacity = len + 1,
    .owned    = false,
  };
}


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


str str_clone(allocator a, str s) {
  str copy = {
    .data     = allocate_v(a, s.length, sizeof(char)),
    .length   = s.length,
    .capacity = s.length * sizeof(char),
    .owned    = true,
  };
  memcpy(copy.data, s.data, s.length);

  return copy;
}


void str_free(allocator a, str *s) {
  assert(s != NULL);
  assert(s->owned);

  deallocate(a, s->data, s->capacity * sizeof(char));
  *s = str_null();
}


str str_slice(str s, usize start, usize length) {
  assert(start <= s.length);
  assert(start + length <= s.length);

  return (str){
    .data     = s.data + start,
    .length   = length,
    .capacity = length,
    .owned    = false,
  };
}


bool str_equal(str a, str b) {
  if (a.length != b.length) return false;
  return memcmp(a.data, b.data, a.length) == 0;
}


bool str_startswith(str s, str prefix) {
  if (prefix.length > s.length) return false;
  return memcmp(s.data, prefix.data, prefix.length) == 0;
}


OPTION(usize) str_rfind(str s, char c) {
  for (usize i = s.length; i > 0; i--) {
    if (s.data[i - 1] == c) {
      return (OPTION(usize)) SOME(i - 1);
    }
  }

  return (OPTION(usize)) NONE();
}
