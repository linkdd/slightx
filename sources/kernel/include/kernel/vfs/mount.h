#pragma once

#include <klibc/types.h>
#include <klibc/mem/str.h>

#include <kernel/vfs/error.h>
#include <kernel/vfs/node.h>


struct vfs_mount_desc {
  str       path;
  vfs_node *root;
};


void vfs_mtable_init(void);

RESULT(UNIT, vfs_err) vfs_mount (str path, vfs_node *root);
RESULT(UNIT, vfs_err) vfs_umount(str path);
