#pragma once

#include <slightx/types.h>


#define VFS_BUFFER_MAX  8192
#define VFS_PATH_MAX    4096


typedef enum {
  VFS_UNKNOWN = 1,
  VFS_ENOTFOUND,
  VFS_EEXISTS,
  VFS_EACCESS,
  VFS_EIO,
  VFS_ENOMEM,
  VFS_ENOSPC,
  VFS_EINVAL,
  VFS_EISDIR,
  VFS_ENOTDIR,
  VFS_ENOTEMPTY,
  VFS_ELOOP,
  VFS_EBUSY,
  VFS_ENOSYS,
} vfs_err;

typedef enum {
  VFS_NODETYPE_UNKNOWN = 0,
  VFS_NODETYPE_FILE,
  VFS_NODETYPE_DIRECTORY,
  VFS_NODETYPE_SYMLINK,
  VFS_NODETYPE_DEVICE,
} vfs_node_type;

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

typedef struct vfs_stat vfs_stat;
struct vfs_stat {
  vfs_node_type type;
  vfs_perms     perms;
  u64           size;
  u64           ctime;
  u64           mtime;
  u64           atime;
};

typedef enum {
  VFS_SEEK_SET = 0,
  VFS_SEEK_CUR,
  VFS_SEEK_END,
} vfs_seek_whence;

typedef u64 vfs_node;
typedef u64 vfs_file;
typedef u64 vfs_dirent;
