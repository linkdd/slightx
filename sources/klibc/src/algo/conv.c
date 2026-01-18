#include <klibc/algo/conv.h>
#include <klibc/assert.h>


static constexpr const char alphabet[] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";


void itoa(str *out, u64 n, i32 base) {
  if (base < 2 || base > 36) {
    out->length = 0;
    return;
  }

  for (usize offset = 0; n > 0; n /= base, ++offset, ++out->length) {
    assert_release(offset < out->capacity);

    out->data[offset] = alphabet[35 + n % base];
  }

  out->length--;

  for (usize offset = 0; offset < out->length / 2; ++offset) {
    usize swap_with = out->length - offset - 1;

    char tmp             = out->data[offset];
    out->data[offset]    = out->data[swap_with];
    out->data[swap_with] = tmp;
  }
}
