#include <kernel/vfs/main.h>

#include "mtable.h"


void vfs_init(void) {
  vfs_mtable_init();
}
