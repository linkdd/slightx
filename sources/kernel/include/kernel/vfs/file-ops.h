#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>
#include <klibc/mem/span.h>

#include <kernel/vfs/error.h>
#include <kernel/vfs/file.h>


typedef enum {
  VFS_SEEK_SET = 0,
  VFS_SEEK_CUR,
  VFS_SEEK_END,
} vfs_seek_whence;


struct vfs_file_ops {
  RESULT(UNIT,           vfs_err) (*close)   (vfs_file *self);
  RESULT(usize,          vfs_err) (*read)    (vfs_file *self,       span buf);
  RESULT(usize,          vfs_err) (*write)   (vfs_file *self, const_span buf);
  RESULT(u64,            vfs_err) (*seek)    (vfs_file *self, i64 offset, vfs_seek_whence whence);
  RESULT(vfs_dirent_ptr, vfs_err) (*readdir) (vfs_file *self, allocator a);
  RESULT(UNIT,           vfs_err) (*truncate)(vfs_file *self, u64 size);
  RESULT(UNIT,           vfs_err) (*flush)   (vfs_file *self);
  RESULT(u64,            vfs_err) (*ioctl)   (vfs_file *self, u64 op, va_list args);
};


RESULT(UNIT,           vfs_err) vfs_close   (vfs_file *file);
RESULT(usize,          vfs_err) vfs_read    (vfs_file *file,       span buf);
RESULT(usize,          vfs_err) vfs_write   (vfs_file *file, const_span buf);
RESULT(u64,            vfs_err) vfs_seek    (vfs_file *file, i64 offset, vfs_seek_whence whence);
RESULT(vfs_dirent_ptr, vfs_err) vfs_readdir (vfs_file *file, allocator a);
RESULT(UNIT,           vfs_err) vfs_truncate(vfs_file *file, u64 size);
RESULT(UNIT,           vfs_err) vfs_flush   (vfs_file *file);
RESULT(u64,            vfs_err) vfs_ioctl   (vfs_file *file, u64 op, ...);
