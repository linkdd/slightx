#pragma once

#include <klibc/mem/str.h>


[[noreturn]] extern void panic(const char *fmt, ...);


#ifdef assert
#undef assert
#endif

#define _assert_disabled(cond)                                                 \
  do {                                                                         \
    (void) (sizeof(cond));                                                     \
  } while (false)

#define _assert_enabled(cond)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                             \
      panic(                                                                   \
        "Assertion failed: %s, file %s, line %d\n",                            \
        str_literal(#cond), str_literal(__FILE__), __LINE__                    \
      );                                                                       \
    }                                                                          \
  } while (false)


#ifdef NDEBUG
#define assert(cond)         _assert_disabled(cond)
#define assert_release(cond) _assert_enabled (cond)
#else
#define assert(cond)         _assert_enabled (cond)
#define assert_release(cond) _assert_enabled (cond)
#endif
