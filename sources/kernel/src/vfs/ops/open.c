#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(vfs_file_ref, vfs_err) vfs_op_open(str path, u32 flags) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  bool created = false;
  auto node    = vfs_lookup(normpath);

  if (!node.is_ok && node.err == VFS_ENOTFOUND && (flags & VFS_O_CREATE)) {
    auto nodepath = vfs_lookup_parent(normpath);
    if (!nodepath.is_ok) {
      return (RESULT(vfs_file_ref, vfs_err)) ERR(nodepath.err);
    }

    vfs_node_ref parent = nodepath.ok.node;
    str          name   = nodepath.ok.path;

    if (parent->type != VFS_NODETYPE_DIRECTORY) {
      vfs_node_decref(parent);
      return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_ENOTDIR);
    }

    if (parent->ops == NULL || parent->ops->create == NULL) {
      vfs_node_decref(parent);
      return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_ENOSYS);
    }

    vfs_perms default_perms = {
      .uid  = 0,
      .gid  = 0,
      .mode = VFS_MODE_OWNER_R | VFS_MODE_OWNER_W
             | VFS_MODE_GROUP_R
             | VFS_MODE_OTHER_R,
    };

    auto newnode = parent->ops->create(parent, name, VFS_NODETYPE_FILE, default_perms);
    vfs_node_decref(parent);

    if (!newnode.is_ok) {
      return (RESULT(vfs_file_ref, vfs_err)) ERR(newnode.err);
    }

    node    = (RESULT(vfs_node_ref, vfs_err)) OK(newnode.ok);
    created = true;
  }
  else if (!node.is_ok) {
    return (RESULT(vfs_file_ref, vfs_err)) ERR(node.err);
  }

  // VFS_O_EXCL: fail if file already existed
  if ((flags & VFS_O_CREATE) && (flags & VFS_O_EXCL) && !created) {
    vfs_node_decref(node.ok);
    return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_EEXISTS);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->open == NULL) {
    vfs_node_decref(self);
    return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->open(self, flags);
  vfs_node_decref(self);
  return res;
}
