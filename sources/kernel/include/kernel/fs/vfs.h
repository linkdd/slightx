#pragma once

#include <klibc/types.h>

#include <klibc/mem/alloc.h>
#include <klibc/mem/str.h>
#include <klibc/mem/span.h>


#define VFS_NAME_MAX          255
#define VFS_PATH_MAX          4096
#define VFS_SYMLINK_DEPTH_MAX 16

#define VFS_O_READ       (1u << 0)
#define VFS_O_WRITE      (1u << 1)
#define VFS_O_CREATE     (1u << 2)
#define VFS_O_TRUNC      (1u << 3)
#define VFS_O_APPEND     (1u << 4)
#define VFS_O_EXCL       (1u << 5)
#define VFS_O_NOFOLLOW   (1u << 6)
#define VFS_O_DIRECTORY  (1u << 7)


typedef struct vfs_node   vfs_node;
typedef struct vfs_file   vfs_file;
typedef struct vfs_dirent vfs_dirent;
typedef struct vfs_stat   vfs_stat;
typedef struct vfs_mount  vfs_mount;

typedef vfs_node*   vfs_node_ref;
typedef vfs_file*   vfs_file_ref;
typedef vfs_dirent* vfs_dirent_ptr;

typedef struct vfs_node_ops vfs_node_ops;
typedef struct vfs_file_ops vfs_file_ops;


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

typedef enum {
  VFS_SEEK_SET = 0,
  VFS_SEEK_CUR,
  VFS_SEEK_END,
} vfs_seek_whence;

typedef enum {
  VFS_NODETYPE_UNKNOWN = 0,
  VFS_NODETYPE_FILE,
  VFS_NODETYPE_DIRECTORY,
  VFS_NODETYPE_SYMLINK,
  VFS_NODETYPE_DEVICE,
} vfs_node_type;

struct vfs_node {
  const vfs_node_ops *ops;

  vfs_node_type  type;
  vfs_mount     *mounted;

  void *fs_data;

  atomic_uint refcount;
  void (*release)(vfs_node *self);
};

struct vfs_file {
  const vfs_file_ops *ops;

  vfs_node *node;
  u32       flags;
  u64       offset;

  void *fs_data;
};

struct vfs_dirent {
  u64           node_id;
  vfs_node_type type;
  usize         namelen;
  char          name[VFS_NAME_MAX + 1];
};

struct vfs_stat {
  vfs_node_type type;
  vfs_perms     perms;
  u64           size;
  u64           ctime;
  u64           mtime;
  u64           atime;
};

struct vfs_mount {
  str       path;
  vfs_node *root;
};

struct vfs_node_ops {
  RESULT(vfs_node_ref, vfs_err) (*lookup)  (vfs_node *self, str name);
  RESULT(vfs_node_ref, vfs_err) (*create)  (vfs_node *self, str name, vfs_node_type type, vfs_perms perms);
  RESULT(UNIT,         vfs_err) (*link)    (vfs_node *self, str name, vfs_node_ref target);
  RESULT(UNIT,         vfs_err) (*unlink)  (vfs_node *self, str name);
  RESULT(vfs_node_ref, vfs_err) (*symlink) (vfs_node *self, str name, str target);
  RESULT(str,          vfs_err) (*readlink)(vfs_node *self, allocator a);
  RESULT(vfs_file_ref, vfs_err) (*open)    (vfs_node *self, u32 flags);
  RESULT(vfs_stat,     vfs_err) (*stat)    (vfs_node *self);
  RESULT(UNIT,         vfs_err) (*chmod)   (vfs_node *self, vfs_mode mode);
  RESULT(UNIT,         vfs_err) (*chown)   (vfs_node *self, u32 uid, u32 gid);
};

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


void vfs_init(void);

vfs_node_ref vfs_node_incref(vfs_node_ref node);
void         vfs_node_decref(vfs_node_ref node);

RESULT(UNIT, vfs_err) vfs_op_mount  (str path, vfs_node *root);
RESULT(UNIT, vfs_err) vfs_op_unmount(str path);

RESULT(vfs_node_ref, vfs_err) vfs_op_lookup  (str path);
RESULT(vfs_node_ref, vfs_err) vfs_op_create  (str path, vfs_node_type type, vfs_perms perms);
RESULT(UNIT,         vfs_err) vfs_op_link    (str path, vfs_node_ref target);
RESULT(UNIT,         vfs_err) vfs_op_unlink  (str path);
RESULT(UNIT,         vfs_err) vfs_op_rename  (str oldpath, str newpath);
RESULT(vfs_node_ref, vfs_err) vfs_op_symlink (str path, str target);
RESULT(str,          vfs_err) vfs_op_readlink(str path, allocator a);
RESULT(vfs_file_ref, vfs_err) vfs_op_open    (str path, u32 flags);
RESULT(vfs_stat,     vfs_err) vfs_op_stat    (str path);
RESULT(UNIT,         vfs_err) vfs_op_chmod   (str path, vfs_mode mode);
RESULT(UNIT,         vfs_err) vfs_op_chown   (str path, u32 uid, u32 gid);

RESULT(UNIT,           vfs_err) vfs_op_close   (vfs_file *file);
RESULT(usize,          vfs_err) vfs_op_read    (vfs_file *file,       span buf);
RESULT(usize,          vfs_err) vfs_op_write   (vfs_file *file, const_span buf);
RESULT(u64,            vfs_err) vfs_op_seek    (vfs_file *file, i64 offset, vfs_seek_whence whence);
RESULT(vfs_dirent_ptr, vfs_err) vfs_op_readdir (vfs_file *file, allocator a);
RESULT(UNIT,           vfs_err) vfs_op_truncate(vfs_file *file, u64 size);
RESULT(UNIT,           vfs_err) vfs_op_flush   (vfs_file *file);
RESULT(u64,            vfs_err) vfs_op_ioctl   (vfs_file *file, u64 op, ...);

str vfs_strerror(vfs_err err);
