#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "path.h"


str vfs_normalize_path(str path, char buf[VFS_PATH_MAX]) {
  assert(path.length > 0);
  assert(path.data[0] == '/');

  usize w = 0;
  usize r = 0;

  while (r < path.length) {
    // skip consecutive slashes
    if (path.data[r] == '/') {
      if (w == 0 || buf[w - 1] != '/') {
        if (w >= VFS_PATH_MAX - 1) return str_null();
        buf[w++] = '/';
      }

      r++;
      continue;
    }

    // find the end of the current path component
    usize start = r;
    while (r < path.length && path.data[r] != '/') {
      r++;
    }
    usize len = r - start;

    // "." --> skip
    if (len == 1 && path.data[start] == '.') {
      continue;
    }

    // ".." --> pop parent
    if (len == 2 && str_equal(str_slice(path, start, len), str_literal(".."))) {
      if (w > 1) {
        w--;
        while (w > 1 && buf[w - 1] != '/') {
          w--;
        }
      }
      continue;
    }

    // regular component --> copy
    if (buf[w - 1] != '/') {
      if (w >= VFS_PATH_MAX - 1) return str_null();
      buf[w++] = '/';
    }

    if (w + len >= VFS_PATH_MAX) return str_null();

    for (usize i = 0; i < len; i++) {
      buf[w++] = path.data[start + i];
    }
  }

  // strip trailing slash (except for root)
  if (w > 1 && buf[w - 1] == '/') {
    w--;
  }

  // empty path --> root
  if (w == 0) {
    buf[w++] = '/';
  }

  return (str){
    .data     = buf,
    .length   = w,
    .capacity = VFS_PATH_MAX,
    .owned    = false,
  };
}


RESULT(vfs_node_path, vfs_err) vfs_lookup_parent(str normpath, bool nofollow) {
  assert(normpath.length > 0);
  assert(normpath.data[0] == '/');

  if (str_equal(normpath, str_literal("/"))) {
    return (RESULT(vfs_node_path, vfs_err)) ERR(VFS_EINVAL);
  }

  auto last_slash = str_rfind(normpath, '/');
  if (!last_slash.is_some) {
    return (RESULT(vfs_node_path, vfs_err)) ERR(VFS_EINVAL);
  }

  usize dirlen = (last_slash.some > 0 ? last_slash.some : 1);

  str dirname  = str_slice(normpath, 0, dirlen);
  str basename = str_slice(normpath, last_slash.some + 1, normpath.length - last_slash.some - 1);

  auto node = vfs_lookup(dirname, nofollow);
  if (!node.is_ok) {
    return (RESULT(vfs_node_path, vfs_err)) ERR(node.err);
  }

  vfs_node_path nodepath = {
    .node = node.ok,
    .path = basename,
  };

  return (RESULT(vfs_node_path, vfs_err)) OK(nodepath);
}
