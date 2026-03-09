#pragma once

#include <klibc/types.h>
#include <klibc/mem/span.h>

#include <kernel/vfs/node.h>


typedef struct initrd initrd;
struct initrd {
  const_span buffer;
};


void initrd_load(initrd *self, const_span buffer);

vfs_node *initrd_root(initrd *self);
