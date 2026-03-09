#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>
#include <klibc/mem/str.h>

#include <kernel/vfs/error.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/file.h>
#include <kernel/vfs/stat.h>


#define VFS_O_READ       (1u << 0)
#define VFS_O_WRITE      (1u << 1)
#define VFS_O_CREATE     (1u << 2)
#define VFS_O_TRUNC      (1u << 3)
#define VFS_O_APPEND     (1u << 4)
#define VFS_O_EXCL       (1u << 5)
#define VFS_O_NOFOLLOW   (1u << 6)
#define VFS_O_DIRECTORY  (1u << 7)


struct vfs_node_ops {
  RESULT(vfs_node_ref,  vfs_err) (*lookup)  (vfs_node *self, str name);
  RESULT(vfs_node_ref,  vfs_err) (*create)  (vfs_node *self, str name, vfs_node_type type, vfs_perms perms);
  RESULT(UNIT,          vfs_err) (*link)    (vfs_node *self, str name, vfs_node_ref target);
  RESULT(UNIT,          vfs_err) (*unlink)  (vfs_node *self, str name);
  RESULT(vfs_node_ref,  vfs_err) (*symlink) (vfs_node *self, str name, str target);
  RESULT(str,           vfs_err) (*readlink)(vfs_node *self, allocator a);
  RESULT(vfs_file_ref,  vfs_err) (*open)    (vfs_node *self, u32 flags);
  RESULT(vfs_stat_desc, vfs_err) (*stat)    (vfs_node *self);
  RESULT(UNIT,          vfs_err) (*chmod)   (vfs_node *self, vfs_mode mode);
  RESULT(UNIT,          vfs_err) (*chown)   (vfs_node *self, u32 uid, u32 gid);
};


RESULT(vfs_node_ref,  vfs_err) vfs_lookup  (str path, bool nofollow);
RESULT(vfs_node_ref,  vfs_err) vfs_create  (str path, vfs_node_type type, vfs_perms perms);
RESULT(UNIT,          vfs_err) vfs_link    (str path, vfs_node_ref target);
RESULT(UNIT,          vfs_err) vfs_unlink  (str path);
RESULT(UNIT,          vfs_err) vfs_rename  (str oldpath, str newpath);
RESULT(vfs_node_ref,  vfs_err) vfs_symlink (str path, str target);
RESULT(str,           vfs_err) vfs_readlink(str path, allocator a);
RESULT(vfs_file_ref,  vfs_err) vfs_open    (str path, u32 flags);
RESULT(vfs_stat_desc, vfs_err) vfs_stat    (str path);
RESULT(UNIT,          vfs_err) vfs_chmod   (str path, vfs_mode mode);
RESULT(UNIT,          vfs_err) vfs_chown   (str path, u32 uid, u32 gid);
