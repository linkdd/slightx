#include <klibc/mem/align.h>


bool is_alignment_valid(usize alignment) {
  return (alignment & (alignment - 1)) == 0;
}


bool is_size_aligned(usize size, usize alignment) {
  return (size % alignment) == 0;
}


usize align_size_up(usize size, usize alignment) {
  if (size % alignment != 0) {
    size += alignment - (size % alignment);
  }

  return size;
}


usize align_size_down(usize size, usize alignment) {
  if (size % alignment != 0) {
    size -= size % alignment;
  }

  return size;
}


bool is_ptr_aligned(uptr ptr, usize alignment) {
  return (ptr % alignment) == 0;
}


uptr align_ptr_up(uptr ptr, usize alignment) {
  if (ptr % alignment != 0) {
    ptr += alignment - (ptr % alignment);
  }

  return ptr;
}


uptr align_ptr_down(uptr ptr, usize alignment) {
  if (ptr % alignment != 0) {
    ptr -= ptr % alignment;
  }

  return ptr;
}
