#pragma once

#include <slightx/sys/proc.h>
#include <slightx/io.h>


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
      print(                                                                   \
        "Assertion failed: %s, file %s, line %d\r\n",                          \
        str_literal(#cond), str_literal(__FILE__), __LINE__                    \
      );                                                                       \
      sys_exit(127);                                                           \
    }                                                                          \
  } while (false)


#ifdef NDEBUG
#define assert(cond)         _assert_disabled(cond)
#define assert_release(cond) _assert_enabled (cond)
#else
#define assert(cond)         _assert_enabled (cond)
#define assert_release(cond) _assert_enabled (cond)
#endif
