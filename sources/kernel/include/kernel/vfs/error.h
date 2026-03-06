#pragma once

#include <klibc/types.h>
#include <klibc/mem/str.h>

#include <kernel/vfs/_decls.h>
#include <kernel/vfs/stat.h>


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


RESULT_DECL(UNIT,  vfs_err);
RESULT_DECL(str,   vfs_err);
RESULT_DECL(usize, vfs_err);
RESULT_DECL(u64,   vfs_err);

RESULT_DECL(vfs_node_ref,   vfs_err);
RESULT_DECL(vfs_file_ref,   vfs_err);
RESULT_DECL(vfs_stat_desc,  vfs_err);
RESULT_DECL(vfs_dirent_ptr, vfs_err);


str vfs_strerror(vfs_err status);
