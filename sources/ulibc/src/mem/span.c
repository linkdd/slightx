#include <slightx/mem/span.h>
#include <slightx/mem/bytes.h>
#include <slightx/assert.h>


span make_span(void *data, usize size) {
  assert(data != NULL && size > 0);

  return (span){
    .data = data,
    .size = size,
  };
}


const_span make_const_span(const void *data, usize size) {
  assert(data != NULL && size > 0);

  return (const_span){
    .data = data,
    .size = size,
  };
}


span span_clone(allocator a, const_span src) {
  assert(src.data != NULL && src.size > 0);

  span dest = make_span(allocate(a, src.size), src.size);
  memcpy(dest.data, src.data, src.size);
  return dest;
}
