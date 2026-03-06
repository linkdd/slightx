#pragma once

#include <klibc/types.h>
#include <klibc/mem/str.h>


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


str vfs_strerror(vfs_err status);