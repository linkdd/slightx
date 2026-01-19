#pragma once

#include <klibc/types.h>

#include <klibc/mem/span.h>


typedef struct bitmap bitmap;
struct bitmap {
  span buffer;
};


void bitmap_init(bitmap *self, span buffer);

bool bitmap_isset(bitmap *self, u64 bit);
void bitmap_set  (bitmap *self, u64 bit);
void bitmap_clear(bitmap *self, u64 bit);
