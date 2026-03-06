#pragma once

#include <klibc/types.h>

#include <kernel/vfs/node.h>


typedef enum : u16 {
  VFS_MODE_OWNER_R = 0400,
  VFS_MODE_OWNER_W = 0200,
  VFS_MODE_OWNER_X = 0100,

  VFS_MODE_GROUP_R = 0040,
  VFS_MODE_GROUP_W = 0020,
  VFS_MODE_GROUP_X = 0010,

  VFS_MODE_OTHER_R = 0004,
  VFS_MODE_OTHER_W = 0002,
  VFS_MODE_OTHER_X = 0001,
} vfs_mode;

typedef struct vfs_perms vfs_perms;
struct vfs_perms {
  u32      uid;
  u32      gid;
  vfs_mode mode;
};

struct vfs_stat_desc {
  vfs_node_type type;
  vfs_perms     perms;
  u64           size;
  u64           ctime;
  u64           mtime;
  u64           atime;
};
