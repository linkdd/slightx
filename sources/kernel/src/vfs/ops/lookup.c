#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>
#include <kernel/mem/heap.h>

#include "../mtable.h"
#include "../path.h"


RESULT(vfs_node_ref, vfs_err) vfs_lookup(str path, bool nofollow) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  usize symlink_depth = 0;

symlink_followed:
  vfs_mtable_lock();

  vfs_mtable_entry *best     = NULL;
  usize             best_len = 0;

  for (
    vfs_mtable_entry *entry = vfs_mtable_get_head();
    entry != NULL;
    entry = entry->next
  ) {
    if (str_startswith(normpath, entry->mount.path)) {
      usize len = entry->mount.path.length;

      bool on_boundary = (
        normpath.length == len                         ||
        normpath.data[len] == '/'                      ||
        str_equal(entry->mount.path, str_literal("/"))
      );

      if (on_boundary && len > best_len) {
        best     = entry;
        best_len = len;
      }
    }
  }

  if (best == NULL) {
    vfs_mtable_unlock();

    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTFOUND);
  }

  vfs_node_ref node = vfs_node_incref(best->mount.root);
  vfs_mtable_unlock();

  // exact match --> return root of the mounted fs
  if (best_len >= normpath.length) {
    return (RESULT(vfs_node_ref, vfs_err)) OK(node);
  }

  // walk remaining components
  usize pos = best_len;

  while (pos < normpath.length) {
    if (normpath.data[pos] == '/') {
      pos++;
      continue;
    }

    usize start = pos;
    while (pos < normpath.length && normpath.data[pos] != '/') {
      pos++;
    }
    usize len = pos - start;

    str component = str_slice(normpath, start, len);

    if (node->type != VFS_NODETYPE_DIRECTORY) {
      vfs_node_decref(node);
      return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTDIR);
    }

    if (node->ops == NULL || node->ops->lookup == NULL) {
      vfs_node_decref(node);
      return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOSYS);
    }

    auto res = node->ops->lookup(node, component);
    vfs_node_decref(node);
    if (!res.is_ok) return res;
    node = res.ok;

    if (node->type == VFS_NODETYPE_SYMLINK && !nofollow) {
      if (++symlink_depth > VFS_SYMLINK_DEPTH_MAX) {
        vfs_node_decref(node);
        return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ELOOP);
      }

      if (node->ops == NULL || node->ops->readlink == NULL) {
        vfs_node_decref(node);
        return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOSYS);
      }

      allocator a = heap_allocator();
      auto link = node->ops->readlink(node, a);
      vfs_node_decref(node);

      if (!link.is_ok) {
        return (RESULT(vfs_node_ref, vfs_err)) ERR(link.err);
      }

      str target    = link.ok;
      str remaining = (
        pos < normpath.length ? str_slice(normpath, pos, normpath.length - pos) :
        /* else */              str_literal("")
      );

      str newpath = {};

      if (str_startswith(target, str_literal("/"))) {
        newpath = str_format(a, "%s%s", target, remaining);
      }
      else {
        usize parent_end = (start > 1 ? start - 1 : 1);
        str   parent     = str_slice(normpath, 0, parent_end);

        newpath = str_format(a, "%s/%s%s", parent, target, remaining);
      }

      normpath = vfs_normalize_path(newpath, pathbuf);
      str_free(a, &target);
      str_free(a, &newpath);

      if (normpath.data == NULL) {
        return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_EINVAL);
      }

      goto symlink_followed;
    }

    while (node->mounted != NULL) {
      vfs_node_ref root = vfs_node_incref(node->mounted->root);
      vfs_node_decref(node);
      node = root;
    }
  }

  return (RESULT(vfs_node_ref, vfs_err)) OK(node);
}
