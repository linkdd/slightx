#pragma once

#include <klibc/types.h>

#include <kernel/vfs/_decls.h>


typedef enum {
  VFS_NODETYPE_UNKNOWN = 0,
  VFS_NODETYPE_FILE,
  VFS_NODETYPE_DIRECTORY,
  VFS_NODETYPE_SYMLINK,
  VFS_NODETYPE_DEVICE,
} vfs_node_type;

struct vfs_node {
  const vfs_node_ops *ops;

  vfs_node_type   type;
  vfs_mount_desc *mounted;

  void *fs_data;

  atomic_uint refcount;
  void (*release)(vfs_node *self);
};


vfs_node_ref vfs_node_incref(vfs_node_ref node);
void         vfs_node_decref(vfs_node_ref node);
