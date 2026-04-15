#include <slightx/assert.h>

#include <vfs/path.h>


str vfs_path_as_str(vfs_path path) {
  return (str){
    .data     = path.data,
    .length   = path.length,
    .capacity = VFS_PATH_MAX,
    .owned    = false,
  };
}


vfs_path vfs_path_normalize(vfs_path path) {
  assert(path.length > 0);
  assert(path.data[0] == '/');

  vfs_path result = {0};

  usize w = 0;
  usize r = 0;

  while (r < path.length) {
    // skip consecutive slashes
    if (path.data[r] == '/') {
      if (w == 0 || result.data[w - 1] != '/') {
        if (w >= VFS_PATH_MAX - 1) return vfs_path_null();
        result.data[w++] = '/';
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

    if (len == 2 && str_equal(str_slice(vfs_path_as_str(path), start, len), str_literal(".."))) {
      if (w > 1) {
        w--;
        while (w > 1 && result.data[w - 1] != '/') {
          w--;
        }
      }
      continue;
    }

    // regular component --> copy
    if (result.data[w - 1] != '/') {
      if (w >= VFS_PATH_MAX - 1) return vfs_path_null();
      result.data[w++] = '/';
    }

    if (w + len >= VFS_PATH_MAX) return vfs_path_null();

    for (usize i = 0; i < len; i++) {
      result.data[w++] = path.data[start + i];
    }
  }

  // strip trailing slash (except for root)
  if (w > 1 && result.data[w - 1] == '/') {
    w--;
  }

  // empty path --> root
  if (w == 0) {
    result.data[w++] = '/';
  }

  result.length = w;
  return result;
}
