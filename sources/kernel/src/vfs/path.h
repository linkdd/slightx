#pragma once

#include <klibc/types.h>
#include <klibc/mem/str.h>

#include <kernel/vfs/_decls.h>
#include <kernel/vfs/error.h>


typedef struct vfs_node_path vfs_node_path;
struct vfs_node_path {
  vfs_node_ref node;
  str          path;
};


str vfs_normalize_path(str path, char buf[VFS_PATH_MAX]);

RESULT(vfs_node_path, vfs_err) vfs_lookup_parent(str normpath);
