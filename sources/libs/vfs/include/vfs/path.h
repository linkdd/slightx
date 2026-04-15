#pragma once

#include <slightx/types.h>
#include <slightx/mem/str.h>

#include <vfs/types.h>


typedef struct vfs_path vfs_path;
struct vfs_path {
  usize length;
  char  data[VFS_PATH_MAX];
};


#define vfs_path_null() ((vfs_path){ .length = 0, .data = {0} })


str      vfs_path_as_str   (vfs_path path);
vfs_path vfs_path_normalize(vfs_path path);
