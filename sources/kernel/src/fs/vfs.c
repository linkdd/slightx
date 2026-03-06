#include <kernel/fs/vfs.h>
#include <kernel/mem/heap.h>

#include <klibc/sync/lock.h>
#include <klibc/assert.h>


// MARK: - mtable

typedef struct vfs_mtable_entry vfs_mtable_entry;
struct vfs_mtable_entry {
  vfs_mount mount;

  vfs_mtable_entry *prev;
  vfs_mtable_entry *next;
};

typedef struct vfs_mtable vfs_mtable;
struct vfs_mtable {
  spinlock lock;

  vfs_mtable_entry *head;
};

static vfs_mtable g_mtable = {};


void vfs_init(void) {
  spinlock_init(&g_mtable.lock);
  g_mtable.head = NULL;
}


// MARK: - refcount

vfs_node_ref vfs_node_incref(vfs_node_ref node) {
  assert(node != NULL);
  atomic_fetch_add_explicit(&node->refcount, 1, memory_order_relaxed);
  return node;
}


void vfs_node_decref(vfs_node_ref node) {
  assert(node != NULL);

  u32 prev = atomic_fetch_sub_explicit(&node->refcount, 1, memory_order_acq_rel);
  if (prev == 1) {
    atomic_thread_fence(memory_order_acquire);
    if (node->release != NULL) node->release(node);
  }
}


// MARK: - normalize
static str vfs_normalize_path(str path, char buf[VFS_PATH_MAX]) {
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


// MARK: - mount

RESULT(UNIT, vfs_err) vfs_op_mount(str path, vfs_node *root) {
  assert(path.length > 0);
  assert(path.data[0] == '/');
  assert(root != NULL);

  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  spinlock_acquire(&g_mtable.lock);

  if (root->type != VFS_NODETYPE_DIRECTORY) {
    spinlock_release(&g_mtable.lock);

    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (root->mounted != NULL) {
    spinlock_release(&g_mtable.lock);

    return (RESULT(UNIT, vfs_err)) ERR(VFS_EBUSY);
  }

  for (
    vfs_mtable_entry *entry = g_mtable.head;
    entry != NULL;
    entry = entry->next
  ) {
    if (str_equal(entry->mount.path, normpath)) {
      spinlock_release(&g_mtable.lock);

      return (RESULT(UNIT, vfs_err)) ERR(VFS_EEXISTS);
    }
  }

  allocator a = heap_allocator();

  vfs_mtable_entry *entry = allocate(a, sizeof(vfs_mtable_entry));
  entry->mount.path = str_clone(a, normpath);
  entry->mount.root = vfs_node_incref(root);

  root->mounted = &entry->mount;

  if (g_mtable.head == NULL) {
    entry->prev = NULL;
    entry->next = NULL;
    g_mtable.head = entry;
  }
  else {
    entry->prev       = NULL;
    entry->next       = g_mtable.head;
    entry->next->prev = entry;
    g_mtable.head     = entry;
  }

  spinlock_release(&g_mtable.lock);

  return (RESULT(UNIT, vfs_err)) OK({});
}


// MARK: - unmount

RESULT(UNIT, vfs_err) vfs_op_unmount(str path) {
  assert(path.length > 0);
  assert(path.data[0] == '/');

  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  spinlock_acquire(&g_mtable.lock);

  vfs_mtable_entry *entry = NULL;

  for (
    entry = g_mtable.head;
    entry != NULL;
    entry = entry->next
  ) {
    if (str_equal(entry->mount.path, normpath)) {
      break;
    }
  }

  if (entry == NULL) {
    spinlock_release(&g_mtable.lock);

    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTFOUND);
  }

  entry->mount.root->mounted = NULL;
  vfs_node_decref(entry->mount.root);

  if (entry->prev != NULL) entry->prev->next = entry->next;
  else                     g_mtable.head     = entry->next;
  if (entry->next != NULL) entry->next->prev = entry->prev;

  allocator a = heap_allocator();

  str_free  (a, &entry->mount.path);
  deallocate(a, entry, sizeof(vfs_mtable_entry));

  spinlock_release(&g_mtable.lock);

  return (RESULT(UNIT, vfs_err)) OK({});
}


// MARK: - lookup

RESULT(vfs_node_ref, vfs_err) vfs_op_lookup(str path) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  usize symlink_depth = 0;

symlink_followed:
  spinlock_acquire(&g_mtable.lock);

  vfs_mtable_entry *best     = NULL;
  usize             best_len = 0;

  for (
    vfs_mtable_entry *entry = g_mtable.head;
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
    spinlock_release(&g_mtable.lock);

    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTFOUND);
  }

  vfs_node_ref node = vfs_node_incref(best->mount.root);
  spinlock_release(&g_mtable.lock);

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

    if (node->type == VFS_NODETYPE_SYMLINK) {
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


typedef struct vfs_node_path vfs_node_path;
struct vfs_node_path {
  vfs_node_ref node;
  str          path;
};


static RESULT(vfs_node_path, vfs_err) vfs_lookup_parent(str normpath) {
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

  auto node = vfs_op_lookup(dirname);
  if (!node.is_ok) {
    return (RESULT(vfs_node_path, vfs_err)) ERR(node.err);
  }

  vfs_node_path nodepath = {
    .node = node.ok,
    .path = basename,
  };

  return (RESULT(vfs_node_path, vfs_err)) OK(nodepath);
}


// MARK: - create

RESULT(vfs_node_ref, vfs_err) vfs_op_create(str path, vfs_node_type type, vfs_perms perms) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  auto nodepath = vfs_lookup_parent(normpath);
  if (!nodepath.is_ok) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(nodepath.err);
  }

  vfs_node_ref node = nodepath.ok.node;
  str          name = nodepath.ok.path;

  if (node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(node);
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (node->ops == NULL || node->ops->create == NULL) {
    vfs_node_decref(node);
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = node->ops->create(node, name, type, perms);
  vfs_node_decref(node);
  return res;
}


// MARK: - link

RESULT(UNIT, vfs_err) vfs_op_link(str path, vfs_node_ref target) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto nodepath = vfs_lookup_parent(normpath);
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


// MARK: - unlink

RESULT(UNIT, vfs_err) vfs_op_unlink(str path) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto nodepath = vfs_lookup_parent(normpath);
  if (!nodepath.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(nodepath.err);
  }

  vfs_node_ref node = nodepath.ok.node;
  str          name = nodepath.ok.path;

  if (node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (node->ops == NULL || node->ops->unlink == NULL) {
    vfs_node_decref(node);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = node->ops->unlink(node, name);
  vfs_node_decref(node);
  return res;
}


// MARK: - rename

RESULT(UNIT, vfs_err) vfs_op_rename(str oldpath, str newpath) {
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


// MARK: - symlink

RESULT(vfs_node_ref, vfs_err) vfs_op_symlink(str path, str target) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  auto nodepath = vfs_lookup_parent(normpath);
  if (!nodepath.is_ok) {
    return (RESULT(vfs_node_ref, vfs_err)) ERR(nodepath.err);
  }

  vfs_node_ref node = nodepath.ok.node;
  str          name = nodepath.ok.path;

  if (node->type != VFS_NODETYPE_DIRECTORY) {
    vfs_node_decref(node);
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (node->ops == NULL || node->ops->symlink == NULL) {
    vfs_node_decref(node);
    return (RESULT(vfs_node_ref, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = node->ops->symlink(node, name, target);
  vfs_node_decref(node);
  return res;
}


// MARK: - readlink

RESULT(str, vfs_err) vfs_op_readlink(str path, allocator a) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(str, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_op_lookup(normpath);
  if (!node.is_ok) {
    return (RESULT(str, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->readlink == NULL) {
    vfs_node_decref(self);
    return (RESULT(str, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->readlink(self, a);
  vfs_node_decref(self);
  return res;
}


// MARK: - open

RESULT(vfs_file_ref, vfs_err) vfs_op_open(str path, u32 flags) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_file_ref, vfs_err)) ERR(VFS_EINVAL);
  }

  bool created = false;
  auto node    = vfs_op_lookup(normpath);

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


// MARK: - stat

RESULT(vfs_stat, vfs_err) vfs_op_stat(str path) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_stat, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_op_lookup(normpath);
  if (!node.is_ok) {
    return (RESULT(vfs_stat, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->stat == NULL) {
    vfs_node_decref(self);
    return (RESULT(vfs_stat, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->stat(self);
  vfs_node_decref(self);
  return res;
}


// MARK: - chmod

RESULT(UNIT, vfs_err) vfs_op_chmod(str path, vfs_mode mode) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_op_lookup(normpath);
  if (!node.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->chmod == NULL) {
    vfs_node_decref(self);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->chmod(self, mode);
  vfs_node_decref(self);
  return res;
}


// MARK: - chown

RESULT(UNIT, vfs_err) vfs_op_chown(str path, u32 uid, u32 gid) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_op_lookup(normpath);
  if (!node.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->chown == NULL) {
    vfs_node_decref(self);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->chown(self, uid, gid);
  vfs_node_decref(self);
  return res;
}


// MARK: - close

RESULT(UNIT, vfs_err) vfs_op_close(vfs_file *file) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->close == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->close(file);
}


// MARK: - read

RESULT(usize, vfs_err) vfs_op_read(vfs_file *file, span buf) {
  assert(file != NULL);
  assert(buf.data != NULL && buf.size > 0);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(usize, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->read == NULL) {
    return (RESULT(usize, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->read(file, buf);
}


// MARK: - write

RESULT(usize, vfs_err) vfs_op_write(vfs_file *file, const_span buf) {
  assert(file != NULL);
  assert(buf.data != NULL && buf.size > 0);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(usize, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->write == NULL) {
    return (RESULT(usize, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->write(file, buf);
}


// MARK: - seek

RESULT(u64, vfs_err) vfs_op_seek(vfs_file *file, i64 offset, vfs_seek_whence whence) {
  assert(file != NULL);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(u64, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->seek == NULL) {
    return (RESULT(u64, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->seek(file, offset, whence);
}


// MARK: - readdir

RESULT(vfs_dirent_ptr, vfs_err) vfs_op_readdir(vfs_file *file, allocator a) {
  assert(file != NULL);

  if (file->node->type != VFS_NODETYPE_DIRECTORY) {
    return (RESULT(vfs_dirent_ptr, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (file->ops == NULL || file->ops->readdir == NULL) {
    return (RESULT(vfs_dirent_ptr, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->readdir(file, a);
}


// MARK: - truncate

RESULT(UNIT, vfs_err) vfs_op_truncate(vfs_file *file, u64 size) {
  assert(file != NULL);

  if (file->node->type == VFS_NODETYPE_DIRECTORY) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EISDIR);
  }

  if (file->ops == NULL || file->ops->truncate == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->truncate(file, size);
}


// MARK: - flush

RESULT(UNIT, vfs_err) vfs_op_flush(vfs_file *file) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->flush == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  return file->ops->flush(file);
}


// MARK: - ioctl

RESULT(u64, vfs_err) vfs_op_ioctl(vfs_file *file, u64 op, ...) {
  assert(file != NULL);

  if (file->ops == NULL || file->ops->ioctl == NULL) {
    return (RESULT(u64, vfs_err)) ERR(VFS_ENOSYS);
  }

  va_list args;
  va_start(args, op);
  auto res = file->ops->ioctl(file, op, args);
  va_end(args);
  return res;
}


// MARK: - strerror

str vfs_strerror(vfs_err status) {
  switch (status) {
    case VFS_UNKNOWN:   return str_literal("Unknown error");
    case VFS_ENOTFOUND: return str_literal("Not found");
    case VFS_EEXISTS:   return str_literal("Already exists");
    case VFS_EACCESS:   return str_literal("Permission denied");
    case VFS_EIO:       return str_literal("I/O error");
    case VFS_ENOMEM:    return str_literal("Out of memory");
    case VFS_ENOSPC:    return str_literal("No space left on device");
    case VFS_EINVAL:    return str_literal("Invalid argument");
    case VFS_EISDIR:    return str_literal("Is a directory");
    case VFS_ENOTDIR:   return str_literal("Not a directory");
    case VFS_ENOTEMPTY: return str_literal("Directory not empty");
    case VFS_ELOOP:     return str_literal("Too many symbolic links encountered");
    case VFS_EBUSY:     return str_literal("Device or resource busy");
    case VFS_ENOSYS:    return str_literal("Function not implemented");
  }

  unreachable();
}
