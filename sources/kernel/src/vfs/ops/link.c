#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(UNIT, vfs_err) vfs_link(str path, vfs_node_ref target) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto nodepath = vfs_lookup_parent(normpath, false);
  if (!nodepath.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(nodepath.err);
  }

  vfs_node_ref node = nodepath.ok.node;
  str          name = nodepath.ok.path;

  if (node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (node->ops == NULL || node->ops->link == NULL) {
    vfs_node_decref(node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = node->ops->link(node, name, target);
  vfs_node_decref(node);
  return res;
}
