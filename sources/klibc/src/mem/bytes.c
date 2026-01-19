#include <klibc/mem/bytes.h>
#include <klibc/types.h>
#include <klibc/assert.h>


void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
  assert(dst != NULL);
  assert(src != NULL);

  u8       *d = (u8 *)dst;
  const u8 *s = (const u8 *)src;

  for (usize i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dst;
}


void *memmove(void *dst, const void *src, size_t n) {
  assert(dst != NULL);
  assert(src != NULL);

  u8       *d = (u8 *)dst;
  const u8 *s = (const u8 *)src;

  if (d < s) {
    for (usize i = 0; i < n; i++) {
      d[i] = s[i];
    }
  }
  else if (d > s) {
    for (usize i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }

  return dst;
}


void *memset(void *dst, int c, size_t n) {
  assert(dst != NULL);

  u8 *d = (u8 *)dst;
  u8  b = (u8)c;

  for (usize i = 0; i < n; i++) {
    d[i] = b;
  }

  return dst;
}


int memcmp(const void *a, const void *b, size_t n) {
  assert(a != NULL);
  assert(b != NULL);

  const u8 *p = (const u8 *)a;
  const u8 *q = (const u8 *)b;

  for (usize i = 0; i < n; i++) {
    if (p[i] < q[i]) {
      return -1;
    }
    else if (p[i] > q[i]) {
      return 1;
    }
  }

  return 0;
}


int strlen(const char *s) {
  assert(s != NULL);

  int count = 0;
  while (*s++ != 0) count ++;

  return count;
}
