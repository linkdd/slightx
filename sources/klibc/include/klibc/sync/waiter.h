#pragma once

#include <klibc/types.h>


typedef struct waiter waiter;
struct waiter {
  void (*wake) (void *udata);

  void *udata;
};
