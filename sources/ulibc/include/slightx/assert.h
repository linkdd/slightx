#pragma once

#include <slightx/sys/proc.h>
#include <slightx/sys/io.h>


#ifdef assert
#undef assert
#endif

#define _assert_stringify(x)       _assert_stringify_impl(x)
#define _assert_stringify_impl(x)  #x

#define _assert_disabled(cond)                                                 \
  do {                                                                         \
    (void) (sizeof(cond));                                                     \
  } while (false)

#define _assert_enabled(cond)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                             \
      sys_puts(str_literal(                                                    \
        "Assertion failed: " #cond ", "                                        \
        "file " __FILE__ ", line " _assert_stringify(__LINE__)                 \
        "\r\n"                                                                 \
      ));                                                                      \
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
