#include <klibc/collections/bitmap.h>
#include <klibc/mem/bytes.h>
#include <klibc/assert.h>


void bitmap_init(bitmap *self, span buffer) {
  assert(self != NULL);

  self->buffer = buffer;

  if (buffer.data != NULL && buffer.size > 0) {
    memset(buffer.data, 0, buffer.size);
  }
}


bool bitmap_isset(bitmap *self, u64 bit) {
  assert(self != NULL);
  assert(self->buffer.data != NULL && self->buffer.size > 0);

  u8 *base = self->buffer.data;

  u64 byte_index = bit / 8;
  u8  bit_index  = bit % 8;
  u8  bitmask    = 0b10000000 >> bit_index;
  assert_release(byte_index < self->buffer.size);

  return (base[byte_index] & bitmask) == bitmask;
}


void bitmap_set(bitmap *self, u64 bit) {
  assert(self != NULL);
  assert(self->buffer.data != NULL && self->buffer.size > 0);

  u8 *base = self->buffer.data;

  u64 byte_index = bit / 8;
  u8  bit_index  = bit % 8;
  u8  bitmask    = 0b10000000 >> bit_index;
  assert_release(byte_index < self->buffer.size);

  base[byte_index] |= bitmask;
}


void bitmap_clear(bitmap *self, u64 bit) {
  assert(self != NULL);
  assert(self->buffer.data != NULL && self->buffer.size > 0);

  u8 *base = self->buffer.data;

  u64 byte_index = bit / 8;
  u8  bit_index  = bit % 8;
  u8  bitmask    = 0b10000000 >> bit_index;
  assert_release(byte_index < self->buffer.size);

  base[byte_index] &= ~bitmask;
}
