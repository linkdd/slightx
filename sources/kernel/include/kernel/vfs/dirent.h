#pragma once

#include <klibc/types.h>

#include <kernel/vfs/node.h>


struct vfs_dirent {
  u64           node_id;
  vfs_node_type type;
  usize         namelen;
  char          name[VFS_NAME_MAX + 1];
};
