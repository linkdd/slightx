#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <stdarg.h>


typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     f32;
typedef double    f64;

typedef ptrdiff_t isize;
typedef size_t    usize;

typedef uintptr_t uptr;


typedef struct UNIT {} UNIT;

#define OPTION(T)       struct option_##T
#define OPTION_DECL(T)  OPTION(T) { bool is_some; union { T some; UNIT none; }; }
#define SOME(V)         { .is_some = true,  .some = V }
#define NONE()          { .is_some = false            }

#define RESULT(T, E)        struct result_##T##_##E
#define RESULT_DECL(T, E)   RESULT(T, E) { bool is_ok; union { T ok; E err;  }; }
#define OK(V)               { .is_ok = true,  .ok  = V }
#define ERR(V)              { .is_ok = false, .err = V }


OPTION_DECL(u8);
OPTION_DECL(u16);
OPTION_DECL(u32);
OPTION_DECL(u64);

OPTION_DECL(i8);
OPTION_DECL(i16);
OPTION_DECL(i32);
OPTION_DECL(i64);

OPTION_DECL(f32);
OPTION_DECL(f64);

OPTION_DECL(isize);
OPTION_DECL(usize);

OPTION_DECL(uptr);

OPTION_DECL(UNIT);
