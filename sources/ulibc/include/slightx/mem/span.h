#pragma once

#include <slightx/types.h>
#include <slightx/mem/alloc.h>


typedef struct span span;
struct span {
  void  *data;
  usize  size;
};

typedef struct const_span const_span;
struct const_span {
  const void *data;
  usize       size;
};


#define span_null()        ((span)      { .data = NULL,     .size = 0 })
#define const_span_null()  ((const_span){ .data = NULL,     .size = 0 })
#define span_as_const(s)   ((const_span){ .data = (s).data, .size = (s).size })


span       make_span      (      void *data, usize size);
const_span make_const_span(const void *data, usize size);

span span_clone(allocator a, const_span src);
