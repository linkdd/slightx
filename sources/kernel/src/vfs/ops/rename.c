#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(UNIT, vfs_err) vfs_rename(str oldpath, str newpath) {
  char old_pathbuf[VFS_PATH_MAX] = {};
  str  old_normpath              = vfs_normalize_path(oldpath, old_pathbuf);
  if (old_normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  char new_pathbuf[VFS_PATH_MAX] = {};
  str  new_normpath              = vfs_normalize_path(newpath, new_pathbuf);
  if (new_normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto old_nodepath = vfs_lookup_parent(old_normpath);
  if (!old_nodepath.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(old_nodepath.err);
  }

  auto new_nodepath = vfs_lookup_parent(new_normpath);
  if (!new_nodepath.is_ok) {
    vfs_node_decref(old_nodepath.ok.node);
    return (RESULT(UNIT, vfs_err)) ERR(new_nodepath.err);
  }

  vfs_node_ref old_node = old_nodepath.ok.node;
  str          old_name = old_nodepath.ok.path;

  vfs_node_ref new_node = new_nodepath.ok.node;
  str          new_name = new_nodepath.ok.path;

  if (old_node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (new_node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (old_node->ops == NULL || old_node->ops->lookup == NULL || old_node->ops->unlink == NULL) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  if (new_node->ops == NULL || new_node->ops->link == NULL) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  // resolve the source node
  auto source = old_node->ops->lookup(old_node, old_name);
  if (!source.is_ok) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(source.err);
  }

  // link into new directory
  auto link = new_node->ops->link(new_node, new_name, source.ok);
  vfs_node_decref(source.ok);
  if (!link.is_ok) {
    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(link.err);
  }

  // unlink from old directory
  auto unlink = old_node->ops->unlink(old_node, old_name);
  if (!unlink.is_ok) {
    new_node->ops->unlink(new_node, new_name);

    vfs_node_decref(old_node);
    vfs_node_decref(new_node);
    return (RESULT(UNIT, vfs_err)) ERR(unlink.err);
  }

  vfs_node_decref(old_node);
  vfs_node_decref(new_node);
  return (RESULT(UNIT, vfs_err)) OK({});
}
