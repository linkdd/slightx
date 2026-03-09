#pragma once

#include <klibc/types.h>

#include <kernel/vfs/node.h>


struct vfs_file {
  const vfs_file_ops *ops;

  vfs_node *node;
  u32       flags;
  u64       offset;
};
